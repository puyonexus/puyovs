#include <algorithm>
#include "characterSelect.h"
#include "game.h"
#include "../PVS_ENet/PVS_Client.h"
using namespace std;

namespace ppvs
{

characterSelect::characterSelect(game* g)
{
	currentgame = g;
	data = g->data;

	background.setImage(nullptr);
	background.setScale(2.f * 640.f, 480.f);
	background.setPosition(-640.f / 2.f, -480.f / 4.f);
	background.setColor(0.f, 0.f, 0.f);

	constexpr int height = 3;
	for (int i = 0; i < height; i++)
	{
		constexpr int width = 8;
		for (int j = 0; j < width; j++)
		{
			holder[i * width + j].setImage(g->data->imgCharHolder);
			holder[i * width + j].setCenter(0, 0);
			holder[i * width + j].setPosition(
				static_cast<float>(64 + j * 66),
				static_cast<float>(64 + i * 52)
			);
			charSprite[i * width + j].setImage(g->data->front->loadImage(folder_user_character + currentgame->settings->characterSetup[order[i * width + j]] + "/icon.png"));
			charSprite[i * width + j].setCenter(0, 0);
			charSprite[i * width + j].setPosition(
				static_cast<float>(64 + j * 66 + 1),
				static_cast<float>(64 + i * 52 + 1)
			);
		}
	}
}

characterSelect::~characterSelect()
{
	delete[] selectSprite;
	delete[] selectedCharacter;
	delete[] name;
	delete[] nameHolder;
	delete[] nameHolderNumber;
	delete[] dropset;
	delete[] playernumber;
	delete[] sel;
	delete[] madeChoice;
}

void characterSelect::draw()
{
	// Set colors
	for (auto& i : charSprite)
	{
		i.setColor(128.f, 128.f, 128.f);
	}

	for (int i = 0; i < Nplayers; i++)
	{
		charSprite[sel[i]].setColor(255.f, 255.f, 255.f);
	}

	// Draw the rest
	background.draw(data->front);

	for (auto& i : holder)
	{
		i.draw(data->front);
	}

	for (auto& i : charSprite)
	{
		i.draw(data->front);
	}

	for (int i = 0; i < Nplayers; i++)
	{
		selectSprite[i].draw(data->front);
		if (Nplayers <= 4) // Only draw characters if players>4
		{
			selectedCharacter[i].draw(data->front);
		}
		nameHolder[i].draw(data->front);
		name[i].draw(data->front);
		for (int j = 0; j < 16; j++)
		{
			dropset[i * 16 + j].draw(data->front);
		}

		for (int j = 0; j < 3; j++)
		{
			playernumber[i * 3 + j].draw(data->front);
			nameHolderNumber[i * 3 + j].draw(data->front);
		}
	}

}

void characterSelect::play()
{
	const int Nplayers = static_cast<int>(currentgame->players.size());

	if (timer != 0)
	{
		timer++;
	}

	if (timer <= 60 && timer > 0)
	{
		background.setTransparency(static_cast<float>(interpolate("linear", 0.0, 0.5, timer / 60.0)));
	}

	if (timer <= 80 && timer > 0)
	{
		constexpr int height = 3;
		for (int i = 0; i < height; i++)
		{
			constexpr int width = 8;
			for (int j = 0; j < width; j++)
			{
				const double tt = timer / 20.0 - (i * width + j) / 12.0;
				float move = static_cast<float>(interpolate("exponential", 1, 0, tt, -2, 1));
				if (move > 1)
				{
					move = 1;
				}
				else if (move < 0)
				{
					move = 0;
				}
				holder[i * width + j].setPosition(
					static_cast<float>(64 + j * 66) + 640.f * move,
					static_cast<float>(64 + i * 52)
				);
				charSprite[i * width + j].setPosition(
					static_cast<float>(64 + j * 66) + 1.f + 640.f * move,
					static_cast<float>(64 + i * 52) + 1.f);
			}
		}
	}

	// Move in
	if (timer <= 160 && timer > 0)
	{
		for (int i = 0; i < Nplayers; i++)
		{
			// Normal display
			float posX = 640.f / static_cast<float>(Nplayers * 2) * static_cast<float>(i * 2 + 1);
			float posY = 480.f;

			// Display only name
			if (Nplayers > 4)
			{
				const int width = static_cast<int>(ceil(sqrt(static_cast<double>(Nplayers))));
				posX = 640.f / static_cast<float>(width * 2) * static_cast<float>(i % width * 2 + 1);
				posY = 480.f - static_cast<float>(i / width) * 128.f * scale;
			}
			double tt = static_cast<double>(timer) / 30.0 - static_cast<double>(i) * 1.0 / static_cast<double>(Nplayers);
			float move = static_cast<float>(interpolate("elastic", 1, 0, tt, -5, 0.5));
			if (move > -0.001f && move < 0.001f) move = 0;
			nameHolder[i].setPosition(posX, posY + 2.f + 320.f * move);
		}
	}

	if (timer == 80)
	{
		for (int i = 0; i < Nplayers; i++)
		{
			selectSprite[i].setVisible(true);
			if (i > 8)
			{
				playernumber[i * 3 + 0].setVisible(true);
				nameHolderNumber[i * 3 + 0].setVisible(true);
			}
			playernumber[i * 3 + 1].setVisible(true);
			playernumber[i * 3 + 2].setVisible(true);
			name[i].setVisible(true);
			nameHolder[i].setVisible(true);
			nameHolderNumber[i * 3 + 1].setVisible(true);
			nameHolderNumber[i * 3 + 2].setVisible(true);
			selectedCharacter[i].setVisible(true);
			for (int j = 0; j < 16; j++)
			{
				dropset[i * 16 + j].setVisible(true);
			}
		}
	}

	// Count players that made a choice
	int allChoice = 0;
	bool endnow = false; // Override: end character selection immediately
	for (int i = 0; i < Nplayers; i++)
	{
		if (madeChoice[i])
		{
			allChoice++;
		}
	}

	if (timer > 40)
	{
		for (int i = 0; i < Nplayers; i++)
		{
			int currentplayer = i;

			// ONLINE: don't care about CPU players
			if (currentgame->settings->useCPUplayers)
			{
				// Move CPU with player 1
				if (currentgame->players[currentplayer]->getPlayerType() == CPU && i == allChoice)
					currentplayer = 0;

				// TEMP chance to cancel
				if (madeChoice[i])
				{
					if (currentgame->players[currentplayer]->controls.B == 1)
					{
						int j = i;

						// Check if CPU should be canceled first
						if (currentplayer == 0)
						{
							for (int ii = 0; ii < Nplayers; ii++)
							{
								if (currentgame->players[ii]->getPlayerType() == CPU && madeChoice[ii])
								{
									j = ii;
								}
							}
						}
						data->snd.cancel.Play(data);
						currentgame->players[currentplayer]->controls.B++;
						madeChoice[j] = false;
						selectSprite[j].setVisible(true);
						if (j > 8)
						{
							playernumber[j * 3 + 0].setVisible(true);
						}
						playernumber[j * 3 + 1].setVisible(true);
						playernumber[j * 3 + 2].setVisible(true);

					}
					continue;
				}
			}

			bool moved = false;
			int selX = sel[i] % 8;
			int selY = sel[i] / 8;
			if (currentgame->players[currentplayer]->getPlayerType() != ONLINE)
			{
				if (currentgame->players[currentplayer]->controls.Right == 1 ||
					currentgame->players[currentplayer]->controls.Right > 16 &&
					currentgame->players[currentplayer]->controls.Right % 3 == 0)
				{
					selX++;
					moved = true;
				}
				if (currentgame->players[currentplayer]->controls.Left == 1 ||
					currentgame->players[currentplayer]->controls.Left > 16 &&
					currentgame->players[currentplayer]->controls.Left % 3 == 0)
				{
					selX--;
					moved = true;
				}
				if (currentgame->players[currentplayer]->controls.Down == 1 ||
					currentgame->players[currentplayer]->controls.Down > 16 &&
					currentgame->players[currentplayer]->controls.Down % 3 == 0)
				{
					selY++;
					moved = true;
				}
				if (currentgame->players[currentplayer]->controls.Up == 1 ||
					currentgame->players[currentplayer]->controls.Up > 16 &&
					currentgame->players[currentplayer]->controls.Up % 3 == 0)
				{
					selY--;
					moved = true;
				}
			}

			if (sel[i] < 0)
			{
				sel[i] += 24;
			}

			if (selX < 0)
			{
				selX += 8;
			}

			if (selY < 0)
			{
				selY += 3;
			}

			selX %= 8;
			selY %= 3;

			sel[i] = selY * 8 + selX;
			const int jj = sel[i] % 8;
			const int ii = sel[i] / 8;

			// Normal display
			float posX = 640.f / static_cast<float>(Nplayers * 2) * static_cast<float>(i * 2 + 1);
			float posY = 480.f;

			// Display only name
			if (Nplayers > 4)
			{
				const int width = static_cast<int>(ceil(sqrt(static_cast<double>(Nplayers))));
				posX = 640.f / static_cast<float>(width * 2) * static_cast<float>(i % width * 2 + 1);
				posY = 480.f - static_cast<float>(i / width) * 128.f * scale;
			}

			// Make choice
			if (currentgame->players[currentplayer]->controls.A == 1 && timer > 80)
			{
				data->snd.decide.Play(data);
				madeChoice[i] = true;
				currentgame->players[i]->setCharacter(order[sel[i]]);
				currentgame->players[i]->characterVoices.choose.Play(data);
				selectSprite[i].setVisible(false);
				playernumber[i * 3 + 0].setVisible(false);
				playernumber[i * 3 + 1].setVisible(false);
				playernumber[i * 3 + 2].setVisible(false);

				// ONLINE: send message
				if (currentgame->connected)
				{
					currentgame->network->sendToChannel(CHANNEL_GAME, std::string("choice|") + to_string((int)order[sel[i]]), currentgame->channelName);
					endnow = true;
				}
			}

			if (moved)
			{
				data->snd.cursor.Play(data);
				if (currentgame->settings->useCharacterField)
				{
					currentgame->players[i]->setFieldImage(order[sel[i]]);
				}

				selectSprite[i].setPosition(
					static_cast<float>(64 + jj * 66),
					static_cast<float>(64 + ii * 52)
				);

				selectedCharacter[i].setImage(data->imgCharSelect[static_cast<unsigned char>(order[sel[i]])]);
				selectedCharacter[i].setSubRect(0, 0, 256, 256);
				selectedCharacter[i].setCenterBottom();
				selectedCharacter[i].setPosition(posX, posY - 38 * scale);
				selectedCharacter[i].setScale(scale);
				if (selectedCharacter[i].getImage() == nullptr)
				{
					selectedCharacter[i].setVisible(false);
				}

				name[i].setImage(data->imgCharName[static_cast<unsigned char>(order[sel[i]])]);
				name[i].setCenterBottom();
				name[i].setPosition(posX, posY);
				name[i].setScale(scale);

				setDropset(
					static_cast<int>(posX),
					static_cast<int>(posY - 48 * scale),
					i
				);

				// ONLINE: send message
				if (currentgame->connected)
				{
					currentgame->network->sendToChannel(CHANNEL_GAME, std::string("select|") + to_string((int)order[sel[i]]), currentgame->channelName);
				}
			}

			// Player number
			float xx = playernumber[i * 3].getX() + (selectSprite[i].getX() - playernumber[i * 3].getX()) / 3.f + 2.f;
			float yy = playernumber[i * 3].getY() + (selectSprite[i].getY() - playernumber[i * 3].getY()) / 3.f + 6.f;
			playernumber[i * 3 + 0].setPosition(xx, yy + 1.f * static_cast<float>(sin(data->globalTimer / 30.0 + i * 10.0)));
			playernumber[i * 3 + 1].setPosition(xx + 10, yy + 1.f * static_cast<float>(sin(data->globalTimer / 30.0 + i * 10.0)));
			playernumber[i * 3 + 2].setPosition(xx + 20, yy + 1.f * static_cast<float>(sin(data->globalTimer / 30.0 + i * 10.0)));
		}
	}

	// All choices made
	if (allChoice == Nplayers && timer > 0 || endnow)
	{
		// Fade out
		timer = -120;
	}
	if (timer<0 && timer>-60)
	{
		const float t = static_cast<float>(timer + 60) / 60.0f; // From 0 to 1
		background.setTransparency(
			static_cast<float>(interpolate("linear", 0.0, 0.5, -timer / 60.0))
		);

		// Set invisible
		for (int i = 0; i < Nplayers; i++)
		{
			selectSprite[i].setVisible(false);
			selectedCharacter[i].setTransparency(1 - t);
			name[i].setTransparency(1 - t);
			playernumber[i * 3 + 0].setTransparency(1 - t);
			playernumber[i * 3 + 1].setTransparency(1 - t);
			playernumber[i * 3 + 2].setTransparency(1 - t);
			nameHolderNumber[i * 3 + 0].setTransparency(1 - t);
			nameHolderNumber[i * 3 + 1].setTransparency(1 - t);
			nameHolderNumber[i * 3 + 2].setTransparency(1 - t);
			for (int j = 0; j < 16; j++)
			{
				dropset[i * 16 + j].setTransparency(1 - t);
			}
			nameHolder[i].setTransparency(1 - t);
		}
		for (int i = 0; i < 24; i++)
		{
			holder[i].setTransparency(1 - t);
			charSprite[i].setTransparency(1 - t);
		}
	}
	if (timer<0 && timer>-80)
	{
		constexpr int height = 3;
		for (int i = 0; i < height; i++)
		{
			constexpr int width = 8;
			for (int j = 0; j < width; j++)
			{
				double tt = static_cast<double>(-timer) / 20.0 - static_cast<double>(i * width + j) / 12.0;
				float move = static_cast<float>(interpolate("exponential", 1, 0, tt, -2, 1));

				if (move > 1)
				{
					move = 1;
				}
				else if (move < 0)
				{
					move = 0;
				}

				holder[i * width + j].setPosition(
					static_cast<float>(64 + j * 66) + 640.f * move,
					static_cast<float>(64 + i * 52)
				);
				charSprite[i * width + j].setPosition(
					static_cast<float>(64 + j * 66 + 1) + 640.f * move,
					static_cast<float>(64 + i * 52 + 1)
				);
			}
		}
	}
	if (timer < 0 && timer >= -160)
	{
		for (int i = 0; i < Nplayers; i++)
		{
			// Normal display
			float posX = 640.f / static_cast<float>(Nplayers * 2) * static_cast<float>(i * 2 + 1);
			float posY = 480.f;

			// Display only name
			if (Nplayers > 4)
			{
				const int width = static_cast<int>(ceil(sqrt(static_cast<double>(Nplayers))));
				posX = 640.f / static_cast<float>(width * 2) * static_cast<float>(i % width * 2 + 1);
				posY = 480.f - static_cast<float>(i / width) * 128.f * scale;
			}
			const double tt = static_cast<double>(-timer) / 30.0 - static_cast<double>(i) / static_cast<double>(Nplayers);
			float move = static_cast<float>(interpolate("elastic", 1, 0, tt, -5, 0.5));
			if (move > -0.001f && move < 0.001f)
			{
				move = 0;
			}
			nameHolder[i].setPosition(posX, posY + 2.f + 320.f * move);
		}
	}
	if (timer == -1)
	{
		for (int i = 0; i < Nplayers; i++)
		{
			selectSprite[i].setVisible(false);
			if (i > 8)
			{
				playernumber[i * 3 + 0].setVisible(false);
				nameHolderNumber[i * 3 + 0].setVisible(false);
			}
			playernumber[i * 3 + 1].setVisible(false);
			playernumber[i * 3 + 2].setVisible(false);
			name[i].setVisible(false);
			nameHolderNumber[i * 3 + 1].setVisible(false);
			nameHolderNumber[i * 3 + 2].setVisible(false);
			selectedCharacter[i].setVisible(false);
			for (int j = 0; j < 16; j++)
			{
				dropset[i * 16 + j].setVisible(false);
			}
		}
		end();
	}

}

// End of character select
void characterSelect::end()
{
	timer = 0;
	size_t Nplayers = currentgame->players.size();

	if (!currentgame->settings->useCPUplayers)
	{
		// ONLINE: do nothing, just go back to menu
		currentgame->menuSelect = 2;
	}
	else
	{
		// This state is only at start
		if (currentgame->players[0]->losewin == NOWIN)
		{
			// Set all players to pick colors, or play game now
			if (currentgame->settings->pickColors)
			{
				for (int i = 0; i < Nplayers; i++)
				{
					currentgame->players[i]->currentphase = PICKCOLORS;
				}
			}
			else
			{
				// Play game right now
				for (int i = 0; i < Nplayers; i++)
				{
					currentgame->players[i]->currentphase = GETREADY;
				}
				currentgame->readyGoObj.prepareAnimation("readygo");
				data->matchTimer = 0;
			}
			currentgame->menuSelect = 0;
			currentgame->loadMusic();
		}
		else
		{
			// Return to menu
			currentgame->menuSelect = 2;
		}
	}

}

void characterSelect::prepare()
{
	timer = 1;
	delete[] selectSprite;
	delete[] selectedCharacter;
	delete[] name;
	delete[] nameHolder;
	delete[] nameHolderNumber;
	delete[] dropset;
	delete[] playernumber;
	delete[] sel;
	delete[] madeChoice;

	Nplayers = static_cast<int>(currentgame->players.size());
	scale = static_cast<float>(min(2.0 / Nplayers, 1.0));
	selectSprite = new sprite[Nplayers];
	selectedCharacter = new sprite[Nplayers];
	name = new sprite[Nplayers];
	nameHolder = new sprite[Nplayers];
	nameHolderNumber = new sprite[Nplayers * 3];
	dropset = new sprite[Nplayers * 16];
	playernumber = new sprite[Nplayers * 3];

	sel = new int[Nplayers];
	madeChoice = new bool[Nplayers];

	// Objects for every player
	for (int i = 0; i < Nplayers; i++)
	{
		// Release buttons for cpu
		if (currentgame->players[i]->getPlayerType() == CPU)
		{
			currentgame->players[i]->controls.release();
		}

		if (firstStart)
		{
			sel[i] = i % 24;
		}
		else
		{
			sel[i] = findCurrentCharacter(i);
		}
		const int jj = sel[i] % 8;
		const int ii = sel[i] / 8;
		madeChoice[i] = false;

		// Set initial field image
		if (currentgame->settings->useCharacterField)
		{
			currentgame->players[i]->setFieldImage(order[sel[i]]);
		}

		// Normal display
		float posX = 640.f / static_cast<float>(Nplayers * 2) * static_cast<float>(i * 2 + 1);
		float posY = 480.f;

		// Display only name
		if (Nplayers > 4)
		{
			const int width = static_cast<int>(ceil(sqrt(static_cast<double>(Nplayers))));
			scale = 2.0f / static_cast<float>(width);
			posX = 640.f / static_cast<float>(width * 2) * static_cast<float>(i % width * 2 + 1);
			posY = 480.f - static_cast<float>(i / width) * 128.f * scale;
		}

		selectSprite[i].setImage(data->imgPlayerCharSelect);
		selectSprite[i].setCenter(1, 1);
		selectSprite[i].setPosition(
			static_cast<float>(64 + jj * 66),
			static_cast<float>(64 + ii * 52)
		);
		selectSprite[i].setVisible(false);

		selectedCharacter[i].setImage(data->imgSelect[static_cast<unsigned char>(order[sel[i]])]);
		selectedCharacter[i].setCenterBottom();
		selectedCharacter[i].setScale(scale);
		selectedCharacter[i].setPosition(posX, posY - 38.f * scale);
		selectedCharacter[i].setVisible(false);
		if (selectedCharacter[i].getImage() == nullptr)
			selectedCharacter[i].setVisible(false);

		for (int j = 0; j < 16; j++)
		{
			dropset[i * 16 + j].setImage(data->imgDropset);
			dropset[i * 16 + j].setVisible(false);
		}

		for (int j = 0; j < 3; j++)
		{
			playernumber[i * 3 + j].setImage(data->imgPlayerNumber);
			nameHolderNumber[i * 3 + j].setImage(data->imgPlayerNumber);
		}
		// Number
		playernumber[i * 3 + 0].setSubRect((i + 1) / 10 * 24, 0, 24, 32);
		playernumber[i * 3 + 0].setCenterBottom();
		playernumber[i * 3 + 0].setScale(0.5);
		playernumber[i * 3 + 1].setSubRect((i + 1) % 10 * 24, 0, 24, 32);
		playernumber[i * 3 + 1].setCenterBottom();
		playernumber[i * 3 + 1].setScale(0.5);
		playernumber[i * 3 + 2].setSubRect(240, 0, 24, 32);
		playernumber[i * 3 + 2].setCenterBottom();
		playernumber[i * 3 + 2].setScale(0.5);
		nameHolderNumber[i * 3 + 0].setSubRect((i + 1) / 10 * 24, 0, 24, 32);
		nameHolderNumber[i * 3 + 0].setCenterBottom();
		nameHolderNumber[i * 3 + 0].setScale(scale);
		nameHolderNumber[i * 3 + 1].setSubRect((i + 1) % 10 * 24, 0, 24, 32);
		nameHolderNumber[i * 3 + 1].setCenterBottom();
		nameHolderNumber[i * 3 + 1].setScale(scale);
		nameHolderNumber[i * 3 + 2].setSubRect(240, 0, 24, 32);
		nameHolderNumber[i * 3 + 2].setCenterBottom();
		nameHolderNumber[i * 3 + 2].setScale(scale);
		if (i < 9)
		{
			nameHolderNumber[i * 3 + 0].setVisible(false);
		}
		playernumber[i * 3 + 0].setPosition(static_cast<float>(64 + jj * 66 + 4), static_cast<float>(64 + ii * 52 + 6));
		playernumber[i * 3 + 1].setPosition(static_cast<float>(64 + jj * 66 + 4), static_cast<float>(64 + ii * 52 + 6));
		playernumber[i * 3 + 2].setPosition(static_cast<float>(64 + jj * 66 + 4), static_cast<float>(64 + ii * 52 + 6));
		playernumber[i * 3 + 0].setVisible(false);
		playernumber[i * 3 + 1].setVisible(false);
		playernumber[i * 3 + 2].setVisible(false);
		nameHolderNumber[i * 3 + 0].setPosition(posX + 00 * scale, posY - 70 * scale);
		nameHolderNumber[i * 3 + 1].setPosition(posX + 20 * scale, posY - 70 * scale);
		nameHolderNumber[i * 3 + 2].setPosition(posX + 40 * scale, posY - 70 * scale);
		nameHolderNumber[i * 3 + 0].setVisible(false);
		nameHolderNumber[i * 3 + 1].setVisible(false);
		nameHolderNumber[i * 3 + 2].setVisible(false);
		name[i].setImage(data->imgCharName[(int)order[sel[i]]]);
		name[i].setCenterBottom();
		name[i].setPosition(posX, posY);
		name[i].setScale(scale);
		name[i].setVisible(false);
		nameHolder[i].setImage(data->imgNameHolder);
		nameHolder[i].setCenterBottom();
		nameHolder[i].setPosition(posX, posY);
		nameHolder[i].setScale(scale);
		setDropset(static_cast<int>(posX), static_cast<int>(posY - 48.f * scale), i);
		// Set visible
		for (int k = 0; k < Nplayers; k++)
		{
			selectedCharacter[k].setTransparency(1);
			name[k].setTransparency(1);
			playernumber[k * 3 + 0].setTransparency(1);
			playernumber[k * 3 + 1].setTransparency(1);
			playernumber[k * 3 + 2].setTransparency(1);
			nameHolderNumber[k * 3 + 0].setTransparency(1);
			nameHolderNumber[k * 3 + 1].setTransparency(1);
			nameHolderNumber[k * 3 + 2].setTransparency(1);
			for (int j = 0; j < 16; j++)
			{
				dropset[k * 16 + j].setTransparency(1);
			}
			nameHolder[k].setTransparency(1);
		}
		for (auto& i : holder)
		{
			i.setTransparency(1);
		}
		for (auto& i : charSprite)
		{
			i.setTransparency(1);
		}
	}
	firstStart = false;

}

void characterSelect::setDropset(int x, int y, int pl)
{
	puyoCharacter pc = order[sel[pl]];
	float xx = -128;

	for (int j = 0; j < 16; j++)
	{
		movePuyoType mpt = getFromDropPattern(pc, j);
		dropset[pl * 16 + j].setPosition(static_cast<float>(x) + xx * scale, static_cast<float>(y));
		dropset[pl * 16 + j].setScale(scale);
		switch (mpt)
		{
		case DOUBLET:
			dropset[pl * 16 + j].setSubRect(0, 0, 16, 24);
			dropset[pl * 16 + j].setCenter(0, 24);
			xx += 10;
			break;
		case TRIPLET:
			dropset[pl * 16 + j].setSubRect(16, 0, 24, 24);
			dropset[pl * 16 + j].setCenter(0, 24);
			xx += 18;
			break;
		case TRIPLETR:
			dropset[pl * 16 + j].setSubRect(40, 0, 24, 24);
			dropset[pl * 16 + j].setCenter(0, 24);
			xx += 18;
			break;
		case QUADRUPLET:
			dropset[pl * 16 + j].setSubRect(64, 0, 24, 24);
			dropset[pl * 16 + j].setCenter(0, 24);
			xx += 18;
			break;
		case BIG:
			dropset[pl * 16 + j].setSubRect(88, 0, 24, 24);
			dropset[pl * 16 + j].setCenter(0, 24);
			xx += 18;
			break;
		}
	}

}

int characterSelect::findCurrentCharacter(int i)
{
	// Find selection
	const puyoCharacter pc = currentgame->players[i]->getCharacter();
	for (int j = 0; j < 24; j++)
	{
		if (order[j] == pc)
		{
			return j;
		}
	}
	return -1;
}

// Sets animation for setting a character, does not actually set the player character if charactersleect is not active!
void characterSelect::setCharacter(int playernum, int selection, bool choice)
{
	if (playernum >= static_cast<int>(currentgame->players.size()))
	{
		return;
	}

	if (timer > 40)
	{
		const int i = playernum;
		// Find selection from character order
		int s = 0;
		for (int j = 0; j < 24; j++)
		{
			if (selection == static_cast<int>(order[j]))
			{
				s = j; break;
			}
		}
		sel[i] = s;
		const int jj = sel[i] % 8;
		const int ii = sel[i] / 8;

		// Normal display
		float posX = 640.f / static_cast<float>(Nplayers * 2) * static_cast<float>(i * 2 + 1);
		float posY = 480.f;

		// Display only name
		if (Nplayers > 4)
		{
			const int width = static_cast<int>(ceil(sqrt(static_cast<double>(Nplayers))));
			posX = 640.f / (width * 2) * (i % width * 2 + 1);
			posY = 480.f - static_cast<float>(i / width) * 128.f * scale;
		}

		// Move
		if (!choice)
		{
			data->snd.cursor.Play(data);
			madeChoice[i] = false;
			if (currentgame->settings->useCharacterField)
			{
				currentgame->players[i]->setFieldImage(order[sel[i]]);
			}

			selectSprite[i].setPosition(static_cast<float>(64 + jj * 66), static_cast<float>(64 + ii * 52));

			selectedCharacter[i].setImage(data->front->loadImage(folder_user_character + currentgame->settings->characterSetup[order[sel[i]]] + "/select.png"));
			selectedCharacter[i].setSubRect(0, 0, 256, 256);
			selectedCharacter[i].setCenterBottom();
			selectedCharacter[i].setPosition(posX, posY - 38 * scale);
			selectedCharacter[i].setScale(scale);
			if (selectedCharacter[i].getImage() == nullptr)
			{
				selectedCharacter[i].setVisible(false);
			}

			name[i].setImage(data->imgCharName[static_cast<unsigned char>(order[sel[i]])]);
			name[i].setCenterBottom();
			name[i].setPosition(posX, posY);
			name[i].setScale(scale);

			setDropset(static_cast<int>(posX), static_cast<int>(posY - 48 * scale), i);
		}
		else
		{
			// Made choice
			data->snd.decide.Play(data);
			madeChoice[i] = true;
			currentgame->players[i]->setCharacter(order[sel[i]]);
			currentgame->players[i]->characterVoices.choose.Play(data);
			selectSprite[i].setVisible(false);
			playernumber[i * 3 + 0].setVisible(false);
			playernumber[i * 3 + 1].setVisible(false);
			playernumber[i * 3 + 2].setVisible(false);
		}

	}
}

}
