#pragma once

#include "../Puyolib/AssetManager.h" //ppvs::AssetManager
#include "../Puyolib/DebugLog.h" // ppvs::DebugLog
#include "../Puyolib/GameSettings.h" // ppvs::RuleSetInfo
#include "netclient.h"
#include <QBasicTimer>
#include <QList>
#include <QObject>
#include <string>

class GameAudio;
class GameWidget;
class NetClient;
class Settings;

namespace ppvs {
class GameAssetSettings;
}

// WARNING: There may only be one of this class due to a current implementation detail...

class GameManager : public QObject {
	Q_OBJECT

public:
	explicit GameManager(NetClient* network, ppvs::AssetManager* am, QObject* parent = nullptr);
	~GameManager() override;

	bool closeAll();
	void exec();

public slots:
	void addGame(GameWidget* game);
	ppvs::RuleSetInfo createRules();

	ppvs::DebugLog* createDebug();
	GameWidget* createGame(const QString& rules, const QString& roomName, bool spectating = false);
	GameWidget* createGame(ppvs::GameSettings* gamesettings, const QString& roomName, bool spectating = false, bool replay = false);
	GameWidget* findGame(const QString& roomName) const;
	void gameDestroyed(GameWidget* game);
	bool rankedMatch() const;
	void hotReloadGameAssets();

private slots:
	void channelJoined(QString channel, NetPeerList peers) const;
	void peerJoinedChannel(QString channel, QString peer) const;
	void peerPartedChannel(QString channel, QString peer) const;
	void channelMessageReceived(QString channel, uchar subchannel, QString peer, QString message) const;
	void peerChannelMessageReceived(QString channel, uchar subchannel, QString peer, QString message) const;
	void peerStatusReceived(QString channel, QString peer, uchar status) const;
	void updateControls(GameWidget* game);
	void updateAllControls();
	void rankedMatchmessageReceived(QString message);
signals:
	void exiting();
	void refreshAssetManagerTemplate();

protected:
	static void handleDebugLog(std::string text, ppvs::DebugMessageType severity);

	void process() const;
	bool getGame(const QString& channel, ppvs::Game*& game, GameWidget*& widget) const;

	QList<GameWidget*> games;
	NetClient* network;
	GameAudio* audio;

	ppvs::AssetManager* assetManagerTemplate{};
	ppvs::DebugLog* dbg;
};
