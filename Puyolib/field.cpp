#include <algorithm>
#include "field.h"
#include "game.h"
#include "player.h"
#include "../PVS_ENet/PVS_Client.h"

using namespace std;

namespace ppvs
{

field::field()
{
	m_visible = false;
	m_sweepFall = 0;
	m_fieldInit = false;
	m_transformScale = 1;
	m_player = nullptr;
	m_centerX = 0; m_centerY = 0;
	m_posXreal = 0; m_posYreal = 0;
}

field::~field()
{
	// destroy particles, throw puyo, destroying puyos
	while (!m_particles.empty())
	{
		delete m_particles.back();
		m_particles.pop_back();
	}
	while (!m_particlesThrow.empty())
	{
		delete m_particlesThrow.back();
		m_particlesThrow.pop_back();
	}
	while (!m_deletedPuyo.empty())
	{
		delete m_deletedPuyo.back();
		m_deletedPuyo.pop_back();
	}
	if (m_fieldInit)
	{
		freePuyo(true);
		freePuyo(false);
		freePuyoArray();
	}
}

field& field::operator=(const field& rhs)
{
	// handle self assignment
	if (this == &rhs)
		return *this;

	return *this;
}

void field::init(fieldProp properties, player* pl)
{
	m_player = pl;
	data = m_player->data;

	m_visible = true;
	m_properties = properties;

	// calculate center (does not include x,y scale)
	m_centerX = static_cast<float>(properties.gridX * properties.gridWidth) / 2.0f;
	m_centerY = static_cast<float>((properties.gridY - 3) * properties.gridHeight);
	m_properties.centerX = m_centerX;
	m_properties.centerY = m_centerY;

	// Create the puyo array
	createPuyoArray();
	
	// recalculate position (global screen position)
	m_posXreal = properties.offsetX + m_centerX;
	m_posYreal = properties.offsetY + m_centerY;
}

// initializes both puyo array and puyoarray copy
void field::createPuyoArray()
{
	fieldPuyoArray = new puyo * *[m_properties.gridX];
	fieldPuyoArrayCopy = new puyo * *[m_properties.gridX];
	int i, j;
	for (i = 0; i < m_properties.gridX; i++)
	{
		fieldPuyoArray[i] = new puyo * [m_properties.gridY];
		fieldPuyoArrayCopy[i] = new puyo * [m_properties.gridY];
	}

	// initialize the puyo array with null
	for (i = 0; i < m_properties.gridX; i++)
	{
		for (j = 0; j < m_properties.gridY; j++)
		{
			fieldPuyoArray[i][j] = nullptr;
			fieldPuyoArrayCopy[i][j] = nullptr;
		}
	}

	m_fieldInit = true;
}

// Delete all puyo in array or copy
void field::freePuyo(bool copy)
{
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			if (!copy)
			{
				delete fieldPuyoArray[i][j];
				fieldPuyoArray[i][j] = nullptr;
			}
			else
			{
				delete fieldPuyoArrayCopy[i][j];
				fieldPuyoArrayCopy[i][j] = nullptr;
			}
		}
	}
	m_fieldInit = false;
}

// Delete array and array copy
void field::freePuyoArray()
{
	for (int i = 0; i < m_properties.gridX; i++)
	{
		delete[]fieldPuyoArray[i];
		delete[]fieldPuyoArrayCopy[i];
	}
	delete[]fieldPuyoArray;
	delete[]fieldPuyoArrayCopy;
	m_fieldInit = false;
}

// Get screen coordinates of top, with an offset
posVectorFloat field::getTopCoord(float offset) const
{
	posVectorFloat pv;
	pv.x = m_properties.offsetX + m_properties.centerX * m_properties.scaleX * m_player->getGlobalScale();
	pv.y = m_properties.offsetY + offset * PUYOY * m_properties.scaleY * m_player->getGlobalScale();
	return pv;
}

// Get screen coordinates of bottom
posVectorFloat field::getBottomCoord(bool relative) const
{
	posVectorFloat pv;
	if (relative)
	{
		pv.x = m_properties.centerX * m_properties.scaleX;
		pv.y = (m_properties.gridHeight * (m_properties.gridY - 3 - 0)) * m_properties.scaleY;
	}
	else
	{
		pv.x = m_properties.offsetX + m_properties.centerX * m_properties.scaleX * m_player->getGlobalScale();
		pv.y = m_properties.offsetY + (m_properties.gridHeight * (m_properties.gridY - 3 - 0)) * m_properties.scaleY * m_player->getGlobalScale();
	}
	return pv;
}

// Convert an indexed position to screen position
posVectorFloat field::getGlobalCoord(int x, int y) const
{
	posVectorFloat pv;
	pv.x = m_properties.offsetX + (m_properties.gridWidth * x) * m_properties.scaleX * m_player->getGlobalScale();
	pv.y = m_properties.offsetY + (m_properties.gridHeight * (m_properties.gridY - 3 - y)) * m_properties.scaleY * m_player->getGlobalScale();
	return pv;
}

// Convert an indexed position to field position
posVectorFloat field::getLocalCoord(int x, int y) const
{
	posVectorFloat pv;
	pv.x = static_cast<float>(m_properties.gridWidth * x);
	pv.y = static_cast<float>(m_properties.gridHeight * (m_properties.gridY - 3 - y));
	return pv;
}

// Returns current field size
posVectorFloat field::getFieldSize() const
{
	return posVectorFloat(
		static_cast<float>(m_properties.gridX * m_properties.gridWidth),
		static_cast<float>((m_properties.gridY - 3) * m_properties.gridHeight)
	);
}

posVectorFloat field::getFieldScale() const
{
	return posVectorFloat(m_properties.scaleX, m_properties.scaleY);
}

//============================
// Puyo Field related functions
//============================

// Check if a position in field is an empty space
bool field::isEmpty(int x, int y) const
{
	// special case: uninitialized position -> just say it's empty
	if (x < -2 && y < -2)
		return true;

	// x or y is outside the field
	// outside the field is regarded as nonempty! that doesnt mean there is a puyo there -> use isPuyo instead
	if (x > m_properties.gridX - 1 || x < 0 || y < 0)
		return false;

	// exception: the upper field stretches out to infinity
	if (y > m_properties.gridY - 1 && x > 0 && x < m_properties.gridX - 1)
		return true;

	if (fieldPuyoArray[x][y] == nullptr)
		return true;

	return false;
}

// Check if a puyo exist in this position
bool field::isPuyo(int x, int y) const
{
	// outside the field are no puyos
	if (x > m_properties.gridX - 1 || x<0 || y>m_properties.gridY - 1 || y < 0)
		return false;

	if (fieldPuyoArray[x][y] == nullptr)
		return false;

	return true;
}

// Retrieves color from field (if puyo is a colorpuyo anyway)
int field::getColor(int x, int y) const
{
	// x or y is outside the field
	if (x > m_properties.gridX - 1 || x<0 || y>m_properties.gridY - 1 || y < 0)
		return -1;

	// Ask the corresponding puyo what color it is
	if (isPuyo(x, y))
	{
		if (fieldPuyoArray[x][y]->getType() == COLORPUYO)
			return fieldPuyoArray[x][y]->getColor();

		return -1;
	}
	return -1;
}

// This function creates a colored puyo on the field
bool field::addColorPuyo(int x, int y, int color, int fallFlag, int offset, int fallDelay)
{
	// invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0)
		return false;

	// a puyo already exists here
	if (isPuyo(x, y))
	{
		return false;
	}

	// calculate position of sprite
	float spriteXreal = float(x * m_properties.gridWidth + PUYOX / 2);
	float spriteYreal = float((m_properties.gridY - 3 - y - offset) * m_properties.gridHeight);

	// create a new puyo at x,y index
	fieldPuyoArray[x][y] = new colorPuyo(x, y, color, this, spriteXreal, spriteYreal, data);

	// set initial condition
	fieldPuyoArray[x][y]->fallFlag = fallFlag;
	fieldPuyoArray[x][y]->fallDelay = static_cast<float>(fallDelay);

	return true;
}

// Add nuisance to the field
bool field::addNuisancePuyo(int x, int y, int fallFlag, int offset, int fallDelay)
{
	// invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0)
		return false;

	// a puyo already exists here
	if (isPuyo(x, y))
	{
		return false;
	}

	// calculate position of sprite
	float spriteXreal = float(x * m_properties.gridWidth + PUYOX / 2);
	float spriteYreal = float((m_properties.gridY - 3 - y - offset) * m_properties.gridHeight);

	// create a new puyo at x,y index
	fieldPuyoArray[x][y] = new nuisancePuyo(x, y, 0, this, spriteXreal, spriteYreal, data);

	// set initial state
	fieldPuyoArray[x][y]->fallFlag = fallFlag;
	fieldPuyoArray[x][y]->fallDelay = static_cast<float>(fallDelay);

	return true;
}

// Drop all puyos down.
void field::drop() const
{
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			dropSingle(i, j);
		}
	}
}

// Drop a single puyo down, returns new y position. returns -1 if no puyo dropped at all
int field::dropSingle(int x, int y) const
{
	if (!isPuyo(x, y))
		return -1; // nothing to drop

	if (!fieldPuyoArray[x][y]->dropable)
		return -1; // cannot drop

	// out of bounds
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0)
		return -1;

	bool foundEmpty;
	int emptyY;

	// puyo is on bottom: don't bother
	if (y == 0)
		return y;

	// initial checking if empty
	if (!(foundEmpty = isEmpty(x, y - 1)))
		return y;

	emptyY = y;

	while (foundEmpty && emptyY >= 1)
	{
		// check if next position is empty
		if ((foundEmpty = isEmpty(x, emptyY - 1)) == true)
			emptyY -= 1;
	}

	// drop puyo down
	fieldPuyoArray[x][emptyY] = fieldPuyoArray[x][y];
	fieldPuyoArray[x][emptyY]->SetindexY(emptyY);
	fieldPuyoArray[x][y] = nullptr;

	return emptyY;
}

// Get puyo at position
puyo* field::get(int x, int y) const
{
	if (isEmpty(x, y))
		return nullptr;

	return fieldPuyoArray[x][y];
}

// Set puyo, returns true if success
bool field::set(int x, int y, puyo* newpuyo) const
{
	// Invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0)
		return false;

	if (newpuyo == nullptr)
		fieldPuyoArray[x][y] = nullptr;
	else
		fieldPuyoArray[x][y] = newpuyo;

	return true;
}

void field::clearFieldVal(int x, int y) const
{
	if (isPuyo(x, y))
		fieldPuyoArray[x][y] = nullptr;
}

// Get Puyo type in a safe way
puyoType field::getPuyoType(int x, int y) const
{
	// invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0)
		return NOPUYO;

	if (!isPuyo(x, y))
		return NOPUYO;

	return fieldPuyoArray[x][y]->getType();
}

// Connect to a neighbouring puyo
void field::setLink(int x, int y, direction dir) const
{
	// invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0)
		return;

	if (isPuyo(x, y))
		fieldPuyoArray[x][y]->setLink(dir);
}

// Disconnect a puyo
void field::unsetLink(int x, int y, direction dir) const
{
	// invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0)
		return;

	if (isPuyo(x, y))
		fieldPuyoArray[x][y]->unsetLink(dir);
}

// Find n connected color puyo and put coordinates into vector v
bool field::findConnected(int x, int y, int n, std::vector<posVectorInt>& v)
{
	int connected = 0;
	posVectorInt pos;
	pos.x = x;
	pos.y = y;

	// Initial check of first puyo
	if (isPuyo(pos.x, pos.y) && fieldPuyoArray[pos.x][pos.y]->getType() == COLORPUYO
		&& fieldPuyoArray[pos.x][pos.y]->mark == false && fieldPuyoArray[pos.x][pos.y]->destroy == false)
	{
		// find connecting color puyo and fill vector v
		fieldPuyoArray[pos.x][pos.y]->mark = true;
		connected = 1;
		v.push_back(pos);
		findConnectedLoop(pos, connected, v);
	}
	else
	{
		return false;
	}

	if (connected < n)
	{
		// did not match criterium: remove the elements from v
		while (connected)
		{
			v.pop_back();
			connected--;
		}
		return false;
	}
	return true;
}

// Unmark all puyo in field
void field::unmark() const
{
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			if (isPuyo(i, j))
			{
				fieldPuyoArray[i][j]->mark = false;
			}
		}
	}
}

// Recursive function that checks neighbours
void field::findConnectedLoop(posVectorInt pos, int& connected, std::vector<posVectorInt>& v)
{
	int k, hor, ver;
	fieldPuyoArray[pos.x][pos.y]->mark = true;
	posVectorInt pos_copy;
	pos_copy.x = pos.x;
	pos_copy.y = pos.y;
	for (k = 0; k < 4; k++)
	{
		switch (k)
		{
		case 0: hor = 0; ver = 1; break;
		case 1: hor = 1; ver = 0; break;
		case 2: hor = 0; ver = -1; break;
		case 3: hor = -1; ver = 0; break;
		}
		if (isPuyo(pos.x + hor, pos.y + ver) && fieldPuyoArray[pos.x + hor][pos.y + ver]->getType() == COLORPUYO
			&& fieldPuyoArray[pos.x + hor][pos.y + ver]->mark == false && fieldPuyoArray[pos.x + hor][pos.y + ver]->mark == false
			&& fieldPuyoArray[pos.x + hor][pos.y + ver]->getColor() == fieldPuyoArray[pos.x][pos.y]->getColor() && pos.y + ver != m_properties.gridY - 3)
		{
			connected++;
			pos_copy.x = pos.x + hor; // need a copy of pos to pass on
			pos_copy.y = pos.y + ver;
			v.push_back(pos_copy);
			findConnectedLoop(pos_copy, connected, v);
		}
	}
}

// Count puyos in field
int field::count() const
{
	int n = 0, i, j;

	for (i = 0; i < m_properties.gridX; i++)
	{
		for (j = 0; j < m_properties.gridY; j++)
		{
			if (isPuyo(i, j))
			{
				n++;
			}
		}
	}
	return n;
}

// Predict chain
int field::predictChain()
{
	// Copy original array into arraycopy
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			// Copy pointer
			fieldPuyoArrayCopy[i][j] = fieldPuyoArray[i][j];
			// Copy puyo
			if (isPuyo(i, j))
				fieldPuyoArray[i][j] = fieldPuyoArray[i][j]->clone();
		}
	}
	
	// Find chain
	bool foundChain = false; // local variable
	posVectorInt pv;
	int chainN = 0;
	do
	{
		foundChain = false;
		unmark();
		// loop through field to find connected puyo
		for (int i = 0; i < m_properties.gridX; i++)
		{
			for (int j = 0; j < m_properties.gridY - 3; j++)
			{
				if (findConnected(i, j, m_player->currentgame->currentruleset->puyoToClear, m_vector))
				{
					foundChain = true;
					// loop through connected puyo
					while (m_vector.size() > 0)
					{
						pv = m_vector.back();
						m_vector.pop_back();
						// set popped group
						// check neighbours to find nuisance
						if (isPuyo(pv.x, pv.y + 1) && pv.y + 1 != m_properties.gridY - 3)
							fieldPuyoArray[pv.x][pv.y + 1]->neighbourPop(this, true);
						if (isPuyo(pv.x + 1, pv.y))
							fieldPuyoArray[pv.x + 1][pv.y]->neighbourPop(this, true);
						if (isPuyo(pv.x, pv.y - 1))
							fieldPuyoArray[pv.x][pv.y - 1]->neighbourPop(this, true);
						if (isPuyo(pv.x - 1, pv.y))
							fieldPuyoArray[pv.x - 1][pv.y]->neighbourPop(this, true);
						// delete puyo
						delete fieldPuyoArray[pv.x][pv.y];
						fieldPuyoArray[pv.x][pv.y] = nullptr;
					}

				}
			}
		}

		// drop puyo and add to chain number
		if (foundChain)
		{
			chainN++;
			drop();
		}
	} while (foundChain);

	// delete temporary puyos
	freePuyo(false);
	m_fieldInit = true;
	// restore original
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			// Copy pointer
			fieldPuyoArray[i][j] = fieldPuyoArrayCopy[i][j];
			fieldPuyoArrayCopy[i][j] = nullptr;
		}
	}

	return chainN;
}

// Function that marks puyo and puts them in the deletedpuyo list
void field::removePuyo(int x, int y)
{
	if (isPuyo(x, y))
	{
		fieldPuyoArray[x][y]->destroy = true;
		m_deletedPuyo.push_back(fieldPuyoArray[x][y]);
	}
}

// Clean up field
void field::clearField() const
{
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			if (!isEmpty(i, j))
			{
				delete fieldPuyoArray[i][j];
				fieldPuyoArray[i][j] = nullptr;
			}
		}
	}
}

//============================
// Sprite related functions
//============================

// Set field and content visible
void field::setVisible(bool visibility)
{
	m_visible = visibility;
}

void field::clear()
{
}

// Draw field background and content
void field::drawField() const
{
	if (!m_visible)
		return;

	// Lower layer: consider moving this part to player!
	m_player->drawFieldBack(getBottomCoord(true), m_properties.angle);
	m_player->drawFieldFeverBack(getBottomCoord(true), m_properties.angle);
	m_player->drawCross(data->front);
	m_player->drawAllClear(getBottomCoord(true), 1, 1, m_properties.angle);
	data->front->setDepthFunction(equal);

	// Draw puyos on the field
	for (int i = 0; i < m_properties.gridX; i++)
		for (int j = 0; j < m_properties.gridY; j++)
			if (isPuyo(i, j))
				fieldPuyoArray[i][j]->draw(data->front);

	// Draw deleting puyo
	for (size_t i = 0; i < m_deletedPuyo.size(); i++)
		m_deletedPuyo[i]->draw(data->front);

	// Draw particles on the field
	for (size_t i = 0; i < m_particles.size(); i++)
		m_particles[i]->draw(data->front);

	// Draw throwPuyo on the field
	for (size_t i = 0; i < m_particlesThrow.size(); i++)
		m_particlesThrow[i]->draw(data->front);
}

void field::draw() const
{
	drawField();
}

//============================
// Gameplay related functions
//============================

// Phase 20: Create puyo and get ready to drop
void field::createPuyo()
{
	if (m_player->createPuyo == false)
		return;
	// turn off glow
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			if (isPuyo(i, j))
			{
				fieldPuyoArray[i][j]->glow = false;
			}
		}
	}
	// only create puyo after rotation has really finished
	if (((m_player->movePuyos.getRotateCounter() == 0 && m_player->movePuyos.getFlipCounter() == 0) && m_player->getPlayerType() != ONLINE && m_player->currentgame->settings->recording != PVS_REPLAYING)
		|| (m_player->getPlayerType() == ONLINE && !m_player->messages.empty() && m_player->messages.front()[0] == 'p')
		|| ((m_player->getPlayerType() == HUMAN && m_player->currentgame->settings->recording == PVS_REPLAYING) && m_player->messages.front()[0] == 'p'))
	{
		m_player->movePuyos.setVisible(false);
		// create colored puyo pair on field
		// get values from movePuyos
		movePuyoType type = m_player->movePuyos.getType();
		int color1 = m_player->movePuyos.getColor1();
		int color2 = m_player->movePuyos.getColor2();
		int colorBig = m_player->movePuyos.getColorBig();
		int posX1 = m_player->movePuyos.getPosX1();
		int posY1 = m_player->movePuyos.getPosY1();
		int posX2 = m_player->movePuyos.getPosX2();
		int posY2 = m_player->movePuyos.getPosY2();
		int posX3 = m_player->movePuyos.getPosX3();
		int posY3 = m_player->movePuyos.getPosY3();
		int posX4 = m_player->movePuyos.getPosX4();
		int posY4 = m_player->movePuyos.getPosY4();
		bool transpose = m_player->movePuyos.getTranspose();
		int rotation = m_player->movePuyos.getRotation();

		m_player->divider = max(2, m_player->currentgame->getActivePlayers());
		// award a bonus garbage when offsetting?
		if (m_player->currentgame->currentruleset->bonusEQ && m_player->getPlayerType() != ONLINE)
		{
			// check if any garbage, award bonus EQ
			if (m_player->getGarbageSum() > 0)
				m_player->bonusEQ = true;
		}

		// send placement info
		// 0["p"]1[color1]2[color2]3[colorBig]
		// 4[posx1]5[posy1]6[posx2]7[posy2]8[posx3]9[posy3]10[posx4]11[posy4]
		// 12[scoreVal]13[dropBonus]14[margintime]15[divider]16[bonusEQ]
		if (m_player->currentgame->connected && m_player->getPlayerType() == HUMAN)
		{
			char str[200];
			sprintf(str, "p|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i",
				color1, color2, colorBig,
				posX1, posY1,
				posX2, posY2,
				posX3, posY3,
				posX4, posY4,
				m_player->scoreVal, m_player->dropBonus,
				m_player->margintimer, m_player->divider, int(m_player->bonusEQ));
			m_player->currentgame->network->sendToChannel(CHANNEL_GAME, str, m_player->currentgame->channelName.c_str());

			// record for replay
			if (m_player->currentgame->settings->recording == PVS_RECORDING)
			{
				messageEvent me = { data->matchTimer,"" };
				std::string mes = str;
				m_player->recordMessages.push_back(me);
				if (mes.length() < 64)
					strcpy(m_player->recordMessages.back().message, mes.c_str());
			}
		}
		// receive
		if ((m_player->getPlayerType() == ONLINE || m_player->currentgame->settings->recording == PVS_REPLAYING)
			&& !m_player->messages.empty() && m_player->messages.front()[0] == 'p')
		{
			int bEQ = 0;
			int marginTime = 0;
			sscanf(m_player->messages.front().c_str(), "p|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i",
				&color1, &color2, &colorBig,
				&posX1, &posY1,
				&posX2, &posY2,
				&posX3, &posY3,
				&posX4, &posY4,
				&m_player->scoreVal, &m_player->dropBonus,
				&marginTime, &m_player->divider, &bEQ);

			// adjust margintime downwards with 30 second error interval
			if (m_player->margintimer + 20 * 60 >= marginTime)
				m_player->margintimer = marginTime;

			if (bEQ) m_player->bonusEQ = true;

			// clear message
			m_player->messages.pop_front();
		}
		
		// place puyos
		if (type == DOUBLET)
		{
			addColorPuyo(posX1, posY1, color1);
			addColorPuyo(posX2, posY2, color2);
		}
		else if (type == TRIPLET && transpose == false)
		{
			addColorPuyo(posX1, posY1, color1);
			addColorPuyo(posX2, posY2, color2);
			addColorPuyo(posX3, posY3, color1);
		}
		else if (type == TRIPLET && transpose == true)
		{
			addColorPuyo(posX1, posY1, color1);
			addColorPuyo(posX2, posY2, color1);
			addColorPuyo(posX3, posY3, color2);
		}
		else if (type == QUADRUPLET)
		{
			if (rotation == 1)
			{
				addColorPuyo(posX1, posY1, color2);
				addColorPuyo(posX2, posY2, color1);
				addColorPuyo(posX3, posY3, color2);
				addColorPuyo(posX4, posY4, color1);
			}
			else if (rotation == 2)
			{
				addColorPuyo(posX1, posY1, color1);
				addColorPuyo(posX2, posY2, color1);
				addColorPuyo(posX3, posY3, color2);
				addColorPuyo(posX4, posY4, color2);
			}
			else if (rotation == 3)
			{
				addColorPuyo(posX1, posY1, color1);
				addColorPuyo(posX2, posY2, color2);
				addColorPuyo(posX3, posY3, color1);
				addColorPuyo(posX4, posY4, color2);
			}
			else if (rotation == 0)
			{
				addColorPuyo(posX1, posY1, color2);
				addColorPuyo(posX2, posY2, color2);
				addColorPuyo(posX3, posY3, color1);
				addColorPuyo(posX4, posY4, color1);
			}
		}
		else if (type == BIG)
		{
			addColorPuyo(posX1, posY1, colorBig);
			addColorPuyo(posX2, posY2, colorBig);
			addColorPuyo(posX3, posY3, colorBig);
			addColorPuyo(posX4, posY4, colorBig);
		}
		// play sound
		data->snd.drop.Play(data);
		
		// check if garbage should fall now
		if (m_player->activeGarbage->GQ <= 0
			// bugfix: only outside fever
			&& !m_player->feverMode)
			m_player->forgiveGarbage = true;
		// if it is fever: you can, but fever must have not ended yet

		// end phase 20
		if (m_player->currentphase == CREATEPUYO)
			m_player->endPhase();
	}
}

// DEPRECATED. set puyo ready to fall and disconnect them
void field::searchFallDelay() const
{
	int count = 0;
	int countDelay = 0;
	bool foundEmpty = false;

	for (int i = 0; i < m_properties.gridX; i++)
	{
		foundEmpty = false;
		count = 0;
		countDelay = 0;
		// predict the position to fall to
		for (int j = 0; j < m_properties.gridY; j++)
		{
			// Search for empty spaces. After first empty space, start marking
			if (isEmpty(i, j) && foundEmpty == false)
			{
				foundEmpty = true;
				countDelay = 0;
				count = j;
			}
			// after empty space found
			if (foundEmpty == true && isPuyo(i, j))
			{
				fieldPuyoArray[i][j]->fallDelay = static_cast<float>(countDelay);
				fieldPuyoArray[i][j]->fallFlag = 1;
				fieldPuyoArray[i][j]->setFallTarget(count);
				count++;
				countDelay++;
				if (getPuyoType(i, j) == COLORPUYO)
					unsetLinkAll(i, j);
			}
			// Replace Mark puyos to fall
			// (these events are not placed above)
		}
	}
}

// Disconnect colorpuyo from all directions
void field::unsetLinkAll(int i, int j) const
{
	// Check down -> unset uplink
	if (getPuyoType(i, j - 1) == COLORPUYO)
		unsetLink(i, j - 1, ABOVE);

	// Check up
	if (getPuyoType(i, j + 1) == COLORPUYO)
		unsetLink(i, j + 1, BELOW);

	// Check right
	if (getPuyoType(i + 1, j) == COLORPUYO)
		unsetLink(i + 1, j, LEFT);

	// Check left
	if (getPuyoType(i - 1, j) == COLORPUYO)
		unsetLink(i - 1, j, RIGHT);

	// Disconnect self
	unsetLink(i, j, ABOVE);
	unsetLink(i, j, BELOW);
	unsetLink(i, j, LEFT);
	unsetLink(i, j, RIGHT);
}

// Cleanly drop all puyo (set fall target etc.)
void field::dropPuyo()
{
	for (int i = 0; i < m_properties.gridX; i++)
	{
		int countDelay = 0;
		for (int j = 0; j < m_properties.gridY; j++)
		{
			// drop
			int newy = dropSingle(i, j);
			if (newy == -1)
				continue;
			// set fall delay
			if (j != newy)
			{
				if (getPuyoType(i, newy) == COLORPUYO)
				{
					unsetLinkAll(i, j);
					unsetLink(i, newy, ABOVE);
					unsetLink(i, newy, BELOW);
					unsetLink(i, newy, LEFT);
					unsetLink(i, newy, RIGHT);
				}
				fieldPuyoArray[i][newy]->fallDelay = static_cast<float>(countDelay);
				fieldPuyoArray[i][newy]->fallFlag = 1;
				fieldPuyoArray[i][newy]->setFallTarget(newy);
				countDelay++;
			}
		}
	}

	m_sweepFall = 0;
	m_player->endPhase();
}

// Phase 21: falling field puyos
void field::fallPuyo()
{
	int i, j;
	// do a sweep
	if (m_player->currentgame->currentruleset->delayedFall == true)
		m_sweepFall += 0.5f;
	else
		m_sweepFall = 100;

	// loop through puyos
	for (i = 0; i < m_properties.gridX; i++)
	{
		for (j = 0; j < m_properties.gridY; j++)
		{
			if (isPuyo(i, j))
			{
				if (fieldPuyoArray[i][j]->fallFlag == 1 && fieldPuyoArray[i][j]->fallDelay <= m_sweepFall)
				{
					fieldPuyoArray[i][j]->fallFlag = 2;
					fieldPuyoArray[i][j]->SetAccelY(0);
				}
				// set gravity
				if (fieldPuyoArray[i][j]->fallFlag == 2)
					fieldPuyoArray[i][j]->AddAccelY(m_player->gravity);
				// If it's not a faller, mark the puyo for a bounce.
				// Search for Puyos that need to bounce. Use flag 0 for this
				if (fieldPuyoArray[i][j]->fallFlag == 0 && fieldPuyoArray[i][j]->bounceFlag0 == false)
				{
					fieldPuyoArray[i][j]->SetAccelY(0);
					fieldPuyoArray[i][j]->bounceFlag = 1;
					fieldPuyoArray[i][j]->bounceTimer = 2;
					fieldPuyoArray[i][j]->bounceFlag0 = true;
					searchBounce(i, j, fieldPuyoArray[i][j]->GetindexY() + 1);
				}
				// sound & voice when dropping nuisance
				// non-nuisance puyo
				if (fieldPuyoArray[i][j]->fallFlag != 0 && fieldPuyoArray[i][j]->GetSpriteY() > fieldPuyoArray[i][j]->GetTargetY()
					&& fieldPuyoArray[i][j]->getType() != NUISANCEPUYO)
				{
					data->snd.drop.Play(data);
				}
				// nuisancepuyo: normal drop
				if (fieldPuyoArray[i][j]->fallFlag != 0 && fieldPuyoArray[i][j]->GetSpriteY() > fieldPuyoArray[i][j]->GetTargetY()
					&& fieldPuyoArray[i][j]->getType() == NUISANCEPUYO && m_player->currentphase != FALLGARBAGE)
				{
					data->snd.drop.Play(data);
				}
				// nuisancepuyo: garbage drop
				if (fieldPuyoArray[i][j]->fallFlag != 0 && fieldPuyoArray[i][j]->GetSpriteY() > fieldPuyoArray[i][j]->GetTargetY()
					&& fieldPuyoArray[i][j]->getType() == NUISANCEPUYO && fieldPuyoArray[i][j]->lastNuisance && m_player->garbageDropped < 6)
				{
					m_player->garbageDropped = 0;
					fieldPuyoArray[i][j]->lastNuisance = false;
					data->snd.nuisanceS.Play(data);
				}
				if (fieldPuyoArray[i][j]->fallFlag != 0 && fieldPuyoArray[i][j]->GetSpriteY() > fieldPuyoArray[i][j]->GetTargetY()
					&& fieldPuyoArray[i][j]->getType() == NUISANCEPUYO && fieldPuyoArray[i][j]->lastNuisance && m_player->garbageDropped >= 6)
				{
					if (m_player->garbageDropped < 24)
						m_player->characterVoices.damage1.Play(data);
					else
						m_player->characterVoices.damage2.Play(data);
					m_player->garbageDropped = 0;
					fieldPuyoArray[i][j]->lastNuisance = false;
					data->snd.nuisanceL.Play(data);

				}

				// landproper
				if (fieldPuyoArray[i][j]->fallFlag == 2 && fieldPuyoArray[i][j]->GetSpriteY() > fieldPuyoArray[i][j]->GetTargetY())
				{
					fieldPuyoArray[i][j]->bounceFlag0 = false;
					// landproper
					fieldPuyoArray[i][j]->landProper();
				}
			}
		}
	}
}

// Phase 21: bouncing field puyos
void field::bouncePuyo() const
{
	// loop through puyos
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			if (isPuyo(i, j))
			{
				// bounce puyo
				if (fieldPuyoArray[i][j]->fallFlag == 0)
					fieldPuyoArray[i][j]->bounce();
				// end of bounce
				if (fieldPuyoArray[i][j]->bounceTimer > m_player->puyoBounceEnd)
				{
					// search connect
					if (fieldPuyoArray[i][j]->getType() == COLORPUYO)
						searchLink(i, j);
					fieldPuyoArray[i][j]->SetScaleX(1);
					fieldPuyoArray[i][j]->SetScaleY(1);
					fieldPuyoArray[i][j]->bounceY = 0;
					if (fieldPuyoArray[i][j]->bounceFlag == 1)
					{
						fieldPuyoArray[i][j]->bounceFlag = 0;
					}
					fieldPuyoArray[i][j]->bounceTimer = 0;
				}
			}
		}
	}
}

// search bounce strength
void field::searchBounce(int x, int y, int posy) const
{
	int i, j, funFlag = 1, count = 1, rememberColor;
	for (i = 0; i < posy; i++)
	{
		if (isPuyo(x, y - i))
		{
			// stop loop
			if (funFlag == 1 && fieldPuyoArray[x][y - i]->hard == true)
			{
				// loop back up
				for (j = 0; j <= i; j++)
				{
					fieldPuyoArray[x][y - i + j]->bottomY = y - i;
				}
				funFlag = 0;
				break;
			}
			// default value
			fieldPuyoArray[x][y - i]->bottomY = 0;
		}
		if (funFlag == 1 && y - i < 0)
			funFlag = 0;
		// set bouncemultiplied, also start disconnecting puyos
		// search for disconnect
		// the count is what determines how many puyos disconnect
		if (funFlag == 1 && count < 5)
		{
			if (isPuyo(x, y - i) && fieldPuyoArray[x][y - i]->fallFlag == 0)
			{
				rememberColor = fieldPuyoArray[x][y - i]->getColor();
				unsetLinkAll(x, y - i);
				// Set bouncemultiplier
				fieldPuyoArray[x][y - i]->bounceMultiplier = 1.f / static_cast<float>(pow(2.f, count - 1));
				fieldPuyoArray[x][y - i]->bounceTimer = 2.f;
			}
			count++;
		}
	}
}

// search for puyos connecting
void field::searchLink(int x, int y) const
{
	// check down
	if (isPuyo(x, y - 1) && fieldPuyoArray[x][y - 1]->getType() == COLORPUYO && fieldPuyoArray[x][y - 1]->fallFlag == 0 &&
		fieldPuyoArray[x][y - 1]->getColor() == fieldPuyoArray[x][y]->getColor() && y != m_properties.gridY - 3)
	{
		// do not connect in the invisible layers
		fieldPuyoArray[x][y - 1]->setLink(ABOVE);
		fieldPuyoArray[x][y]->setLink(BELOW);
	}
	// check up
	if (isPuyo(x, y + 1) && fieldPuyoArray[x][y + 1]->getType() == COLORPUYO && fieldPuyoArray[x][y + 1]->fallFlag == 0 &&
		fieldPuyoArray[x][y + 1]->getColor() == fieldPuyoArray[x][y]->getColor() && y != m_properties.gridY - 4)
	{
		// do not connect in the invisible layers
		fieldPuyoArray[x][y + 1]->setLink(BELOW);
		fieldPuyoArray[x][y]->setLink(ABOVE);
	}
	// check right
	if (isPuyo(x + 1, y) && fieldPuyoArray[x + 1][y]->getType() == COLORPUYO && fieldPuyoArray[x + 1][y]->fallFlag == 0 &&
		fieldPuyoArray[x + 1][y]->getColor() == fieldPuyoArray[x][y]->getColor())
	{
		fieldPuyoArray[x + 1][y]->setLink(LEFT);
		fieldPuyoArray[x][y]->setLink(RIGHT);
	}
	if (isPuyo(x + 1, y) && fieldPuyoArray[x + 1][y]->getType() == COLORPUYO && fieldPuyoArray[x + 1][y]->fallFlag != 0 &&
		fieldPuyoArray[x + 1][y]->getColor() == fieldPuyoArray[x][y]->getColor())
	{
		fieldPuyoArray[x + 1][y]->unsetLink(LEFT);
		fieldPuyoArray[x][y]->unsetLink(RIGHT);
	}
	// check left
	if (isPuyo(x - 1, y) && fieldPuyoArray[x - 1][y]->getType() == COLORPUYO && fieldPuyoArray[x - 1][y]->fallFlag == 0 &&
		fieldPuyoArray[x - 1][y]->getColor() == fieldPuyoArray[x][y]->getColor())
	{
		fieldPuyoArray[x - 1][y]->setLink(RIGHT);
		fieldPuyoArray[x][y]->setLink(LEFT);
	}
	else if (isPuyo(x - 1, y) && fieldPuyoArray[x - 1][y]->getType() == COLORPUYO && fieldPuyoArray[x - 1][y]->fallFlag != 0 &&
		fieldPuyoArray[x - 1][y]->getColor() == fieldPuyoArray[x][y]->getColor())
	{
		fieldPuyoArray[x - 1][y]->unsetLink(RIGHT);
		fieldPuyoArray[x][y]->unsetLink(LEFT);
	}
}

// End of phase 20
void field::endFallPuyoPhase() const
{
	// loop through puyos
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			if (isPuyo(i, j))
			{
				fieldPuyoArray[i][j]->glow = false;
				// check if any is bouncing falling or destroying
				if (fieldPuyoArray[i][j]->fallFlag != 0 || fieldPuyoArray[i][j]->bounceTimer != 0)
					return;
			}
		}
	}

	// play voice
	if (m_player->feverMode)
	{
		if (m_player->feverSuccess == 1)
			m_player->characterVoices.feversuccess.Play(data);
		else if (m_player->feverSuccess == 2)
			m_player->characterVoices.feverfail.Play(data);
		m_player->feverSuccess = 0;
	}
	// nothing is bouncing
	m_player->endPhase();
}

void field::searchChain()
{
	posVectorInt pv;

	// Destroy 14th and 15th row
	for (int i = 0; i < m_properties.gridX; i++)
	{
		if (isPuyo(i, m_properties.gridY - 1))
		{
			delete fieldPuyoArray[i][m_properties.gridY - 1];
			fieldPuyoArray[i][m_properties.gridY - 1] = nullptr;
		}
		if (isPuyo(i, m_properties.gridY - 2))
		{
			delete fieldPuyoArray[i][m_properties.gridY - 2];
			fieldPuyoArray[i][m_properties.gridY - 2] = nullptr;
		}
	}

	// predict chain
	if (m_player->chain == 0)
	{
		m_player->predictedChain = 0;
		m_player->predictedChain = predictChain();
	}

	// reset chain values
	m_player->foundChain = 0;
	m_player->puyosPopped = 0;
	m_player->groupR = 0;
	m_player->groupG = 0;
	m_player->groupB = 0;
	m_player->groupY = 0;
	m_player->groupP = 0;
	m_player->point = 0;
	m_player->linkBonus = 0;
	m_player->bonus = 0;
	m_player->rememberMaxY = 0;
	m_player->rememberX = 0;

	unmark();

	// Find chain
	// loop through field
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY - 3; j++)
		{
			if (findConnected(i, j, m_player->currentgame->currentruleset->puyoToClear, m_vector))
			{
				m_player->foundChain = 1;
				m_player->poppedChain = true;
				// loop through connected puyo
				while (m_vector.size() > 0)
				{
					pv = m_vector.back();
					m_vector.pop_back();
					// set popped group
					if (fieldPuyoArray[pv.x][pv.y]->getType() == COLORPUYO)
					{
						switch (fieldPuyoArray[pv.x][pv.y]->getColor())
						{
						case -1: break;
						case 0: m_player->groupR = 1; break;
						case 1: m_player->groupG = 1; break;
						case 2: m_player->groupB = 1; break;
						case 3: m_player->groupY = 1; break;
						case 4: m_player->groupP = 1; break;
						}
					}
					// add to puyo count
					m_player->puyosPopped++;
					// check neighbours to find nuisance
					if (isPuyo(pv.x, pv.y + 1) && pv.y + 1 != m_properties.gridY - 3)
						fieldPuyoArray[pv.x][pv.y + 1]->neighbourPop(this, false);
					if (isPuyo(pv.x + 1, pv.y))
						fieldPuyoArray[pv.x + 1][pv.y]->neighbourPop(this, false);
					if (isPuyo(pv.x, pv.y - 1))
						fieldPuyoArray[pv.x][pv.y - 1]->neighbourPop(this, false);
					if (isPuyo(pv.x - 1, pv.y))
						fieldPuyoArray[pv.x - 1][pv.y]->neighbourPop(this, false);
					// check if highest puyo
					if (pv.y > m_player->rememberMaxY)
					{
						m_player->rememberMaxY = pv.y;
						m_player->rememberX = pv.x;
					}
					// pop puyo
					removePuyo(pv.x, pv.y);
					fieldPuyoArray[pv.x][pv.y] = nullptr;
				}
				// Add popped puyos to score
				m_player->point += m_player->puyosPopped * 10;
				// add linkbonus
				m_player->linkBonus += m_player->currentgame->currentruleset->getLinkBonus(m_player->puyosPopped);
				// reset popped puyos
				m_player->puyosPopped = 0;
			}
		}
	}

	// calculate score
	// (point and link bonus are calculated during loop)
	if (m_player->foundChain)
	{
		m_player->chain++;
	}
	else
	{
		// no chain: remove any bonus EQ
		m_player->bonusEQ = false;
	}
	// sum chain bonus and colorbonus
	m_player->bonus += m_player->currentgame->currentruleset->getChainBonus(m_player);
	if (m_player->currentgame->settings->recording == PVS_REPLAYING && m_player->currentgame->currentReplayVersion <= 1)
		m_player->bonus += m_player->currentgame->currentruleset->getColorBonus(0); //replay compatibility v1
	else
		m_player->bonus += m_player->currentgame->currentruleset->getColorBonus(m_player->groupR + m_player->groupG + m_player->groupB + m_player->groupY + m_player->groupP);
	m_player->bonus += m_player->linkBonus;

	// set bonus to 1 if zero (ruleset specific??)
	if (m_player->chain == 1 && m_player->bonus == 0)
		m_player->bonus++;

	if (m_player->chain > 0)
	{
		// add score
		m_player->scoreVal += m_player->point * m_player->bonus;
		// add currentscore
		m_player->currentScore += m_player->point * m_player->bonus;
		// add dropBonus
		if (m_player->currentgame->currentruleset->addDropBonus)
		{
			// cap dropbonus
			if (m_player->dropBonus > 300)
				m_player->dropBonus = 300;

			m_player->currentScore += m_player->dropBonus;
			m_player->dropBonus = 0;
		}

		// show calculation
		m_player->setScoreCounterPB();
	}

	// trigger all clear action here for tsu
	if (m_player->chain > 0 && m_player->allClear == 1)
		m_player->currentgame->currentruleset->onAllClearPop(m_player);

	// set rule related variables
	if (m_player->chain > 0 && m_player->foundChain)
		m_player->currentgame->currentruleset->onChain(m_player);

	// End phase
	m_player->endPhase();
}

// Do pop animation for deleted puyos
void field::popPuyoAnim()
{
	for (unsigned int i = 0; i < m_deletedPuyo.size(); i++)
	{
		m_deletedPuyo[i]->pop();
		// Check if it should be deleted
		if (m_deletedPuyo[i]->destroyPuyo())
		{
			delete m_deletedPuyo[i];
			m_deletedPuyo.erase(std::remove(m_deletedPuyo.begin(), m_deletedPuyo.end(), m_deletedPuyo[i]), m_deletedPuyo.end());
		}
	}
}

// Other objects

// Creata a particle at position
void field::createParticle(float x, float y, int color)
{
	m_particles.push_back(new particle(x + getRandom(11) - 5, y - m_properties.gridHeight / 2 + getRandom(11) - 5, color, data));
}

// Creata a particle at position
void field::createParticleThrow(puyo* p)
{
	if (!p)
		return;
	if (p->getType() == COLORPUYO)
		m_particlesThrow.push_back(new particleThrow(p->GetSpriteX(), p->GetSpriteY() - m_properties.gridHeight / 2, p->getColor(), data));
	else if (p->getType() == NUISANCEPUYO)
		m_particlesThrow.push_back(new particleThrow(p->GetSpriteX(), p->GetSpriteY() - m_properties.gridHeight / 2, 6, data));
}

// Move the particles
void field::animateParticle()
{
	if (getParticleNumber() > 0)
	{
		for (int i = 0; i < getParticleNumber(); i++)
		{
			m_particles[i]->play();
			// check if it needs to be deleted
			if (m_particles[i]->destroy())
			{
				delete m_particles[i];
				m_particles.erase(std::remove(m_particles.begin(), m_particles.end(), m_particles[i]), m_particles.end());
			}
		}
	}

	// same for thrown puyos
	if (m_particlesThrow.size() > 0)
	{
		for (size_t i = 0; i < m_particlesThrow.size(); i++)
		{
			m_particlesThrow[i]->play();
			//check if it needs to be deleted
			if (m_particlesThrow[i]->destroy())
			{
				delete m_particlesThrow[i];
				m_particlesThrow.erase(std::remove(m_particlesThrow.begin(), m_particlesThrow.end(), m_particlesThrow[i]), m_particlesThrow.end());
			}
		}
	}
}

int field::getParticleNumber() const
{
	return static_cast<int>(m_particles.size());
}

void field::triggerGlow(posVectorInt shadowPos[4], int n, int colors[4])
{
	// check empty
	for (int i = 0; i < n; i++)
	{
		// placing on impossible spot
		if (!isEmpty(shadowPos[i].x, shadowPos[i].y))
		{
			// return immediately
			return;
		}

		// out of bounds
		if (shadowPos[i].x > m_properties.gridX - 1 || shadowPos[i].x<0 || shadowPos[i].y>m_properties.gridY - 1 || shadowPos[i].y < 0)
			return;
	}

	// Temporarily add puyos at the shadow positions
	for (int i = 0; i < n; i++)
	{
		if (isEmpty(shadowPos[i].x, shadowPos[i].y))
		{
			addColorPuyo(shadowPos[i].x, shadowPos[i].y, colors[i]);
		}
		else
		{
			// error
			shadowPos[i].x = -1;
			shadowPos[i].y = -1;
			debugstring += "error";
		}
	}
	posVectorInt pv;

	// check if puyos are connected at these positions
	unmark();

	// unglow all
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			if (isPuyo(i, j))
			{
				fieldPuyoArray[i][j]->glow = false;
			}
		}
	}
	for (int i = 0; i < n; i++)
	{
		if (findConnected(shadowPos[i].x, shadowPos[i].y, m_player->currentgame->currentruleset->puyoToClear, m_vector))
		{
			// loop through connected puyo
			while (m_vector.size() > 0)
			{
				pv = m_vector.back();
				m_vector.pop_back();
				// set to glow
				fieldPuyoArray[pv.x][pv.y]->glow = true;
			}
		}
	}

	// remove temporary puyo
	for (int i = 0; i < n; i++)
	{
		if (shadowPos[i].x >= 0 && shadowPos[i].y >= 0)
		{
			delete fieldPuyoArray[shadowPos[i].x][shadowPos[i].y];
			fieldPuyoArray[shadowPos[i].x][shadowPos[i].y] = nullptr;
		}
	}
}
int field::virtualChain(posVectorInt shadowPos[4], int n, int colors[4])
{
	// check empty
	for (int i = 0; i < n; i++)
	{
		if (!isEmpty(shadowPos[i].x, shadowPos[i].y))
		{
			// return immediately
			return 0;
		}
	}

	// check for any incorrect values
	for (int i = 0; i < n; i++)
	{
		if (shadowPos[i].y >= m_properties.gridY)
		{
			shadowPos[i].x = -1;
			shadowPos[i].y = -1;
		}
	}

	// Temporarily add puyos at the shadow positions
	for (int i = 0; i < n; i++)
	{
		if (isEmpty(shadowPos[i].x, shadowPos[i].y))
		{
			addColorPuyo(shadowPos[i].x, shadowPos[i].y, colors[i]);
		}
		else
		{
			// error
			shadowPos[i].x = -1;
			shadowPos[i].y = -1;
		}
	}
	posVectorInt pv;
	// check if puyos are connected at these positions
	unmark();

	// predict chain
	int predictedChain = predictChain();

	// remove temporary puyo
	for (int i = 0; i < n; i++)
	{
		if (shadowPos[i].x >= 0 && shadowPos[i].y >= 0)
		{
			delete fieldPuyoArray[shadowPos[i].x][shadowPos[i].y];
			fieldPuyoArray[shadowPos[i].x][shadowPos[i].y] = nullptr;
		}
	}
	return predictedChain;
}


void field::dropGarbage(bool automatic, int dropAmount)
{
	int lastx = -1;
	int lasty = -1;
	bool dropped = false;

	int dropN = 0;
	if (automatic)
		dropN = m_player->activeGarbage->GQ;
	else
		dropN = dropAmount;

	if ((dropN > 0 && !m_player->forgiveGarbage)
		// forgiveGarbage is something that applies only to human players
		|| (dropN > 0 && m_player->getPlayerType() == ONLINE))
	{
		// play animation
		if (dropN >= 6 && dropN < 24)
			m_player->characterAnimation.prepareAnimation("damage1");
		else if (dropN >= 24)
			m_player->characterAnimation.prepareAnimation("damage2");

		m_player->garbageDropped = min(dropN, 30);

		// reset nuisance drop pattern (doesnt affect legacy)
		m_player->resetNuisanceDropPattern();
		
		// drop nuisancepuyos
		for (int i = 0; i < m_player->garbageDropped; i++)
		{
			int x;
			if (m_player->currentgame->legacyNuisanceDrop) {
				x = nuisanceDropPattern(m_properties.gridX, m_player->garbageCycle);
			}
			else {
				x = m_player->nuisanceDropPattern();
			}
			int y = m_properties.gridY - 2;
			if (addNuisancePuyo(x, y, 1, i / m_properties.gridX))
			{
				// drop after creation
				int newy = dropSingle(x, y);
				// set falltarget
				fieldPuyoArray[x][newy]->setFallTarget(newy);
				// remember last one dropped
				lastx = x; lasty = newy;
			}

			// end iteration
			m_player->garbageCycle++;
		}
		//mark if last nuisance for dropping sound
		if (lastx != -1 && lasty != -1)
			fieldPuyoArray[lastx][lasty]->lastNuisance = true;


		// remove from GQ
		m_player->activeGarbage->GQ -= min(dropN, 30);

		// update tray
		m_player->updateTray(m_player->activeGarbage);

		dropped = true;
		// send message
		// 0[g]1[garbagedropped]
		if (m_player->currentgame->connected && m_player->getPlayerType() == HUMAN)
		{
			char str[100];
			sprintf(str, "g|%i", m_player->garbageDropped);
			m_player->currentgame->network->sendToChannel(CHANNEL_GAME, str, m_player->currentgame->channelName.c_str());
		}
	}
	if (m_player->forgiveGarbage)
	{
		m_player->forgiveGarbage = false;
	}

	// nothing dropped: send
	// 0["n"]
	if (dropped == false && m_player->currentgame->connected && m_player->getPlayerType() == HUMAN)
	{
		m_player->currentgame->network->sendToChannel(CHANNEL_GAME, "n", m_player->currentgame->channelName.c_str());
	}
	
	// your garbage drop must be confirmed
	if (m_player->currentgame->connected && m_player->getPlayerType() == HUMAN)
	{
		for (size_t i = 0; i < m_player->currentgame->players.size(); i++)
		{
			if (m_player->currentgame->players[i] != m_player && m_player->currentgame->players[i]->active)
				m_player->currentgame->players[i]->waitForConfirm++;
		}
	}


	// end
	m_sweepFall = 0;
	if (automatic)
		m_player->endPhase();
}

void field::loseDrop() const
{
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			if (isPuyo(i, j))
			{
				unsetLinkAll(i, j);
				if (fieldPuyoArray[i][j]->GetAccelY() < 0.1)
					fieldPuyoArray[i][j]->SetAccelY(static_cast<float>(static_cast<int>(sqrt(i + 2.) * 12) % 10) / 2.0f);
				fieldPuyoArray[i][j]->AddAccelY(0.2f);
			}
		}
	}
}

//drop a field of puyos
void field::dropField(const std::string& fieldstring)
{
	int j = 0; // j backwards counter
	for (size_t i = 0; i < fieldstring.size(); i++)
	{
		j = static_cast<int>(fieldstring.size() - 1 - i);
		int x = m_properties.gridX - 1 - (i % m_properties.gridX);
		int y = m_properties.gridY - 2;
		std::string str = fieldstring.substr(j, 1);
		if (to_int(str) > 0 && to_int(str) < 6)
		{
			// color puyo
			if (addColorPuyo(x, y, to_int(str) - 1, 1, static_cast<int>(i / m_properties.gridX), static_cast<int>(i / m_properties.gridX)))
			{
				// drop after creation
				int newy = dropSingle(x, y);
				// set falltarget
				fieldPuyoArray[x][newy]->setFallTarget(newy);
			}
		}
		else if (to_int(str) == 6)
		{
			if (addNuisancePuyo(x, y, 1, static_cast<int>(i / m_properties.gridX)))
			{
				// drop after creation
				int newy = dropSingle(x, y);
				// set falltarget
				fieldPuyoArray[x][newy]->setFallTarget(newy);
			}
		}
	}
	m_sweepFall = 0;
}

void field::setFieldFromString(const std::string& fieldstring)
{
	// clear field
	clearField();

	/*
	loop through puyos horizontally from x=0
	puyos:
	0=empty
	1-5= colorpuyo
	6=nuisancepuyo
	*/
	for (size_t i = 0; i < fieldstring.size(); i++)
	{
		int x = static_cast<int>(i % m_properties.gridX);
		int y = static_cast<int>(i / m_properties.gridX);
		std::string str = fieldstring.substr(i, 1);
		if (to_int(str) > 0 && to_int(str) < 6)
		{
			// color puyo
			if (addColorPuyo(x, y, to_int(str) - 1, 1))
			{
				// drop after creation
				int newy = dropSingle(x, y);
				// set falltarget
				fieldPuyoArray[x][newy]->setFallTarget(newy);
			}
		}
		else if (to_int(str) == 6)
		{
			if (addNuisancePuyo(x, y, 1))
			{
				// drop after creation
				int newy = dropSingle(x, y);
				// set falltarget
				fieldPuyoArray[x][newy]->setFallTarget(newy);
			}
		}
	}
}

// loop through field and create a string
std::string field::getFieldString() const
{
	std::string out;
	for (int j = 0; j < m_properties.gridY; j++)
	{
		for (int i = 0; i < m_properties.gridX; i++)
		{
			if (isPuyo(i, j))
			{
				if (fieldPuyoArray[i][j]->getType() == COLORPUYO)
					out += to_string(fieldPuyoArray[i][j]->getColor() + 1);
				else if (fieldPuyoArray[i][j]->getType() == NUISANCEPUYO)
					out += "6";
			}
			else
			{
				out += "0";
			}
		}
	}
	// trim zeroes
	int z = 0;
	for (size_t i = 0; i < out.length(); i++)
	{
		if (out[out.length() - 1 - i] != '0')
		{
			z = static_cast<int>(i);
			break;
		}
	}
	return out.substr(0, out.length() - z);
}

// destroys entire field and shows an animation of "throwing away" puyo
void field::throwAwayField()
{
	for (int i = 0; i < m_properties.gridX; i++)
	{
		for (int j = 0; j < m_properties.gridY; j++)
		{
			if (isPuyo(i, j))
			{
				// create new throwpuyo
				createParticleThrow(fieldPuyoArray[i][j]);
				// delete puyo
				delete fieldPuyoArray[i][j];
				fieldPuyoArray[i][j] = nullptr;
			}
		}
	}
}

}
