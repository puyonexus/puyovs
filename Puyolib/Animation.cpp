#include "Animation.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace ppvs {

Animation::Animation()
{
	m_t = 1000;
	m_duration = 120;
	m_error = true;
}

Animation::~Animation()
{
	clearSprites();
	clearSounds();
}

void Animation::init(GameData* g, PosVectorFloat offsetPos, float scale, std::string folder, const std::string& scriptName, int maxTime)
{
	m_gameData = g;
	clearSprites();
	m_paths.clear();
	m_sourceFolder = std::move(folder);
	m_offset = offsetPos;
	m_globalScale = scale;
	m_duration = maxTime;
	m_error = false;

	if (bool loadOk = m_doc.LoadFile(m_sourceFolder + scriptName); !loadOk) {
		m_error = true;
		std::cout << m_sourceFolder + scriptName + " could not be loaded\n"
				  << std::endl;
		return;
	}

	// Initialize sprites etc.
	TiXmlHandle docHandle(&m_doc);

	// Find init
	if (TiXmlElement* init = docHandle.FirstChild("animations").FirstChild("init").ToElement()) {
		// Traverse through init's inner elements
		TiXmlNode* initChild = nullptr;
		while ((initChild = init->IterateChildren(initChild)) != nullptr) {
			if (initChild->ValueStr() == "sprite") {
				// Get sprite name
				std::string spriteName;
				if (initChild->ToElement()->Attribute("name")) {
					spriteName = initChild->ToElement()->Attribute("name");
				} else {
					// Cannot find sprite name
					continue; // Must have a name!
				}

				// Check if before something else
				std::string before;
				if (initChild->ToElement()->Attribute("before")) {
					before = initChild->ToElement()->Attribute("before");
				}

				// Traverse through sprite's elements
				TiXmlNode* spriteChild = nullptr;
				while ((spriteChild = initChild->IterateChildren(spriteChild)) != nullptr) {
					// Get image name
					if (spriteChild->ValueStr() == "image") {
						std::string imageName = spriteChild->ToElement()->GetText();
						// Now we can add the sprite
						newSprite(spriteName, imageName, before);
					}

					// Get sub-rectangle
					if (spriteChild->ValueStr() == "rect") {
						// Find x, y, width and height attributes.
						int x, y, width, height;
						if (spriteChild->ToElement()->Attribute("x", &x) && spriteChild->ToElement()->Attribute("y", &y) && spriteChild->ToElement()->Attribute("width", &width) && spriteChild->ToElement()->Attribute("height", &height)) {
							setRect(
								spriteName,
								static_cast<float>(x),
								static_cast<float>(y),
								static_cast<float>(width),
								static_cast<float>(height));
						}
					}

					// Get center
					if (spriteChild->ValueStr() == "center") {
						// Find x, y, width and height attributes.
						int x, y;
						if (spriteChild->ToElement()->Attribute("x", &x) && spriteChild->ToElement()->Attribute("y", &y)) {
							setCenter(spriteName, static_cast<float>(x), static_cast<float>(y));
						}
					}

					// Bound sprite
					if (spriteChild->ValueStr() == "bind") {
						// Find x, y, width and height attributes.
						int x, y;
						if (spriteChild->ToElement()->Attribute("parent") && spriteChild->ToElement()->Attribute("x", &x) && spriteChild->ToElement()->Attribute("y", &y)) {
							std::string parent = spriteChild->ToElement()->Attribute("parent");

							// Set parent and add self to parent as child
							m_sprites[spriteName].parent = parent;
							addChild(
								parent,
								spriteName,
								static_cast<float>(x),
								static_cast<float>(y));
						}
					}

					if (spriteChild->ValueStr() == "blend") {
						if (spriteChild->ToElement()->Attribute("mode")) {
							std::string mode = spriteChild->ToElement()->Attribute("mode");
							setBlendMode(spriteName, mode);
						}
					}
				}
			} else if (initChild->ValueStr() == "path") {
				std::string pathname;

				// Get path name
				if (initChild->ToElement()->Attribute("name")) {
					pathname = initChild->ToElement()->Attribute("name");
				} else {
					// Cannot find name
					continue; // Must have a name!
				}

				// Traverse through path's elements
				TiXmlNode* pathChild = nullptr;
				while ((pathChild = initChild->IterateChildren(pathChild)) != nullptr) {
					// Add nodes
					if (pathChild->ValueStr() == "node") {
						int x, y;
						if (pathChild->ToElement()->Attribute("x", &x) && pathChild->ToElement()->Attribute("y", &y)) {
							// Add node
							PosVectorFloat pv(static_cast<float>(x), static_cast<float>(y));
							m_paths[pathname].push_back(pv);
						}
					}
				}
			} else if (initChild->ValueStr() == "sound") {
				std::string spriteName;
				// Get sprite name
				if (initChild->ToElement()->Attribute("name")) {
					spriteName = initChild->ToElement()->Attribute("name");
				} else {
					// Cannot find sprite name
					continue; // Must have a name!
				}

				// Traverse through sprite's elements
				TiXmlNode* spriteChild = nullptr;
				while ((spriteChild = initChild->IterateChildren(spriteChild)) != nullptr) {
					// Get file name
					if (spriteChild->ValueStr() == "file") {
						std::string imageName = spriteChild->ToElement()->GetText();
						// Now we can add the sprite
						newSound(spriteName, imageName);
					}
				}
			}
		}
	} else {
		m_error = true;
	}
}

void Animation::playAnimation()
{
	if (m_error) {
		return;
	}

	// The timer t runs from 0 to 120 (2 seconds)
	if (m_t <= m_duration) {
		// animationName must be set beforehand
		if (m_animationName.empty()) {
			return;
		}

		TiXmlHandle docHandle(&m_doc);
		// Find animation element
		if (TiXmlElement* anim = docHandle.FirstChild("animations").FirstChild(m_animationName.c_str()).ToElement()) {
			// Traverse through keyframes
			TiXmlNode* animChild = nullptr;
			while ((animChild = anim->IterateChildren(animChild)) != nullptr) {
				if (animChild->ValueStr() == "event") {
					// Get start time
					int startTime;
					if (animChild->ToElement()->Attribute("start", &startTime)) {
						// Check if keyframe is started
						if (startTime > m_t)
							continue;
					} else {
						// Erroneous keyframe element: start time must be defined
						continue;
					}

					// Traverse through keyframes' elements
					TiXmlNode* keyFrameChild = nullptr;
					while ((keyFrameChild = animChild->IterateChildren(keyFrameChild)) != nullptr) {
						// Play sound
						if (startTime == m_t) {
							if (keyFrameChild->ValueStr() == "playSound") {
								if (keyFrameChild->ToElement()->Attribute("name")) {
									std::string soundName = keyFrameChild->ToElement()->Attribute("name");
									playSound(soundName);
								}
							}
						}

						// Set loop point
						if (keyFrameChild->ValueStr() == "loop") {
							int time = m_t;
							if (keyFrameChild->ToElement()->Attribute("t", &time))
								m_t = time;
						}
						// Get sprite name
						std::string spriteName;
						if (keyFrameChild->ToElement()->Attribute("name")) {
							spriteName = keyFrameChild->ToElement()->Attribute("name");
						} else {
							// All animation functions must point to a sprite name!
							continue;
						}

						// Instantly change sprite property
						// Set sub-rectangle
						if (keyFrameChild->ValueStr() == "rect") {
							// Find x, y, width and height attributes.
							int x, y, width, height;
							if (keyFrameChild->ToElement()->Attribute("x", &x) && keyFrameChild->ToElement()->Attribute("y", &y) && keyFrameChild->ToElement()->Attribute("width", &width) && keyFrameChild->ToElement()->Attribute("height", &height)) {
								setRect(
									spriteName,
									static_cast<float>(x),
									static_cast<float>(y),
									static_cast<float>(width),
									static_cast<float>(height));
							}
						}

						// Set center
						if (keyFrameChild->ValueStr() == "center") {
							// Find x and y attributes.
							int x, y;
							if (keyFrameChild->ToElement()->Attribute("x", &x) && keyFrameChild->ToElement()->Attribute("y", &y))
								setCenter(
									spriteName,
									static_cast<float>(x),
									static_cast<float>(y));
						}

						// Set position
						if (keyFrameChild->ValueStr() == "position") {
							int x, y;
							if (keyFrameChild->ToElement()->Attribute("x", &x) && keyFrameChild->ToElement()->Attribute("y", &y)) {
								setPosition(
									spriteName,
									static_cast<float>(x),
									static_cast<float>(y));
							}
						}
						if (keyFrameChild->ValueStr() == "blend") {
							if (keyFrameChild->ToElement()->Attribute("mode")) {
								std::string mode = keyFrameChild->ToElement()->Attribute("mode");
								setBlendMode(spriteName, mode);
							}
						}

						// Is the animation ready to be played?
						int keyFrameDuration;
						if (keyFrameChild->ToElement()->Attribute("duration", &keyFrameDuration)) {
							if (m_t > startTime + keyFrameDuration) {
								// The duration has passed
								continue;
							}
						} else {
							// Duration of a function must be defined!
							continue;
						}

						// Get type
						std::string type;
						if (keyFrameChild->ToElement()->Attribute("type")) {
							type = keyFrameChild->ToElement()->Attribute("type");
						} else {
							// Default type
							type = "none";
						}

						// Animation
						// Transform scale
						if (keyFrameChild->ValueStr() == "scaleX") {
							double startVal, endVal, alpha, beta;
							if (!keyFrameChild->ToElement()->Attribute("alpha", &alpha)) {
								alpha = 0;
							}
							if (!keyFrameChild->ToElement()->Attribute("beta", &beta)) {
								beta = 0;
							}
							if (keyFrameChild->ToElement()->Attribute("startVal", &startVal) && keyFrameChild->ToElement()->Attribute("endVal", &endVal)) {
								setScaleX(spriteName, static_cast<float>(getLocalTimer(type, startVal, endVal, (m_t - startTime) * 1.0 / keyFrameDuration, alpha, beta)));
							}
						}
						if (keyFrameChild->ValueStr() == "scaleY") {
							double startVal, endVal, alpha, beta;
							if (!keyFrameChild->ToElement()->Attribute("alpha", &alpha)) {
								alpha = 0;
							}
							if (!keyFrameChild->ToElement()->Attribute("beta", &beta)) {
								beta = 0;
							}
							if (keyFrameChild->ToElement()->Attribute("startVal", &startVal) && keyFrameChild->ToElement()->Attribute("endVal", &endVal)) {
								setScaleY(spriteName, static_cast<float>(getLocalTimer(type, startVal, endVal, (m_t - startTime) * 1.0 / keyFrameDuration, alpha, beta)));
							}
						}

						// Set transparency
						if (keyFrameChild->ValueStr() == "transparency") {
							double startVal, endVal, alpha, beta;
							if (!keyFrameChild->ToElement()->Attribute("alpha", &alpha)) {
								alpha = 0;
							}
							if (!keyFrameChild->ToElement()->Attribute("beta", &beta)) {
								beta = 0;
							}
							if (keyFrameChild->ToElement()->Attribute("startVal", &startVal) && keyFrameChild->ToElement()->Attribute("endVal", &endVal)) {
								setTransparency(spriteName, static_cast<float>(getLocalTimer(type, startVal, endVal, (m_t - startTime) * 1.0 / keyFrameDuration, alpha, beta)));
							}
						}

						if (keyFrameChild->ValueStr() == "rotation") {
							double startVal, endVal, alpha, beta;
							if (!keyFrameChild->ToElement()->Attribute("alpha", &alpha)) {
								alpha = 0;
							}
							if (!keyFrameChild->ToElement()->Attribute("beta", &beta)) {
								beta = 0;
							}
							if (keyFrameChild->ToElement()->Attribute("startVal", &startVal) && keyFrameChild->ToElement()->Attribute("endVal", &endVal)) {
								setAngle(spriteName, static_cast<float>(getLocalTimer(type, startVal, endVal, (m_t - startTime) * 1.0 / keyFrameDuration, alpha, beta)));
							}
						}

						if (keyFrameChild->ValueStr() == "move") {
							double startVal, endVal, alpha, beta;
							if (!keyFrameChild->ToElement()->Attribute("alpha", &alpha)) {
								alpha = 0;
							}
							if (!keyFrameChild->ToElement()->Attribute("beta", &beta)) {
								beta = 0;
							}
							if (keyFrameChild->ToElement()->Attribute("startVal", &startVal) && keyFrameChild->ToElement()->Attribute("endVal", &endVal) && keyFrameChild->ToElement()->Attribute("path")) {
								std::string pathName = keyFrameChild->ToElement()->Attribute("path");
								move(spriteName, pathName, static_cast<float>(getLocalTimer(type, startVal, endVal, (m_t - startTime) * 1.0 / keyFrameDuration, alpha, beta)));
							}
						}

						// Color
						if (keyFrameChild->ValueStr() == "color") {
							double startVal, endVal, alpha, beta;
							if (!keyFrameChild->ToElement()->Attribute("alpha", &alpha)) {
								alpha = 0;
							}
							if (!keyFrameChild->ToElement()->Attribute("beta", &beta)) {
								beta = 0;
							}
							if (keyFrameChild->ToElement()->Attribute("startVal", &startVal) && keyFrameChild->ToElement()->Attribute("endVal", &endVal) && keyFrameChild->ToElement()->Attribute("rgb")) {
								// Split into two
								std::string rgb = keyFrameChild->ToElement()->Attribute("rgb");
								setColor(spriteName, rgb, static_cast<float>(getLocalTimer(type, startVal, endVal, (m_t - startTime) * 1.0 / keyFrameDuration, alpha, beta)));
							}
						}
					}
				}
			}
			m_t++;
		}
	}

	if (m_t > m_duration && m_t < 900) {
		// End animation: hide all sprites
		for (auto& sprite : m_sprites) {
			sprite.second.transparency = 0;
		}
		m_t = 1000;
	}
}

// Draw all sprites
void Animation::draw()
{
	updateSprites();
	for (const auto& drawSprite : m_drawSprites) {
		drawSprite->sprite->draw(m_gameData->front);
	}
}

// Define new sprite
void Animation::newSprite(const std::string& name, const std::string& image, const std::string& before)
{
	m_sprites[name].sprite = new Sprite();
	m_sprites[name].sprite->setImage(m_gameData->front->loadImage(m_sourceFolder + image));
	m_gameData->front->loadImage(m_sourceFolder + image)->setFilter(FilterType::LinearFilter);
	m_sprites[name].angle = 0;
	m_sprites[name].transparency = 0;
	m_sprites[name].position.x = 0;
	m_sprites[name].position.y = 0;
	m_sprites[name].pathPos.x = 0;
	m_sprites[name].pathPos.y = 0;
	m_sprites[name].scale.x = 1;
	m_sprites[name].scale.y = 1;
	m_sprites[name].childOffset.x = 0;
	m_sprites[name].childOffset.y = 0;
	m_sprites[name].parent = "";

	if (before.empty()) {
		// Add sprite at the end
		m_drawSprites.push_back(&m_sprites[name]);
	} else {
		// Find other sprite and insert at that position
		m_drawSprites.insert(std::find(m_drawSprites.begin(), m_drawSprites.end(), &m_sprites[before]), &m_sprites[name]);
	}
}

void Animation::newSound(const std::string& name, const std::string& buffer)
{
	m_sounds[name] = new Sound;
	setBuffer(*m_sounds[name], m_gameData->front->loadSound(m_sourceFolder + buffer));
}

// Add child to parent
void Animation::addChild(const std::string& parent, const std::string& child, const float x, const float y)
{
	m_sprites[parent].children.push_back(child);
	m_sprites[child].transparency = 1; // Child's transparency is 1 by default
	m_sprites[child].childOffset.x = x;
	m_sprites[child].childOffset.y = y;
}

void Animation::setRect(const std::string& name, const float x, const float y, const float width, const float height)
{
	if (!spriteExists(name))
		return;
	m_sprites[name].sprite->setSubRect(
		static_cast<int>(x),
		static_cast<int>(y),
		static_cast<int>(width),
		static_cast<int>(height));
}

void Animation::setCenter(const std::string& name, const float x, const float y)
{
	if (!spriteExists(name))
		return;
	m_sprites[name].sprite->setCenter(
		static_cast<int>(x),
		static_cast<int>(y));
}

void Animation::setPosition(const std::string& name, const float x, const float y)
{
	if (!spriteExists(name))
		return;
	m_sprites[name].position.x = x;
	m_sprites[name].position.y = y;
}

void Animation::setBlendMode(const std::string& name, std::string blend)
{
	if (!spriteExists(name))
		return;
	blend = Lower(blend);
	if (blend == "none")
		m_sprites[name].sprite->setBlendMode(BlendingMode::NoBlending);
	else if (blend == "add")
		m_sprites[name].sprite->setBlendMode(BlendingMode::AdditiveBlending);
	else if (blend == "multiply")
		m_sprites[name].sprite->setBlendMode(BlendingMode::MultiplyBlending);
	else if (blend == "alpha")
		m_sprites[name].sprite->setBlendMode(BlendingMode::AlphaBlending);
}

void Animation::setScaleX(const std::string& name, const float x)
{
	if (!spriteExists(name))
		return;
	m_sprites[name].scale.x = x;
}

void Animation::setScaleY(const std::string& name, const float y)
{
	if (!spriteExists(name))
		return;
	m_sprites[name].scale.y = y;
}

void Animation::setAngle(const std::string& name, const float x)
{
	if (!spriteExists(name))
		return;
	m_sprites[name].angle = x;
}

void Animation::setTransparency(const std::string& name, const float x)
{
	if (!spriteExists(name))
		return;
	m_sprites[name].transparency = x;
}

void Animation::setVisible(const std::string& name, const bool x)
{
	if (!spriteExists(name))
		return;
	m_sprites[name].sprite->setVisible(x);
}

void Animation::setColor(const std::string& name, const std::string& color, float x)
{
	if (!spriteExists(name))
		return;

	// Get rgb values from string
	int r1, g1, b1, r2, g2, b2;
	sscanf(color.c_str(), "#%2x%2x%2x#%2x%2x%2x", &r1, &g1, &b1, &r2, &g2, &b2);

	// Set color
	m_sprites[name].sprite->setColor(
		static_cast<float>(r1) + x * static_cast<float>(r2 - r1),
		static_cast<float>(g1) + x * static_cast<float>(g2 - g1),
		static_cast<float>(b1) + x * static_cast<float>(b2 - b1));
}

// Set position according to path
void Animation::move(const std::string& name, const std::string& path, float x)
{
	// Check if path has more than 1 node
	if (!spriteExists(name)) {
		return;
	}

	if (m_paths[path].empty()) {
		return;
	}

	if (m_paths[path].size() == 1) {
		m_sprites[name].pathPos.x = m_paths[path][0].x;
		m_sprites[name].pathPos.y = m_paths[path][0].y;
		return;
	}

	// Get total distance of path
	const float totalDist = getTotalDistance(path);
	if (totalDist == 0) // NOLINT(clang-diagnostic-float-equal)
		return; // Danger of dividing by 0

	// x must be between 0 and 1
	if (x >= 0 && x < 1) {
		const PosVectorFloat pv = getPosition(path, x * totalDist);
		m_sprites[name].pathPos.x = pv.x;
		m_sprites[name].pathPos.y = pv.y;
	} else if (x < 0) {
		// Extrapolate from start
		const PosVectorFloat pv = getPositionExtra(path, x, totalDist, true);
		m_sprites[name].pathPos.x = pv.x;
		m_sprites[name].pathPos.y = pv.y;
	} else if (x >= 1) {
		// Extrapolate from end
		const PosVectorFloat pv = getPositionExtra(path, x - 1, totalDist, false);
		m_sprites[name].pathPos.x = pv.x;
		m_sprites[name].pathPos.y = pv.y;
	}
}

void Animation::playSound(const std::string& name)
{
	if (m_gameData && m_gameData->playSounds)
		m_sounds[name]->play(m_gameData);
}

bool Animation::spriteExists(const std::string& name)
{
	return m_sprites.find(name) == m_sprites.end() ? false : true;
}

void Animation::clearSprites()
{
	for (const auto& sprite : m_sprites) {
		delete sprite.second.sprite;
	}
	m_sprites.clear();
	m_drawSprites.clear();
}

void Animation::clearSounds()
{
	for (const auto& it : m_sounds) {
		delete it.second;
	}
	// Clear sounds map
	m_sounds.clear();
}

// Updates all properties of the sprite
void Animation::updateSprites()
{
	for (auto& [name, sprite] : m_sprites) {
		// Do not update if it's a child
		if (!sprite.parent.empty()) {
			continue;
		}

		sprite.sprite->setPosition(
			(sprite.position.x + sprite.pathPos.x) * m_globalScale + m_offset.x,
			(sprite.position.y + sprite.pathPos.y) * m_globalScale + m_offset.y);
		sprite.sprite->setScaleX(sprite.scale.x * m_globalScale);
		sprite.sprite->setScaleY(sprite.scale.y * m_globalScale);
		sprite.sprite->setRotation(sprite.angle);
		sprite.sprite->setTransparency(sprite.transparency);
		updateChildren(name);
	}
}

void Animation::updateChildren(const std::string& parent)
{
	AnimationSprite& parentSprite = m_sprites[parent];
	for (size_t i = 0; i < parentSprite.children.size(); i++) {
		std::string& child = parentSprite.children[i];
		const AnimationSprite& childSprite = m_sprites[child];

		// Scale
		const float psx = parentSprite.sprite->getScaleX();
		const float psy = parentSprite.sprite->getScaleY();
		const float csx = childSprite.scale.x;
		const float csy = childSprite.scale.y;
		childSprite.sprite->setScaleX(psx * csx);
		childSprite.sprite->setScaleY(psy * csy);

		// Rotation
		const float pr = parentSprite.sprite->getAngle();
		const float cr = childSprite.angle;
		childSprite.sprite->setRotation(pr + cr);

		// Transparency
		const float pt = parentSprite.sprite->getTransparency();
		const float ct = childSprite.transparency;
		childSprite.sprite->setTransparency(pt * ct);

		// Position
		const float px = (parentSprite.sprite->getX() - m_offset.x) / m_globalScale;
		const float py = (parentSprite.sprite->getY() - m_offset.y) / m_globalScale;
		const float cx = childSprite.position.x + childSprite.pathPos.x + childSprite.childOffset.x;
		const float cy = childSprite.position.y + childSprite.pathPos.y + childSprite.childOffset.y;
		const float r = sqrt(cx * cx * psx * psx + cy * cy * psy * psy);
		const float a = atan2(-cy * psy, cx * psx) * 180 / kPiF;
		childSprite.sprite->setPosition(
			m_offset.x + (px + r * cos((pr + a) * kPiF / 180)) * m_globalScale,
			m_offset.y + (py + r * sin((pr + a) * kPiF / -180)) * m_globalScale);

		// Update children
		updateChildren(child);
	}
}

void Animation::resetSprites()
{
	for (auto& sprite : m_sprites) {
		sprite.second.pathPos.x = 0;
		sprite.second.pathPos.y = 0;
		sprite.second.position.x = 0;
		sprite.second.position.y = 0;
		sprite.second.scale.x = 1;
		sprite.second.scale.y = 1;
		sprite.second.angle = 0;
		sprite.second.transparency = 0;
		if (!sprite.second.parent.empty()) {
			sprite.second.transparency = 1;
		}
	}
}

double Animation::getLocalTimer(const std::string& type, const double startVal, const double endVal, const double t, const double alpha, const double beta) const
{
	// Input t must go from 0 to 1
	double out = 0;
	if (type == "linear")
		out = (endVal - startVal) * t + startVal;
	else if (type == "quadratic")
		out = (endVal - startVal) * t * t + startVal;
	else if (type == "squareroot")
		out = (endVal - startVal) * pow(t, 0.5) + startVal;
	else if (type == "cubic")
		out = (endVal - startVal) * t * t * t + startVal;
	else if (type == "cuberoot")
		out = (endVal - startVal) * pow(t, 1.0 / 3.0) + startVal;
	else if (type == "exponential")
		out = (endVal - startVal) / (exp(alpha) - 1) * exp(alpha * t) + startVal - (endVal - startVal) / (exp(alpha) - 1);
	else if (type == "elastic") // beta=wave number
		out = (endVal - startVal) / (exp(alpha) - 1) * cos(beta * t * 2 * kPiD) * exp(alpha * t) + startVal - (endVal - startVal) / (exp(alpha) - 1);
	else if (type == "sin")
		out = startVal + endVal * sin(alpha * t * 2 * kPiD); // s=offset, e=amplitude, alpha=wave number
	else if (type == "cos")
		out = startVal + endVal * cos(alpha * t * 2 * kPiD); // s=offset, e=amplitude, alpha=wave number
	return out;
}

float Animation::getTotalDistance(const std::string& path)
{
	if (m_paths[path].size() <= 1) {
		return 0;
	}
	float distance = 0;

	for (auto it = m_paths[path].begin() + 1; it < m_paths[path].end(); ++it) {
		const PosVectorFloat diff = *(it - 1) - *it;
		distance += sqrt(diff.x * diff.x + diff.y * diff.y);
	}
	return distance;
}

PosVectorFloat Animation::getPosition(const std::string& path, const float target)
{
	float distance = 0;

	for (auto it = m_paths[path].begin() + 1; it < m_paths[path].end(); ++it) {
		const PosVectorFloat diff = *(it - 1) - *it;
		const float nodeDistance = sqrt(diff.x * diff.x + diff.y * diff.y);
		distance += nodeDistance;

		// Distance reached: interpolate
		if (distance > target) {
			const float overshoot = distance - target;
			const float ratio = overshoot / nodeDistance;
			PosVectorFloat out;
			out.x = (*it).x - ratio * ((*it).x - (*(it - 1)).x);
			out.y = (*it).y - ratio * ((*it).y - (*(it - 1)).y);
			return out;
		}
	}

	return {};
}

PosVectorFloat Animation::getPositionExtra(const std::string& path, const float target, const float totalDist, const bool start)
{
	PosVectorFloat diff;
	std::vector<PosVectorFloat>::iterator it;
	int x = 0;
	if (start) {
		// Get first nodes
		it = m_paths[path].begin() + 1;
		diff = *(it - 1) - *it;
		x = -1;
	} else {
		// Get end nodes
		it = m_paths[path].end() - 1;
		diff = *(it - 1) - *it;
	}

	// Get ratio of difference
	const float nodeDistance = sqrt(diff.x * diff.x + diff.y * diff.y);
	const float ratio = nodeDistance / totalDist;

	// Get direction
	PosVectorFloat out;
	out.x = (*(it + x)).x - target / ratio * diff.x;
	out.y = (*(it + x)).y - target / ratio * diff.y;
	return out;
}

}
