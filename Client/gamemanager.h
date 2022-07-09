#ifndef GAMETIMER_H
#define GAMETIMER_H

#include <QObject>
#include <QList>
#include <QBasicTimer>
#include "../Puyolib/gameSettings.h" // ppvs::rulesetInfo_t
#include "netclient.h"

class GameAudio;
class GameWidget;
class NetClient;
class Settings;

// WARNING: There may only be one of this class due to a current implementation detail...

class GameManager : public QObject
{
    Q_OBJECT
public:
    GameManager(NetClient *network, QObject *parent = 0);
    ~GameManager();

    bool closeAll();
    void exec();

public slots:
    void addGame(GameWidget *game);
    ppvs::rulesetInfo_t createRules();
    GameWidget *createGame(const QString &rules, const QString &roomName, bool spectating = false);
    GameWidget *createGame(ppvs::gameSettings *gamesettings, const QString &roomName, bool spectating = false,bool replay=false);
    GameWidget *findGame(const QString &roomName);
    void gameDestroyed(GameWidget *game);
    bool rankedMatch();

private slots:
    void channelJoined(QString channel, NetPeerList peers);
    void peerJoinedChannel(QString channel, QString peer);
    void peerPartedChannel(QString channel, QString peer);
    void channelMessageReceived(QString channel, uchar subchannel, QString peer, QString message);
    void peerChannelMessageReceived(QString channel, uchar subchannel, QString peer, QString message);
    void peerStatusReceived(QString channel, QString peer, uchar status);
    void updateControls(GameWidget *game);
    void updateAllControls();
    void rankedMatchmessageReceived(QString message);
signals:
    void exiting();

protected:
    void process();
    bool getGame(const QString &channel, ppvs::game *&game, GameWidget *&widget);

    QList<GameWidget *> games;
    NetClient *network;
    GameAudio *audio;
};

#endif // GAMETIMER_H
