#include "CharacterSelect.h"
#include "../PVS_ENet/PVS_Client.h"
#include "DropPattern.h"
#include "Game.h"
#include <algorithm>
#include <cmath>

using namespace std;

namespace ppvs {

CharacterSelect::CharacterSelect(Game* g)
{
	m_currentGame = g;
	m_data = g->m_data;

	m_background.setImage(nullptr);
	m_background.setScale(2.f * 640.f, 480.f);
	m_background.setPosition(-640.f / 2.f, -480.f / 4.f);
	m_background.setColor(0.f, 0.f, 0.f);

	constexpr int height = 3;
	for (int i = 0; i < height; i++) {
		constexpr int width = 8;
		for (int j = 0; j < width; j++) {
			m_holder[i * width + j].setImage(g->m_data->imgCharHolder);
			m_holder[i * width + j].setCenter(0, 0);
			m_holder[i * width + j].setPosition(
				static_cast<float>(64 + j * 66),
				static_cast<float>(64 + i * 52));
			m_charSprite[i * width + j].setImage(g->m_data->front->loadImage(kFolderUserCharacter + m_currentGame->m_settings->characterSetup[m_order[i * width + j]] + "/icon.png"));
			m_charSprite[i * width + j].setCenter(0, 0);
			m_charSprite[i * width + j].setPosition(
				static_cast<float>(64 + j * 66 + 1),
				static_cast<float>(64 + i * 52 + 1));
		}
	}
}

CharacterSelect::~CharacterSelect()
{
	delete[] m_selectSprite;
	delete[] m_selectedCharacter;
	delete[] m_name;
	delete[] m_nameHolder;
	delete[] m_nameHolderNumber;
	delete[] m_dropSet;
	delete[] m_playerNumber;
	delete[] m_sel;
	delete[] m_madeChoice;
}

void CharacterSelect::draw()
{
	// Set colors
	for (auto& i : m_charSprite) {
		i.setColor(128.f, 128.f, 128.f);
	}

	for (int i = 0; i < m_numPlayers; i++) {
		m_charSprite[m_sel[i]].setColor(255.f, 255.f, 255.f);
	}

	// Draw the rest
	m_background.draw(m_data->front);

	for (auto& i : m_holder) {
		i.draw(m_data->front);
	}

	for (auto& i : m_charSprite) {
		i.draw(m_data->front);
	}

	for (int i = 0; i < m_numPlayers; i++) {
		m_selectSprite[i].draw(m_data->front);
		if (m_numPlayers <= 4) // Only draw characters if players>4
		{
			m_selectedCharacter[i].draw(m_data->front);
		}
		m_nameHolder[i].draw(m_data->front);
		m_name[i].draw(m_data->front);
		for (int j = 0; j < 16; j++) {
			m_dropSet[i * 16 + j].draw(m_data->front);
		}

		for (int j = 0; j < 3; j++) {
			m_playerNumber[i * 3 + j].draw(m_data->front);
			m_nameHolderNumber[i * 3 + j].draw(m_data->front);
		}
	}
}

void CharacterSelect::play()
{
	const int numPlayers = static_cast<int>(m_currentGame->m_players.size());

	if (m_timer != 0) {
		m_timer++;
	}

	if (m_timer <= 60 && m_timer > 0) {
		m_background.setTransparency(static_cast<float>(interpolate("linear", 0.0, 0.5, m_timer / 60.0)));
	}

	if (m_timer <= 80 && m_timer > 0) {
		constexpr int height = 3;
		for (int i = 0; i < height; i++) {
			constexpr int width = 8;
			for (int j = 0; j < width; j++) {
				const double tt = m_timer / 20.0 - (i * width + j) / 12.0;
				float move = static_cast<float>(interpolate("exponential", 1, 0, tt, -2, 1));
				if (move > 1) {
					move = 1;
				} else if (move < 0) {
					move = 0;
				}
				m_holder[i * width + j].setPosition(
					static_cast<float>(64 + j * 66) + 640.f * move,
					static_cast<float>(64 + i * 52));
				m_charSprite[i * width + j].setPosition(
					static_cast<float>(64 + j * 66) + 1.f + 640.f * move,
					static_cast<float>(64 + i * 52) + 1.f);
			}
		}
	}

	// Move in
	if (m_timer <= 160 && m_timer > 0) {
		for (int i = 0; i < numPlayers; i++) {
			// Normal display
			float posX = 640.f / static_cast<float>(numPlayers * 2) * static_cast<float>(i * 2 + 1);
			float posY = 480.f;

			// Display only name
			if (numPlayers > 4) {
				const int width = static_cast<int>(ceil(sqrt(static_cast<double>(numPlayers))));
				posX = 640.f / static_cast<float>(width * 2) * static_cast<float>(i % width * 2 + 1);
                posY = 480.f - static_cast<float>(i / width) * 128.f * m_scale; // NOLINT(bugprone-integer-division)
			}
			const double tt = static_cast<double>(m_timer) / 30.0 - static_cast<double>(i) * 1.0 / static_cast<double>(numPlayers);
			float move = static_cast<float>(interpolate("elastic", 1, 0, tt, -5, 0.5));
			if (move > -0.001f && move < 0.001f)
				move = 0;
			m_nameHolder[i].setPosition(posX, posY + 2.f + 320.f * move);
		}
	}

	if (m_timer == 80) {
		for (int i = 0; i < numPlayers; i++) {
			m_selectSprite[i].setVisible(true);
			if (i > 8) {
				m_playerNumber[i * 3 + 0].setVisible(true);
				m_nameHolderNumber[i * 3 + 0].setVisible(true);
			}
			m_playerNumber[i * 3 + 1].setVisible(true);
			m_playerNumber[i * 3 + 2].setVisible(true);
			m_name[i].setVisible(true);
			m_nameHolder[i].setVisible(true);
			m_nameHolderNumber[i * 3 + 1].setVisible(true);
			m_nameHolderNumber[i * 3 + 2].setVisible(true);
			m_selectedCharacter[i].setVisible(true);
			for (int j = 0; j < 16; j++) {
				m_dropSet[i * 16 + j].setVisible(true);
			}
		}
	}

	// Count players that made a choice
	int allChoice = 0;
	bool endNow = false; // Override: end character selection immediately
	for (int i = 0; i < numPlayers; i++) {
		if (m_madeChoice[i]) {
			allChoice++;
		}
	}

	if (m_timer > 40) {
		for (int i = 0; i < numPlayers; i++) {
			int currentPlayer = i;

			// ONLINE: don't care about CPU players
			if (m_currentGame->m_settings->useCpuPlayers) {
				// Move CPU with player 1
				if (m_currentGame->m_players[currentPlayer]->getPlayerType() == CPU && i == allChoice)
					currentPlayer = 0;

				// TEMP chance to cancel
				if (m_madeChoice[i]) {
                    if ((m_currentGame->m_players[currentPlayer]->m_controls.m_b == 1 && m_currentGame->m_settings->swapABConfirm == false) || (m_currentGame->m_players[currentPlayer]->m_controls.m_a == 1 && m_currentGame->m_settings->swapABConfirm == true)) {
						int j = i;

						// Check if CPU should be canceled first
						if (currentPlayer == 0) {
							for (int ii = 0; ii < numPlayers; ii++) {
								if (m_currentGame->m_players[ii]->getPlayerType() == CPU && m_madeChoice[ii]) {
									j = ii;
								}
							}
						}
						m_data->snd.cancel.play(m_data);
                        if (m_currentGame->m_settings->swapABConfirm == false) {
                            m_currentGame->m_players[currentPlayer]->m_controls.m_b++;
                        } else {
                            m_currentGame->m_players[currentPlayer]->m_controls.m_a++;
                        }
						m_madeChoice[j] = false;
						m_selectSprite[j].setVisible(true);
						if (j > 8) {
							m_playerNumber[j * 3 + 0].setVisible(true);
						}
						m_playerNumber[j * 3 + 1].setVisible(true);
						m_playerNumber[j * 3 + 2].setVisible(true);
					}
					continue;
				}
			}

			bool moved = false;
			int selX = m_sel[i] % 8;
			int selY = m_sel[i] / 8;
			if (m_currentGame->m_players[currentPlayer]->getPlayerType() != ONLINE) {
				if (m_currentGame->m_players[currentPlayer]->m_controls.m_right == 1 || m_currentGame->m_players[currentPlayer]->m_controls.m_right > 16 && m_currentGame->m_players[currentPlayer]->m_controls.m_right % 3 == 0) {
					selX++;
					moved = true;
				}
				if (m_currentGame->m_players[currentPlayer]->m_controls.m_left == 1 || m_currentGame->m_players[currentPlayer]->m_controls.m_left > 16 && m_currentGame->m_players[currentPlayer]->m_controls.m_left % 3 == 0) {
					selX--;
					moved = true;
				}
				if (m_currentGame->m_players[currentPlayer]->m_controls.m_down == 1 || m_currentGame->m_players[currentPlayer]->m_controls.m_down > 16 && m_currentGame->m_players[currentPlayer]->m_controls.m_down % 3 == 0) {
					selY++;
					moved = true;
				}
				if (m_currentGame->m_players[currentPlayer]->m_controls.m_up == 1 || m_currentGame->m_players[currentPlayer]->m_controls.m_up > 16 && m_currentGame->m_players[currentPlayer]->m_controls.m_up % 3 == 0) {
					selY--;
					moved = true;
				}
			}

			if (m_sel[i] < 0) {
				m_sel[i] += 24;
			}

			if (selX < 0) {
				selX += 8;
			}

			if (selY < 0) {
				selY += 3;
			}

			selX %= 8;
			selY %= 3;

			m_sel[i] = selY * 8 + selX;
			const int jj = m_sel[i] % 8;
			const int ii = m_sel[i] / 8;

			// Normal display
			float posX = 640.f / static_cast<float>(numPlayers * 2) * static_cast<float>(i * 2 + 1);
			float posY = 480.f;

			// Display only name
			if (numPlayers > 4) {
				const int width = static_cast<int>(ceil(sqrt(static_cast<double>(numPlayers))));
				posX = 640.f / static_cast<float>(width * 2) * static_cast<float>(i % width * 2 + 1);
                posY = 480.f - static_cast<float>(i / width) * 128.f * m_scale; // NOLINT(bugprone-integer-division)
			}

			// Make choice
            if ((m_currentGame->m_players[currentPlayer]->m_controls.m_a == 1 && m_currentGame->m_settings->swapABConfirm == false && m_timer > 80) || (m_currentGame->m_players[currentPlayer]->m_controls.m_b == 1 && m_currentGame->m_settings->swapABConfirm == true && m_timer > 80)) {
				m_data->snd.decide.play(m_data);
				m_madeChoice[i] = true;
				m_currentGame->m_players[i]->setCharacter(m_order[m_sel[i]]);
				m_currentGame->m_players[i]->m_characterVoices.choose.play(m_data);
				m_selectSprite[i].setVisible(false);
				m_playerNumber[i * 3 + 0].setVisible(false);
				m_playerNumber[i * 3 + 1].setVisible(false);
				m_playerNumber[i * 3 + 2].setVisible(false);

				// ONLINE: send message
				if (m_currentGame->m_connected) {
					m_currentGame->m_network->sendToChannel(CHANNEL_GAME, std::string("choice|") + toString(static_cast<int>(m_order[m_sel[i]])), m_currentGame->m_channelName);
					endNow = true;
				}
			}

			if (moved) {
				m_data->snd.cursor.play(m_data);
				if (m_currentGame->m_settings->useCharacterField) {
					m_currentGame->m_players[i]->setFieldImage(m_order[m_sel[i]]);
				}

				m_selectSprite[i].setPosition(
					static_cast<float>(64 + jj * 66),
					static_cast<float>(64 + ii * 52));

				m_selectedCharacter[i].setImage(m_data->imgCharSelect[static_cast<unsigned char>(m_order[m_sel[i]])]);
				m_selectedCharacter[i].setSubRect(0, 0, 256, 256);
				m_selectedCharacter[i].setCenterBottom();
				m_selectedCharacter[i].setPosition(posX, posY - 38 * m_scale);
				m_selectedCharacter[i].setScale(m_scale);
				if (m_selectedCharacter[i].getImage() == nullptr) {
					m_selectedCharacter[i].setVisible(false);
				}

				m_name[i].setImage(m_data->imgCharName[static_cast<unsigned char>(m_order[m_sel[i]])]);
				m_name[i].setCenterBottom();
				m_name[i].setPosition(posX, posY);
				m_name[i].setScale(m_scale);

				setDropset(
					static_cast<int>(posX),
					static_cast<int>(posY - 48 * m_scale),
					i);

				// ONLINE: send message
				if (m_currentGame->m_connected) {
					m_currentGame->m_network->sendToChannel(CHANNEL_GAME, std::string("select|") + toString(static_cast<int>(m_order[m_sel[i]])), m_currentGame->m_channelName);
				}
			}

			// Player number
			const float xx = m_playerNumber[static_cast<ptrdiff_t>(i) * 3].getX() + (m_selectSprite[i].getX() - m_playerNumber[static_cast<ptrdiff_t>(i) * 3].getX()) / 3.f + 2.f;
			const float yy = m_playerNumber[static_cast<ptrdiff_t>(i) * 3].getY() + (m_selectSprite[i].getY() - m_playerNumber[static_cast<ptrdiff_t>(i) * 3].getY()) / 3.f + 6.f;
			m_playerNumber[i * 3 + 0].setPosition(xx, yy + 1.f * static_cast<float>(sin(m_data->globalTimer / 30.0 + i * 10.0)));
			m_playerNumber[i * 3 + 1].setPosition(xx + 10, yy + 1.f * static_cast<float>(sin(m_data->globalTimer / 30.0 + i * 10.0)));
			m_playerNumber[i * 3 + 2].setPosition(xx + 20, yy + 1.f * static_cast<float>(sin(m_data->globalTimer / 30.0 + i * 10.0)));
		}
	}

	// All choices made
	if (allChoice == numPlayers && m_timer > 0 || endNow) {
		// Fade out
		m_timer = -120;
	}
	if (m_timer < 0 && m_timer > -60) {
		const float t = static_cast<float>(m_timer + 60) / 60.0f; // From 0 to 1
		m_background.setTransparency(
			static_cast<float>(interpolate("linear", 0.0, 0.5, -m_timer / 60.0)));

		// Set invisible
		for (int i = 0; i < numPlayers; i++) {
			m_selectSprite[i].setVisible(false);
			m_selectedCharacter[i].setTransparency(1 - t);
			m_name[i].setTransparency(1 - t);
			m_playerNumber[i * 3 + 0].setTransparency(1 - t);
			m_playerNumber[i * 3 + 1].setTransparency(1 - t);
			m_playerNumber[i * 3 + 2].setTransparency(1 - t);
			m_nameHolderNumber[i * 3 + 0].setTransparency(1 - t);
			m_nameHolderNumber[i * 3 + 1].setTransparency(1 - t);
			m_nameHolderNumber[i * 3 + 2].setTransparency(1 - t);
			for (int j = 0; j < 16; j++) {
				m_dropSet[i * 16 + j].setTransparency(1 - t);
			}
			m_nameHolder[i].setTransparency(1 - t);
		}
		for (int i = 0; i < 24; i++) {
			m_holder[i].setTransparency(1 - t);
			m_charSprite[i].setTransparency(1 - t);
		}
	}
	if (m_timer < 0 && m_timer > -80) {
		constexpr int height = 3;
		for (int i = 0; i < height; i++) {
			constexpr int width = 8;
			for (int j = 0; j < width; j++) {
				const double tt = static_cast<double>(-m_timer) / 20.0 - static_cast<double>(i * width + j) / 12.0;
				float move = static_cast<float>(interpolate("exponential", 1, 0, tt, -2, 1));

				if (move > 1) {
					move = 1;
				} else if (move < 0) {
					move = 0;
				}

				m_holder[i * width + j].setPosition(
					static_cast<float>(64 + j * 66) + 640.f * move,
					static_cast<float>(64 + i * 52));
				m_charSprite[i * width + j].setPosition(
					static_cast<float>(64 + j * 66 + 1) + 640.f * move,
					static_cast<float>(64 + i * 52 + 1));
			}
		}
	}
	if (m_timer < 0 && m_timer >= -160) {
		for (int i = 0; i < numPlayers; i++) {
			// Normal display
			float posX = 640.f / static_cast<float>(numPlayers * 2) * static_cast<float>(i * 2 + 1);
			float posY = 480.f;

			// Display only name
			if (numPlayers > 4) {
				const int width = static_cast<int>(ceil(sqrt(static_cast<double>(numPlayers))));
				posX = 640.f / static_cast<float>(width * 2) * static_cast<float>(i % width * 2 + 1);
                posY = 480.f - static_cast<float>(i / width) * 128.f * m_scale; // NOLINT(bugprone-integer-division)
			}
			const double tt = static_cast<double>(-m_timer) / 30.0 - static_cast<double>(i) / static_cast<double>(numPlayers);
			float move = static_cast<float>(interpolate("elastic", 1, 0, tt, -5, 0.5));
			if (move > -0.001f && move < 0.001f) {
				move = 0;
			}
			m_nameHolder[i].setPosition(posX, posY + 2.f + 320.f * move);
		}
	}
	if (m_timer == -1) {
		for (int i = 0; i < numPlayers; i++) {
			m_selectSprite[i].setVisible(false);
			if (i > 8) {
				m_playerNumber[i * 3 + 0].setVisible(false);
				m_nameHolderNumber[i * 3 + 0].setVisible(false);
			}
			m_playerNumber[i * 3 + 1].setVisible(false);
			m_playerNumber[i * 3 + 2].setVisible(false);
			m_name[i].setVisible(false);
			m_nameHolderNumber[i * 3 + 1].setVisible(false);
			m_nameHolderNumber[i * 3 + 2].setVisible(false);
			m_selectedCharacter[i].setVisible(false);
			for (int j = 0; j < 16; j++) {
				m_dropSet[i * 16 + j].setVisible(false);
			}
		}
		end();
	}
}

// End of character select
void CharacterSelect::end()
{
	m_timer = 0;
	const size_t numPlayers = m_currentGame->m_players.size();

	if (!m_currentGame->m_settings->useCpuPlayers) {
		// ONLINE: do nothing, just go back to menu
		m_currentGame->m_menuSelect = 2;
	} else {
		// This state is only at start
		if (m_currentGame->m_players[0]->m_loseWin == LoseWinState::NOWIN) {
			// Set all players to pick colors, or play game now
			if (m_currentGame->m_settings->pickColors) {
				for (size_t i = 0; i < numPlayers; i++) {
					m_currentGame->m_players[i]->m_currentPhase = Phase::PICKCOLORS;
				}
			} else {
				// Play game right now
				for (size_t i = 0; i < numPlayers; i++) {
					m_currentGame->m_players[i]->m_currentPhase = Phase::GETREADY;
				}
				m_currentGame->m_readyGoObj.prepareAnimation("readygo");
				m_data->matchTimer = 0;
			}
			m_currentGame->m_menuSelect = 0;
			m_currentGame->loadMusic();
		} else {
			// Return to menu
			m_currentGame->m_menuSelect = 2;
		}
	}
}

void CharacterSelect::prepare()
{
	m_timer = 1;
	delete[] m_selectSprite;
	delete[] m_selectedCharacter;
	delete[] m_name;
	delete[] m_nameHolder;
	delete[] m_nameHolderNumber;
	delete[] m_dropSet;
	delete[] m_playerNumber;
	delete[] m_sel;
	delete[] m_madeChoice;

	m_numPlayers = static_cast<int>(m_currentGame->m_players.size());
	m_scale = static_cast<float>(min(2.0 / m_numPlayers, 1.0));
	m_selectSprite = new Sprite[m_numPlayers];
	m_selectedCharacter = new Sprite[m_numPlayers];
	m_name = new Sprite[m_numPlayers];
	m_nameHolder = new Sprite[m_numPlayers];
	m_nameHolderNumber = new Sprite[static_cast<size_t>(m_numPlayers) * 3];
	m_dropSet = new Sprite[static_cast<size_t>(m_numPlayers) * 16];
	m_playerNumber = new Sprite[static_cast<size_t>(m_numPlayers) * 3];

	m_sel = new int[m_numPlayers];
	m_madeChoice = new bool[m_numPlayers];

	// Objects for every player
	for (int i = 0; i < m_numPlayers; i++) {
		// Release buttons for cpu
		if (m_currentGame->m_players[i]->getPlayerType() == CPU) {
			m_currentGame->m_players[i]->m_controls.release();
		}

		if (m_firstStart) {
			m_sel[i] = i % 24;
		} else {
			m_sel[i] = findCurrentCharacter(i);
		}
		const int jj = m_sel[i] % 8;
		const int ii = m_sel[i] / 8;
		m_madeChoice[i] = false;

		// Set initial field image
		if (m_currentGame->m_settings->useCharacterField) {
			m_currentGame->m_players[i]->setFieldImage(m_order[m_sel[i]]);
		}

		// Normal display
		float posX = 640.f / static_cast<float>(m_numPlayers * 2) * static_cast<float>(i * 2 + 1);
		float posY = 480.f;

		// Display only name
		if (m_numPlayers > 4) {
			const int width = static_cast<int>(ceil(sqrt(static_cast<double>(m_numPlayers))));
			m_scale = 2.0f / static_cast<float>(width);
			posX = 640.f / static_cast<float>(width * 2) * static_cast<float>(i % width * 2 + 1);
            posY = 480.f - static_cast<float>(i / width) * 128.f * m_scale; // NOLINT(bugprone-integer-division)
		}

		m_selectSprite[i].setImage(m_data->imgPlayerCharSelect);
		m_selectSprite[i].setCenter(1, 1);
		m_selectSprite[i].setPosition(
			static_cast<float>(64 + jj * 66),
			static_cast<float>(64 + ii * 52));
		m_selectSprite[i].setVisible(false);

		m_selectedCharacter[i].setImage(m_data->imgSelect[static_cast<unsigned char>(m_order[m_sel[i]])]);
		m_selectedCharacter[i].setCenterBottom();
		m_selectedCharacter[i].setScale(m_scale);
		m_selectedCharacter[i].setPosition(posX, posY - 38.f * m_scale);
		m_selectedCharacter[i].setVisible(false);
		if (m_selectedCharacter[i].getImage() == nullptr)
			m_selectedCharacter[i].setVisible(false);

		for (int j = 0; j < 16; j++) {
			m_dropSet[i * 16 + j].setImage(m_data->imgDropSet);
			m_dropSet[i * 16 + j].setVisible(false);
		}

		for (int j = 0; j < 3; j++) {
			m_playerNumber[i * 3 + j].setImage(m_data->imgPlayerNumber);
			m_nameHolderNumber[i * 3 + j].setImage(m_data->imgPlayerNumber);
		}
		// Number
		m_playerNumber[i * 3 + 0].setSubRect((i + 1) / 10 * 24, 0, 24, 32);
		m_playerNumber[i * 3 + 0].setCenterBottom();
		m_playerNumber[i * 3 + 0].setScale(0.5);
		m_playerNumber[i * 3 + 1].setSubRect((i + 1) % 10 * 24, 0, 24, 32);
		m_playerNumber[i * 3 + 1].setCenterBottom();
		m_playerNumber[i * 3 + 1].setScale(0.5);
		m_playerNumber[i * 3 + 2].setSubRect(240, 0, 24, 32);
		m_playerNumber[i * 3 + 2].setCenterBottom();
		m_playerNumber[i * 3 + 2].setScale(0.5);
		m_nameHolderNumber[i * 3 + 0].setSubRect((i + 1) / 10 * 24, 0, 24, 32);
		m_nameHolderNumber[i * 3 + 0].setCenterBottom();
		m_nameHolderNumber[i * 3 + 0].setScale(m_scale);
		m_nameHolderNumber[i * 3 + 1].setSubRect((i + 1) % 10 * 24, 0, 24, 32);
		m_nameHolderNumber[i * 3 + 1].setCenterBottom();
		m_nameHolderNumber[i * 3 + 1].setScale(m_scale);
		m_nameHolderNumber[i * 3 + 2].setSubRect(240, 0, 24, 32);
		m_nameHolderNumber[i * 3 + 2].setCenterBottom();
		m_nameHolderNumber[i * 3 + 2].setScale(m_scale);
		if (i < 9) {
			m_nameHolderNumber[i * 3 + 0].setVisible(false);
		}
		m_playerNumber[i * 3 + 0].setPosition(static_cast<float>(64 + jj * 66 + 4), static_cast<float>(64 + ii * 52 + 6));
		m_playerNumber[i * 3 + 1].setPosition(static_cast<float>(64 + jj * 66 + 4), static_cast<float>(64 + ii * 52 + 6));
		m_playerNumber[i * 3 + 2].setPosition(static_cast<float>(64 + jj * 66 + 4), static_cast<float>(64 + ii * 52 + 6));
		m_playerNumber[i * 3 + 0].setVisible(false);
		m_playerNumber[i * 3 + 1].setVisible(false);
		m_playerNumber[i * 3 + 2].setVisible(false);
		m_nameHolderNumber[i * 3 + 0].setPosition(posX + 00 * m_scale, posY - 70 * m_scale);
		m_nameHolderNumber[i * 3 + 1].setPosition(posX + 20 * m_scale, posY - 70 * m_scale);
		m_nameHolderNumber[i * 3 + 2].setPosition(posX + 40 * m_scale, posY - 70 * m_scale);
		m_nameHolderNumber[i * 3 + 0].setVisible(false);
		m_nameHolderNumber[i * 3 + 1].setVisible(false);
		m_nameHolderNumber[i * 3 + 2].setVisible(false);
		m_name[i].setImage(m_data->imgCharName[static_cast<int>(m_order[m_sel[i]])]);
		m_name[i].setCenterBottom();
		m_name[i].setPosition(posX, posY);
		m_name[i].setScale(m_scale);
		m_name[i].setVisible(false);
		m_nameHolder[i].setImage(m_data->imgNameHolder);
		m_nameHolder[i].setCenterBottom();
		m_nameHolder[i].setPosition(posX, posY);
		m_nameHolder[i].setScale(m_scale);
		setDropset(static_cast<int>(posX), static_cast<int>(posY - 48.f * m_scale), i);
		// Set visible
		for (int k = 0; k < m_numPlayers; k++) {
			m_selectedCharacter[k].setTransparency(1);
			m_name[k].setTransparency(1);
			m_playerNumber[k * 3 + 0].setTransparency(1);
			m_playerNumber[k * 3 + 1].setTransparency(1);
			m_playerNumber[k * 3 + 2].setTransparency(1);
			m_nameHolderNumber[k * 3 + 0].setTransparency(1);
			m_nameHolderNumber[k * 3 + 1].setTransparency(1);
			m_nameHolderNumber[k * 3 + 2].setTransparency(1);
			for (int j = 0; j < 16; j++) {
				m_dropSet[k * 16 + j].setTransparency(1);
			}
			m_nameHolder[k].setTransparency(1);
		}
		for (auto& sprite : m_holder) {
			sprite.setTransparency(1);
		}
		for (auto& sprite : m_charSprite) {
			sprite.setTransparency(1);
		}
	}
	m_firstStart = false;
}

void CharacterSelect::setDropset(const int x, const int y, const int pl)
{
	const PuyoCharacter pc = m_order[m_sel[pl]];
	float xx = -128;

	for (int j = 0; j < 16; j++) {
		const MovePuyoType mpt = getFromDropPattern(pc, j);
		m_dropSet[pl * 16 + j].setPosition(static_cast<float>(x) + xx * m_scale, static_cast<float>(y));
		m_dropSet[pl * 16 + j].setScale(m_scale);
		switch (mpt) {
		case MovePuyoType::DOUBLET:
			m_dropSet[pl * 16 + j].setSubRect(0, 0, 16, 24);
			m_dropSet[pl * 16 + j].setCenter(0, 24);
			xx += 10;
			break;
		case MovePuyoType::TRIPLET:
			m_dropSet[pl * 16 + j].setSubRect(16, 0, 24, 24);
			m_dropSet[pl * 16 + j].setCenter(0, 24);
			xx += 18;
			break;
		case MovePuyoType::TRIPLET_R:
			m_dropSet[pl * 16 + j].setSubRect(40, 0, 24, 24);
			m_dropSet[pl * 16 + j].setCenter(0, 24);
			xx += 18;
			break;
		case MovePuyoType::QUADRUPLET:
			m_dropSet[pl * 16 + j].setSubRect(64, 0, 24, 24);
			m_dropSet[pl * 16 + j].setCenter(0, 24);
			xx += 18;
			break;
		case MovePuyoType::BIG:
			m_dropSet[pl * 16 + j].setSubRect(88, 0, 24, 24);
			m_dropSet[pl * 16 + j].setCenter(0, 24);
			xx += 18;
			break;
		}
	}
}

int CharacterSelect::findCurrentCharacter(const int i) const
{
	// Find selection
	const PuyoCharacter pc = m_currentGame->m_players[i]->getCharacter();
	for (int j = 0; j < 24; j++) {
		if (m_order[j] == pc) {
			return j;
		}
	}
	return -1;
}

// Sets animation for setting a character, does not actually set the player character if character select is not active!
void CharacterSelect::setCharacter(const int playerNum, const int selection, const bool choice)
{
	if (playerNum >= static_cast<int>(m_currentGame->m_players.size())) {
		return;
	}

	if (m_timer > 40) {
		const int i = playerNum;

		// Find selection from character order
		int s = 0;
		for (int j = 0; j < 24; j++) {
			if (selection == static_cast<int>(m_order[j])) {
				s = j;
				break;
			}
		}
		m_sel[i] = s;
		const int jj = m_sel[i] % 8;
		const int ii = m_sel[i] / 8;

		// Normal display
		float posX = 640.f / static_cast<float>(m_numPlayers * 2) * static_cast<float>(i * 2 + 1);
		float posY = 480.f;

		// Display only name
		if (m_numPlayers > 4) {
			const int width = static_cast<int>(ceil(sqrt(static_cast<double>(m_numPlayers))));
			posX = 640.f / static_cast<float>((width * 2) * (i % width * 2 + 1));
            posY = 480.f - static_cast<float>(i / width) * 128.f * m_scale; // NOLINT(bugprone-integer-division)
		}

		// Move
		if (!choice) {
			m_data->snd.cursor.play(m_data);
			m_madeChoice[i] = false;
			if (m_currentGame->m_settings->useCharacterField) {
				m_currentGame->m_players[i]->setFieldImage(m_order[m_sel[i]]);
			}

			m_selectSprite[i].setPosition(static_cast<float>(64 + jj * 66), static_cast<float>(64 + ii * 52));

			m_selectedCharacter[i].setImage(m_data->front->loadImage(kFolderUserCharacter + m_currentGame->m_settings->characterSetup[m_order[m_sel[i]]] + "/select.png"));
			m_selectedCharacter[i].setSubRect(0, 0, 256, 256);
			m_selectedCharacter[i].setCenterBottom();
			m_selectedCharacter[i].setPosition(posX, posY - 38 * m_scale);
			m_selectedCharacter[i].setScale(m_scale);
			if (m_selectedCharacter[i].getImage() == nullptr) {
				m_selectedCharacter[i].setVisible(false);
			}

			m_name[i].setImage(m_data->imgCharName[static_cast<unsigned char>(m_order[m_sel[i]])]);
			m_name[i].setCenterBottom();
			m_name[i].setPosition(posX, posY);
			m_name[i].setScale(m_scale);

			setDropset(static_cast<int>(posX), static_cast<int>(posY - 48 * m_scale), i);
		} else {
			// Made choice
			m_data->snd.decide.play(m_data);
			m_madeChoice[i] = true;
			m_currentGame->m_players[i]->setCharacter(m_order[m_sel[i]]);
			m_currentGame->m_players[i]->m_characterVoices.choose.play(m_data);
			m_selectSprite[i].setVisible(false);
			m_playerNumber[i * 3 + 0].setVisible(false);
			m_playerNumber[i * 3 + 1].setVisible(false);
			m_playerNumber[i * 3 + 2].setVisible(false);
		}
	}
}

}
