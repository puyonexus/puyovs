#include "Puyo.h"
#include "Field.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

using namespace std;

namespace ppvs {

Puyo::Puyo(int indexX, int indexY, int color, Field* f, float spriteX, float spriteY, GameData* data)
{
	m_field = f;
	m_data = data;
	FieldProp prop = m_field->getProperties();

	// Set position and color
	m_posX = indexX;
	m_posY = indexY;
	m_color = color;
	m_spriteX = spriteX;
	m_spriteY = spriteY;
	m_spriteYsc = 0;
	m_scaleXd = 1;
	m_scaleYd = 1;
	m_scaleX = 1;
	m_scaleY = 1;
	m_scaleYcor = 1;
	m_accelerationY = 0;
	m_targetY = 0;

	// Set sprite
	m_sprite.setImage(m_data->imgPuyo);

	// Correction factor in Y
	m_scaleYcor = static_cast<float>(prop.gridHeight) / (static_cast<float>(kPuyoY + prop.gridHeight) / 2.01f);

	// Other stuff
	m_droppable = true;
	m_type = NOPUYO;
	m_targetY = 0;
	m_fallDelay = 0;
	m_fallFlag = 0;
	m_bounceFlag0 = false;
	m_bounceFlag = 0;
	m_searchBounce = 0;
	m_bounceY = 0;
	m_bounceTimer = 2;
	m_bounceMultiplier = 0;
	m_bottomY = 0;
	m_glow = false;
	m_lastNuisance = false;
	m_hard = false;
	m_mark = false;
	m_destroy = false;
	m_destroyTimer = 0;
	m_linkDown = false;
	m_linkUp = false;
	m_linkRight = false;
	m_linkLeft = false;
	m_temporary = false;
}

Puyo::Puyo(const Puyo& self)
{
	// Initialize copy of base class
	// Copy only important things, no sprite stuff
	m_posX = self.posX();
	m_posY = self.posY();
	m_type = self.getType();
	m_color = self.getColor();
	m_droppable = self.m_droppable;
	m_mark = false;
	m_destroy = false;
	m_temporary = self.m_temporary;
}

Puyo::~Puyo() = default;

Puyo* Puyo::clone()
{
	return new Puyo(*this);
}

// Add to acceleration and set position
void Puyo::addAccelerationY(float val)
{
	m_accelerationY += val;
	m_spriteY += m_accelerationY;
}

// Only colorpuyo return something valid
int Puyo::getColor() const
{
	return -1;
}

void Puyo::updateSprite()
{
	// Bug fixes
	if (m_fallFlag != 0) {
		// There is a memory effect, nasty effects
		m_scaleX = 1.0f;
		m_scaleY = 1.0f;
		m_bounceY = 0;
	}

	// Weird scaling events
	if (m_scaleX > 3.0f || m_scaleY > 3.0f || m_scaleX < 0.1f) {
		m_scaleX = 1.0f;
		m_scaleY = 1.0f;
	}

	if (m_bounceY > kPuyoY || m_bounceY < -kPuyoY) {
		m_bounceY = 0;
	}

	// Position
    const FieldProp prop = m_field->getProperties();

	// Note: puyo width is now 30 (to remove artifacts)
	m_sprite.setPosition(m_spriteX - static_cast<float>(m_posX) * 2 + static_cast<float>(prop.gridX) / 2, m_spriteY + m_bounceY);

	// Scale
	m_sprite.setScale(m_scaleX * m_scaleXd, m_scaleY * m_scaleYd * m_scaleYcor);
}

// Draw puyo on screen
void Puyo::draw(FeRenderTarget* target)
{
	updateSprite();
	if (!m_glow)
		m_sprite.draw(target);
	else if (m_data->glowShader)
		m_sprite.draw(target, m_data->glowShader);
}

// Popping animation
void Puyo::pop()
{
	if (m_destroy == true) {
		m_destroyTimer++;
		if (m_destroyTimer < 10) {
			m_scaleXd = 1.3f;
			m_scaleYd = 1.3f;
			m_spriteYsc += (m_scaleYd - 1.f) / 2.0f * static_cast<float>(m_field->getProperties().gridHeight);
		} else {
			m_scaleXd = 1.3f - 1.3f * static_cast<float>(m_destroyTimer - 10) / static_cast<float>(m_field->getPlayer()->m_chainPopSpeed);
			m_scaleYd = 1.3f - 1.3f * static_cast<float>(m_destroyTimer - 10) / static_cast<float>(m_field->getPlayer()->m_chainPopSpeed);
			m_spriteYsc += (m_scaleY - 1.f) / 2.0f * static_cast<float>(m_field->getProperties().gridHeight);
		}
	} else {
		m_scaleXd = 1.0f;
		m_scaleYd = 1.0f;
	}
}

bool Puyo::destroyPuyo()
{
	if (m_destroyTimer > m_field->getPlayer()->m_chainPopSpeed) {
		m_destroy = false;
		return true;
	}
	return false;
}

// Connect puyo to another in some direction (though it can be done in base class, only has meaning for colorpuyo for now)
void Puyo::setLink(Direction dir)
{
	switch (dir) {
	case BELOW:
		m_linkDown = true;
		break;

	case ABOVE:
		m_linkUp = true;
		break;

	case RIGHT:
		m_linkRight = true;
		break;

	case LEFT:
		m_linkLeft = true;
		break;
	}
}

// Unconnect puyo in some direction
void Puyo::unsetLink(Direction dir)
{
	switch (dir) {
	case BELOW:
		m_linkDown = false;
		break;

	case ABOVE:
		m_linkUp = false;
		break;

	case RIGHT:
		m_linkRight = false;
		break;

	case LEFT:
		m_linkLeft = false;
		break;
	}
}

// Don't remember what this is for actually, see MMF source code
void Puyo::landProper()
{
	m_fallFlag = 0;
	m_spriteY = m_targetY;
	m_accelerationY = 0;
	m_bounceFlag = 1;
	m_bounceTimer = 2;
	m_searchBounce = 1;
}

// Do the bounce animation
void Puyo::bounce()
{
	FieldProp prop = m_field->getProperties();

	if (m_hard == true) {
		// Hard puyos do not bounce
		if (m_bounceTimer > 0)
			m_bounceTimer += 20; // Even hard puyos need to end their bounce

		return;
	}

	if (m_bounceTimer > 2 && m_fallFlag == 0) {
		// Change scale
		m_scaleX = (exp(m_bounceTimer * -1.0f / 20.f) * m_bounceMultiplier * cos((m_bounceTimer * 6 - 45) * kPiF / 180.f) + 1.05f) * (1 - 0.5f * exp(m_bounceTimer * -1 / 5.f));
		m_scaleY = -1 * exp(m_bounceTimer * -1.0f / 20.f) * m_bounceMultiplier * cos((m_bounceTimer * 6 - 45) * kPiF / 180.f) + 1 + exp(m_bounceTimer / 2.f * -1);
	} else if (m_fallFlag != 0) {
		m_scaleX = 1.0f;
		m_scaleY = 1.0f;
	}
	if (m_bounceTimer > 0 && m_posY > 0 && m_fallFlag == 0) {
		// Change y
		m_bounceY = exp(m_bounceTimer * -1.f / 20.f) * static_cast<float>(prop.gridHeight) * 1.14f * min(1.2f - 0.2f * static_cast<float>(max(5 - m_posY + m_bottomY, 0) + 1), m_bounceMultiplier) * cos((m_bounceTimer * 6 - 45) * kPiF / 180.f) + 1 + exp(m_bounceTimer * -1.0f / 2.f);
	} else if (m_fallFlag != 0) {
		m_bounceY = 0;
	}

	// Set timer
	if (m_bounceTimer > 0) {
		m_bounceTimer += static_cast<float>(m_field->getPlayer()->m_puyoBounceSpeed);
	}
}

// Calculate the position the puyo has to fall to
void Puyo::setFallTarget(int target)
{
	const FieldProp prop = m_field->getProperties();
	m_targetY = static_cast<float>(prop.gridHeight * ((prop.gridY - 3) - target));
}

// What should happen if a neighbouring puyo is popped
void Puyo::neighborPop(Field*, bool)
{
}

//=======================================
// colorPuyo
//=======================================

ColorPuyo::ColorPuyo(int indexX, int indexY, int color, Field* f, float spriteX, float spriteY, GameData* data)
	: Puyo(indexX, indexY, color, f, spriteX, spriteY, data)
{
	m_type = COLORPUYO;
}

ColorPuyo::ColorPuyo(const ColorPuyo& self)
	: Puyo(self)
{
}

ColorPuyo* ColorPuyo::clone()
{
	return new ColorPuyo(*this);
}

int ColorPuyo::getColor() const
{
	return m_color;
}

void ColorPuyo::updateSprite()
{
	// Set rectangle
	int xRect = kPuyoX * (m_linkDown + (m_linkUp << 1) + (m_linkRight << 2) + (m_linkLeft << 3));
	int yRect = kPuyoY * m_color;

	// Destroy animation
	if (m_destroy == true) {
		xRect = 2 * kPuyoX * (m_color / 2);
		yRect = 12 * kPuyoY + kPuyoY * (m_color % 2);
	}

	m_sprite.setSubRect(xRect, yRect, kPuyoX, kPuyoY);
	m_sprite.setCenter(kPuyoX / 2, kPuyoY);

	Puyo::updateSprite();
}

// Draw puyo on screen
void ColorPuyo::draw(FeRenderTarget* target)
{
	updateSprite();

	if (!m_glow) {
		m_sprite.draw(target);
	} else {
		if (m_data->glowShader) {
			m_data->glowShader->setParameter("color", 0.10 + 0.10 * sin(static_cast<double>(m_data->globalTimer) / 8.0));
			m_sprite.draw(target, m_data->glowShader);
		} else {
			// Without shaders
			m_sprite.setBlendMode(AlphaBlending);
			m_sprite.draw(target);
			m_sprite.setBlendMode(AdditiveBlending);
			m_sprite.setTransparency(0.20f + 0.20f * sin(static_cast<float>(m_data->globalTimer) / 8.f));
			m_sprite.draw(target);
			m_sprite.setTransparency(1);
			m_sprite.setBlendMode(AlphaBlending);
		}
	}
}

// Popping animation
void ColorPuyo::pop()
{
	if (m_destroy == true) {
		m_glow = false;
		m_destroyTimer++;
		if (m_destroyTimer < 10) {
			m_scaleXd = 1.3f;
			m_scaleYd = 1.3f;
			m_spriteYsc = m_spriteYsc + (m_scaleYd - 1) / 2.0f * static_cast<float>(m_field->getProperties().gridHeight);
		} else {
			m_scaleXd = 1.3f - 1.3f * static_cast<float>(m_destroyTimer - 10) / static_cast<float>(m_field->getPlayer()->m_chainPopSpeed);
			m_scaleYd = 1.3f - 1.3f * static_cast<float>(m_destroyTimer - 10) / static_cast<float>(m_field->getPlayer()->m_chainPopSpeed);
			m_spriteYsc = m_spriteYsc + (m_scaleY - 1) / 2.0f * static_cast<float>(m_field->getProperties().gridHeight);
		}
	} else {
		m_scaleXd = 1.0f;
		m_scaleYd = 1.0f;
	}
}

// Don't remember what this is for actually, see MMF source code
void ColorPuyo::landProper()
{
	m_fallFlag = 0;
	m_spriteY = m_targetY;
	m_accelerationY = 0;
	m_bounceFlag = 1;
	m_bounceTimer = 2;
	m_searchBounce = 1;
}

// Color puyo also chucks out particles before being destroyed
bool ColorPuyo::destroyPuyo()
{
	if (m_destroyTimer > m_field->getPlayer()->m_chainPopSpeed) {
		m_field->createParticle(m_spriteX, m_spriteY, m_color);
		m_field->createParticle(m_spriteX, m_spriteY, m_color);
		m_field->createParticle(m_spriteX, m_spriteY, m_color);
		m_destroy = false;
		return true;
	}
	return false;
}

// What should happen if a neighbouring puyo is popped
void ColorPuyo::neighborPop(Field*, bool /*virt*/)
{
}

//=======================================
// nuiancePuyo
//=======================================

NuisancePuyo::NuisancePuyo(int indexX, int indexY, int color, Field* f, float spriteX, float spriteY, GameData* data)
	: Puyo(indexX, indexY, color, f, spriteX, spriteY, data)
{
	m_type = NUISANCEPUYO;
}

NuisancePuyo::NuisancePuyo(const NuisancePuyo& self)
	: Puyo(self)
{
}

NuisancePuyo* NuisancePuyo::clone()
{
	return new NuisancePuyo(*this);
}

void NuisancePuyo::landProper()
{
	m_fallFlag = 0;
	m_spriteY = m_targetY;
	m_accelerationY = 0;
	m_bounceFlag = 0;
	m_bounceTimer = 0;
	m_searchBounce = 0;
}

void NuisancePuyo::updateSprite()
{
	m_sprite.setSubRect(kPuyoX * 6, kPuyoY * 12, kPuyoX, kPuyoY);
	m_sprite.setCenter(kPuyoX / 2, kPuyoY);

	Puyo::updateSprite();
}

void NuisancePuyo::draw(FeRenderTarget* target)
{
	updateSprite();
	m_sprite.draw(target);
}

int NuisancePuyo::getColor() const
{
	return -1;
}

// Popping animation
void NuisancePuyo::pop()
{
	if (m_destroy == true) {
		m_destroyTimer++;
		if (m_destroyTimer < 10) {
			m_scaleXd = 1.3f;
			m_scaleYd = 1.3f;
			m_spriteYsc += (m_scaleYd - 1.f) / 2.0f * static_cast<float>(m_field->getProperties().gridHeight);
		} else {
			m_scaleXd = 1.3f - 1.3f * static_cast<float>(m_destroyTimer - 10) / static_cast<float>(m_field->getPlayer()->m_chainPopSpeed);
			m_scaleYd = 1.3f - 1.3f * static_cast<float>(m_destroyTimer - 10) / static_cast<float>(m_field->getPlayer()->m_chainPopSpeed);
			m_spriteYsc += (m_scaleY - 1.f) / 2.0f * static_cast<float>(m_field->getProperties().gridHeight);
		}
	} else {
		m_scaleXd = 1;
		m_scaleYd = 1;
	}
}

bool NuisancePuyo::destroyPuyo()
{
	if (m_destroyTimer > m_field->getPlayer()->m_chainPopSpeed) {
		m_destroy = false;
		return true;
	}
	return false;
}

// What should happen if a neighbouring puyo is popped
void NuisancePuyo::neighborPop(Field* f, bool v)
{
	if (v == false) {
		// Delete self by adding to deletedpuyo list
		f->removePuyo(m_posX, m_posY);
		f->set(m_posX, m_posY, nullptr);
	} else {
		// As a copy: unlink and delete self
		f->set(m_posX, m_posY, nullptr);
		delete this;
	}
}

//=======================================
// hardPuyo
//=======================================

HardPuyo::HardPuyo(int indexX, int indexY, int color, Field* f, float spriteX, float spriteY, GameData* data)
	: Puyo(indexX, indexY, color, f, spriteX, spriteY, data)
{
	m_type = HARDPUYO;
	m_hard = true;
	m_life = 2;
	m_vlife = 2;
}

HardPuyo::HardPuyo(const HardPuyo& self)
	: Puyo(self)
{
	m_life = self.m_life;
	m_vlife = self.m_vlife;
}

HardPuyo* HardPuyo::clone()
{
	return new HardPuyo(*this);
}

void HardPuyo::landProper()
{
	m_fallFlag = 0;
	m_spriteY = m_targetY;
	m_accelerationY = 0;
	m_bounceFlag = 0;
	m_bounceTimer = 0;
	m_searchBounce = 0;
}

void HardPuyo::updateSprite()
{
	if (m_life == 2) {
		m_sprite.setSubRect(kPuyoX * 14, kPuyoY * 11, kPuyoX, kPuyoY);
		m_sprite.setCenter(kPuyoX / 2, kPuyoY);
	} else {
		m_sprite.setSubRect(kPuyoX * 6, kPuyoY * 12, kPuyoX, kPuyoY);
		m_sprite.setCenter(kPuyoX / 2, kPuyoY);
	}

	Puyo::updateSprite();
}

void HardPuyo::draw(FeRenderTarget* target)
{
	updateSprite();
	m_sprite.draw(target);
}

int HardPuyo::getColor() const
{
	return -1;
}

// Popping animation
void HardPuyo::pop()
{
	if (m_destroy == true) {
		m_destroyTimer++;
		if (m_destroyTimer < 10) {
			m_scaleXd = 1.3f;
			m_scaleYd = 1.3f;
			m_spriteYsc += (m_scaleYd - 1.f) / 2.0f * static_cast<float>(m_field->getProperties().gridHeight);
		} else {
			m_scaleXd = 1.3f - 1.3f * static_cast<float>(m_destroyTimer - 10) / static_cast<float>(m_field->getPlayer()->m_chainPopSpeed);
			m_scaleYd = 1.3f - 1.3f * static_cast<float>(m_destroyTimer - 10) / static_cast<float>(m_field->getPlayer()->m_chainPopSpeed);
			m_spriteYsc += (m_scaleY - 1.f) / 2.0f * static_cast<float>(m_field->getProperties().gridHeight);
		}
	} else {
		m_scaleXd = 1.f;
		m_scaleYd = 1.f;
	}
}

bool HardPuyo::destroyPuyo()
{
	if (m_destroyTimer > m_field->getPlayer()->m_chainPopSpeed) {
		m_destroy = false;
		return true;
	}

	return false;
}

// What should happen if a neighbouring puyo is popped
void HardPuyo::neighborPop(Field* f, bool v)
{
	if (v == false) {
		// Delete self by adding to deletedpuyo list
		m_life -= 1;
		if (m_life <= 0) {
			f->removePuyo(m_posX, m_posY);
			f->set(m_posX, m_posY, nullptr);
		}
	} else {
		// As a copy: unlink and delete self
		m_vlife -= 1;
		if (m_vlife <= 0) {
			f->set(m_posX, m_posY, nullptr);
			delete this;
		}
	}
}

}
