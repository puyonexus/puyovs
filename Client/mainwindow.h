#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "netclient.h"

class QListWidgetItem;
class LanguageManager;
class Settings;
class GameManager;

namespace Ui {
class MainWindow;
}

namespace ppvs {
class game;
class gameSettings;
struct rulesetInfo_t;
}
class GameWidgetGL;
class CreateChatroomDialog;
class PasswordDialog;
class SearchDialog;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void exec();
    void connectToServer();
    void logIn();
    void logOut();

    bool isLoggedIn();

    void reviewRulesDialog(ppvs::rulesetInfo_t &rs);

    void updateServerList();
    QNetworkReply *serverListReply;
    QNetworkAccessManager *netMan;

protected:
    void closeEvent(QCloseEvent *);
    void addMatchRoom(NetChannel channel);
    void removeMatchRoom(NetChannel channel);
    NetChannel selectedFriendlyMatch();

    void addRankedTsuMatchRoom(NetChannel channel);
    void removeRankedTsuMatchRoom(NetChannel channel);
    NetChannel selectedRankedTsuMatch();
    void addRankedFeverMatchRoom(NetChannel channel);
    void removeRankedFeverMatchRoom(NetChannel channel);
    NetChannel selectedRankedFeverMatch();

public slots:
    void updateActions();
    void updateJoinButton();
    void showSettings();
    void on_StartupDialog_Finished(int result);
    void on_SettingsDialog_Finished(int result);
    void on_SearchDialog_Finished(int result);
    void on_OfflineDialog_Finished(int result);
    void on_CreateChatroomDialog_Finished(CreateChatroomDialog*);

private slots:
    void getServerList();
    void refreshLanguages();
    void networkError(QString e);
    void loginResponse(uchar subchannel,QString message);
    void connected();
    void disconnected();
    void nameSet(QString u);
    void nameDenied(QString u);
    void channelJoined(QString c, NetPeerList peers);
    void channelCreated(NetChannel channel);
    void channelDestroyed(NetChannel channel);
    void channelDescriptionReceived(NetChannel channel);
    void channelListReceived(NetChannelList chanList);
    void kickMessageReceived(QString message);
    void motdMessageReceived(QString message);
    void playerCountMessageReceived(QString message);
    void updateRankedCount();

    void on_ConnectButton_clicked();
    void on_ActionLogOut_triggered();
    void on_SettingsToolButton_clicked();
    void on_ChatroomListWidget_itemClicked(QListWidgetItem *item);
    void on_CreateChatroomButton_clicked();
    void on_OfflineToolButton_clicked();
    void on_tabWidget_tabCloseRequested(int index);
    void on_JoinChatroomButton_clicked();
    void on_ChatroomListWidget_currentItemChanged();

    void on_JoinFriendlyButton_clicked();
    void on_SpectateFriendlyButton_clicked();
    void on_ReviewRulesFriendlyButton_clicked();
    void on_FriendlyMatchesTreeWidget_itemSelectionChanged();

    void on_ReplaysToolButton_clicked();

    void on_PasswordLineEdit_textEdited(const QString &arg1);

    void on_AdminToolButton_clicked();

    void on_RankedTsuPlayButton_clicked();

    void on_RankedTsuSpectateButton_clicked();

    void on_RankedFeverPlayButton_clicked();

    void on_RankedFeverSpectateButton_clicked();

    void on_ActionSearch_triggered();

private:
    void loggedIn();
    void showError(QString message);
    void startRankedMatch(bool tsu);
    void spectateRankedMatch(bool tsu);

    int userLevel;
    NetChannel rememberSelectedFriendly;
    Ui::MainWindow *ui;
    NetClient *client;
    LanguageManager *languageManager;
    GameManager *gameManager;
    QString motd;
    bool passEdited;
    bool showSettingsDlg;
    bool showSearchDlg;
    SearchDialog *searchDlg;
    ppvs::gameSettings *mGameSettings;
};

#endif // MAINWINDOW_H
