#include "gamemanager.h"
#include "../PVS_ENet/PVS_Client.h"
#include "../Puyolib/Game.h"
#include "../Puyolib/global.h"
#include "chatwindow.h"
#include "common.h"
#include "gameaudio.h"
#include "gamewidgetgl.h"
#include "netclient.h"
#include "settings.h"
#include <QApplication>
#include <QMessageBox>

volatile bool loopEnabled;

GameManager::GameManager(NetClient* network, QObject* parent)
	: QObject(parent)
	, network(network)
{
	connect(network, SIGNAL(channelJoined(QString, NetPeerList)),
		SLOT(channelJoined(QString, NetPeerList)));
	connect(network, SIGNAL(peerJoinedChannel(QString, QString)),
		SLOT(peerJoinedChannel(QString, QString)));
	connect(network, SIGNAL(peerPartedChannel(QString, QString)),
		SLOT(peerPartedChannel(QString, QString)));
	connect(network, SIGNAL(channelMessageReceived(QString, uchar, QString, QString)),
		SLOT(channelMessageReceived(QString, uchar, QString, QString)));
	connect(network, SIGNAL(peerChannelMessageReceived(QString, uchar, QString, QString)),
		SLOT(peerChannelMessageReceived(QString, uchar, QString, QString)));
	connect(network, SIGNAL(peerStatusReceived(QString, QString, uchar)),
		SLOT(peerStatusReceived(QString, QString, uchar)));
	connect(network, SIGNAL(rankedMatchMessageReceived(QString)),
		SLOT(rankedMatchmessageReceived(QString)));
	connect(&pvsApp->settings(), SIGNAL(saved()), SLOT(updateAllControls()));

	audio = new GameAudio;
}

GameManager::~GameManager()
{
	delete audio;
	loopEnabled = false;
	emit exiting();
}

bool GameManager::closeAll()
{
	if (games.empty())
		return false;

	// I'm a little afraid of deleting the games list, so I'll "disconnect" the games for now
	for (int i = 0; i < games.count(); i++) {
		games[i]->game()->m_connected = false;
	}
	return true;
}

void GameManager::addGame(GameWidget* game)
{
	updateControls(game);
	games.append(game);

	connect(game, SIGNAL(exiting(GameWidget*)), SLOT(gameDestroyed(GameWidget*)));
	connect(this, SIGNAL(exiting()), game, SLOT(close()));
}

ppvs::RuleSetInfo GameManager::createRules()
{
	Settings& settings = pvsApp->settings();
	ppvs::RuleSetInfo rs;

	// Base rules
	unsigned int ruleId = settings.integer("rules", "baserules", 0);
	switch (ruleId) {
	case 0:
		rs.setRules(ppvs::Rules::TSU_ONLINE);
		break;
	case 1:
		rs.setRules(ppvs::Rules::FEVER_ONLINE);
		break;
	case 2:
		rs.setRules(ppvs::Rules::FEVER15_ONLINE);
		break;
	case 3:
		rs.setRules(ppvs::Rules::ENDLESSFEVERVS_ONLINE);
		break;
	}

	rs.numPlayers = settings.integer("rules", "numplayers", 2);

	// Defaults?
	rs.custom = !settings.boolean("rules", "default", true);
	if (!rs.custom)
		return rs;

	rs.marginTime = settings.integer("rules", "margintime", 192);
	rs.targetPoint = settings.integer("rules", "targetpoints", 70);
	rs.initialFeverCount = settings.integer("rules", "initialfevercount", 0);
	rs.puyoToClear = settings.integer("rules", "puyotoclear", 4);
	rs.requiredChain = settings.integer("rules", "requiredchain", 0);
	rs.feverPower = settings.integer("rules", "feverpower", 192);
	rs.quickDrop = settings.boolean("rules", "quickdrop", false);

	if (settings.boolean("rules", "colorselect", false))
		rs.colors = 0;
	else
		rs.colors = settings.integer("rules", "colors", 4);

	return rs;
}

GameWidget* GameManager::createGame(const QString& rules, const QString& roomName, bool spectating)
{
	ppvs::RuleSetInfo rs;
	readRulesetString(rules, &rs);

	return createGame(new ppvs::GameSettings(rs), roomName, spectating);
}

GameWidget* GameManager::createGame(ppvs::GameSettings* gs, const QString& roomName, bool spectating, bool replay)
{
	Settings& settings = pvsApp->settings();
	NetChannelProxy* proxy = nullptr;

	gs->background = settings.string("custom", "background", "Forest").toUtf8().data();
	gs->puyo = settings.string("custom", "puyo", "Default").toUtf8().data();
	gs->sfx = settings.string("custom", "sound", "Default").toUtf8().data();
	gs->useCharacterField = settings.boolean("custom", "characterfield", true);

	gs->numPlayers = gs->ruleSetInfo.numPlayers;
	gs->numHumans = spectating || replay ? 0 : 1;
	gs->spectating = spectating || replay;

	if (replay) {
		gs->recording = ppvs::RecordState::REPLAYING;
		gs->pickColors = false;
		gs->showNames = settings.integer("launcher", "hidenames", 0);
	}
	QStringList characters(settings.charMap());
	for (int i = 0; i < characters.count(); i++) {
		ppvs::PuyoCharacter ch = ppvs::PuyoCharacter(i);
		gs->characterSetup[ch] = characters.at(i).toStdString();
	}

	// Set default character
	gs->defaultPuyoCharacter = ppvs::PuyoCharacter(settings.integer("account", "defaultcharacter", 2));

	// Load controls
	gs->controls["1a"] = settings.string("controlsp1", "a", "x").toStdString();
	gs->controls["1b"] = settings.string("controlsp1", "b", "z").toStdString();
	gs->controls["1up"] = settings.string("controlsp1", "up", "up").toStdString();
	gs->controls["1down"] = settings.string("controlsp1", "down", "down").toStdString();
	gs->controls["1left"] = settings.string("controlsp1", "left", "left").toStdString();
	gs->controls["1right"] = settings.string("controlsp1", "right", "right").toStdString();
	gs->controls["1start"] = settings.string("controlsp1", "start", "return").toStdString();

	gs->playMusic = settings.boolean("launcher", "enablemusic", true);
	gs->playSound = settings.boolean("launcher", "enablesound", true);

	ppvs::Game* game = new ppvs::Game(gs);

	game->m_translatableStrings.waitingForPlayer = tr("Waiting for player...", "Messages:GameWaitingForPlayer").toStdString();
	game->m_translatableStrings.disconnected = tr("Player disconnected.", "Messages:GameDisconnected").toStdString();
	game->m_translatableStrings.rankedWaiting = tr("Please press the Start button\n to find a new opponent. (default: Enter button)", "Messages:GameRankedWaiting").toStdString();
	game->m_translatableStrings.rankedSearching = tr("Searching for opponent...", "Messages:GameRankedSearching").toStdString();

	if (!roomName.isNull() || gs->rankedMatch) {
		game->m_channelName = roomName.toStdString();
		game->m_network = network->client();
		game->m_forceStatusText = spectating;

		if (settings.boolean("launcher", "savereplays", true))
			gs->recording = ppvs::RecordState::RECORDING;

		proxy = new NetChannelProxy(roomName, network);
	} else {
		game->m_channelName = std::string();
		game->m_network = nullptr;
		if (!replay) {
			gs->startWithCharacterSelect = true;
			gs->useCpuPlayers = true;
		} else {
			gs->startWithCharacterSelect = false;
			gs->useCpuPlayers = false;
		}
	}

	if (gs->rankedMatch) {
		network->sendMessageToServer(SUBCHANNEL_SERVERREQ_MATCH,
			QString("new|") + QString("%1|").arg(PVSVERSION) + QString(gs->ruleSetInfo.ruleSetType == ppvs::Rules::TSU_ONLINE ? "0" : "1"));
	}

	GameWidget* widget = new GameWidgetGL(game, proxy, audio, static_cast<QWidget*>(parent()));
	addGame(widget);

	return widget;
}

GameWidget* GameManager::findGame(const QString& roomName) const
{
	std::string roomNameStd = roomName.toStdString();
	for (int i = games.count() - 1; i >= 0; --i) {
		if (roomNameStd == games.at(i)->game()->m_channelName)
			return games.at(i);
	}

	return nullptr;
}

void GameManager::gameDestroyed(GameWidget* game)
{
	// Ranked match: tell server
	network->sendMessageToServer(SUBCHANNEL_SERVERREQ_MATCH, "quit");

	games.removeOne(game);

	if (games.isEmpty()) {
	}
}

bool GameManager::rankedMatch() const
{
	for (int i = 0; i < games.count(); i++) {
		if (games.at(i)->game()->m_settings->rankedMatch)
			return true;
	}
	return false;
}

void GameManager::channelJoined(QString channel, NetPeerList peers) const
{
	ppvs::Game* game = nullptr;
	GameWidget* widget = nullptr;

	if (!getGame(channel, game, widget))
		return;

	// Connected
	game->m_connected = true;

	// Count players (status==1)
	int first = 1, players = 0, waiting = 0;
	PVS_Client* client = network->client();
	for (int i = peers.count() - 1; i >= 0; --i) {
		char status = peers.at(i).status;

		if (status == 1)
			players++;
		else if (status == 0)
			waiting++;
	}

	// Attempt to join match
	if (game->m_settings->spectating == false) {
		// First person to join room
		if (players + waiting == 1) {
			client->changeStatus(channel.toUtf8().data(), 1);
			// Able to press rematch
			game->m_currentGameStatus = GameStatus::IDLE;
		}
		// There is room to join: just claim your spot
		else if (players + waiting <= game->m_settings->numPlayers) {
			client->changeStatus(channel.toUtf8().data(), 1);
		}
		// Room is full: leave channel or join as spectator? (let's do the former)
		else if (players + waiting > game->m_settings->numPlayers) {
			client->changeStatus(channel.toUtf8().data(), 2);

			// Why would one set a local variable just before returning?
			// Only one way to find out...
			//   The reason: if we want to change the player into a spectator instead of kicking him out
			//   then this variable should start at 0

			QMessageBox* ms = new QMessageBox(QMessageBox::Critical, widget->windowTitle(),
				tr("Room is full.", "Messages:JoinError"), QMessageBox::Ok);
			ms->show();

			// Exit game.
			delete widget;
			return;
		}
	}
	// Player wants to spectate
	else {
		client->changeStatus(channel.toUtf8().data(), 2);
		first = 0;
	}

	// Bind players
	int playernum = 0;
	for (int i = 0; i < peers.count(); i++) {
		NetPeer peer = peers.at(i);
		if (peer.status == 1) {
			for (unsigned j = first; j < game->m_players.size(); j++) {
				if (game->m_players[j]->m_onlineName.compare("") == 0) {
					playernum = j + 1;
					game->m_players[j]->bindPlayer(peer.username.toStdString(), peer.id, true);
					widget->chatWindow()->statusMessage(QString::asprintf(tr("%s was set to player %i", "Messages:SetPlayer").toUtf8().data(), peer.username.toUtf8().data(), playernum));
					break;
				}
			}
		}

		// Skip self
		if (peer.username == network->username())
			continue;

		// Populate userlist
		if (peer.status == 1) {
			widget->chatWindow()->addUser(peer.username, QString(tr("Player %1", "Launcher:ChatwindowPl")).arg(playernum));
		} else if (peer.status == 2) {
			widget->chatWindow()->addUser(peer.username, tr("Spectator","Launcher:ChatwindowSp"));
		}
	}
}

void GameManager::peerJoinedChannel(QString channel, QString peer) const
{
	ppvs::Game* game = nullptr;
	GameWidget* widget = nullptr;

	if (!getGame(channel, game, widget))
		return;

	// As player, send them your status.
	if (network->getStatus(channel) == 1) {
		ppvs::Player* player = game->m_players[0];
		network->sendMessageToPeer(channel, CHANNEL_GAME,
			(QStringList() << "update"
						   << QString::number((int)player->getCharacter())
						   << QString::number(player->m_proposedRandomSeed)
						   << QString::number(game->m_currentGameStatus == GameStatus::REMATCHING ? 1 : 0)
						   << QString::number(player->m_wins))
				.join("|"),
			peer);

		std::string send = game->sendUpdate();
		network->sendMessageToPeer(channel, CHANNEL_GAME, send.data(), peer);
	}
}

void GameManager::peerPartedChannel(QString channel, QString peer) const
{
	ppvs::Game* game = nullptr;
	GameWidget* widget = nullptr;
	std::string peerStd = peer.toStdString();

	if (!getGame(channel, game, widget))
		return;

	// Player left
	if (network->getStatus(channel, peer) == 1) {
		QString langStr = tr("Player %s left.", "Messages:PlayerLeave");
		widget->chatWindow()->statusMessage(QString::asprintf(langStr.toUtf8().data(), peer.toUtf8().data()));

		// Find the player
		for (unsigned i = 0; i < game->m_players.size(); i++) {
			if (game->m_players[i]->m_onlineName.compare(peerStd) == 0) {
				game->m_players[i]->unbindPlayer();
				game->sendDescription();

				break;
			}
		}
	}
	// Spectator or pending player left
	else {
		QString langStr = tr("%s left."); // TODO: Needs a translation string!
		widget->chatWindow()->statusMessage(QString::asprintf(langStr.toUtf8().data(), peer.toUtf8().data()));
	}
	// Remove from chatwindow
	widget->chatWindow()->removeUser(peer);
}

void GameManager::channelMessageReceived(QString channel, uchar subchannel, QString peer, QString message) const
{
	ppvs::Game* game = nullptr;
	GameWidget* widget = nullptr;

	if (!getGame(channel, game, widget))
		return;

	switch (subchannel) {
	case CHANNEL_CHAT:
		widget->chatWindow()->chatMessage(peer, message);
		break;
	case CHANNEL_GAME:
		QStringList items = message.split('|');
		std::string peerStd = peer.toStdString();

		int playernum = -1;
		for (int i = static_cast<int>(game->m_players.size() - 1); i >= 0; i--) {
			if (game->m_players[i]->m_onlineName.compare(peerStd) == 0) {
				playernum = i;
				break;
			}
		}
		if (playernum < 0)
			return;

		if (items.at(0) == "select") {
			int c = items.at(1).toInt();
			game->m_charSelectMenu->setCharacter(playernum, c, false);
			game->m_players[playernum]->setCharacter(static_cast<ppvs::PuyoCharacter>(c), true);
			game->m_players[playernum]->m_pickingCharacter = true;
		} else if (items.at(0) == "choice") {
			int c = items.at(1).toInt();
			game->m_charSelectMenu->setCharacter(playernum, c, true);
			game->m_players[playernum]->setCharacter(static_cast<ppvs::PuyoCharacter>(c), true);
			game->m_players[playernum]->m_pickingCharacter = false;
		} else if (items.at(0) == "rematch") {
			if (game->m_settings->spectating == false && game->m_currentGameStatus == GameStatus::WAITING)
				game->m_currentGameStatus = GameStatus::IDLE;
			else if (game->m_settings->spectating == true)
				game->m_currentGameStatus = GameStatus::REMATCHING;

			for (unsigned i = 0; i < game->m_players.size(); i++) {
				if (game->m_players[i]->m_onlineName.compare(peerStd) == 0) {
					game->m_players[i]->m_rematch = true;
					game->m_players[i]->m_rematchIcon.setVisible(true);
					game->m_players[i]->m_rematchIconTimer = 0;
					game->m_players[i]->m_messages.clear();
					int rnd = items.at(1).toInt();
					game->m_players[i]->m_proposedRandomSeed = rnd;
					game->m_players[i]->m_wins = items.at(2).toInt();
					break;
				}
			}

			// Loop through ids and set all players to active in that list
			for (int i = 3; i < items.count(); i++) {
				unsigned id = items.at(i).toInt();
				for (unsigned j = 0; j < game->m_players.size(); j++) {
					if (game->m_players[j]->m_onlineId == id && game->m_players[j]->m_active == false) {
						game->m_players[j]->m_prepareActive = true;
						break;
					}
				}
			}
		} else {
			qDebug("Passing %s", message.toUtf8().data());
			game->m_players[playernum]->addMessage(message.toUtf8().data());
		}
	}
}

void GameManager::peerChannelMessageReceived(QString channel, uchar subchannel, QString peer, QString message) const
{
	ppvs::Game* game = nullptr;
	GameWidget* widget = nullptr;

	if (!getGame(channel, game, widget))
		return;

	std::string peerStd = peer.toStdString();

	// Split message
	QStringList items = message.split('|');

	// Find player number
	int playernum = -1;
	for (int i = static_cast<int>(game->m_players.size() - 1); i >= 0; i--) {
		if (game->m_players[i]->m_onlineName.compare(peerStd) == 0) {
			playernum = i;
			break;
		}
	}
	if (playernum < 0)
		return;

	// Character select
	if (items.at(0) == "choice") {
		int c = items.at(1).toInt();
		game->m_charSelectMenu->setCharacter(playernum, c, true);
		game->m_players[playernum]->setCharacter(static_cast<ppvs::PuyoCharacter>(c), true);
	} else if (items.at(0) == "update") {
		// Update character choice
		int c = items.at(1).toInt();
		game->m_charSelectMenu->setCharacter(playernum, c, true);
		game->m_players[playernum]->setCharacter(static_cast<ppvs::PuyoCharacter>(c), true);

		// Get propesed random seed
		int rnd = items.at(2).toInt();
		game->m_players[playernum]->m_proposedRandomSeed = rnd;

		// Update rematching status
		int r = items.at(3).toInt();
		if (r == 1) {
			game->m_players[playernum]->m_rematch = true;
			game->m_players[playernum]->m_rematchIcon.setVisible(true);
			game->m_players[playernum]->m_rematchIconTimer = 0;
		}

		// Update wins
		int wins = items.at(4).toInt();
		game->m_players[playernum]->m_wins = wins;
	} else if (items.at(0) == "spectate") {
		// Update player
		game->m_players[playernum]->getUpdate(message.toUtf8().data());
	} else {
		// Pass to player
		if (subchannel == CHANNEL_GAME)
			game->m_players[playernum]->addMessage(message.toUtf8().data());
	}
}

void GameManager::peerStatusReceived(QString channel, QString peer, uchar status) const
{
	ppvs::Game* game = nullptr;
	GameWidget* widget = nullptr;

	if (!getGame(channel, game, widget))
		return;

	int pln = 0;
	// Self
	if (peer == network->username()) {
		// Accepted as player
		if (status == 1) {
			// Set player 1 to human
			if (game->m_players.size() > 0) {
				ppvs::PuyoCharacter chara = ppvs::PuyoCharacter(pvsApp->settings().integer("account", "defaultcharacter", 2));
				pln = 1;

				game->m_players[0]->setPlayerType(ppvs::HUMAN);
				game->m_menuSelect = 2;
				game->m_players[0]->setCharacter(chara);
				game->m_players[0]->bindPlayer(peer.toStdString(), network->id(), true);
				if (game->m_settings->rankedMatch) {
					// Always able to press rematch
					game->m_currentGameStatus = GameStatus::IDLE;
				}

				widget->chatWindow()->statusMessage(tr("Joined game.", "Messages:JoinedGame"));
				network->sendMessage(channel, CHANNEL_GAME, (QStringList() << "choice" << QString::number((int)chara)).join("|"));
			}
		} else if (status == 2) {
			// Accepted as spectator
		}
	}
	// Everyone else
	else {
		bool found = false;
		if (status == 2) {
			// Spectator joined
			QString langStr = tr("%s joined as spectator.", "Messages:JoinSpectator");
			widget->chatWindow()->statusMessage(QString::asprintf(langStr.toUtf8().data(), peer.toUtf8().data()));
		} else {
			int first = 1;
			if (game->m_settings->spectating)
				first = 0;
			// Player joined
			for (unsigned i = first; i < game->m_players.size(); i++) {
				if (game->m_players[i]->m_onlineName.empty()) {
					pln = i + 1;
					found = true;
					game->m_players[i]->bindPlayer(peer.toStdString(), network->id(channel, peer), false);

					QString langStr = tr("%s was set to player %i", "Messages:SetPlayer");
					widget->chatWindow()->statusMessage(QString::asprintf(langStr.toUtf8().data(), peer.toUtf8().data(), pln));
					break;
				}
			}
			// Error: could not find free player
			if (found == false) {
				QString langStr = tr("The room is full.\nYou can not join.", "Messages:JoinError");

				QMessageBox message(QMessageBox::Critical, "Puyo VS", langStr, QMessageBox::Ok);
				message.show();
			}
		}
	}

	// Add to userlist
	if (status == 1)
		widget->chatWindow()->addUser(peer, QString(tr("Player %1", "Launcher:ChatwindowPl")).arg(pln));
	else if (status == 2)
		widget->chatWindow()->addUser(peer, tr("Spectator","Launcher:ChatwindowSp"));
}

void GameManager::updateControls(GameWidget* game)
{
	Settings& settings = pvsApp->settings();
	game->setControls(
		settings.string("controlsp1", "up", "up"),
		settings.string("controlsp1", "down", "down"),
		settings.string("controlsp1", "left", "left"),
		settings.string("controlsp1", "right", "right"),
		settings.string("controlsp1", "a", "x"),
		settings.string("controlsp1", "b", "z"),
		settings.string("controlsp1", "start", "return"));
}

void GameManager::updateAllControls()
{
	QListIterator<GameWidget*> it(games);
	while (it.hasNext()) {
		GameWidget* game = it.next();
		updateControls(game);
	}
}

void GameManager::rankedMatchmessageReceived(QString message)
{
	// Get the ranked gamewidget
	GameWidget* g = nullptr;
	for (int i = 0; i < games.count(); i++) {
		if (games[i]->game()->m_settings->rankedMatch) {
			g = games[i];
			break;
		}
	}
	if (g == nullptr)
		return;

	QStringList tokens = message.split('|');

	// Mo match
	if (tokens.front() == "empty") {
		g->chatWindow()->statusMessage(tr("Please wait while the server finds you an opponent...", "Messages:WaitMatch"));
	} else if (tokens.count() == 8 && tokens.front() == "match") {
		QString roomName = tokens[1];
		QString opponent = tokens[2];
		QString rating = tokens[3];
		QString ratingDev = tokens[4];
		int ratingDevInt = ratingDev.toInt();
		QString wins = tokens[5];
		QString losses = tokens[6];
		QString maxwins = tokens[7];
		int maxwinsInt = maxwins.toInt();

		g->game()->m_settings->maxWins = maxwinsInt;
		g->chatWindow()->statusMessage(tr("Opponent: ", "Messages:Opponent") + opponent);
		g->chatWindow()->statusMessage(tr("R: %1", "Messages:RatingElo").arg(rating));
		g->chatWindow()->statusMessage(tr("W: %1", "Messages:Wins").arg(wins));
		g->chatWindow()->statusMessage(tr("L: %1", "Messages:Losses").arg(losses));
		g->chatWindow()->statusMessage(tr("First to %1 match", "Messages:FirstTo").arg(maxwinsInt));
		g->game()->m_channelName = roomName.toStdString();
		g->replaceProxy(new NetChannelProxy(roomName, network));
		network->createChannel(roomName, network->username() + " vs " + opponent, false);

		// Alert
		QApplication::beep();
		QApplication::alert(g);
	} else if (tokens.count() == 1 && tokens.front() == "end") {
		// Disconnect from channel
		g->chatWindow()->statusMessage(tr("End of match.", "Messages:EndMatch"));
		g->chatWindow()->clearUsers();
		g->game()->m_connected = false;
		g->game()->m_channelName = "";
		g->game()->m_currentGameStatus = GameStatus::WAITING;
		g->game()->m_rankedTimer = 5 * 60;
		g->game()->m_rankedState = 2;

		g->replaceProxy(new NetChannelProxy("", network));

		// Request new match immediately
		ppvs::GameSettings* gs = g->game()->m_settings;
		g->game()->m_newRankedMatchMessage = QString(QString("new|") + QString("%1|").arg(PVSVERSION)
			+ QString(gs->ruleSetInfo.ruleSetType == ppvs::Rules::TSU_ONLINE ? "0" : "1"))
												 .toStdString();
	} else if (tokens.count() == 3 && tokens.front() == "result") {
		QString rating = tokens[1];
		QString ratingDev = tokens[2];
		int ratingDevInt = ratingDev.toInt();
		g->chatWindow()->statusMessage(tr("Your new rating:  %1", "Messages:ResultsElo").arg(rating).arg(0));
	}
}

bool GameManager::getGame(const QString& channel, ppvs::Game*& game, GameWidget*& widget) const
{
	if (channel.left(4) != "PVSM" && channel.left(4) != "PVST" && channel.left(4) != "PVSF")
		return false;

	widget = findGame(channel);

	if (!widget)
		return false;

	game = widget->game();

	if (!game)
		return false;

	return true;
}

void GameManager::exec()
{
	static int iteration_max = 1;
	unsigned long iteration = 0;
	unsigned long nextTime = timeGetTime();

	loopEnabled = true;
	while (loopEnabled) {
		process();

		nextTime += 8;
		unsigned long sleepTime = nextTime - timeGetTime();
		if (sleepTime < 1000)
			msleep(sleepTime);
		else
			nextTime = timeGetTime();

		// We have to do this at the end of the loop to ensure that the condition
		// is handled promptly.
		if (iteration++ % iteration_max == 0) {
			qApp->sendPostedEvents();
			qApp->processEvents();
		}
	}
}

void GameManager::process() const
{
	network->processEvents();

	QListIterator<GameWidget*> it(games);
	while (it.hasNext()) {
		GameWidget* game = it.next();
		game->process();
	}
}
