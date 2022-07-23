#include "animation.h"

#include <utility>

namespace ppvs
{

animation::animation()
{
	t = 1000;
	duration = 120;
	error = true;
}

animation::~animation()
{
	clearSprites();
	clearSounds();
}

void animation::init(gameData* g, posVectorFloat offsetPos, float scale, std::string folder, const std::string& scriptName, int maxTime)
{
	gamedata = g;
	clearSprites();
	paths.clear();
	sourceFolder = std::move(folder);
	offset = offsetPos;
	globalScale = scale;
	duration = maxTime;
	error = false;

	bool loadOK = doc.LoadFile(sourceFolder + scriptName);
	if (!loadOK)
	{
		error = true;
		std::cout << sourceFolder + scriptName + " could not be loaded\n" << std::endl;
		return;
	}

	// Initialize sprites etc.
	TiXmlHandle docHandle(&doc);

	// Find init
	TiXmlElement* init = docHandle.FirstChild("animations").FirstChild("init").ToElement();
	if (init)
	{
		// Traverse through init's inner elements
		TiXmlNode* init_child = nullptr;
		while ((init_child = init->IterateChildren(init_child)) != nullptr)
		{
			if (init_child->ValueStr() == "sprite")
			{
				// Get sprite name
				std::string spritename;
				if (init_child->ToElement()->Attribute("name"))
				{
					spritename = init_child->ToElement()->Attribute("name");
				}
				else
				{
					// Cannot find spritename
					continue; // Must have a name!
				}

				// Check if before something else
				std::string before = "";
				if (init_child->ToElement()->Attribute("before"))
				{
					before = init_child->ToElement()->Attribute("before");
				}

				// Traverse through sprite's elements
				TiXmlNode* sprite_child = nullptr;
				while ((sprite_child = init_child->IterateChildren(sprite_child)) != nullptr)
				{
					// Get image name
					std::string imagename;
					if (sprite_child->ValueStr() == "image")
					{
						imagename = sprite_child->ToElement()->GetText();
						// Now we can add the sprite
						newSprite(spritename, imagename, before);
					}

					// Get subrectangle
					if (sprite_child->ValueStr() == "rect")
					{
						// Find x, y, width and height attributes.
						int x, y, width, height;
						if (sprite_child->ToElement()->Attribute("x", &x) &&
							sprite_child->ToElement()->Attribute("y", &y) &&
							sprite_child->ToElement()->Attribute("width", &width) &&
							sprite_child->ToElement()->Attribute("height", &height)) {
							setRect(
								spritename,
								static_cast<float>(x),
								static_cast<float>(y),
								static_cast<float>(width),
								static_cast<float>(height)
							);
						}
					}

					// Get center
					if (sprite_child->ValueStr() == "center")
					{
						// Find x, y, width and height attributes.
						int x, y;
						if (sprite_child->ToElement()->Attribute("x", &x) &&
							sprite_child->ToElement()->Attribute("y", &y))
							setCenter(spritename, static_cast<float>(x), static_cast<float>(y));
					}

					// Bound sprite
					if (sprite_child->ValueStr() == "bind")
					{
						// Find x, y, width and height attributes.
						std::string parent;
						int x, y;
						if (sprite_child->ToElement()->Attribute("parent") &&
							sprite_child->ToElement()->Attribute("x", &x) &&
							sprite_child->ToElement()->Attribute("y", &y))
						{
							parent = sprite_child->ToElement()->Attribute("parent");

							// Set parent and add self to parent as child
							sprites[spritename].parent = parent;
							addChild(
								parent,
								spritename,
								static_cast<float>(x),
								static_cast<float>(y)
							);
						}
					}

					if (sprite_child->ValueStr() == "blend")
					{
						std::string mode;
						if (sprite_child->ToElement()->Attribute("mode"))
						{
							mode = sprite_child->ToElement()->Attribute("mode");
							setBlendmode(spritename, mode);
						}
					}

				}
			}
			else if (init_child->ValueStr() == "path")
			{
				std::string pathname;

				// Get path name
				if (init_child->ToElement()->Attribute("name"))
				{
					pathname = init_child->ToElement()->Attribute("name");
				}
				else
				{
					// Cannot find name
					continue; // Must have a name!
				}

				// Traverse through path's elements
				TiXmlNode* path_child = nullptr;
				while ((path_child = init_child->IterateChildren(path_child)) != nullptr)
				{
					// Add nodes
					if (path_child->ValueStr() == "node")
					{
						int x, y;
						if (path_child->ToElement()->Attribute("x", &x) &&
							path_child->ToElement()->Attribute("y", &y))
						{
							//add node
							posVectorFloat pv(static_cast<float>(x), static_cast<float>(y));
							paths[pathname].push_back(pv);
						}
					}
				}
			}
			else if (init_child->ValueStr() == "sound")
			{
				std::string spritename;
				// Get sprite name
				if (init_child->ToElement()->Attribute("name"))
				{
					spritename = init_child->ToElement()->Attribute("name");
				}
				else
				{
					// Cannot find sprite name
					continue; // Must have a name!
				}

				// Traverse through sprite's elements
				TiXmlNode* sprite_child = nullptr;
				while ((sprite_child = init_child->IterateChildren(sprite_child)) != nullptr)
				{
					// Get file name
					std::string imagename;
					if (sprite_child->ValueStr() == "file")
					{
						imagename = sprite_child->ToElement()->GetText();
						//now we can add the sprite
						newSound(spritename, imagename);
					}
				}
			}

		}
	}
	else
	{
		error = true;
	}
}

void animation::playAnimation()
{
	if (error)
		return;

	// The timer t runs from 0 to 120 (2 seconds)
	if (t <= duration)
	{
		// animationName must be set beforehand
		if (animationName == "")
			return;

		TiXmlHandle docHandle(&doc);
		// Find animation element
		TiXmlElement* anim = docHandle.FirstChild("animations").FirstChild(animationName.c_str()).ToElement();
		if (anim)
		{
			// Traverse through keyframes
			TiXmlNode* anim_child = nullptr;
			while ((anim_child = anim->IterateChildren(anim_child)) != nullptr)
			{
				if (anim_child->ValueStr() == "event")
				{
					// Get start time
					int starttime;
					if (anim_child->ToElement()->Attribute("start", &starttime))
					{
						// Check if keyframe is started
						if (starttime > t)
							continue;
					}
					else
					{
						// Erroneous keyframe element: start time must be defined
						continue;
					}

					// Traverse through keyframes' elements
					TiXmlNode* kf_child = nullptr;
					while ((kf_child = anim_child->IterateChildren(kf_child)) != nullptr)
					{
						// Play sound
						if (starttime == t)
							if (kf_child->ValueStr() == "playSound")
							{
								std::string soundName;
								if (kf_child->ToElement()->Attribute("name"))
								{
									soundName = kf_child->ToElement()->Attribute("name");
									playSound(soundName);
								}
							}

						// Set loop point
						if (kf_child->ValueStr() == "loop")
						{
							int time = t;
							if (kf_child->ToElement()->Attribute("t", &time))
								t = time;
						}
						// Get sprite name
						std::string spritename;
						if (kf_child->ToElement()->Attribute("name"))
						{
							spritename = kf_child->ToElement()->Attribute("name");
						}
						else
						{
							// All animation functions must point to a sprite name!
							continue;
						}

						// Instantly change sprite property
						// Set subrectangle
						if (kf_child->ValueStr() == "rect")
						{
							//find x, y, width and height attributes.
							int x, y, width, height;
							if (kf_child->ToElement()->Attribute("x", &x) &&
								kf_child->ToElement()->Attribute("y", &y) &&
								kf_child->ToElement()->Attribute("width", &width) &&
								kf_child->ToElement()->Attribute("height", &height)) {
								setRect(
									spritename,
									static_cast<float>(x),
									static_cast<float>(y),
									static_cast<float>(width),
									static_cast<float>(height)
								);
							}
						}

						// Set center
						if (kf_child->ValueStr() == "center")
						{
							// Find x and y attributes.
							int x, y;
							if (kf_child->ToElement()->Attribute("x", &x) &&
								kf_child->ToElement()->Attribute("y", &y))
								setCenter(
									spritename,
									static_cast<float>(x),
									static_cast<float>(y)
								);
						}

						// Set position
						if (kf_child->ValueStr() == "position")
						{
							int x, y;
							if (kf_child->ToElement()->Attribute("x", &x) &&
								kf_child->ToElement()->Attribute("y", &y)) {
								setPosition(
									spritename,
									static_cast<float>(x),
									static_cast<float>(y)
								);
							}
						}
						if (kf_child->ValueStr() == "blend")
						{
							if (kf_child->ToElement()->Attribute("mode"))
							{
								std::string mode = kf_child->ToElement()->Attribute("mode");
								setBlendmode(spritename, mode);
							}
						}

						// Is the animation ready to be played?
						int duration;
						if (kf_child->ToElement()->Attribute("duration", &duration))
						{
							if (t > starttime + duration) {
								// The duration has passed
								continue;
							}
						}
						else
						{
							// Duration of a function must be defined!
							continue;
						}

						// Get type
						std::string type;
						if (kf_child->ToElement()->Attribute("type"))
						{
							type = kf_child->ToElement()->Attribute("type");
						}
						else
						{
							// Default type
							type = "none";
						}

						// Animation
						// Transform scale
						if (kf_child->ValueStr() == "scaleX")
						{
							double startVal, endVal, alpha, beta;
							if (!kf_child->ToElement()->Attribute("alpha", &alpha))alpha = 0;
							if (!kf_child->ToElement()->Attribute("beta", &beta))beta = 0;
							if (kf_child->ToElement()->Attribute("startVal", &startVal) &&
								kf_child->ToElement()->Attribute("endVal", &endVal))
								setScaleX(spritename, static_cast<float>(getLocalTimer(type, startVal, endVal, (t - starttime) * 1.0 / duration, alpha, beta)));
						}
						if (kf_child->ValueStr() == "scaleY")
						{
							double startVal, endVal, alpha, beta;
							if (!kf_child->ToElement()->Attribute("alpha", &alpha))alpha = 0;
							if (!kf_child->ToElement()->Attribute("beta", &beta))beta = 0;
							if (kf_child->ToElement()->Attribute("startVal", &startVal) &&
								kf_child->ToElement()->Attribute("endVal", &endVal))
								setScaleY(spritename, static_cast<float>(getLocalTimer(type, startVal, endVal, (t - starttime) * 1.0 / duration, alpha, beta)));
						}

						// Set transparency
						if (kf_child->ValueStr() == "transparency")
						{
							double startVal, endVal, alpha, beta;
							if (!kf_child->ToElement()->Attribute("alpha", &alpha))alpha = 0;
							if (!kf_child->ToElement()->Attribute("beta", &beta))beta = 0;
							if (kf_child->ToElement()->Attribute("startVal", &startVal) &&
								kf_child->ToElement()->Attribute("endVal", &endVal))
								setTransparency(spritename, static_cast<float>(getLocalTimer(type, startVal, endVal, (t - starttime) * 1.0 / duration, alpha, beta)));
						}

						if (kf_child->ValueStr() == "rotation")
						{
							double startVal, endVal, alpha, beta;
							if (!kf_child->ToElement()->Attribute("alpha", &alpha))alpha = 0;
							if (!kf_child->ToElement()->Attribute("beta", &beta))beta = 0;
							if (kf_child->ToElement()->Attribute("startVal", &startVal) &&
								kf_child->ToElement()->Attribute("endVal", &endVal))
								setAngle(spritename, static_cast<float>(getLocalTimer(type, startVal, endVal, (t - starttime) * 1.0 / duration, alpha, beta)));
						}

						if (kf_child->ValueStr() == "move")
						{
							std::string pathname;
							double startVal, endVal, alpha, beta;
							if (!kf_child->ToElement()->Attribute("alpha", &alpha))alpha = 0;
							if (!kf_child->ToElement()->Attribute("beta", &beta))beta = 0;
							if (kf_child->ToElement()->Attribute("startVal", &startVal) &&
								kf_child->ToElement()->Attribute("endVal", &endVal) &&
								kf_child->ToElement()->Attribute("path"))
							{
								pathname = kf_child->ToElement()->Attribute("path");
								move(spritename, pathname, static_cast<float>(getLocalTimer(type, startVal, endVal, (t - starttime) * 1.0 / duration, alpha, beta)));
							}
						}

						// Color
						if (kf_child->ValueStr() == "color")
						{
							double startVal, endVal, alpha, beta;
							if (!kf_child->ToElement()->Attribute("alpha", &alpha))alpha = 0;
							if (!kf_child->ToElement()->Attribute("beta", &beta))beta = 0;
							if (kf_child->ToElement()->Attribute("startVal", &startVal) &&
								kf_child->ToElement()->Attribute("endVal", &endVal) &&
								kf_child->ToElement()->Attribute("rgb"))
							{
								// Split into two
								std::string rgb = kf_child->ToElement()->Attribute("rgb");
								setColor(spritename, rgb, static_cast<float>(getLocalTimer(type, startVal, endVal, (t - starttime) * 1.0 / duration, alpha, beta)));
							}
						}
					}
				}
			}
			t++;
		}
	}

	if (t > duration && t < 900)
	{
		// End animation: hide all sprites
		for (auto& sprite : sprites)
		{
			sprite.second.transparency = 0;
		}
		t = 1000;
	}
}

// Draw all sprites
void animation::draw()
{
	updateSprites();
	for (const auto& drawSprite : drawSprites)
	{
		drawSprite->spr->draw(gamedata->front);
	}
}

// Define new sprite
void animation::newSprite(std::string& name, std::string& image, std::string& before)
{
	sprites[name].spr = new sprite();
	sprites[name].spr->setImage(gamedata->front->loadImage(sourceFolder + image));
	gamedata->front->loadImage(sourceFolder + image)->setFilter(linearFilter);
	sprites[name].angle = 0;
	sprites[name].transparency = 0;
	sprites[name].position.x = 0;
	sprites[name].position.y = 0;
	sprites[name].pathPos.x = 0;
	sprites[name].pathPos.y = 0;
	sprites[name].scale.x = 1;
	sprites[name].scale.y = 1;
	sprites[name].childOffset.x = 0;
	sprites[name].childOffset.y = 0;
	sprites[name].parent = "";

	if (before == "")
	{
		// Add sprite at the end
		drawSprites.push_back(&sprites[name]);
	}
	else
	{
		// Find other sprite and insert at that position
		drawSprites.insert(std::find(drawSprites.begin(), drawSprites.end(), &sprites[before]), &sprites[name]);
	}

}

void animation::newSound(std::string& name, std::string& buffer)
{
	sounds[name] = new sound;
	setBuffer(*sounds[name], gamedata->front->loadSound(sourceFolder + buffer));
}

// Add child to parent
void animation::addChild(std::string& parent, std::string& child, float x, float y)
{
	sprites[parent].children.push_back(child);
	sprites[child].transparency = 1; // Child's transparency is 1 by default
	sprites[child].childOffset.x = x;
	sprites[child].childOffset.y = y;
}

void animation::setRect(std::string& name, float x, float y, float width, float height)
{
	if (!spriteExists(name)) return;
	sprites[name].spr->setSubRect(
		static_cast<int>(x),
		static_cast<int>(y),
		static_cast<int>(width),
		static_cast<int>(height)
	);
}

void animation::setCenter(std::string& name, float x, float y)
{
	if (!spriteExists(name)) return;
	sprites[name].spr->setCenter(
		static_cast<int>(x),
		static_cast<int>(y)
	);
}

void animation::setPosition(std::string& name, float x, float y)
{
	if (!spriteExists(name)) return;
	sprites[name].position.x = x;
	sprites[name].position.y = y;
}

void animation::setBlendmode(std::string& name, std::string& blend)
{
	if (!spriteExists(name)) return;
	blend = Lower(blend);
	if (blend == "none")
		sprites[name].spr->setBlendMode(noBlending);
	else if (blend == "add")
		sprites[name].spr->setBlendMode(additiveBlending);
	else if (blend == "multiply")
		sprites[name].spr->setBlendMode(multiplyBlending);
	else if (blend == "alpha")
		sprites[name].spr->setBlendMode(alphaBlending);
}

void animation::setScaleX(std::string& name, float x)
{
	if (!spriteExists(name)) return;
	sprites[name].scale.x = x;
}

void animation::setScaleY(std::string& name, float y)
{
	if (!spriteExists(name)) return;
	sprites[name].scale.y = y;
}

void animation::setAngle(std::string& name, float x)
{
	if (!spriteExists(name)) return;
	sprites[name].angle = x;
}

void animation::setTransparency(std::string& name, float x)
{
	if (!spriteExists(name)) return;
	sprites[name].transparency = x;
}

void animation::setVisible(std::string& name, bool x)
{
	if (!spriteExists(name)) return;
	sprites[name].spr->setVisible(x);
}

void animation::setColor(std::string& name, std::string& color, float x)
{
	if (!spriteExists(name)) return;

	// Get rgb values from string
	int r1, g1, b1, r2, g2, b2;
	sscanf(color.c_str(), "#%2x%2x%2x#%2x%2x%2x", &r1, &g1, &b1, &r2, &g2, &b2);

	// Set color
	sprites[name].spr->setColor(
		static_cast<float>(r1) + x * static_cast<float>(r2 - r1),
		static_cast<float>(g1) + x * static_cast<float>(g2 - g1),
		static_cast<float>(b1) + x * static_cast<float>(b2 - b1)
	);
}

// Set position according to path
void animation::move(std::string& name, std::string& path, float x)
{
	// Check if path has more than 1 node
	if (!spriteExists(name)) return;

	if (paths[path].empty())
		return;

	if (paths[path].size() == 1)
	{
		sprites[name].pathPos.x = paths[path][0].x;
		sprites[name].pathPos.y = paths[path][0].y;
		return;
	}

	// Get total distance of path
	const float totalDist = getTotalDistance(path);
	if (totalDist == 0) return; // Danger of dividing by 0

	// x must be between 0 and 1
	if (x >= 0 && x < 1)
	{
		const posVectorFloat pv = getPosition(path, x * totalDist);
		sprites[name].pathPos.x = pv.x;
		sprites[name].pathPos.y = pv.y;
	}
	else if (x < 0)
	{
		// Extrapolate from start
		const posVectorFloat pv = getPositionExtra(path, x, totalDist, true);
		sprites[name].pathPos.x = pv.x;
		sprites[name].pathPos.y = pv.y;
	}
	else if (x >= 1)
	{
		// Extrapolate from end
		const posVectorFloat pv = getPositionExtra(path, x - 1, totalDist, false);
		sprites[name].pathPos.x = pv.x;
		sprites[name].pathPos.y = pv.y;
	}
}

void animation::playSound(std::string& name)
{
	if (gamedata && gamedata->playSounds)
		sounds[name]->Play(gamedata);
}

bool animation::spriteExists(const std::string& name)
{
	return sprites.find(name) == sprites.end() ? false : true;
}

void animation::clearSprites()
{
	for (const auto& sprite : sprites)
	{
		delete sprite.second.spr;
	}
	sprites.clear();
	drawSprites.clear();
}

void animation::clearSounds()
{
	for (const auto& it : sounds)
	{
		delete it.second;
	}
	// Clear sounds map
	sounds.clear();
}

// Updates all properties of the sprite
void animation::updateSprites()
{
	for (auto& [name, sprite] : sprites)
	{
		// Do not update if it's a child
		if (!sprite.parent.empty())
		{
			continue;
		}

		sprite.spr->setPosition(
			(sprite.position.x + sprite.pathPos.x) * globalScale + offset.x,
			(sprite.position.y + sprite.pathPos.y) * globalScale + offset.y
		);
		sprite.spr->setScaleX(sprite.scale.x * globalScale);
		sprite.spr->setScaleY(sprite.scale.y * globalScale);
		sprite.spr->setRotation(sprite.angle);
		sprite.spr->setTransparency(sprite.transparency);
		updateChildren(name);
	}
}

void animation::updateChildren(const std::string& parent)
{
	animationSprite& parentSprite = sprites[parent];
	for (size_t i = 0; i < parentSprite.children.size(); i++)
	{
		std::string& child = parentSprite.children[i];
		const animationSprite& childSprite = sprites[child];

		// Scale
		const float psx = parentSprite.spr->getScaleX();
		const float psy = parentSprite.spr->getScaleY();
		const float csx = childSprite.scale.x;
		const float csy = childSprite.scale.y;
		childSprite.spr->setScaleX(psx * csx);
		childSprite.spr->setScaleY(psy * csy);

		// Rotation
		const float pr = parentSprite.spr->getAngle();
		const float cr = childSprite.angle;
		childSprite.spr->setRotation(pr + cr);

		// Transparency
		const float pt = parentSprite.spr->getTransparency();
		const float ct = childSprite.transparency;
		childSprite.spr->setTransparency(pt * ct);

		// Position
		const float px = (parentSprite.spr->getX() - offset.x) / globalScale;
		const float py = (parentSprite.spr->getY() - offset.y) / globalScale;
		const float cx = childSprite.position.x + childSprite.pathPos.x + childSprite.childOffset.x;
		const float cy = childSprite.position.y + childSprite.pathPos.y + childSprite.childOffset.y;
		const float r = sqrt(cx * cx * psx * psx + cy * cy * psy * psy);
		const float a = atan2(-cy * psy, cx * psx) * 180 / PI;
		childSprite.spr->setPosition(
			offset.x + (px + r * cos((pr + a) * PI / 180)) * globalScale,
			offset.y + (py + r * sin((pr + a) * PI / -180)) * globalScale
		);

		// Update children
		updateChildren(child);
	}
}

void animation::resetSprites()
{
	for (auto& sprite : sprites)
	{
		sprite.second.pathPos.x = 0;
		sprite.second.pathPos.y = 0;
		sprite.second.position.x = 0;
		sprite.second.position.y = 0;
		sprite.second.scale.x = 1;
		sprite.second.scale.y = 1;
		sprite.second.angle = 0;
		sprite.second.transparency = 0;
		if (!sprite.second.parent.empty())
			sprite.second.transparency = 1;
	}
}

double animation::getLocalTimer(const std::string& type, double s, double e, double t, double alpha, double beta)
{
	// Input t must go from 0 to 1
	double out = 0;
	if (type == "linear")
		out = (e - s) * t + s;
	else if (type == "quadratic")
		out = (e - s) * t * t + s;
	else if (type == "squareroot")
		out = (e - s) * pow(t, 0.5) + s;
	else if (type == "cubic")
		out = (e - s) * t * t * t + s;
	else if (type == "cuberoot")
		out = (e - s) * pow(t, 1.0 / 3.0) + s;
	else if (type == "exponential")
		out = (e - s) / (exp(alpha) - 1) * exp(alpha * t) + s - (e - s) / (exp(alpha) - 1);
	else if (type == "elastic") // beta=wavenumber
		out = (e - s) / (exp(alpha) - 1) * cos(beta * t * 2 * PI) * exp(alpha * t) + s - (e - s) / (exp(alpha) - 1);
	else if (type == "sin")
		out = s + e * sin(alpha * t * 2 * PI); // s=offset, e=amplitude, alpha=wavenumber
	else if (type == "cos")
		out = s + e * cos(alpha * t * 2 * PI); // s=offset, e=amplitude, alpha=wavenumber
	return out;
}

float animation::getTotalDistance(std::string& path)
{
	if (paths[path].size() <= 1) return 0;
	float distance = 0;

	posVectorFloat previous(0, 0);
	for (auto it = paths[path].begin() + 1; it < paths[path].end(); ++it)
	{
		const posVectorFloat diff = *(it - 1) - *it;
		distance += sqrt(diff.x * diff.x + diff.y * diff.y);
	}
	return distance;
}

posVectorFloat animation::getPosition(std::string& path, float target)
{
	float distance = 0;

	posVectorFloat previous(0, 0);
	for (auto it = paths[path].begin() + 1; it < paths[path].end(); ++it)
	{
		const posVectorFloat diff = *(it - 1) - *it;
		const float nodeDistance = sqrt(diff.x * diff.x + diff.y * diff.y);
		distance += nodeDistance;

		// Distance reached: interpolate
		if (distance > target)
		{
			const float overshoot = distance - target;
			const float ratio = overshoot / nodeDistance;
			posVectorFloat out(0, 0);
			out.x = (*it).x - ratio * ((*it).x - (*(it - 1)).x);
			out.y = (*it).y - ratio * ((*it).y - (*(it - 1)).y);
			return out;
		}
	}

	return {};
}

posVectorFloat animation::getPositionExtra(std::string& path, float target, float totalDist, bool start)
{
	posVectorFloat diff;
	std::vector<posVectorFloat>::iterator it;
	int x = 0;
	if (start)
	{
		// Get first nodes
		it = paths[path].begin() + 1;
		diff = *(it - 1) - *it;
		x = -1;
	}
	else
	{
		// Get end nodes
		it = paths[path].end() - 1;
		diff = *(it - 1) - *it;
	}

	// Get ratio of difference
	const float nodeDistance = sqrt(diff.x * diff.x + diff.y * diff.y);
	const float ratio = nodeDistance / totalDist;

	// Get direction
	posVectorFloat out;
	out.x = (*(it + x)).x - target / ratio * diff.x;
	out.y = (*(it + x)).y - target / ratio * diff.y;
	return out;
}

}
