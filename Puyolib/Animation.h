#pragma once

#include "FieldProp.h"
#include "Sprite.h"
#include "global.h"
#include "tinyxml.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <map>
#include <string>

namespace ppvs {

struct AnimationSprite {
	AnimationSprite() = default;

	Sprite* sprite = nullptr;
	std::string parent;
	PosVectorFloat position;
	PosVectorFloat childOffset;
	PosVectorFloat pathPos;
	PosVectorFloat scale;
	float angle = 0.f;
	float transparency = 0.f;
	std::vector<std::string> children;
};

typedef std::vector<PosVectorFloat> nodes;

class Animation final {
public:
	Animation();
	~Animation();

	void init(GameData* g, PosVectorFloat offsetPos, float scale, std::string folder = "", const std::string& scriptName = "animation.xml", int maxTime = 120);
	void prepareAnimation(std::string c)
	{
		m_t = 0;
		m_animationName = c;
		resetSprites();
	}
	void playAnimation();
	void draw();

	// Callback functions
	void newSprite(const std::string& name, const std::string& image, const std::string& before);
	void newSound(const std::string& name, const std::string& buffer);
	void addChild(const std::string& parent, const std::string& child, float x, float y);
	void setRect(const std::string& name, float x, float y, float width, float height);
	void setCenter(const std::string& name, float x, float y);
	void setPosition(const std::string& name, float x, float y);
	void setBlendMode(const std::string& name, std::string blend);
	void setScaleX(const std::string& name, float x);
	void setScaleY(const std::string& name, float y);
	void setAngle(const std::string& name, float x);
	void setTransparency(const std::string& name, float x);
	void setVisible(const std::string& name, bool x);
	void setColor(const std::string& name, const std::string& color, float x);
	void move(const std::string& name, const std::string& path, float x);
	void playSound(const std::string& name);

	PosVectorFloat m_offset;
	float m_globalScale = 0.f;

private:
	bool spriteExists(const std::string& name);
	void clearSprites();
	void clearSounds();
	void updateSprites();
	void updateChildren(const std::string& parent);
	void resetSprites();
	[[nodiscard]] double getLocalTimer(const std::string& type, double startVal, double endVal, double t, double alpha = 1, double beta = 1) const;
	float getTotalDistance(const std::string& path);
	PosVectorFloat getPosition(const std::string& path, float target);
	PosVectorFloat getPositionExtra(const std::string& path, float target, float total, bool start);

	int m_t;
	int m_duration;
	std::string m_animationName;
	std::map<std::string, AnimationSprite> m_sprites;
	std::list<AnimationSprite*> m_drawSprites;
	std::map<std::string, nodes> m_paths;
	std::map<std::string, Sound*> m_sounds;
	GameData* m_gameData = nullptr;
	std::string m_sourceFolder;
	bool m_error;
	TiXmlDocument m_doc;
};

}
