#pragma once

#include "Sprite.h"
#include "FieldProp.h"
#include "global.h"
#include "tinyxml.h"
#include <map>
#include <list>
#include <string>
#include <cmath>
#include <algorithm>

namespace ppvs
{

struct AnimationSprite
{
	Sprite* spr = nullptr;
	std::string parent;
	PosVectorFloat position;
	PosVectorFloat childOffset;
	PosVectorFloat pathPos;
	PosVectorFloat scale;
	float angle{};
	float transparency{};
	std::vector<std::string> children;
	AnimationSprite() = default;
};

typedef std::vector<PosVectorFloat> nodes;

class Animation
{
public:
	Animation();
	virtual ~Animation();

	void init(GameData* g, PosVectorFloat offsetPos, float scale, std::string folder = "", const std::string& scriptName = "animation.xml", int maxTime = 120);
	void prepareAnimation(std::string c) { t = 0; animationName = c; resetSprites(); }
	void playAnimation();
	void draw();

	// Callback functions
	void newSprite(std::string& name, std::string& image, std::string& before);
	void newSound(std::string& name, std::string& buffer);
	void addChild(std::string& parent, std::string& child, float x, float y);
	void setRect(std::string& name, float x, float y, float width, float height);
	void setCenter(std::string& name, float x, float y);
	void setPosition(std::string& name, float x, float y);
	void setBlendmode(std::string& name, std::string& blend);
	void setScaleX(std::string& name, float x);
	void setScaleY(std::string& name, float x);
	void setAngle(std::string& name, float x);
	void setTransparency(std::string& name, float x);
	void setVisible(std::string& name, bool x);
	void setColor(std::string& name, std::string& color, float x);
	void move(std::string& name, std::string& path, float x);
	void playSound(std::string& name);

	PosVectorFloat offset;
	float globalScale{};

protected:
	bool spriteExists(const std::string& name);
	void clearSprites();
	void clearSounds();
	void updateSprites();
	void updateChildren(const std::string& parent);
	void resetSprites();
	double getLocalTimer(const std::string& type, double startVal, double endVal, double t, double alpha = 1, double beta = 1);
	float getTotalDistance(std::string& path);
	PosVectorFloat getPosition(std::string& path, float target);
	PosVectorFloat getPositionExtra(std::string& path, float target, float total, bool start);

	int t;
	int duration;
	std::string animationName;
	std::map<std::string, AnimationSprite> sprites;
	std::list<AnimationSprite*> drawSprites;
	std::map<std::string, nodes> paths;
	std::map<std::string, Sound*> sounds;
	GameData* gamedata = nullptr;
	std::string sourceFolder;
	bool error;
	TiXmlDocument doc;
};

}
