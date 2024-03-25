#pragma once

#include "FieldProp.h"
#include "OtherObjects.h"
#include "Puyo.h"
#include "global.h"
#include <vector>

namespace ppvs {

class Game;
class Player;

class Field {
public:
	Field();
	~Field();

	Field(const Field&) = delete;
	Field& operator=(const Field& rhs);
	Field(Field&&) = delete;
	Field& operator=(Field&&) = delete;

	void init(FieldProp properties, Player*);

	[[nodiscard]] FieldProp getProperties() const { return m_properties; }
	void setProperties(FieldProp props) { m_properties = props; }
	[[nodiscard]] PosVectorFloat getTopCoordinates(float offset = 0) const;
	[[nodiscard]] PosVectorFloat getBottomCoordinates(bool relative = false) const;
	[[nodiscard]] PosVectorFloat getGlobalCoordinates(int x, int y) const;
	[[nodiscard]] PosVectorFloat getLocalCoordinates(int x, int y) const;
	[[nodiscard]] PosVectorFloat getFieldSize() const;
	[[nodiscard]] PosVectorFloat getFieldScale() const;
	void setTransformScale(float x) { m_transformScale = x; }
	[[nodiscard]] float getFieldRotation() const { return m_properties.angle; }

	[[nodiscard]] Player* getPlayer() const { return m_player; }

	// Puyo field related functions
	[[nodiscard]] bool isEmpty(int x, int y) const;
	[[nodiscard]] bool isPuyo(int x, int y) const;
	[[nodiscard]] int getColor(int x, int y) const;
	bool addColorPuyo(int x, int y, int color, int fallFlag = 0, int offset = 0, int fallDelay = 0);
	bool addNuisancePuyo(int x, int y, int fallFlag = 0, int offset = 0, int fallDelay = 0);
	void drop();
	int dropSingle(int x, int y);
	[[nodiscard]] Puyo* get(int x, int y) const;
	bool set(int x, int y, Puyo*) const;
	void clearFieldVal(int x, int y) const;
	[[nodiscard]] PuyoType getPuyoType(int x, int y) const;
	void setLink(int x, int y, Direction dir) const;
	void unsetLink(int x, int y, Direction dir) const;
	bool findConnected(int x, int y, int n, std::vector<PosVectorInt>& v);
	void unmark() const;
	void findConnectedLoop(PosVectorInt pos, int& connected, std::vector<PosVectorInt>& v);
	[[nodiscard]] int count() const;
	int predictChain();
	void removePuyo(int x, int y);
	void clearField() const;

	// Sprite related functions
	void setVisible(bool);
	void drawField() const;
	void draw() const;
	void hotReload();

	// Game-play related functions
	void createPuyo(); // Phase 20
	void searchFallDelay() const; // Phase 20
	void unsetLinkAll(int x, int y) const;
	void dropPuyo();
	void fallPuyo();
	void bouncePuyo() const;
	void searchBounce(int x, int y, int posY) const;
	void searchLink(int x, int y) const;
	void endFallPuyoPhase() const;
	void searchChain(); // Phase 30
	void popPuyoAnim(); // Phase 32
	void dropGarbage(bool automatic = true, int dropAmount = 0); // Phase 41
	void loseDrop() const; // Phase 44
	void dropField(const std::string& fieldString);
	void setFieldFromString(const std::string& fieldString);
	[[nodiscard]] std::string getFieldString() const;
	void throwAwayField();

	// Other objects
	void createParticle(float x, float y, int color);
	void createParticleThrow(Puyo* p);
	void animateParticle();
	[[nodiscard]] int getParticleNumber() const;
	void triggerGlow(PosVectorInt shadowPos[4], int n, int colors[4]);
	int virtualChain(PosVectorInt shadowPos[4], int n, int colors[4]);

	GameData* m_data = nullptr;

private:
	void createPuyoArray();
	void freePuyo(bool copy);
	void freePuyoArray();

	bool m_fieldInit = false;
	std::vector<Particle*> m_particles;
	std::vector<ParticleThrow*> m_particlesThrow;
	std::vector<Puyo*> m_deletedPuyo;
	Player* m_player = nullptr;
	FieldProp m_properties {};
	float m_centerX = 0.f, m_centerY = 0.f;
	Puyo*** m_fieldPuyoArray = nullptr;
	Puyo*** m_fieldPuyoArrayCopy = nullptr; // Useful for predicting chain

	// Sprite related values
	Sprite m_fieldSpriteFinal; // Drawn onto screen
	bool m_visible = false;
	float m_posXReal = 0.f, m_posYReal = 0.f; // Position of the  field sprite
	float m_transformScale = 1.f; // Fever transition

	// Game-play related values
	float m_sweepFall = 0.f;
	std::vector<PosVectorInt> m_vector;
};

}
