#pragma once

#include <QWidget>
#include <QIcon>
#include "netclient.h"
#include "../Puyolib/GameSettings.h" // ppvs::RuleSetInfo

namespace Ui {
class ChatroomForm;
}

namespace ppvs {
struct RuleSetInfo;
}

class QListWidgetItem;
class GameManager;
class GameWidget;
class Settings;
class ChatroomTextEdit;

struct ChallengeState
{
	ChallengeState() : beingChallenged(false), challenging(false), game(nullptr) { }
	bool beingChallenged;
	bool challenging;

	QString room;
	QString challenger;
	QString challengedUser;
	QStringList challengedList;
	ppvs::RuleSetInfo rules;
	GameWidget* game;
};

class ChatroomForm : public QWidget
{
	Q_OBJECT

public:
	enum StatusEvent
	{
		ChannelJoinedEvent, ChannelLeftEvent,

		PeerJoinedEvent, PeerLeftEvent,

		MatchCancelEvent, MatchLeftEvent,
		MatchAutorejectEvent, MatchAutorejectSelfEvent,
		MatchDeclineEvent, MatchAcceptEvent
	};

	explicit ChatroomForm(NetPeerList peers, NetChannelProxy* proxy, GameManager* manager, QWidget* parent = nullptr);
	~ChatroomForm() override;

	void refreshLanguages();
	void statusMessage(StatusEvent type, QString arg = QString()) const;
	void appendLine(QString html) const;
	void updateChallengeButton() const;
	void updateChallengeDisplay();
	bool isAutoRejectEnabled() const;
	ppvs::RuleSetInfo createRules() const;
	void reviewRulesDialog(ppvs::RuleSetInfo& rs);
	void inviteToMatch();
	void getInvitation(QString invite) const;
	void setAutoreject() const;
	void loadSettings() const;
	void countUsers() const;

protected:
	void closeEvent(QCloseEvent*) override;

private slots:
	void urlClicked(QString url);
	void userInfoReceived(QString msg) const;
	void messageReceived(uchar subchannel, QString nick, QString msg);
	void peerMessageReceived(uchar subchannel, QString nick, QString msg);
	void statusReceived(QString nick, uchar status) const;
	void peerJoined(QString peer) const;
	void peerParted(QString peer);

	void on_EntryTextEdit_returnPressed();
	void on_SendButton_clicked();
	void on_ChallengeButton_clicked();
	void on_AutoRejectCheckBox_toggled(bool checked) const;
	void on_UserListWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous) const;
	void on_ChallengeYesButton_clicked();
	void on_ChallengeNoButton_clicked();
	void on_ReviewRulesButton_clicked();

	void on_PlayersSpinbox_valueChanged(int arg1);

	void on_RulesComboBox_currentIndexChanged(int index);

	void on_SendPrivateButton_clicked();

	void on_UserListWidget_itemChanged(QListWidgetItem* item) const;

	void on_EntryTextEdit_textChanged() const;

private:
	ChallengeState challenge;

	Ui::ChatroomForm* ui;
	QIcon statusIconAvailable;
	QIcon statusIconAway;
	NetChannelProxy* mProxy;
	GameManager* gameManager;
};
