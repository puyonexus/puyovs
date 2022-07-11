#pragma once

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
struct gameSettings;
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
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow() override;

	void exec();
	void connectToServer();
	void logIn() const;
	void logOut() const;

	bool isLoggedIn() const;

	void reviewRulesDialog(ppvs::rulesetInfo_t& rs);

	void updateServerList();
	QNetworkReply* serverListReply;
	QNetworkAccessManager* netMan;

protected:
	void closeEvent(QCloseEvent*) override;
	void addMatchRoom(NetChannel channel);
	void removeMatchRoom(NetChannel channel) const;
	NetChannel selectedFriendlyMatch() const;

	void addRankedTsuMatchRoom(NetChannel channel) const;
	void removeRankedTsuMatchRoom(NetChannel channel) const;
	NetChannel selectedRankedTsuMatch() const;
	void addRankedFeverMatchRoom(NetChannel channel) const;
	void removeRankedFeverMatchRoom(NetChannel channel) const;
	NetChannel selectedRankedFeverMatch() const;

public slots:
	void updateActions() const;
	void updateJoinButton() const;
	void showSettings();
	void showTelemetryPrompt() const;
	void on_StartupDialog_Finished(int result) const;
	void on_SettingsDialog_Finished(int result);
	void on_SearchDialog_Finished(int result);
	void on_OfflineDialog_Finished(int result);
	void on_CreateChatroomDialog_Finished(CreateChatroomDialog*) const;

private slots:
	void getServerList() const;
	void refreshLanguages();
	void networkError(QString e);
	void loginResponse(uchar subchannel, QString message);
	void connected() const;
	void disconnected() const;
	void nameSet(QString u) const;
	void nameDenied(QString u);
	void channelJoined(QString c, NetPeerList peers);
	void channelCreated(NetChannel channel);
	void channelDestroyed(NetChannel channel) const;
	void channelDescriptionReceived(NetChannel channel);
	void channelListReceived(NetChannelList chanList);
	void kickMessageReceived(QString message);
	void motdMessageReceived(QString message);
	void playerCountMessageReceived(QString message) const;
	void updateRankedCount() const;

	void on_ConnectButton_clicked();
	void on_ActionLogOut_triggered() const;
	void on_SettingsToolButton_clicked();
	void on_ChatroomListWidget_itemClicked(QListWidgetItem* item) const;
	void on_CreateChatroomButton_clicked();
	void on_OfflineToolButton_clicked();
	void on_tabWidget_tabCloseRequested(int index) const;
	void on_JoinChatroomButton_clicked() const;
	void on_ChatroomListWidget_currentItemChanged() const;
	void on_JoinFriendlyButton_clicked();
	void on_SpectateFriendlyButton_clicked();
	void on_ReviewRulesFriendlyButton_clicked();
	void on_FriendlyMatchesTreeWidget_itemSelectionChanged() const;
	void on_ReplaysToolButton_clicked();
	void on_PasswordLineEdit_textEdited(const QString& arg1);
	void on_AdminToolButton_clicked() const;
	void on_RankedTsuPlayButton_clicked() const;
	void on_RankedTsuSpectateButton_clicked() const;
	void on_RankedFeverPlayButton_clicked() const;
	void on_RankedFeverSpectateButton_clicked() const;
	void on_ActionSearch_triggered();

private:
	void loggedIn() const;
	void showError(QString message);
	void startRankedMatch(bool tsu) const;
	void spectateRankedMatch(bool tsu) const;

	int userLevel;
	NetChannel rememberSelectedFriendly;
	Ui::MainWindow* ui;
	NetClient* client;
	LanguageManager* languageManager;
	GameManager* gameManager;
	QString motd;
	bool passEdited;
	bool showSettingsDlg;
	bool showSearchDlg;
	SearchDialog* searchDlg;
	ppvs::gameSettings* mGameSettings;
};
