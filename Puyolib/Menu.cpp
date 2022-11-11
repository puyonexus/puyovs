#include "Menu.h"
#include "../PVS_ENet/PVS_Client.h"
#include "Game.h"

namespace ppvs {

Menu::Menu(Game* g)
{
	m_currentGame = g;
	m_data = g->m_data;
	m_background.setImage(nullptr);
	m_background.setScale(2 * 192, 336);
	m_background.setColor(0, 0, 0);
	m_background.setTransparency(0.5f);
	const FieldProp p = m_currentGame->m_players[0]->m_activeField->getProperties();
	m_background.setPosition(p.offsetX - 192.f / 2.f, p.offsetY - 336.f / 4.f);
	m_select = 0;

	m_option[0].setImage(m_data->imgMenu[0][0]);
	m_option[1].setImage(m_data->imgMenu[1][0]);
	m_option[2].setImage(m_data->imgMenu[2][0]);
	m_option[0].setPosition(-200, -200);
	m_option[1].setPosition(-200, -200);
	m_option[2].setPosition(-200, -200);
	m_disableRematch = false;
}

Menu::~Menu() = default;

void Menu::draw()
{
	m_background.draw(m_data->front);
	m_option[0].draw(m_data->front);
	m_option[1].draw(m_data->front);
	m_option[2].draw(m_data->front);
}

void Menu::play()
{
	const FieldProp p = m_currentGame->m_players[0]->m_activeField->getProperties();

	// Set option sprite
	for (int i = 0; i < 3; i++) {
		// Correct image
		if (m_select == i) {
			// Selected
			m_option[i].setImage(m_data->imgMenu[i][1]);
			m_option[i].setCenter();
			m_option[i].setScale(m_option[i].getScaleX() + (1 - m_option[i].getScaleX()) / 3.f);
		} else {
			// Not selected
			m_option[i].setImage(m_data->imgMenu[i][0]);
			m_option[i].setCenter();
			m_option[i].setScale(0.8f);
		}
		// Position
		m_option[i].setPosition(p.offsetX + p.centerX, p.offsetY + p.centerY / 2.f - 60.f + static_cast<float>(i) * 40.f);
		m_option[i].setColor(255, 255, 255);
	}

	// Enable/disable buttons
	if (!m_currentGame->m_settings->useCpuPlayers) {
		m_disableRematch = false;
		m_option[0].setTransparency(1);
		// Disable
		if (m_currentGame->m_currentGameStatus == GameStatus::WAITING
			|| m_currentGame->countBoundPlayers() < 2
			|| m_currentGame->m_rankedState > 0) {
			m_disableRematch = true;
			m_option[0].setTransparency(0.5);
			m_option[0].setImage(m_data->imgMenu[0][0]);
		}
	}

	// Set options
	Controller* controls = &(m_currentGame->m_players[0]->m_controls);
	if (controls->m_down == 1 && m_select < 2) {
		m_select++;
		m_data->snd.cursor.play(m_data);
	}
	if (controls->m_up == 1 && m_select > 0) {
		m_select--;
		m_data->snd.cursor.play(m_data);
	}
	// Choose
    if ((controls->m_a == 1 && m_currentGame->m_settings->swapABConfirm == false) || (controls->m_b == 1 && m_currentGame->m_settings->swapABConfirm == true)) {
		if (m_currentGame->m_settings->swapABConfirm == false) {
			controls->m_a++;
		} else {
			controls->m_b++;
		}
		if (!(m_select == 0 && m_disableRematch))
			m_data->snd.decide.play(m_data);
		if (m_select == 0) {
			// Rematch
			// Offline
			if (m_currentGame->m_settings->useCpuPlayers) {
				m_currentGame->m_menuSelect = 0;
				m_currentGame->resetPlayers();
				m_currentGame->m_currentGameStatus = GameStatus::PLAYING;
			} else {
				if (!m_disableRematch) {
					if (m_currentGame->m_connected) {
						// Find peers,
						std::string peers;

						// Set everyone in channel to active and add them to list
						for (const auto& player : m_currentGame->m_players) {
							if (!player->m_onlineName.empty()) {
								player->m_active = true;
								peers += "|" + toString(player->m_onlineId);
							}
						}

						// Propose new random seed
						m_currentGame->m_players[0]->m_proposedRandomSeed = getRandom(1000000);

						// Send rematch message
						// 0[rematch]1[random seed]2[wins]3....[player ids]
						if (!m_currentGame->m_channelName.empty()) {
							m_currentGame->m_network->sendToChannel(CHANNEL_GAME, "rematch|" + toString(m_currentGame->m_players[0]->m_proposedRandomSeed) + "|" + toString(m_currentGame->m_players[0]->m_wins) + peers, m_currentGame->m_channelName);
							m_currentGame->m_network->sendToServer(CHANNEL_MATCH, "accept");
						}

						// Set state to rematching
						m_currentGame->m_currentGameStatus = GameStatus::REMATCHING;

						// Set own rematch value
						m_currentGame->m_players[0]->m_rematch = true;
						m_currentGame->m_players[0]->m_rematchIcon.setVisible(true);
						m_currentGame->m_players[0]->m_rematchIconTimer = 0;

						// Change channel description
						m_currentGame->sendDescription();

						// Do normal stuff
						m_currentGame->m_menuSelect = 0;
						m_currentGame->loadMusic();
					}
				}
			}
		} else if (m_select == 1) {
			// Character select
			m_currentGame->m_menuSelect = 1;
			m_currentGame->m_charSelectMenu->prepare();
		} else if (m_select == 2) {
			// Quit
			if (m_currentGame->m_settings->rankedMatch && m_currentGame->countBoundPlayers() > 1) {
				// Cannot close
			} else {
				m_currentGame->m_runGame = false;
			}
		}

		// Reset selection
		m_select = 0;
	}

	// Disabled
	if (!m_currentGame->m_settings->startWithCharacterSelect) {
		m_option[1].setColor(128, 128, 128);
		if (controls->m_down == 1 && m_select == 1) {
			m_select++;
		}
		if (controls->m_up == 1 && m_select == 1) {
			m_select--;
		}
	}
}

}
