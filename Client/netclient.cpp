#include "netclient.h"
#include "PVS_Client.h"
#include "common.h"
#include <QTimer>

struct QPVSClientPriv : public PVS_Client
{
	QPVSClientPriv(NetClient* c) : PVS_Client(), client(c) { }

	void onConnect() override { client->onConnect(); }
	void onDisconnect() override { client->onDisconnect(); }
	void onNameSet() override { client->onNameSet(); }
	void onNameDenied() override { client->onNameDenied(); }
	void onChannelJoined() override { client->onChannelJoined(QString::fromStdString(currentChannelName)); }
	void onChannelDenied() override { client->onChannelDenied(); }
	void onChannelLeft() override { client->onChannelLeft(); }
	void onGetPeerList(std::vector<std::string>* v) override { client->onGetPeerList(v); }
	void onGetChannelList(std::vector<std::string>* names, std::vector<std::string>* descriptions) override { client->onGetChannelList(names, descriptions); }
	void onMessageServer() override { client->onMessageServer(); }
	void onMessageChannel() override { client->onMessageChannel(); }
	void onMessageChannelPeer() override { client->onMessageChannelPeer(); }
	void onRawMessageChannel() override { client->onRawMessageChannel(); }
	void onRawMessageChannelPeer() override { client->onRawMessageChannelPeer(); }
	void onPeerJoinedChannel() override { client->onPeerJoinedChannel(); }
	void onPeerLeftChannel() override { client->onPeerLeftChannel(); }
	void onChannelCreated() override { client->onChannelCreated(); }
	void onChannelDestroyed() override { client->onChannelDestroyed(); }
	void onChannelDescription() override { client->onChannelDescription(); }
	void onPeerStatus() override { client->onPeerStatus(); }
	void onError() override { client->onError(QString::fromStdString(errorString)); }

	NetClient* client;
};

NetClient::NetClient()
	: mClient(new QPVSClientPriv(this))
{
	mTryConnectFlag = false;
	mClient->initNetwork();

	mChatRoomPrefix = QString("PVSL%1").arg(int(PVSVERSION), 4, 10, QChar('0'));
	mMatchRoomPrefix = QString("PVSM%1").arg(int(PVSVERSION), 4, 10, QChar('0'));
	mRankedTsuRoomPrefix = QString("PVST%1").arg(int(PVSVERSION));
	mRankedFeverRoomPrefix = QString("PVSF%1").arg(int(PVSVERSION));
}

NetClient::~NetClient()
{
	delete mClient;
}

bool NetClient::connectToHost(const QString& host)
{
	mTryConnectFlag = true;

	return mClient->requestConnect(host.toUtf8().data());
}

void NetClient::disconnectFromHost()
{
	mClient->requestDisconnect();

	mTryConnectFlag = false;
}

void NetClient::setUsername(const QString& username)
{
	mLastName = username;
	mClient->requestName(username.toUtf8().data());
}

void NetClient::createChannel(const QString& channel, const QString& description, bool lock, bool autodestroy, unsigned char status) const
{
	QByteArray chu8 = channel.toUtf8();
	QByteArray dsu8 = description.toUtf8();
	mClient->createChannel(chu8.data(), dsu8.data(), lock, autodestroy, status);
}

void NetClient::sendMessage(const QString& channel, uchar subchannel, const QString& msg) const
{
	QByteArray chu8 = channel.toUtf8();
	QByteArray msu8 = msg.toUtf8();
	mClient->sendToChannel(subchannel, msu8.data(), chu8.data());
}

void NetClient::sendMessageToPeer(const QString& channel, uchar subchannel, const QString& msg, const QString& peer) const
{
	QByteArray chu8 = channel.toUtf8();
	QByteArray msu8 = msg.toUtf8();

	PVS_Peer* peerObject = mClient->channelManager.getPeerInChannel(std::string(chu8.data()), peer.toStdString());

	if (peerObject)
		mClient->sendToPeer(subchannel, msu8.data(), chu8.data(), peerObject->id);
}

void NetClient::sendMessageToServer(uchar subchannel, const QString& msg) const
{
	QByteArray msu8 = msg.toUtf8();
	mClient->sendToServer(subchannel, msu8.data());
}

void NetClient::setStatus(QString channel, uchar status) const
{
	QByteArray chu8 = channel.toUtf8();
	mClient->changeStatus(chu8.data(), status);
}

uchar NetClient::getStatus(QString channel, QString peer) const
{
	QByteArray chu8 = channel.toUtf8();
	PVS_Peer* peerObject = nullptr;

	if (peer.isEmpty())
		peerObject = mClient->getPVS_Peer();
	else
		peerObject = mClient->channelManager.getPeerInChannel(std::string(chu8.data()), peer.toStdString());

	if (peerObject)
		return mClient->channelManager.getStatus(std::string(chu8.data()), peerObject);
	return ~0;
}

void NetClient::partChannel(QString channel) const
{
	if (!isConnected())
		return;

	QByteArray chu8 = channel.toUtf8();
	mClient->leaveChannel(chu8.data());
}

void NetClient::joinChannel(QString channel) const
{
	QByteArray chu8 = channel.toUtf8();
	mClient->joinChannel(chu8.data());
}

PVS_Client* NetClient::client() const
{
	return mClient;
}

unsigned int NetClient::id(const QString& channel, const QString& peer) const
{
	if (channel.isEmpty() || peer.isEmpty())
		return mClient->getID();
	return mClient->channelManager.getPeerInChannel(channel.toStdString(), peer.toStdString())->id;
}

QString NetClient::chatRoomPrefix()
{
	return mChatRoomPrefix;
}

QString NetClient::matchRoomPrefix()
{
	return mMatchRoomPrefix;
}
QString NetClient::rankedTsuRoomPrefix()
{
	return mRankedTsuRoomPrefix;
}
QString NetClient::rankedFeverRoomPrefix()
{
	return mRankedFeverRoomPrefix;
}

bool NetClient::isConnected() const
{
	return mClient->connected;
}

bool NetClient::isInitialized() const
{
	return mClient->networkInitialized;
}

QString NetClient::username() const
{
	if (mClient->connected)
		return QString::fromStdString(mClient->getUserName());

	return QString();
}

void NetClient::onConnect()
{
	emit connected();
}

void NetClient::onDisconnect()
{
	emit disconnected();
}

void NetClient::onNameSet()
{
	emit nameSet(username());
}

void NetClient::onNameDenied()
{
	emit nameDenied(mLastName);
}

void NetClient::onChannelJoined(QString c)
{
	QList<NetPeer> peers;

	PVS_Channel* pvsch = mClient->channelManager.getChannel(mClient->currentChannelName);
	std::list<PVS_Peer*>* pl = pvsch->peers;

	for (const auto& it : *pl)
	{
		peers.append(NetPeer(it->id, QString::fromStdString(it->name), pvsch->status[it->id]));
	}

	emit channelJoined(c, peers);
}

void NetClient::onChannelDenied()
{
	emit channelDenied();
}

void NetClient::onChannelLeft()
{
	emit channelParted();
}

void NetClient::onGetPeerList(std::vector<std::string>* v)
{
	QStringList peers;

	for (auto& peer : *v)
	{
		peers.append(QString::fromStdString(peer));
	}

	//emit peerListReceived(peers);
}

void NetClient::onGetChannelList(std::vector<std::string>* names, std::vector<std::string>* descriptions)
{
	NetChannelList channels;

	for (uint i = 0; i < names->size(); ++i)
		channels.append(NetChannel(QString::fromStdString(names->at(i)), QString::fromStdString(descriptions->at(i))));

	emit channelListReceived(channels);
}

void NetClient::onMessageServer()
{
	if (mClient->subChannel == SUBCHANNEL_SERVERREQ_LOGIN || mClient->subChannel == SUBCHANNEL_SERVERREQ_REGISTER)
		emit loginResponse(mClient->subChannel, QString::fromStdString(mClient->currentString));
	else if (mClient->subChannel == SUBCHANNEL_SERVERREQ_RESPONSE)
		emit kickMessageReceived(QString::fromStdString(mClient->currentString));
	else if (mClient->subChannel == SUBCHANNEL_SERVERREQ_ADMIN || mClient->subChannel == SUBCHANNEL_SERVERREQ_MODERATOR)
		emit adminResponseReceived(QString::fromStdString(mClient->currentString));
	else if (mClient->subChannel == SUBCHANNEL_SERVERREQ_MOTD)
		emit motdMessageReceived(QString::fromStdString(mClient->currentString));
	else if (mClient->subChannel == SUBCHANNEL_SERVERREQ_MATCH)
		emit rankedMatchMessageReceived(QString::fromStdString(mClient->currentString));
	else if (mClient->subChannel == SUBCHANNEL_SERVERREQ_SEARCH)
		emit searchResultReceived(QString::fromStdString(mClient->currentString));
	else if (mClient->subChannel == SUBCHANNEL_SERVERREQ_INFO)
		emit updateRankedPlayerCount(QString::fromStdString(mClient->currentString));
	else
		emit serverMessageReceived(mClient->subChannel, QString::fromStdString(mClient->currentString));
}

void NetClient::onMessageChannel()
{
	emit channelMessageReceived(QString::fromStdString(mClient->currentChannelName), mClient->subChannel,
		QString::fromStdString(mClient->currentPVS_PeerName),
		QString::fromStdString(mClient->currentString));
}

void NetClient::onMessageChannelPeer()
{
	emit peerChannelMessageReceived(QString::fromStdString(mClient->currentChannelName), mClient->subChannel,
		QString::fromStdString(mClient->currentPVS_PeerName),
		QString::fromStdString(mClient->currentString));
}

void NetClient::onRawMessageChannel()
{
	emit rawChannelMessageReceived();
}

void NetClient::onRawMessageChannelPeer()
{
	emit rawPeerChannelMessageReceived();
}

void NetClient::onPeerJoinedChannel()
{
	emit peerJoinedChannel(QString::fromStdString(mClient->currentChannelName),
		QString::fromStdString(mClient->currentPVS_PeerName));
}

void NetClient::onPeerLeftChannel()
{
	emit peerPartedChannel(QString::fromStdString(mClient->currentChannelName),
		QString::fromStdString(mClient->currentPVS_PeerName));
}

void NetClient::onChannelCreated()
{
	emit channelCreated(NetChannel(QString::fromStdString(mClient->currentChannelName),
		QString::fromStdString(mClient->currentChannelDescription)));
}

void NetClient::onChannelDestroyed()
{
	emit channelDestroyed(NetChannel(QString::fromStdString(mClient->currentChannelName),
		QString()));
}

void NetClient::onChannelDescription()
{
	emit channelDescriptionReceived(NetChannel(QString::fromStdString(mClient->currentChannelName),
		QString::fromStdString(mClient->currentChannelDescription)));
}

void NetClient::onPeerStatus()
{
	emit peerStatusReceived(QString::fromStdString(mClient->currentChannelName),
		QString::fromStdString(mClient->currentPVS_PeerName),
		mClient->currentStatus);
}

void NetClient::onError(QString e)
{
	mTryConnectFlag = false;
	emit error(e);
}

void NetClient::processEvents() const
{
	if (mClient->networkInitialized && (mTryConnectFlag || mClient->connected))
	{
		mClient->checkEvent();
	}
}

NetChannelProxy::NetChannelProxy(QString channel, NetClient* client, QObject* parent)
	: QObject(parent), mChannel(channel), mClient(client)
{
	connect(client, SIGNAL(serverMessageReceived(uchar, QString)),
		SLOT(serverMessageReceived(uchar, QString)));
	connect(client, SIGNAL(channelMessageReceived(QString, uchar, QString, QString)),
		SLOT(channelMessageReceived(QString, uchar, QString, QString)));
	connect(client, SIGNAL(peerChannelMessageReceived(QString, uchar, QString, QString)),
		SLOT(peerChannelMessageReceived(QString, uchar, QString, QString)));
	connect(client, SIGNAL(peerStatusReceived(QString, QString, uchar)),
		SLOT(peerStatusReceived(QString, QString, uchar)));
	connect(client, SIGNAL(peerJoinedChannel(QString, QString)),
		SLOT(peerJoinedChannel(QString, QString)));
	connect(client, SIGNAL(peerPartedChannel(QString, QString)),
		SLOT(peerPartedChannel(QString, QString)));
}

NetChannelProxy::~NetChannelProxy()
{
	mClient->partChannel(mChannel);
}

void NetChannelProxy::sendMessage(QString msg) const
{
	if (!msg.isEmpty())
		mClient->sendMessage(mChannel, CHANNEL_CHAT, msg);
}

void NetChannelProxy::sendMessage(uchar subchannel, QString peer, QString msg) const
{
	mClient->sendMessageToPeer(mChannel, subchannel, msg, peer);
}

void NetChannelProxy::sendChallengeMessage(QString msg) const
{
	mClient->sendMessage(mChannel, CHANNEL_CHALLENGE, msg);
}

void NetChannelProxy::sendChallengeMessage(QString peer, QString msg) const
{
	mClient->sendMessageToPeer(mChannel, CHANNEL_CHALLENGE, msg, peer);
}

void NetChannelProxy::sendUserRequestInfo(QString peer) const
{
	mClient->sendMessageToServer(SUBCHANNEL_SERVERREQ_USERINFO, peer);
}

unsigned int NetChannelProxy::id() const
{
	return mClient->id();
}

QString NetChannelProxy::nick() const
{
	return mClient->username();
}

QString NetChannelProxy::channel()
{
	return mChannel;
}

QString NetChannelProxy::friendlyChannel() const
{
	QString f = mChannel;
	f.remove(0, CHANNEL_PREFIX_LENGTH);
	return f;
}

void NetChannelProxy::setStatus(uchar status) const
{
	mClient->setStatus(mChannel, status);
}

void NetChannelProxy::part() const
{
	mClient->partChannel(mChannel);
}

NetClient* NetChannelProxy::client() const
{
	return mClient;
}

void NetChannelProxy::channelMessageReceived(QString channel, uchar subchannel, QString peer, QString message)
{
	if (channel == mChannel)
		emit messageReceived(subchannel, peer, message);
}

void NetChannelProxy::peerChannelMessageReceived(QString channel, uchar subchannel, QString peer, QString message)
{
	if (channel == mChannel)
		emit peerMessageReceived(subchannel, peer, message);
}

void NetChannelProxy::peerStatusReceived(QString channel, QString peer, uchar status)
{
	if (channel == mChannel)
		emit statusReceived(peer, status);
}

void NetChannelProxy::peerJoinedChannel(QString channel, QString peer)
{
	if (channel == mChannel)
		emit peerJoined(peer);
}

void NetChannelProxy::peerPartedChannel(QString channel, QString peer)
{
	if (channel == mChannel)
		emit peerParted(peer);
}

void NetChannelProxy::serverMessageReceived(uchar subchannel, QString message)
{
	if (subchannel == SUBCHANNEL_SERVERREQ_USERINFO)
		emit userInfoReceived(message);
}

