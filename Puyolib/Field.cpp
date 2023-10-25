#include "Field.h"
#include "../PVS_ENet/PVS_Client.h"
#include "Game.h"
#include "Player.h"
#include <algorithm>
#include <utility>
#include <cmath>

using namespace std;

namespace ppvs {

Field::Field() = default;

Field::~Field()
{
	// Destroy particles, throw puyo, destroying puyos
	while (!m_particles.empty()) {
		delete m_particles.back();
		m_particles.pop_back();
	}
	while (!m_particlesThrow.empty()) {
		delete m_particlesThrow.back();
		m_particlesThrow.pop_back();
	}
	while (!m_deletedPuyo.empty()) {
		delete m_deletedPuyo.back();
		m_deletedPuyo.pop_back();
	}
	if (m_fieldInit) {
		freePuyo(true);
		freePuyo(false);
		freePuyoArray();
	}
}

Field& Field::operator=(const Field& rhs)
{
	// Handle self assignment
	if (this == &rhs)
		return *this;

	return *this;
}

void Field::init(const FieldProp properties, Player* pl)
{
	m_player = pl;
	m_data = m_player->m_data;

	m_visible = true;
	m_properties = properties;

	// Calculate center (does not include x, y scale)
	m_centerX = static_cast<float>(properties.gridX * properties.gridWidth) / 2.0f;
	m_centerY = static_cast<float>((properties.gridY - 3) * properties.gridHeight);
	m_properties.centerX = m_centerX;
	m_properties.centerY = m_centerY;

	// Create the puyo array
	createPuyoArray();

	// Recalculate position (global screen position)
	m_posXReal = properties.offsetX + m_centerX;
	m_posYReal = properties.offsetY + m_centerY;
}

// Initializes both puyo array and puyo array copy
void Field::createPuyoArray()
{
	m_fieldPuyoArray = new Puyo**[m_properties.gridX];
	m_fieldPuyoArrayCopy = new Puyo**[m_properties.gridX];

	for (int i = 0; i < m_properties.gridX; i++) {
		m_fieldPuyoArray[i] = new Puyo*[m_properties.gridY];
		m_fieldPuyoArrayCopy[i] = new Puyo*[m_properties.gridY];
	}

	// Initialize the puyo array with null
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			m_fieldPuyoArray[i][j] = nullptr;
			m_fieldPuyoArrayCopy[i][j] = nullptr;
		}
	}

	m_fieldInit = true;
}

// Delete all puyo in array or copy
void Field::freePuyo(const bool copy)
{
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (!copy) {
				delete m_fieldPuyoArray[i][j];
				m_fieldPuyoArray[i][j] = nullptr;
			} else {
				delete m_fieldPuyoArrayCopy[i][j];
				m_fieldPuyoArrayCopy[i][j] = nullptr;
			}
		}
	}
	m_fieldInit = false;
}

// Delete array and array copy
void Field::freePuyoArray()
{
	for (int i = 0; i < m_properties.gridX; i++) {
		delete[] m_fieldPuyoArray[i];
		delete[] m_fieldPuyoArrayCopy[i];
	}
	delete[] m_fieldPuyoArray;
	delete[] m_fieldPuyoArrayCopy;
	m_fieldInit = false;
}

// Get screen coordinates of top, with an offset
PosVectorFloat Field::getTopCoordinates(float offset) const
{
	PosVectorFloat pv;
	pv.x = m_properties.offsetX + m_properties.centerX * m_properties.scaleX * m_player->getGlobalScale();
	pv.y = m_properties.offsetY + offset * kPuyoY * m_properties.scaleY * m_player->getGlobalScale();
	return pv;
}

// Get screen coordinates of bottom
PosVectorFloat Field::getBottomCoordinates(bool relative) const
{
	PosVectorFloat pv;
	if (relative) {
		pv.x = m_properties.centerX * m_properties.scaleX;
		pv.y = static_cast<float>(m_properties.gridHeight * (m_properties.gridY - 3 - 0)) * m_properties.scaleY;
	} else {
		pv.x = m_properties.offsetX + m_properties.centerX * m_properties.scaleX * m_player->getGlobalScale();
		pv.y = m_properties.offsetY + static_cast<float>(m_properties.gridHeight * (m_properties.gridY - 3 - 0)) * m_properties.scaleY * m_player->getGlobalScale();
	}
	return pv;
}

// Convert an indexed position to screen position
PosVectorFloat Field::getGlobalCoordinates(int x, int y) const
{
	PosVectorFloat pv;
	pv.x = m_properties.offsetX + static_cast<float>(m_properties.gridWidth * x) * m_properties.scaleX * m_player->getGlobalScale();
	pv.y = m_properties.offsetY + static_cast<float>(m_properties.gridHeight * (m_properties.gridY - 3 - y)) * m_properties.scaleY * m_player->getGlobalScale();
	return pv;
}

// Convert an indexed position to field position
PosVectorFloat Field::getLocalCoordinates(int x, int y) const
{
	PosVectorFloat pv;
	pv.x = static_cast<float>(m_properties.gridWidth * x);
	pv.y = static_cast<float>(m_properties.gridHeight * (m_properties.gridY - 3 - y));
	return pv;
}

// Returns current field size
PosVectorFloat Field::getFieldSize() const
{
	return {
		static_cast<float>(m_properties.gridX * m_properties.gridWidth),
		static_cast<float>((m_properties.gridY - 3) * m_properties.gridHeight)
	};
}

PosVectorFloat Field::getFieldScale() const
{
	return { m_properties.scaleX, m_properties.scaleY };
}

//============================
// Puyo Field related functions
//============================

// Check if a position in field is an empty space
bool Field::isEmpty(const int x, const int y) const
{
	// Special case: uninitialized position -> just say it's empty
	if (x < -2 && y < -2) {
		return true;
	}

	// x or y is outside the field
	// Outside the field is regarded as nonempty! that doesn't mean there is a puyo there -> use isPuyo instead
	if (x > m_properties.gridX - 1 || x < 0 || y < 0) {
		return false;
	}

	// Exception: the upper field stretches out to infinity
	if (y > m_properties.gridY - 1 && x > 0 && x < m_properties.gridX - 1) {
		return true;
	}

	if (m_fieldPuyoArray[x][y] == nullptr) {
		return true;
	}

	return false;
}

// Check if a puyo exist in this position
bool Field::isPuyo(const int x, const int y) const
{
	// Outside the field are no puyos
	if (x > m_properties.gridX - 1 || x < 0 || y > m_properties.gridY - 1 || y < 0) {
		return false;
	}

	if (m_fieldPuyoArray[x][y] == nullptr) {
		return false;
	}

	return true;
}

// Retrieves color from field (if puyo is a color puyo anyway)
int Field::getColor(const int x, const int y) const
{
	// x or y is outside the field
	if (x > m_properties.gridX - 1 || x < 0 || y > m_properties.gridY - 1 || y < 0) {
		return -1;
	}

	if (!isPuyo(x, y)) {
		return -1;
	}

	if (m_fieldPuyoArray[x][y]->getType() != COLORPUYO) {
		return -1;
	}

	// Ask the corresponding puyo what color it is
	return m_fieldPuyoArray[x][y]->getColor();
}

// This function creates a colored puyo on the field
bool Field::addColorPuyo(const int x, const int y, const int color, const int fallFlag, const int offset, const int fallDelay)
{
	// Invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0) {
		return false;
	}

	// A puyo already exists here
	if (isPuyo(x, y)) {
		return false;
	}

	// Calculate position of sprite
	const auto spriteXReal = static_cast<float>(x * m_properties.gridWidth + kPuyoX / 2); // NOLINT(bugprone-integer-division)
	const auto spriteYReal = static_cast<float>((m_properties.gridY - 3 - y - offset) * m_properties.gridHeight);

	// Create a new puyo at x,y index
	m_fieldPuyoArray[x][y] = new ColorPuyo(x, y, color, this, spriteXReal, spriteYReal, m_data);

	// Set initial condition
	m_fieldPuyoArray[x][y]->m_fallFlag = fallFlag;
	m_fieldPuyoArray[x][y]->m_fallDelay = static_cast<float>(fallDelay);

	return true;
}

// Add nuisance to the field
bool Field::addNuisancePuyo(const int x, const int y, const int fallFlag, const int offset, const int fallDelay)
{
	// Invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0) {
		return false;
	}

	// A puyo already exists here
	if (isPuyo(x, y)) {
		return false;
	}

	// Calculate position of sprite
	const auto spriteXReal = static_cast<float>(x * m_properties.gridWidth + kPuyoX / 2); // NOLINT(bugprone-integer-division)
	const auto spriteYReal = static_cast<float>((m_properties.gridY - 3 - y - offset) * m_properties.gridHeight);

	// Create a new puyo at x,y index
	m_fieldPuyoArray[x][y] = new NuisancePuyo(x, y, 0, this, spriteXReal, spriteYReal, m_data);

	// Set initial state
	m_fieldPuyoArray[x][y]->m_fallFlag = fallFlag;
	m_fieldPuyoArray[x][y]->m_fallDelay = static_cast<float>(fallDelay);

	return true;
}

// Drop all puyos down.
void Field::drop()
{
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			dropSingle(i, j);
		}
	}
}

// Drop a single puyo down, returns new y position. returns -1 if no puyo dropped at all
// ReSharper disable once CppMemberFunctionMayBeConst
int Field::dropSingle(const int x, const int y)
{
	if (!isPuyo(x, y)) {
		return -1; // Nothing to drop
	}

	if (!m_fieldPuyoArray[x][y]->m_droppable) {
		return -1; // Cannot drop
	}

	// Out of bounds
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0) {
		return -1;
	}

	bool foundEmpty;

	// Puyo is on bottom: don't bother
	if (y == 0) {
		return y;
	}

	// Initial checking if empty
	if (!((foundEmpty = isEmpty(x, y - 1)))) {
		return y;
	}

	int emptyY = y;

	while (foundEmpty && emptyY >= 1) {
		// Check if next position is empty
		if ((foundEmpty = isEmpty(x, emptyY - 1)) == true) {
			emptyY -= 1;
		}
	}

	// Drop puyo down
	m_fieldPuyoArray[x][emptyY] = m_fieldPuyoArray[x][y];
	m_fieldPuyoArray[x][emptyY]->setPosY(emptyY);
	m_fieldPuyoArray[x][y] = nullptr;

	return emptyY;
}

// Get puyo at position
Puyo* Field::get(const int x, const int y) const
{
	if (isEmpty(x, y)) {
		return nullptr;
	}

	return m_fieldPuyoArray[x][y];
}

// Set puyo, returns true if success
bool Field::set(int x, int y, Puyo* newPuyo) const
{
	// Invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0) {
		return false;
	}

	m_fieldPuyoArray[x][y] = newPuyo;

	return true;
}

void Field::clearFieldVal(int x, int y) const
{
	if (isPuyo(x, y)) {
		m_fieldPuyoArray[x][y] = nullptr;
	}
}

// Get Puyo type in a safe way
PuyoType Field::getPuyoType(int x, int y) const
{
	// Invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0) {
		return NOPUYO;
	}

	if (!isPuyo(x, y)) {
		return NOPUYO;
	}

	return m_fieldPuyoArray[x][y]->getType();
}

// Connect to a neighboring puyo
void Field::setLink(const int x, const int y, const Direction dir) const
{
	// Invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0) {
		return;
	}

	if (isPuyo(x, y)) {
		m_fieldPuyoArray[x][y]->setLink(dir);
	}
}

// Disconnect a puyo
void Field::unsetLink(const int x, const int y, const Direction dir) const
{
	// Invalid position
	if (x >= m_properties.gridX || x < 0 || y >= m_properties.gridY || y < 0) {
		return;
	}

	if (isPuyo(x, y)) {
		m_fieldPuyoArray[x][y]->unsetLink(dir);
	}
}

// Find n connected color puyo and put coordinates into vector v
bool Field::findConnected(int x, int y, int n, std::vector<PosVectorInt>& v)
{
	PosVectorInt pos;
	pos.x = x;
	pos.y = y;

	// Initial check of first puyo
	if (!isPuyo(pos.x, pos.y) || m_fieldPuyoArray[pos.x][pos.y]->getType() != COLORPUYO || m_fieldPuyoArray[pos.x][pos.y]->m_mark != false || m_fieldPuyoArray[pos.x][pos.y]->m_destroy != false) {
		return false;
	}

	// Find connecting color puyo and fill vector v
	int connected = 1;
	m_fieldPuyoArray[pos.x][pos.y]->m_mark = true;
	v.push_back(pos);
	findConnectedLoop(pos, connected, v);

	if (connected >= n) {
		return true;
	}

	// Did not match criteria: remove the elements from v
	while (connected) {
		v.pop_back();
		connected--;
	}
	return false;
}

// Unmark all puyo in field
void Field::unmark() const
{
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (isPuyo(i, j)) {
				m_fieldPuyoArray[i][j]->m_mark = false;
			}
		}
	}
}

// Recursive function that checks neighbors
void Field::findConnectedLoop(const PosVectorInt pos, int& connected, std::vector<PosVectorInt>& v)
{
	m_fieldPuyoArray[pos.x][pos.y]->m_mark = true;
	PosVectorInt posCopy;
	posCopy.x = pos.x;
	posCopy.y = pos.y;
	constexpr std::pair<int, int> search[4] = {
		{ 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 }
	};
	for (auto [hor, ver] : search) {
		if (isPuyo(pos.x + hor, pos.y + ver) && m_fieldPuyoArray[pos.x + hor][pos.y + ver]->getType() == COLORPUYO
			&& m_fieldPuyoArray[pos.x + hor][pos.y + ver]->m_mark == false
			&& m_fieldPuyoArray[pos.x + hor][pos.y + ver]->getColor() == m_fieldPuyoArray[pos.x][pos.y]->getColor()
			&& pos.y + ver != m_properties.gridY - 3) {
			connected++;
			posCopy.x = pos.x + hor; // Need a copy of pos to pass on
			posCopy.y = pos.y + ver;
			v.push_back(posCopy);
			findConnectedLoop(posCopy, connected, v);
		}
	}
}

// Count puyos in field
int Field::count() const
{
	int n = 0;

	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (isPuyo(i, j)) {
				n++;
			}
		}
	}

	return n;
}

// Predict chain
int Field::predictChain()
{
	// Copy original array into array copy
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			// Copy pointer
			m_fieldPuyoArrayCopy[i][j] = m_fieldPuyoArray[i][j];
			// Copy puyo
			if (isPuyo(i, j)) {
				m_fieldPuyoArray[i][j] = m_fieldPuyoArray[i][j]->clone();
			}
		}
	}

	// Find chain
	bool foundChain; // Local variable
	int chainN = 0;
	do {
		foundChain = false;
		unmark();
		// Loop through field to find connected puyo
		for (int i = 0; i < m_properties.gridX; i++) {
			for (int j = 0; j < m_properties.gridY - 3; j++) {
				if (findConnected(i, j, m_player->m_currentGame->m_currentRuleSet->m_puyoToClear, m_vector)) {
					foundChain = true;

					// Loop through connected puyo
					while (!m_vector.empty()) {
						const PosVectorInt pv = m_vector.back();
						m_vector.pop_back();

						// Set popped group
						// Check neighbors to find nuisance
						if (isPuyo(pv.x, pv.y + 1) && pv.y + 1 != m_properties.gridY - 3)
							m_fieldPuyoArray[pv.x][pv.y + 1]->neighborPop(this, true);
						if (isPuyo(pv.x + 1, pv.y))
							m_fieldPuyoArray[pv.x + 1][pv.y]->neighborPop(this, true);
						if (isPuyo(pv.x, pv.y - 1))
							m_fieldPuyoArray[pv.x][pv.y - 1]->neighborPop(this, true);
						if (isPuyo(pv.x - 1, pv.y))
							m_fieldPuyoArray[pv.x - 1][pv.y]->neighborPop(this, true);

						// Delete puyo
						delete m_fieldPuyoArray[pv.x][pv.y];
						m_fieldPuyoArray[pv.x][pv.y] = nullptr;
					}
				}
			}
		}

		// Drop puyo and add to chain number
		if (foundChain) {
			chainN++;
			drop();
		}
	} while (foundChain);

	// Delete temporary puyos
	freePuyo(false);
	m_fieldInit = true;

	// Restore original
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			// Copy pointer
			m_fieldPuyoArray[i][j] = m_fieldPuyoArrayCopy[i][j];
			m_fieldPuyoArrayCopy[i][j] = nullptr;
		}
	}

	return chainN;
}

// Function that marks puyo and puts them in the deleted puyo list
void Field::removePuyo(const int x, int y)
{
	if (isPuyo(x, y)) {
		m_fieldPuyoArray[x][y]->m_destroy = true;
		m_deletedPuyo.push_back(m_fieldPuyoArray[x][y]);
	}
}

// Clean up field
void Field::clearField() const
{
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (!isEmpty(i, j)) {
				delete m_fieldPuyoArray[i][j];
				m_fieldPuyoArray[i][j] = nullptr;
			}
		}
	}
}

//============================
// Sprite related functions
//============================

// Set field and content visible
void Field::setVisible(bool visibility)
{
	m_visible = visibility;
}

// Draw field background and content
void Field::drawField() const
{
	if (!m_visible) {
		return;
	}

	// Lower layer: consider moving this part to player!
	m_player->drawFieldBack(getBottomCoordinates(true), m_properties.angle);
	m_player->drawFieldFeverBack(getBottomCoordinates(true), m_properties.angle);
	m_player->drawCross(m_data->front);
	m_player->drawAllClear(getBottomCoordinates(true), 1, 1, m_properties.angle);
	m_data->front->setDepthFunction(DepthFunction::Equal);

	// Draw puyos on the field
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (isPuyo(i, j)) {
				m_fieldPuyoArray[i][j]->draw(m_data->front);
			}
		}
	}

	// Draw deleting puyo
	for (const auto puyo : m_deletedPuyo) {
		puyo->draw(m_data->front);
	}

	// Draw particles on the field
	for (const auto particle : m_particles) {
		particle->draw(m_data->front);
	}

	// Draw throwPuyo on the field
	for (const auto particleThrow : m_particlesThrow) {
		particleThrow->draw(m_data->front);
	}
}

void Field::draw() const
{
	drawField();
}

//============================
// Game-play related functions
//============================

// Phase 20: Create puyo and get ready to drop
void Field::createPuyo()
{
	if (m_player->m_createPuyo == false) {
		return;
	}

	// Turn off glow
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (isPuyo(i, j)) {
				m_fieldPuyoArray[i][j]->m_glow = false;
			}
		}
	}

	// Only create puyo after rotation has really finished
	if (m_player->m_movePuyo.getRotateCounter() == 0 && m_player->m_movePuyo.getFlipCounter() == 0 && m_player->getPlayerType() != ONLINE && m_player->m_currentGame->m_settings->recording != RecordState::REPLAYING
		|| m_player->getPlayerType() == ONLINE && !m_player->m_messages.empty() && m_player->m_messages.front()[0] == 'p'
		|| m_player->getPlayerType() == HUMAN && m_player->m_currentGame->m_settings->recording == RecordState::REPLAYING && m_player->m_messages.front()[0] == 'p') {
		m_player->m_movePuyo.setVisible(false);

		// Create colored puyo pair on field
		// Get values from movePuyos
		const MovePuyoType type = m_player->m_movePuyo.getType();
		int color1 = m_player->m_movePuyo.getColor1();
		int color2 = m_player->m_movePuyo.getColor2();
		int colorBig = m_player->m_movePuyo.getColorBig();
		int posX1 = m_player->m_movePuyo.getPosX1();
		int posY1 = m_player->m_movePuyo.getPosY1();
		int posX2 = m_player->m_movePuyo.getPosX2();
		int posY2 = m_player->m_movePuyo.getPosY2();
		int posX3 = m_player->m_movePuyo.getPosX3();
		int posY3 = m_player->m_movePuyo.getPosY3();
		int posX4 = m_player->m_movePuyo.getPosX4();
		int posY4 = m_player->m_movePuyo.getPosY4();
		const bool transpose = m_player->m_movePuyo.getTranspose();
		const int rotation = m_player->m_movePuyo.getRotation();

		m_player->m_divider = max(2, m_player->m_currentGame->getActivePlayers());
		// Award a bonus garbage when offsetting?
		if (m_player->m_currentGame->m_currentRuleSet->m_bonusEq && m_player->getPlayerType() != ONLINE) {
			// Check if any garbage, award bonus EQ
			if (m_player->getGarbageSum() > 0)
				m_player->m_bonusEq = true;
		}

		// Send placement info
		// 0["p"]1[color1]2[color2]3[colorBig]
		// 4[pos x1]5[pos y1]6[pos x2]7[pos y2]8[pos x3]9[pos y3]10[pos x4]11[pos y4]
		// 12[score val]13[drop bonus]14[margin time]15[divider]16[bonus EQ]
		if (m_player->m_currentGame->m_connected && m_player->getPlayerType() == HUMAN) {
			char str[200];
			sprintf(str, "p|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i",
				color1, color2, colorBig,
				posX1, posY1,
				posX2, posY2,
				posX3, posY3,
				posX4, posY4,
				m_player->m_scoreVal, m_player->m_dropBonus,
				m_player->m_marginTimer, m_player->m_divider, static_cast<int>(m_player->m_bonusEq));
			m_player->m_currentGame->m_network->sendToChannel(CHANNEL_GAME, str, m_player->m_currentGame->m_channelName.c_str());

			// Record for replay
			if (m_player->m_currentGame->m_settings->recording == RecordState::RECORDING) {
				const MessageEvent me = { m_data->matchTimer, "" };
				const std::string mes = str;
				m_player->m_recordMessages.push_back(me);
				if (mes.length() < 64) {
					strcpy(m_player->m_recordMessages.back().message, mes.c_str());
				}
			}
		}
		// Receive
		if ((m_player->getPlayerType() == ONLINE || m_player->m_currentGame->m_settings->recording == RecordState::REPLAYING)
			&& !m_player->m_messages.empty() && m_player->m_messages.front()[0] == 'p') {
			int bEq = 0;
			int marginTime = 0;
			sscanf(m_player->m_messages.front().c_str(), "p|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i",
				&color1, &color2, &colorBig,
				&posX1, &posY1,
				&posX2, &posY2,
				&posX3, &posY3,
				&posX4, &posY4,
				&m_player->m_scoreVal, &m_player->m_dropBonus,
				&marginTime, &m_player->m_divider, &bEq);

			// Adjust margin time downwards with 30 second error interval
			if (m_player->m_marginTimer + 20 * 60 >= marginTime) {
				m_player->m_marginTimer = marginTime;
			}

			if (bEq) {
				m_player->m_bonusEq = true;
			}

			// Clear message
			m_player->m_messages.pop_front();
		}

		// Place puyos
		if (type == MovePuyoType::DOUBLET) {
			addColorPuyo(posX1, posY1, color1);
			addColorPuyo(posX2, posY2, color2);
		} else if (type == MovePuyoType::TRIPLET && transpose == false) {
			addColorPuyo(posX1, posY1, color1);
			addColorPuyo(posX2, posY2, color2);
			addColorPuyo(posX3, posY3, color1);
		} else if (type == MovePuyoType::TRIPLET && transpose == true) {
			addColorPuyo(posX1, posY1, color1);
			addColorPuyo(posX2, posY2, color1);
			addColorPuyo(posX3, posY3, color2);
		} else if (type == MovePuyoType::QUADRUPLET) {
			if (rotation == 1) {
				addColorPuyo(posX1, posY1, color2);
				addColorPuyo(posX2, posY2, color1);
				addColorPuyo(posX3, posY3, color2);
				addColorPuyo(posX4, posY4, color1);
			} else if (rotation == 2) {
				addColorPuyo(posX1, posY1, color1);
				addColorPuyo(posX2, posY2, color1);
				addColorPuyo(posX3, posY3, color2);
				addColorPuyo(posX4, posY4, color2);
			} else if (rotation == 3) {
				addColorPuyo(posX1, posY1, color1);
				addColorPuyo(posX2, posY2, color2);
				addColorPuyo(posX3, posY3, color1);
				addColorPuyo(posX4, posY4, color2);
			} else if (rotation == 0) {
				addColorPuyo(posX1, posY1, color2);
				addColorPuyo(posX2, posY2, color2);
				addColorPuyo(posX3, posY3, color1);
				addColorPuyo(posX4, posY4, color1);
			}
		} else if (type == MovePuyoType::BIG) {
			addColorPuyo(posX1, posY1, colorBig);
			addColorPuyo(posX2, posY2, colorBig);
			addColorPuyo(posX3, posY3, colorBig);
			addColorPuyo(posX4, posY4, colorBig);
		}

		// Play sound
		m_data->snd.drop.play(m_data);

		// Check if garbage should fall now
		if (m_player->m_activeGarbage->gq <= 0
			// bug-fix: only outside fever
			&& !m_player->m_feverMode) {
			m_player->m_forgiveGarbage = true;
		}
		// If it is fever: you can, but fever must have not ended yet

		// End phase 20
		if (m_player->m_currentPhase == Phase::CREATEPUYO) {
			m_player->endPhase();
		}
	}
}

// DEPRECATED. set puyo ready to fall and disconnect them
void Field::searchFallDelay() const
{
	for (int i = 0; i < m_properties.gridX; i++) {
		bool foundEmpty = false;
		int count = 0;
		int countDelay = 0;
		// Predict the position to fall to
		for (int j = 0; j < m_properties.gridY; j++) {
			// Search for empty spaces. After first empty space, start marking
			if (isEmpty(i, j) && foundEmpty == false) {
				foundEmpty = true;
				countDelay = 0;
				count = j;
			}
			// After empty space found
			if (foundEmpty == true && isPuyo(i, j)) {
				m_fieldPuyoArray[i][j]->m_fallDelay = static_cast<float>(countDelay);
				m_fieldPuyoArray[i][j]->m_fallFlag = 1;
				m_fieldPuyoArray[i][j]->setFallTarget(count);
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

// Disconnect color puyo from all directions
void Field::unsetLinkAll(const int x, const int y) const
{
	// Check down -> unset up-link
	if (getPuyoType(x, y - 1) == COLORPUYO) {
		unsetLink(x, y - 1, ABOVE);
	}

	// Check up
	if (getPuyoType(x, y + 1) == COLORPUYO) {
		unsetLink(x, y + 1, BELOW);
	}

	// Check right
	if (getPuyoType(x + 1, y) == COLORPUYO) {
		unsetLink(x + 1, y, LEFT);
	}

	// Check left
	if (getPuyoType(x - 1, y) == COLORPUYO) {
		unsetLink(x - 1, y, RIGHT);
	}

	// Disconnect self
	unsetLink(x, y, ABOVE);
	unsetLink(x, y, BELOW);
	unsetLink(x, y, LEFT);
	unsetLink(x, y, RIGHT);
}

// Cleanly drop all puyo (set fall target etc.)
void Field::dropPuyo()
{
	for (int i = 0; i < m_properties.gridX; i++) {
		int countDelay = 0;
		for (int j = 0; j < m_properties.gridY; j++) {
			// Drop
			const int newY = dropSingle(i, j);
			if (newY == -1)
				continue;
			// Set fall delay
			if (j != newY) {
				if (getPuyoType(i, newY) == COLORPUYO) {
					unsetLinkAll(i, j);
					unsetLink(i, newY, ABOVE);
					unsetLink(i, newY, BELOW);
					unsetLink(i, newY, LEFT);
					unsetLink(i, newY, RIGHT);
				}
				m_fieldPuyoArray[i][newY]->m_fallDelay = static_cast<float>(countDelay);
				m_fieldPuyoArray[i][newY]->m_fallFlag = 1;
				m_fieldPuyoArray[i][newY]->setFallTarget(newY);
				countDelay++;
			}
		}
	}

	m_sweepFall = 0;
	m_player->endPhase();
}

// Phase 21: falling field puyos
void Field::fallPuyo()
{
	// Do a sweep
	if (m_player->m_currentGame->m_currentRuleSet->m_delayedFall == true) {
		m_sweepFall += 0.5f;
	} else {
		m_sweepFall = 100;
	}

	// Loop through puyos
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (isPuyo(i, j)) {
				if (m_fieldPuyoArray[i][j]->m_fallFlag == 1 && m_fieldPuyoArray[i][j]->m_fallDelay <= m_sweepFall) {
					m_fieldPuyoArray[i][j]->m_fallFlag = 2;
					m_fieldPuyoArray[i][j]->setAccelerationY(0);
				}
				// Set gravity
				if (m_fieldPuyoArray[i][j]->m_fallFlag == 2)
					m_fieldPuyoArray[i][j]->addAccelerationY(m_player->m_gravity);
				// If it's not a faller, mark the puyo for a bounce.
				// Search for Puyos that need to bounce. Use flag 0 for this
				if (m_fieldPuyoArray[i][j]->m_fallFlag == 0 && m_fieldPuyoArray[i][j]->m_bounceFlag0 == false) {
					m_fieldPuyoArray[i][j]->setAccelerationY(0);
					m_fieldPuyoArray[i][j]->m_bounceFlag = 1;
					m_fieldPuyoArray[i][j]->m_bounceTimer = 2;
					m_fieldPuyoArray[i][j]->m_bounceFlag0 = true;
					searchBounce(i, j, m_fieldPuyoArray[i][j]->posY() + 1);
				}
				// Sound & voice when dropping nuisance
				// Non-nuisance puyo
				if (m_fieldPuyoArray[i][j]->m_fallFlag != 0 && m_fieldPuyoArray[i][j]->spriteY() > m_fieldPuyoArray[i][j]->targetY()
					&& m_fieldPuyoArray[i][j]->getType() != NUISANCEPUYO) {
					m_data->snd.drop.play(m_data);
				}
				// Nuisance puyo: normal drop
				if (m_fieldPuyoArray[i][j]->m_fallFlag != 0 && m_fieldPuyoArray[i][j]->spriteY() > m_fieldPuyoArray[i][j]->targetY()
					&& m_fieldPuyoArray[i][j]->getType() == NUISANCEPUYO && m_player->m_currentPhase != Phase::FALLGARBAGE) {
					m_data->snd.drop.play(m_data);
				}
				// Nuisance puyo: garbage drop
				if (m_fieldPuyoArray[i][j]->m_fallFlag != 0 && m_fieldPuyoArray[i][j]->spriteY() > m_fieldPuyoArray[i][j]->targetY()
					&& m_fieldPuyoArray[i][j]->getType() == NUISANCEPUYO && m_fieldPuyoArray[i][j]->m_lastNuisance && m_player->m_garbageDropped < 6) {
					m_player->m_garbageDropped = 0;
					m_fieldPuyoArray[i][j]->m_lastNuisance = false;
					m_data->snd.nuisanceS.play(m_data);
				}
				if (m_fieldPuyoArray[i][j]->m_fallFlag != 0 && m_fieldPuyoArray[i][j]->spriteY() > m_fieldPuyoArray[i][j]->targetY()
					&& m_fieldPuyoArray[i][j]->getType() == NUISANCEPUYO && m_fieldPuyoArray[i][j]->m_lastNuisance && m_player->m_garbageDropped >= 6) {
					if (m_player->m_garbageDropped < 24)
						m_player->m_characterVoices.damage1.play(m_data);
					else
						m_player->m_characterVoices.damage2.play(m_data);
					m_player->m_garbageDropped = 0;
					m_fieldPuyoArray[i][j]->m_lastNuisance = false;
					m_data->snd.nuisanceL.play(m_data);
				}

				// Land proper
				if (m_fieldPuyoArray[i][j]->m_fallFlag == 2 && m_fieldPuyoArray[i][j]->spriteY() > m_fieldPuyoArray[i][j]->targetY()) {
					m_fieldPuyoArray[i][j]->m_bounceFlag0 = false;
					m_fieldPuyoArray[i][j]->landProper();
				}
			}
		}
	}
}

// Phase 21: bouncing field puyos
void Field::bouncePuyo() const
{
	// Loop through puyos
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (isPuyo(i, j)) {
				// Bounce puyo
				if (m_fieldPuyoArray[i][j]->m_fallFlag == 0) {
					m_fieldPuyoArray[i][j]->bounce();
				}
				// End of bounce
				if (m_fieldPuyoArray[i][j]->m_bounceTimer > static_cast<float>(m_player->m_puyoBounceEnd)) {
					// Search connect
					if (m_fieldPuyoArray[i][j]->getType() == COLORPUYO) {
						searchLink(i, j);
					}
					m_fieldPuyoArray[i][j]->setScaleX(1);
					m_fieldPuyoArray[i][j]->setScaleY(1);
					m_fieldPuyoArray[i][j]->m_bounceY = 0;
					if (m_fieldPuyoArray[i][j]->m_bounceFlag == 1) {
						m_fieldPuyoArray[i][j]->m_bounceFlag = 0;
					}
					m_fieldPuyoArray[i][j]->m_bounceTimer = 0;
				}
			}
		}
	}
}

// Search bounce strength
void Field::searchBounce(const int x, const int y, const int posY) const
{
	int funFlag = 1, count = 1;
	for (int i = 0; i < posY; i++) {
		if (isPuyo(x, y - i)) {
			// Stop loop
			if (funFlag == 1 && m_fieldPuyoArray[x][y - i]->m_hard == true) {
				// Loop back up
				for (int j = 0; j <= i; j++) {
					m_fieldPuyoArray[x][y - i + j]->m_bottomY = y - i;
				}
				break;
			}
			// Default value
			m_fieldPuyoArray[x][y - i]->m_bottomY = 0;
		}
		if (funFlag == 1 && y - i < 0) {
			funFlag = 0;
		}
		// Set bounceMultiplier, also start disconnecting puyos
		// Search for disconnect
		// The count is what determines how many puyos disconnect
		if (funFlag == 1 && count < 5) {
			if (isPuyo(x, y - i) && m_fieldPuyoArray[x][y - i]->m_fallFlag == 0) {
				unsetLinkAll(x, y - i);
				// Set bounceMultiplier
				m_fieldPuyoArray[x][y - i]->m_bounceMultiplier = 1.f / static_cast<float>(pow(2.f, count - 1));
				m_fieldPuyoArray[x][y - i]->m_bounceTimer = 2.f;
			}
			count++;
		}
	}
}

// Search for puyos connecting
void Field::searchLink(int x, int y) const
{
	// Check down
	if (isPuyo(x, y - 1) && m_fieldPuyoArray[x][y - 1]->getType() == COLORPUYO && m_fieldPuyoArray[x][y - 1]->m_fallFlag == 0 && m_fieldPuyoArray[x][y - 1]->getColor() == m_fieldPuyoArray[x][y]->getColor() && y != m_properties.gridY - 3) {
		// Do not connect in the invisible layers
		m_fieldPuyoArray[x][y - 1]->setLink(ABOVE);
		m_fieldPuyoArray[x][y]->setLink(BELOW);
	}
	// Check up
	if (isPuyo(x, y + 1) && m_fieldPuyoArray[x][y + 1]->getType() == COLORPUYO && m_fieldPuyoArray[x][y + 1]->m_fallFlag == 0 && m_fieldPuyoArray[x][y + 1]->getColor() == m_fieldPuyoArray[x][y]->getColor() && y != m_properties.gridY - 4) {
		// Do not connect in the invisible layers
		m_fieldPuyoArray[x][y + 1]->setLink(BELOW);
		m_fieldPuyoArray[x][y]->setLink(ABOVE);
	}
	// Check right
	if (isPuyo(x + 1, y) && m_fieldPuyoArray[x + 1][y]->getType() == COLORPUYO && m_fieldPuyoArray[x + 1][y]->m_fallFlag == 0 && m_fieldPuyoArray[x + 1][y]->getColor() == m_fieldPuyoArray[x][y]->getColor()) {
		m_fieldPuyoArray[x + 1][y]->setLink(LEFT);
		m_fieldPuyoArray[x][y]->setLink(RIGHT);
	}
	if (isPuyo(x + 1, y) && m_fieldPuyoArray[x + 1][y]->getType() == COLORPUYO && m_fieldPuyoArray[x + 1][y]->m_fallFlag != 0 && m_fieldPuyoArray[x + 1][y]->getColor() == m_fieldPuyoArray[x][y]->getColor()) {
		m_fieldPuyoArray[x + 1][y]->unsetLink(LEFT);
		m_fieldPuyoArray[x][y]->unsetLink(RIGHT);
	}
	// Check left
	if (isPuyo(x - 1, y) && m_fieldPuyoArray[x - 1][y]->getType() == COLORPUYO && m_fieldPuyoArray[x - 1][y]->m_fallFlag == 0 && m_fieldPuyoArray[x - 1][y]->getColor() == m_fieldPuyoArray[x][y]->getColor()) {
		m_fieldPuyoArray[x - 1][y]->setLink(RIGHT);
		m_fieldPuyoArray[x][y]->setLink(LEFT);
	} else if (isPuyo(x - 1, y) && m_fieldPuyoArray[x - 1][y]->getType() == COLORPUYO && m_fieldPuyoArray[x - 1][y]->m_fallFlag != 0 && m_fieldPuyoArray[x - 1][y]->getColor() == m_fieldPuyoArray[x][y]->getColor()) {
		m_fieldPuyoArray[x - 1][y]->unsetLink(RIGHT);
		m_fieldPuyoArray[x][y]->unsetLink(LEFT);
	}
}

// End of phase 20
void Field::endFallPuyoPhase() const
{
	// Loop through puyos
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (isPuyo(i, j)) {
				m_fieldPuyoArray[i][j]->m_glow = false;
				// Check if any is bouncing falling or destroying
				if (m_fieldPuyoArray[i][j]->m_fallFlag != 0 || m_fieldPuyoArray[i][j]->m_bounceTimer != 0) // NOLINT(clang-diagnostic-float-equal)
					return;
			}
		}
	}

	// Play voice
	if (m_player->m_feverMode) {
		if (m_player->m_feverSuccess == 1) {
			m_player->m_characterVoices.feverSuccess.play(m_data);
		} else if (m_player->m_feverSuccess == 2) {
			m_player->m_characterVoices.feverFail.play(m_data);
		}
		m_player->m_feverSuccess = 0;
	}
	// Nothing is bouncing
	m_player->endPhase();
}

void Field::searchChain()
{
	// Destroy 14th and 15th row
	for (int i = 0; i < m_properties.gridX; i++) {
		if (isPuyo(i, m_properties.gridY - 1)) {
			delete m_fieldPuyoArray[i][m_properties.gridY - 1];
			m_fieldPuyoArray[i][m_properties.gridY - 1] = nullptr;
		}
		if (isPuyo(i, m_properties.gridY - 2)) {
			delete m_fieldPuyoArray[i][m_properties.gridY - 2];
			m_fieldPuyoArray[i][m_properties.gridY - 2] = nullptr;
		}
	}

	// Predict chain
	if (m_player->m_chain == 0) {
		m_player->m_predictedChain = 0;
		m_player->m_predictedChain = predictChain();
	}

	// Reset chain values
	m_player->m_foundChain = false;
	m_player->m_puyosPopped = 0;
	m_player->m_groupR = 0;
	m_player->m_groupG = 0;
	m_player->m_groupB = 0;
	m_player->m_groupY = 0;
	m_player->m_groupP = 0;
	m_player->m_point = 0;
	m_player->m_linkBonus = 0;
	m_player->m_bonus = 0;
	m_player->m_rememberMaxY = 0;
	m_player->m_rememberX = 0;

	unmark();

	// Find chain
	// Loop through field
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY - 3; j++) {
			if (findConnected(i, j, m_player->m_currentGame->m_currentRuleSet->m_puyoToClear, m_vector)) {
				m_player->m_foundChain = true;
				m_player->m_poppedChain = true;

				// Loop through connected puyo
				while (!m_vector.empty()) {
					const PosVectorInt pv = m_vector.back();
					m_vector.pop_back();

					// Set popped group
					if (m_fieldPuyoArray[pv.x][pv.y]->getType() == COLORPUYO) {
						switch (m_fieldPuyoArray[pv.x][pv.y]->getColor()) {
						default:
							break;
						case 0:
							m_player->m_groupR = 1;
							break;
						case 1:
							m_player->m_groupG = 1;
							break;
						case 2:
							m_player->m_groupB = 1;
							break;
						case 3:
							m_player->m_groupY = 1;
							break;
						case 4:
							m_player->m_groupP = 1;
							break;
						}
					}

					// Add to puyo count
					m_player->m_puyosPopped++;

					// Check neighbors to find nuisance
					if (isPuyo(pv.x, pv.y + 1) && pv.y + 1 != m_properties.gridY - 3) {
						m_fieldPuyoArray[pv.x][pv.y + 1]->neighborPop(this, false);
					}
					if (isPuyo(pv.x + 1, pv.y)) {
						m_fieldPuyoArray[pv.x + 1][pv.y]->neighborPop(this, false);
					}
					if (isPuyo(pv.x, pv.y - 1)) {
						m_fieldPuyoArray[pv.x][pv.y - 1]->neighborPop(this, false);
					}
					if (isPuyo(pv.x - 1, pv.y)) {
						m_fieldPuyoArray[pv.x - 1][pv.y]->neighborPop(this, false);
					}

					// Check if highest puyo
					if (pv.y > m_player->m_rememberMaxY) {
						m_player->m_rememberMaxY = pv.y;
						m_player->m_rememberX = pv.x;
					}

					// Pop puyo
					removePuyo(pv.x, pv.y);
					m_fieldPuyoArray[pv.x][pv.y] = nullptr;
				}

				// Add popped puyos to score
				m_player->m_point += m_player->m_puyosPopped * 10;

				// Add link bonus
				m_player->m_linkBonus += m_player->m_currentGame->m_currentRuleSet->getLinkBonus(m_player->m_puyosPopped);

				// Reset popped puyos
				m_player->m_puyosPopped = 0;
			}
		}
	}

	// Calculate score
	// (point and link bonus are calculated during loop)
	if (m_player->m_foundChain) {
		m_player->m_chain++;
	} else {
		// No chain: remove any bonus EQ
		m_player->m_bonusEq = false;
	}
	// Sum chain bonus and color bonus
	m_player->m_bonus += m_player->m_currentGame->m_currentRuleSet->getChainBonus(m_player);
	if (m_player->m_currentGame->m_settings->recording == RecordState::REPLAYING && m_player->m_currentGame->m_currentReplayVersion <= 1) {
		m_player->m_bonus += m_player->m_currentGame->m_currentRuleSet->getColorBonus(0); // Replay compatibility v1
	} else {
		m_player->m_bonus += m_player->m_currentGame->m_currentRuleSet->getColorBonus(m_player->m_groupR + m_player->m_groupG + m_player->m_groupB + m_player->m_groupY + m_player->m_groupP);
	}
	m_player->m_bonus += m_player->m_linkBonus;

	// Set bonus to 1 if zero (rule-set specific??)
	if (m_player->m_chain == 1 && m_player->m_bonus == 0) {
		m_player->m_bonus++;
	}

	if (m_player->m_chain > 0) {
		// Add score
		m_player->m_scoreVal += m_player->m_point * m_player->m_bonus;

		// Add current score
		m_player->m_currentScore += m_player->m_point * m_player->m_bonus;

		// Add dropBonus
		if (m_player->m_currentGame->m_currentRuleSet->m_addDropBonus) {
			// Cap drop bonus
			if (m_player->m_dropBonus > 300)
				m_player->m_dropBonus = 300;

			m_player->m_currentScore += m_player->m_dropBonus;
			m_player->m_dropBonus = 0;
		}

		// Show calculation
		m_player->setScoreCounterPB();
	}

	// Trigger all clear action here for tsu
	if (m_player->m_chain > 0 && m_player->m_allClear == 1) {
		m_player->m_currentGame->m_currentRuleSet->onAllClearPop(m_player);
	}

	// Set rule related variables
	if (m_player->m_chain > 0 && m_player->m_foundChain) {
		m_player->m_currentGame->m_currentRuleSet->onChain(m_player);
	}

	// End phase
	m_player->endPhase();
}

// Do pop animation for deleted puyos
void Field::popPuyoAnim()
{
	for (unsigned int i = 0; i < m_deletedPuyo.size(); i++) {
		m_deletedPuyo[i]->pop();

		// Check if it should be deleted
		if (m_deletedPuyo[i]->destroyPuyo()) {
			delete m_deletedPuyo[i];
			m_deletedPuyo.erase(std::remove(m_deletedPuyo.begin(), m_deletedPuyo.end(), m_deletedPuyo[i]), m_deletedPuyo.end());
		}
	}
}

// Other objects

// Create a particle at position
void Field::createParticle(const float x, const float y, const int color)
{
	m_particles.push_back(new Particle(
		x + static_cast<float>(getRandom(11)) - 5.f,
		y - static_cast<float>(m_properties.gridHeight) / 2.f + static_cast<float>(getRandom(11)) - 5,
		color,
		m_data));
}

// Create a particle at position
void Field::createParticleThrow(Puyo* p)
{
	if (!p) {
		return;
	}
	if (p->getType() == COLORPUYO) {
		m_particlesThrow.push_back(new ParticleThrow(p->spriteX(), p->spriteY() - static_cast<float>(m_properties.gridHeight) / 2.f, p->getColor(), m_data));
	} else if (p->getType() == NUISANCEPUYO) {
		m_particlesThrow.push_back(new ParticleThrow(p->spriteX(), p->spriteY() - static_cast<float>(m_properties.gridHeight) / 2.f, 6, m_data));
	}
}

// Move the particles
void Field::animateParticle()
{
	if (getParticleNumber() > 0) {
		for (int i = 0; i < getParticleNumber(); i++) {
			m_particles[i]->play();

			// Check if it needs to be deleted
			if (m_particles[i]->shouldDestroy()) {
				delete m_particles[i];
				m_particles.erase(std::remove(m_particles.begin(), m_particles.end(), m_particles[i]), m_particles.end());
			}
		}
	}

	// Same for thrown puyos
	for (size_t i = 0; i < m_particlesThrow.size(); i++) {
		m_particlesThrow[i]->play();

		// Check if it needs to be deleted
		if (m_particlesThrow[i]->shouldDestroy()) {
			delete m_particlesThrow[i];
			m_particlesThrow.erase(std::remove(m_particlesThrow.begin(), m_particlesThrow.end(), m_particlesThrow[i]), m_particlesThrow.end());
		}
	}
}

int Field::getParticleNumber() const
{
	return static_cast<int>(m_particles.size());
}

void Field::triggerGlow(PosVectorInt shadowPos[4], const int n, int colors[4])
{
	// Check empty
	for (int i = 0; i < n; i++) {
		// Placing on impossible spot
		if (!isEmpty(shadowPos[i].x, shadowPos[i].y)) {
			// Return immediately
			return;
		}

		// Out of bounds
		if (shadowPos[i].x > m_properties.gridX - 1 || shadowPos[i].x < 0 || shadowPos[i].y > m_properties.gridY - 1 || shadowPos[i].y < 0) {
			return;
		}
	}

	// Temporarily add puyos at the shadow positions
	for (int i = 0; i < n; i++) {
		if (isEmpty(shadowPos[i].x, shadowPos[i].y)) {
			addColorPuyo(shadowPos[i].x, shadowPos[i].y, colors[i]);
		} else {
			// Error
			shadowPos[i].x = -1;
			shadowPos[i].y = -1;

			m_player->m_currentGame->m_debug->log("Invalid shadow Puyo placement request",DebugMessageType::Error);
		}
	}

	// Check if puyos are connected at these positions
	unmark();

	// Un-glow all
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (isPuyo(i, j)) {
				m_fieldPuyoArray[i][j]->m_glow = false;
			}
		}
	}
	for (int i = 0; i < n; i++) {
		if (findConnected(shadowPos[i].x, shadowPos[i].y, m_player->m_currentGame->m_currentRuleSet->m_puyoToClear, m_vector)) {
			// Loop through connected puyo
			while (!m_vector.empty()) {
				const PosVectorInt pv = m_vector.back();
				m_vector.pop_back();
				// Set to glow
				m_fieldPuyoArray[pv.x][pv.y]->m_glow = true;
			}
		}
	}

	// Remove temporary puyo
	for (int i = 0; i < n; i++) {
		if (shadowPos[i].x >= 0 && shadowPos[i].y >= 0) {
			delete m_fieldPuyoArray[shadowPos[i].x][shadowPos[i].y];
			m_fieldPuyoArray[shadowPos[i].x][shadowPos[i].y] = nullptr;
		}
	}
}
int Field::virtualChain(PosVectorInt shadowPos[4], int n, int colors[4])
{
	// Check empty
	for (int i = 0; i < n; i++) {
		if (!isEmpty(shadowPos[i].x, shadowPos[i].y)) {
			// Return immediately
			return 0;
		}
	}

	// Check for any incorrect values
	for (int i = 0; i < n; i++) {
		if (shadowPos[i].y >= m_properties.gridY) {
			shadowPos[i].x = -1;
			shadowPos[i].y = -1;
		}
	}

	// Temporarily add puyos at the shadow positions
	for (int i = 0; i < n; i++) {
		if (isEmpty(shadowPos[i].x, shadowPos[i].y)) {
			addColorPuyo(shadowPos[i].x, shadowPos[i].y, colors[i]);
		} else {
			// Error
			shadowPos[i].x = -1;
			shadowPos[i].y = -1;
		}
	}

	// Check if puyos are connected at these positions
	unmark();

	// Predict chain
	const int predictedChain = predictChain();

	// Remove temporary puyo
	for (int i = 0; i < n; i++) {
		if (shadowPos[i].x >= 0 && shadowPos[i].y >= 0) {
			delete m_fieldPuyoArray[shadowPos[i].x][shadowPos[i].y];
			m_fieldPuyoArray[shadowPos[i].x][shadowPos[i].y] = nullptr;
		}
	}

	return predictedChain;
}

void Field::dropGarbage(const bool automatic, const int dropAmount)
{
	int lastX = -1;
	int lastY = -1;
	bool dropped = false;

	if (const int dropN = automatic ? m_player->m_activeGarbage->gq : dropAmount;
		dropN > 0 && !m_player->m_forgiveGarbage
		// forgiveGarbage is something that applies only to human players
		|| dropN > 0 && m_player->getPlayerType() == ONLINE) {
		// Play animation
		if (dropN >= 6 && dropN < 24) {
			m_player->m_characterAnimation.prepareAnimation("damage1");
		} else if (dropN >= 24) {
			m_player->m_characterAnimation.prepareAnimation("damage2");
		}

		m_player->m_garbageDropped = min(dropN, 30);

		// Reset nuisance drop pattern (doesn't affect legacy)
		m_player->resetNuisanceDropPattern();

		// Drop nuisance puyos
		for (int i = 0; i < m_player->m_garbageDropped; i++) {
			const int x = m_player->m_currentGame->m_legacyNuisanceDrop
				? nuisanceDropPattern(m_properties.gridX, m_player->m_garbageCycle)
				: m_player->nuisanceDropPattern();
			const int y = m_properties.gridY - 2;
			if (addNuisancePuyo(x, y, 1, i / m_properties.gridX)) {
				// Drop after creation
				const int newY = dropSingle(x, y);

				// Set fall target
				m_fieldPuyoArray[x][newY]->setFallTarget(newY);

				// Remember last one dropped
				lastX = x;
				lastY = newY;
			}

			// End iteration
			m_player->m_garbageCycle++;
		}

		// Mark if last nuisance for dropping sound
		if (lastX != -1 && lastY != -1) {
			m_fieldPuyoArray[lastX][lastY]->m_lastNuisance = true;
		}

		// Remove from GQ
		m_player->m_activeGarbage->gq -= min(dropN, 30);

		// Update tray
		m_player->updateTray(m_player->m_activeGarbage);

		dropped = true;
		// Send message
		// 0[g]1[garbage dropped]
		if (m_player->m_currentGame->m_connected && m_player->getPlayerType() == HUMAN) {
			char str[100];
			sprintf(str, "g|%i", m_player->m_garbageDropped);
			m_player->m_currentGame->m_network->sendToChannel(CHANNEL_GAME, str, m_player->m_currentGame->m_channelName.c_str());
		}
	}

	if (m_player->m_forgiveGarbage) {
		m_player->m_forgiveGarbage = false;
	}

	// Nothing dropped: send
	// 0["n"]
	if (dropped == false && m_player->m_currentGame->m_connected && m_player->getPlayerType() == HUMAN) {
		m_player->m_currentGame->m_network->sendToChannel(CHANNEL_GAME, "n", m_player->m_currentGame->m_channelName.c_str());
	}

	// Your garbage drop must be confirmed
	if (m_player->m_currentGame->m_connected && m_player->getPlayerType() == HUMAN) {
		for (const auto& player : m_player->m_currentGame->m_players) {
			if (player != m_player && player->m_active) {
				player->m_waitForConfirm++;
			}
		}
	}

	// End
	m_sweepFall = 0;
	if (automatic) {
		m_player->endPhase();
	}
}

void Field::loseDrop() const
{
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (isPuyo(i, j)) {
				unsetLinkAll(i, j);
				if (m_fieldPuyoArray[i][j]->accelerationY() < 0.1f) {
					m_fieldPuyoArray[i][j]->setAccelerationY(static_cast<float>(static_cast<int>(sqrt(i + 2.) * 12) % 10) / 2.0f);
				}
				m_fieldPuyoArray[i][j]->addAccelerationY(0.2f);
			}
		}
	}
}

// Drop a field of puyos
void Field::dropField(const std::string& fieldString)
{
	for (size_t i = 0; i < fieldString.size(); i++) {
		const int j = static_cast<int>(fieldString.size() - 1 - i);
		const int x = m_properties.gridX - 1 - static_cast<int>(i) % m_properties.gridX;
		const int y = m_properties.gridY - 2;
		if (std::string str = fieldString.substr(j, 1); toInt(str) > 0 && toInt(str) < 6) {
			// Color puyo
			if (addColorPuyo(x, y, toInt(str) - 1, 1, static_cast<int>(i / m_properties.gridX), static_cast<int>(i / m_properties.gridX))) {
				// Drop after creation
				const int newY = dropSingle(x, y);
				// Set fall target
				m_fieldPuyoArray[x][newY]->setFallTarget(newY);
			}
		} else if (toInt(str) == 6) {
			if (addNuisancePuyo(x, y, 1, static_cast<int>(i / m_properties.gridX))) {
				// Drop after creation
				const int newY = dropSingle(x, y);
				// Set fall target
				m_fieldPuyoArray[x][newY]->setFallTarget(newY);
			}
		}
	}
	m_sweepFall = 0;
}

void Field::setFieldFromString(const std::string& fieldString)
{
	// Clear field
	clearField();

	// Loop through puyos horizontally from x=0
	// - 0     = Empty
	// - 1 - 5 = Color puyo
	// - 6     = Nuisance puyo

	for (size_t i = 0; i < fieldString.size(); i++) {
		const int x = static_cast<int>(i % m_properties.gridX);
		const int y = static_cast<int>(i / m_properties.gridX);
		if (std::string str = fieldString.substr(i, 1); toInt(str) > 0 && toInt(str) < 6) {
			// Color puyo
			if (addColorPuyo(x, y, toInt(str) - 1, 1)) {
				// Drop after creation
				const int newY = dropSingle(x, y);
				// Set fall target
				m_fieldPuyoArray[x][newY]->setFallTarget(newY);
			}
		} else if (toInt(str) == 6) {
			if (addNuisancePuyo(x, y, 1)) {
				// Drop after creation
				const int newY = dropSingle(x, y);
				// Set fall target
				m_fieldPuyoArray[x][newY]->setFallTarget(newY);
			}
		}
	}
}

// Loop through field and create a string
std::string Field::getFieldString() const
{
	std::string out;
	for (int j = 0; j < m_properties.gridY; j++) {
		for (int i = 0; i < m_properties.gridX; i++) {
			if (isPuyo(i, j)) {
				if (m_fieldPuyoArray[i][j]->getType() == COLORPUYO)
					out += toString(m_fieldPuyoArray[i][j]->getColor() + 1);
				else if (m_fieldPuyoArray[i][j]->getType() == NUISANCEPUYO)
					out += "6";
			} else {
				out += "0";
			}
		}
	}

	// Trim zeroes
	int z = 0;
	for (size_t i = 0; i < out.length(); i++) {
		if (out[out.length() - 1 - i] != '0') {
			z = static_cast<int>(i);
			break;
		}
	}
	return out.substr(0, out.length() - z);
}

// Destroys entire field and shows an animation of "throwing away" puyo
void Field::throwAwayField()
{
	for (int i = 0; i < m_properties.gridX; i++) {
		for (int j = 0; j < m_properties.gridY; j++) {
			if (isPuyo(i, j)) {
				// Create new throw puyo
				createParticleThrow(m_fieldPuyoArray[i][j]);

				// Delete puyo
				delete m_fieldPuyoArray[i][j];
				m_fieldPuyoArray[i][j] = nullptr;
			}
		}
	}
}

}
