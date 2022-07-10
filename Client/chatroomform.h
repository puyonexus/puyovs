#pragma once

#include <QWidget>
#include <QIcon>
#include "netclient.h"
#include "../Puyolib/gameSettings.h" // ppvs::rulesetInfo_t

namespace Ui {
class ChatroomForm;
}
namespace ppvs{
struct rulesetInfo_t;
}

class QListWidgetItem;
class GameManager;
class GameWidget;
class Settings;
class ChatroomTextEdit;

struct ChallengeState
{
    ChallengeState() : beingChallenged(false), challenging(false), game(0) { }
    bool beingChallenged;
    bool challenging;

    QString room;
    QString challenger;
    QString challengedUser;
    QStringList challengedList;
    ppvs::rulesetInfo_t rules;
    GameWidget *game;
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

    explicit ChatroomForm(NetPeerList peers, NetChannelProxy *proxy, GameManager *manager, QWidget *parent = 0);
    ~ChatroomForm();

    void refreshLanguages();
    void statusMessage(StatusEvent type, QString arg = QString());
    void appendLine(QString html);
    void updateChallengeButton();
    void updateChallengeDisplay();
    bool isAutoRejectEnabled();
    ppvs::rulesetInfo_t createRules();
    void reviewRulesDialog(ppvs::rulesetInfo_t &rs);
    void inviteToMatch();
    void getInvitation(QString invite);
    void setAutoreject();
    void loadSettings();
    void countUsers();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void urlClicked(QString url);
    void userInfoReceived(QString msg);
    void messageReceived(uchar subchannel, QString nick, QString msg);
    void peerMessageReceived(uchar subchannel, QString nick, QString msg);
    void statusReceived(QString nick, uchar status);
    void peerJoined(QString peer);
    void peerParted(QString peer);

    void on_EntryTextEdit_returnPressed();
    void on_SendButton_clicked();
    void on_ChallengeButton_clicked();
    void on_AutoRejectCheckBox_toggled(bool checked);
    void on_UserListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_ChallengeYesButton_clicked();
    void on_ChallengeNoButton_clicked();
    void on_ReviewRulesButton_clicked();

    void on_PlayersSpinbox_valueChanged(int arg1);

    void on_RulesComboBox_currentIndexChanged(int index);

    void on_SendPrivateButton_clicked();

    void on_UserListWidget_itemChanged(QListWidgetItem *item);

    void on_EntryTextEdit_textChanged();

private:
    ChallengeState challenge;

    Ui::ChatroomForm *ui;
    QIcon statusIconAvailable;
    QIcon statusIconAway;
    NetChannelProxy *mProxy;
    GameManager *gameManager;
};
