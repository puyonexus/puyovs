#pragma once

#include "fieldProp.h"
#include "puyo.h"
#include "otherObjects.h"
#include "global.h"
#include <vector>
#include <algorithm>

namespace ppvs
{

class game;
class player;

class field
{
public:
	field();
	~field();
	field& operator=(const field& other);
	void init(fieldProp properties, player*);
	gameData* data;
	fieldProp getProperties() { return m_properties; }
	void setProperties(fieldProp prop_in) { m_properties = prop_in; }
	posVectorFloat getTopCoord(float offset = 0) const;
	posVectorFloat getBottomCoord(bool s = false) const;
	posVectorFloat getGlobalCoord(int x, int y) const;
	posVectorFloat getLocalCoord(int x, int y) const;
	posVectorFloat getFieldSize() const;
	posVectorFloat getFieldScale() const;
	void setTransformScale(float x) { m_transformScale = x; }
	float getFieldRotation() { return m_properties.angle; }

	player* getPlayer() { return m_player; }

	// Puyo field related functions
	bool isEmpty(int x, int y) const;
	bool isPuyo(int x, int y) const;
	int getColor(int x, int y) const;
	bool addColorPuyo(int x, int y, int color, int fallFlag = 0, int offset = 0, int fallDelay = 0);
	bool addNuisancePuyo(int x, int y, int fallFlag = 0, int offset = 0, int fallDelay = 0);
	void drop() const;
	int dropSingle(int x, int y) const;
	void connectTrigger(int n, std::vector<int>& v);
	puyo* get(int x, int y) const;
	bool set(int x, int y, puyo*) const;
	void clearFieldVal(int x, int y) const;
	puyoType getPuyoType(int x, int y) const;
	void setLink(int x, int y, direction dir) const;
	void unsetLink(int x, int y, direction dir) const;
	bool findConnected(int x, int y, int n, std::vector<posVectorInt>& v);
	void unmark() const;
	void findConnectedLoop(posVectorInt pos, int& connected, std::vector<posVectorInt>& v);
	int count() const;
	int predictChain();
	void removePuyo(int x, int y);
	void clearField() const;

	// Sprite related functions
	void setVisible(bool);
	void clear();
	void drawField() const;
	void draw() const;

	// Gameplay related functions
	void createPuyo(); // Phase 20
	void searchFallDelay() const; // Phase 20
	void unsetLinkAll(int x, int y) const;
	void dropPuyo();
	void fallPuyo();
	void bouncePuyo() const;
	void searchBounce(int x, int y, int posy) const;
	void searchLink(int x, int y) const;
	void endFallPuyoPhase() const;
	void searchChain(); // Phase 30
	void popPuyoAnim(); // Phase 32
	void dropGarbage(bool automatic = true, int dropAmount = 0); // Phase 41
	void loseDrop() const; // Phase 44
	void dropField(const std::string& fieldstring);
	void setFieldFromString(const std::string& fieldstring);
	std::string getFieldString() const;
	void throwAwayField();

	// Other objects
	void createParticle(float x, float y, int color);
	void createParticleThrow(puyo* p);
	void animateParticle();
	int getParticleNumber() const;
	void triggerGlow(posVectorInt shadowPos[4], int n, int colors[4]);
	int virtualChain(posVectorInt shadowPos[4], int n, int colors[4]);

private:
	void createPuyoArray();
	void freePuyo(bool copy);
	void freePuyoArray();

	bool m_fieldInit;
	std::vector<particle*> m_particles;
	std::vector<particleThrow*> m_particlesThrow;
	std::vector<puyo*> m_deletedPuyo;
	player* m_player;
	fieldProp m_properties;
	float m_centerX, m_centerY;
	puyo*** fieldPuyoArray;
	puyo*** fieldPuyoArrayCopy; // Useful for predicting chain

	// Sprite related values
	sprite m_fieldSpriteFinal; // Drawn onto screen
	bool m_visible;
	float m_posXreal, m_posYreal; // Position of the  fieldsprite
	float m_transformScale; // Fever transition

	// Gameplay related values
	float m_sweepFall;
	std::vector<posVectorInt> m_vector;
};

}
