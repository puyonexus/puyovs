#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMetaType>

#include <string>
#include <vector>

class NetClient;
struct QPVSClientPriv;
struct PVS_Client;
class QTimer;

// ! WARNING !
// These definitions are temporary until the libpuyo code is moved in.
#define CHANNEL_CHAT                0
#define CHANNEL_CHALLENGE           1
#define CHANNEL_CHALLENGERESPONSE   2
#define CHANNEL_GAME                3

#define CHANNEL_PREFIX_LENGTH       8

#define SUBCHANNEL_SERVERREQ_DEBUG         0
#define SUBCHANNEL_SERVERREQ_USERINFO      1
#define SUBCHANNEL_SERVERREQ_LOGIN         2
#define SUBCHANNEL_SERVERREQ_REGISTER      3
#define SUBCHANNEL_SERVERREQ_ERROR         4
#define SUBCHANNEL_SERVERREQ_MODERATOR     5
#define SUBCHANNEL_SERVERREQ_ADMIN         6
#define SUBCHANNEL_SERVERREQ_RESPONSE      7
#define SUBCHANNEL_SERVERREQ_MOTD          8
#define SUBCHANNEL_SERVERREQ_MATCH         9
#define SUBCHANNEL_SERVERREQ_SEARCH        10
#define SUBCHANNEL_SERVERREQ_INFO          11

struct NetChannel
{
	NetChannel() {}
	NetChannel(const QString& n, const QString& d) : name(n), description(d) {}

	QString friendlyName() const
	{
		QString f = name; f.remove(0, CHANNEL_PREFIX_LENGTH); return f;
	}
	bool isMatchRoom() const { return name.left(4) == "PVSM"; }
	bool isChatRoom() const { return name.left(4) == "PVSL"; }
	bool isRankedTsuRoom() const { return name.left(4) == "PVST"; }
	bool isRankedFeverRoom() const { return name.left(4) == "PVSF"; }
	bool isNull() const { return name.isEmpty(); }

	QString name;
	QString description;
};
Q_DECLARE_METATYPE(NetChannel)

typedef QList<NetChannel> NetChannelList;
Q_DECLARE_METATYPE(NetChannelList)

struct NetPeer
{
	NetPeer() : id(0), status(0) {}
	NetPeer(int i, const QString& u, unsigned char s) : id(i), username(u), status(s) {}

	int id;
	QString username;
	unsigned char status;
};
Q_DECLARE_METATYPE(NetPeer)

typedef QList<NetPeer> NetPeerList;
Q_DECLARE_METATYPE(NetPeerList)

class NetClient : public QObject
{
	Q_OBJECT

		friend struct QPVSClientPriv;
public:
	NetClient();
	~NetClient() override;

	bool connectToHost(const QString& host);
	void disconnectFromHost();
	void setUsername(const QString& username);
	void createChannel(const QString& channel, const QString& description, bool lock, bool autodestroy = true, unsigned char status = 0) const;
	void sendMessage(const QString& channel, uchar subchannel, const QString& msg) const;
	void sendMessageToPeer(const QString& channel, uchar subchannel, const QString& msg, const QString& peer) const;
	void sendMessageToServer(uchar subchannel, const QString& msg) const;
	void setStatus(QString channel, uchar status) const;
	uchar getStatus(QString channel, QString peer = QString()) const;
	void partChannel(QString channel) const;
	void joinChannel(QString channel) const;

	bool isConnected() const;
	bool isInitialized() const;

	QString username() const;
	PVS_Client* client() const;
	unsigned int id(const QString& channel = QString(), const QString& peer = QString()) const;

	QString chatRoomPrefix();
	QString matchRoomPrefix();
	QString rankedTsuRoomPrefix();
	QString rankedFeverRoomPrefix();

signals:
	void connected();
	void disconnected();
	void nameSet(QString);
	void nameDenied(QString);
	void channelJoined(QString channel, NetPeerList peers);
	void channelDenied();
	void channelParted();
	void channelListReceived(NetChannelList channels);
	void serverMessageReceived(uchar subchannel, QString message);
	void kickMessageReceived(QString message);
	void motdMessageReceived(QString message);
	void rankedMatchMessageReceived(QString message);
	void adminResponseReceived(QString message);
	void loginResponse(uchar subchannel, QString message);
	void channelMessageReceived(QString channel, uchar subchannel, QString peer, QString message);
	void peerChannelMessageReceived(QString channel, uchar subchannel, QString peer, QString message);
	void rawChannelMessageReceived();
	void rawPeerChannelMessageReceived();
	void peerJoinedChannel(QString channel, QString peer);
	void peerPartedChannel(QString channel, QString peer);
	void channelCreated(NetChannel channel);
	void channelDestroyed(NetChannel channel);
	void channelDescriptionReceived(NetChannel channel);
	void peerStatusReceived(QString channel, QString peer, uchar status);
	void searchResultReceived(QString message);
	void updateRankedPlayerCount(QString message);
	void error(QString);

public slots:
	void processEvents() const;

private:
	void onConnect();
	void onDisconnect();
	void onNameSet();
	void onNameDenied();
	void onChannelJoined(QString);
	void onChannelDenied();
	void onChannelLeft();
	void onGetPeerList(std::vector<std::string>* v);
	void onGetChannelList(std::vector<std::string>* names, std::vector<std::string>* descriptions);
	void onMessageServer();
	void onMessageChannel();
	void onMessageChannelPeer();
	void onRawMessageChannel();
	void onRawMessageChannelPeer();
	void onPeerJoinedChannel();
	void onPeerLeftChannel();
	void onChannelCreated();
	void onChannelDestroyed();
	void onChannelDescription();
	void onPeerStatus();
	void onError(QString);

	PVS_Client* mClient;
	QString mLastName;
	QString mChatRoomPrefix;
	QString mMatchRoomPrefix;
	QString mRankedTsuRoomPrefix;
	QString mRankedFeverRoomPrefix;
	bool mTryConnectFlag;
};

class NetChannelProxy : public QObject
{
	Q_OBJECT
public:
	NetChannelProxy(QString channel, NetClient* client, QObject* parent = nullptr);
	~NetChannelProxy() override;

	void sendMessage(QString msg) const;
	void sendMessage(uchar subchannel, QString peer, QString msg) const;
	void sendChallengeMessage(QString msg) const;
	void sendChallengeMessage(QString peer, QString msg) const;
	void sendUserRequestInfo(QString peer) const;
	unsigned int id() const;
	QString nick() const;
	QString channel();
	QString friendlyChannel() const;
	void setStatus(uchar status) const;
	void part() const;

	NetClient* client() const;

signals:
	void messageReceived(uchar subchannel, QString from, QString msg);
	void peerMessageReceived(uchar subchannel, QString from, QString msg);
	void userInfoReceived(QString msg);
	void statusReceived(QString from, uchar status);
	void peerJoined(QString peer);
	void peerParted(QString peer);

private slots:
	void serverMessageReceived(uchar subchannel, QString message);
	void channelMessageReceived(QString channel, uchar subchannel, QString peer, QString message);
	void peerChannelMessageReceived(QString channel, uchar subchannel, QString peer, QString message);
	void peerStatusReceived(QString channel, QString peer, uchar status);
	void peerJoinedChannel(QString channel, QString peer);
	void peerPartedChannel(QString channel, QString peer);

private:
	QString mChannel;
	NetClient* mClient;
};
