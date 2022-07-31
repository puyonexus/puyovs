#include <QCloseEvent>
#include <QCryptographicHash>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QTabBar>
#include <QTabWidget>
#include <QTreeWidgetItem>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../Puyolib/GameSettings.h"
#include "admindialog.h"
#include "chatroomform.h"
#include "chatwindow.h"
#include "common.h"
#include "createchatroomdialog.h"
#include "gamemanager.h"
#include "gamewidget.h"
#include "glmanager.h"
#include "language.h"
#include "offlinedialog.h"
#include "passworddialog.h"
#include "pvsapplication.h"
#include "replaydialog.h"
#include "searchdialog.h"
#include "settings.h"
#include "settingsdialog.h"
#include "startupdialog.h"
#include "telemetrydialog.h"

constexpr static Qt::ItemDataRole kRulesRole = static_cast<Qt::ItemDataRole>(Qt::UserRole + 1);
constexpr static Qt::ItemDataRole kScoreRole = static_cast<Qt::ItemDataRole>(Qt::UserRole + 2);

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	setAttribute(Qt::WA_DeleteOnClose);

	// Get settings manager
	Settings& settings = pvsApp->settings();

	// Language system
	languageManager = new LanguageManager(this);

	// UI
	languageManager->setLanguageFromFilename(settings.string("launcher", "language", ""));
	ui->setupUi(this);
	ui->LogoLabel->setPixmap(QPixmap("Data/Lobby/title.png"));
	ui->UsernameLineEdit->setFocus();
	ui->statusBar->showMessage("Developed by Hernan and contributors. https://puyovs.com");

	// Profile Menu
    const auto profileMenu = new QMenu(this);
	profileMenu->addAction(ui->ActionSearch);
	profileMenu->addAction(ui->ActionLogOut);
	ui->ProfileToolButton->setMenu(profileMenu);

	// Show admin console when
	ui->AdminToolButton->hide();
	userLevel = 0;

	// Hack: use sub-classing to do this 'right.'
	// Set fixed tabs.
	class F : public QTabWidget {
	public:
		QTabBar* tabBar() const { return QTabWidget::tabBar(); }
	};
	QTabBar* tabBar = static_cast<F*>(ui->tabWidget)->tabBar();
	tabBar->setTabButton(0, QTabBar::RightSide, nullptr);
	tabBar->setTabButton(1, QTabBar::RightSide, nullptr);

	// Client
	client = new NetClient;
	connect(client, &NetClient::connected, this, &MainWindow::connected);
	connect(client, &NetClient::disconnected, this, &MainWindow::disconnected);
	connect(client, &NetClient::nameSet, this, &MainWindow::nameSet);
	connect(client, &NetClient::nameDenied, this, &MainWindow::nameDenied);
	connect(client, &NetClient::channelJoined, this, &MainWindow::channelJoined);
	connect(client, &NetClient::channelCreated, this, &MainWindow::channelCreated);
	connect(client, &NetClient::channelDestroyed, this, &MainWindow::channelDestroyed);
	connect(client, &NetClient::channelDescriptionReceived, this, &MainWindow::channelDescriptionReceived);
	connect(client, &NetClient::channelListReceived, this, &MainWindow::channelListReceived);
	connect(client, &NetClient::error, this, &MainWindow::networkError);
	connect(client, &NetClient::loginResponse, this, &MainWindow::loginResponse);
	connect(client, &NetClient::kickMessageReceived, this, &MainWindow::kickMessageReceived);
	connect(client, &NetClient::motdMessageReceived, this, &MainWindow::motdMessageReceived);
	connect(client, &NetClient::updateRankedPlayerCount, this, &MainWindow::playerCountMessageReceived);

	// Update ranked counter every 10 seconds
    const auto timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &MainWindow::updateRankedCount);
	timer->start(10000); // Time specified in ms

	// Setup languages
	connect(languageManager, SIGNAL(languagesModified()), SLOT(refreshLanguages()));
	refreshLanguages();

	// UI settings
	ui->UsernameLineEdit->setText(settings.string("account", "name", ""));
	ui->ServerComboBox->addItem(settings.string("account", "server", PUYOVS_DEFAULT_SERVER));
	ui->ServerComboBox->lineEdit()->setText(settings.string("account", "server", PUYOVS_DEFAULT_SERVER));
	if (!settings.string("account", "password", "").isEmpty())
		ui->PasswordLineEdit->setText("*********");
	ui->AutoLoginCheckbox->setChecked(settings.boolean("launcher", "autologin", false));
	passEdited = false;
	mGameSettings = nullptr;
	showSettingsDlg = false;
	showSearchDlg = false;

	// Update the actions.
	updateActions();
	updateJoinButton();

	// Update server list
	updateServerList();

	// Install game timer
	gameManager = new GameManager(client, this);

	// Fix friendly match columns.
	// I really wish the UI editor had something for this.
	ui->FriendlyMatchesTreeWidget->setColumnHidden(0, true);

	// Initialize OpenGL.
	GlobalGLInit();
}

MainWindow::~MainWindow()
{
	delete gameManager;
	delete netMan;

	GlobalGLDeInit();

	delete client;
	delete ui;
	delete languageManager;
}

void MainWindow::exec()
{
	show();
	if (ui->AutoLoginCheckbox->isChecked() && ui->UsernameLineEdit->text().count())
		connectToServer();
	gameManager->exec();
}

void MainWindow::connectToServer()
{
	Settings& settings = pvsApp->settings();
	QString reqUsername = ui->UsernameLineEdit->text();
	if (reqUsername.count() < 3 || reqUsername.count() > 255) {
		showError(tr("Invalid username.", "Messages:ErrorName"));
		return;
	}
	for (int i = 0; i < reqUsername.count(); ++i) {
		ushort unichar = reqUsername.at(i).unicode();
		if (unichar <= 32 || unichar > 122 || unichar == '|') {
			showError(tr("Invalid username.", "Messages:ErrorName"));
			return;
		}
	}

	if (settings.string("account", "password", "").isEmpty() || passEdited) {
		// Check password length
		QString password = ui->PasswordLineEdit->text();
		if (password.count() < 3) {
			showError(tr("Password too short.", "Messages:ErrorPassword"));
			return;
		}
		QString passhash = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex());
		settings.setString("account", "password", passhash);
	}

	if (ui->ServerComboBox->currentText().isEmpty()) {
		showError("Enter server address");
		return;
	}

	if (client->connectToHost(ui->ServerComboBox->currentText())) {
		ui->ConnectButton->setEnabled(false);
		ui->AutoLoginCheckbox->setEnabled(false);
	} else {
		showError("Unknown error...");
	}
}

bool MainWindow::isLoggedIn() const
{
	if (!client)
		return false;

	return client->isConnected() && client->isInitialized();
}

void MainWindow::closeEvent(QCloseEvent* e)
{
	Settings& settings = pvsApp->settings();
	if (client->isConnected()) {
		if (QMessageBox(QMessageBox::Question, windowTitle(), tr("Are you sure you want to exit?", "Messages:Exit"), QMessageBox::Yes | QMessageBox::No, this).exec() == QMessageBox::Yes)
			client->disconnectFromHost();
		else {
			e->ignore();
			return;
		}
	}

	settings.setString("account", "name", ui->UsernameLineEdit->text());
	settings.setString("account", "server", ui->ServerComboBox->currentText());
	QString password = ui->PasswordLineEdit->text();
	if (settings.string("account", "password", "").isEmpty() && !password.isEmpty()) {
		QString passhash = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex());
		settings.setString("account", "password", passhash);
	}
	settings.setBoolean("launcher", "autologin", ui->AutoLoginCheckbox->isChecked());
	settings.save();

	// Destroy tabs to force destructor
	while (ui->tabWidget->count() > 0)
		delete ui->tabWidget->widget(ui->tabWidget->count() - 1);

	close();
}

void MainWindow::addMatchRoom(NetChannel channel)
{
	rememberSelectedFriendly = selectedFriendlyMatch();

	removeMatchRoom(channel);

	if (channel.description.isEmpty())
		return;
	if (!channel.name.startsWith(client->matchRoomPrefix()))
		return;

	bool friendly = true;

	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setData(0, Qt::DisplayRole, channel.name);
	item->setData(0, kRulesRole, channel.description);

	QStringList items = channel.description.split('|');

	for (int i = 0; i < items.count(); i++) {
		QStringList subitem = items.at(i).split(':');

		if (subitem[0] == "type") {
			if (subitem[1] != "friendly")
				friendly = false;
		} else if (subitem[0] == "rules") {
			item->setData(1, Qt::DisplayRole, subitem[1]);
		} else if (subitem[0] == "custom") {
			if (subitem[1] == "1")
				item->setData(2, Qt::DisplayRole, tr("Custom"));
			else
				item->setData(2, Qt::DisplayRole, tr("Default"));
		} else if (subitem[0] == "Nplayers") {
			item->setData(3, Qt::DisplayRole, subitem[1]);
		} else if (subitem[0] == "current") {
			if (subitem.at(1).toInt() <= 0) {
				delete item;

				return;
			}
			item->setData(4, Qt::DisplayRole, subitem[1]);
		} else if (subitem[0] == "currentscore") {
			item->setData(0, kScoreRole, subitem[1]);
		} else if (subitem[0] == "channelname") {
			// WHY WOULD YOU TELL US THAT
		}
	}

	if (friendly)
		ui->FriendlyMatchesTreeWidget->addTopLevelItem(item);

	// Reselected
	if (!rememberSelectedFriendly.isNull()) {
		QList<QTreeWidgetItem*> items = ui->FriendlyMatchesTreeWidget->findItems(rememberSelectedFriendly.name, Qt::MatchCaseSensitive, 0);
		if (!items.empty()) {
			QTreeWidgetItem* olditem = items.at(0);
			// Select that one
			ui->FriendlyMatchesTreeWidget->setCurrentItem(olditem);
		}
	}
	return;
}

void MainWindow::removeMatchRoom(NetChannel channel) const
{
	QList<QTreeWidgetItem*> oldItems = ui->FriendlyMatchesTreeWidget->findItems(channel.name, Qt::MatchCaseSensitive, 0);

	if (!oldItems.isEmpty())
		qDeleteAll(oldItems);

	return;
}

NetChannel MainWindow::selectedFriendlyMatch() const
{
	NetChannel result;

	if (ui->FriendlyMatchesTreeWidget->selectedItems().isEmpty())
		return result;

	QTreeWidgetItem* item = ui->FriendlyMatchesTreeWidget->selectedItems().at(0);

	result.name = item->data(0, Qt::DisplayRole).toString();
	result.description = item->data(0, kRulesRole).toString();

	return result;
}

void MainWindow::addRankedTsuMatchRoom(NetChannel channel) const
{
	removeRankedTsuMatchRoom(channel);

	if (channel.description.isEmpty())
		return;
	if (!channel.name.startsWith(client->rankedTsuRoomPrefix()))
		return;

	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setData(0, Qt::DisplayRole, channel.description);
	item->setData(0, Qt::UserRole, channel.name);
	ui->RankedTsuTreeWidget->addTopLevelItem(item);
}

void MainWindow::removeRankedTsuMatchRoom(NetChannel channel) const
{
	QList<QTreeWidgetItem*> oldItems;
	QTreeWidgetItemIterator it(ui->RankedTsuTreeWidget);
	while (*it) {
		if ((*it)->data(0, Qt::UserRole) == channel.name)
			oldItems.append(*it);
		++it;
	}

	if (!oldItems.isEmpty())
		qDeleteAll(oldItems);

	return;
}

NetChannel MainWindow::selectedRankedTsuMatch() const
{
	NetChannel result;

	if (ui->RankedTsuTreeWidget->selectedItems().isEmpty())
		return result;

	QTreeWidgetItem* item = ui->RankedTsuTreeWidget->selectedItems().at(0);
	result.name = item->data(0, Qt::UserRole).toString();
	return result;
}

void MainWindow::addRankedFeverMatchRoom(NetChannel channel) const
{
	removeRankedFeverMatchRoom(channel);

	if (channel.description.isEmpty())
		return;
	if (!channel.name.startsWith(client->rankedFeverRoomPrefix()))
		return;

	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setData(0, Qt::DisplayRole, channel.description);
	item->setData(0, Qt::UserRole, channel.name);
	ui->RankedFeverTreeWidget->addTopLevelItem(item);
}

void MainWindow::removeRankedFeverMatchRoom(NetChannel channel) const
{
	QList<QTreeWidgetItem*> oldItems;
	QTreeWidgetItemIterator it(ui->RankedFeverTreeWidget);
	while (*it) {
		if ((*it)->data(0, Qt::UserRole) == channel.name)
			oldItems.append(*it);
		++it;
	}

	if (!oldItems.isEmpty())
		qDeleteAll(oldItems);

	return;
}

NetChannel MainWindow::selectedRankedFeverMatch() const
{
	NetChannel result;

	if (ui->RankedFeverTreeWidget->selectedItems().isEmpty())
		return result;

	QTreeWidgetItem* item = ui->RankedFeverTreeWidget->selectedItems().at(0);
	result.name = item->data(0, Qt::UserRole).toString();
	return result;
}

void MainWindow::logIn() const
{
	QString password = pvsApp->settings().string("account", "password", "");
	client->sendMessageToServer(SUBCHANNEL_SERVERREQ_LOGIN, ui->UsernameLineEdit->text() + "|" + password);
}

void MainWindow::logOut() const
{
	ui->stackedWidget->setCurrentWidget(ui->LoginPage);

	if (client->isConnected())
		client->disconnectFromHost();

	while (ui->tabWidget->count() > 2) {
		QWidget* w = ui->tabWidget->widget(ui->tabWidget->count() - 1);
		ui->tabWidget->removeTab(ui->tabWidget->count() - 1);
		delete w;
	}
	gameManager->closeAll();
	ui->FriendlyMatchesTreeWidget->clear();
	ui->RankedTsuTreeWidget->clear();
	ui->RankedFeverTreeWidget->clear();

	ui->ConnectButton->setEnabled(true);
	ui->AutoLoginCheckbox->setEnabled(true);
	ui->AdminToolButton->hide();
}

// Public slots

void MainWindow::updateActions() const
{
	ui->ActionSearch->setEnabled(isLoggedIn());
	ui->ActionLogOut->setEnabled(isLoggedIn());
}

void MainWindow::updateJoinButton() const
{
	ui->JoinChatroomButton->setEnabled(false);

	QListWidgetItem* item = ui->ChatroomListWidget->currentItem();

	if (!item)
		return;

	ui->JoinChatroomButton->setEnabled(true);
}

void MainWindow::showSettings()
{
	if (!showSettingsDlg) {
		SettingsDialog* settingsDlg = new SettingsDialog(languageManager);
		connect(settingsDlg, SIGNAL(finished(int)), this, SLOT(on_SettingsDialog_Finished(int)));
		settingsDlg->show();
		showSettingsDlg = true;
	}
}

void MainWindow::showTelemetryPrompt() const
{
	QPointer<TelemetryDialog> telemetryDialog = new TelemetryDialog(languageManager);
	if (telemetryDialog->shouldShow()) {
		if (telemetryDialog->exec() == QDialog::Accepted) {
			telemetryDialog->save();
		}
	}
	delete telemetryDialog;
}

void MainWindow::on_StartupDialog_Finished(int result) const
{
	if (result == 1)
		client->createChannel(client->chatRoomPrefix() + "Main", "PuyoVS main lobby.", false, true);
	else if (result == 2)
		startRankedMatch(true);
	else if (result == 3)
		startRankedMatch(false);
}

void MainWindow::on_SettingsDialog_Finished(int result)
{
	if (result == QDialog::Accepted) {
		// Update chatroomforms
		for (int i = 0; i < ui->tabWidget->count(); ++i) {
			ChatroomForm* chatroom = qobject_cast<ChatroomForm*>(ui->tabWidget->widget(i));
			if (chatroom) {
				chatroom->loadSettings();
			}
		}
	}
	showSettingsDlg = false;
}

void MainWindow::on_SearchDialog_Finished(int result)
{
	showSearchDlg = false;
}

void MainWindow::refreshLanguages()
{
	if (languageManager->error()) {
		showError(languageManager->errorString());
	} else {
		ui->retranslateUi(this);

		for (int i = 0; i < ui->tabWidget->count(); ++i) {
			ChatroomForm* chatroom = qobject_cast<ChatroomForm*>(ui->tabWidget->widget(i));

			if (chatroom) {
				chatroom->refreshLanguages();
			}
		}

		setWindowTitle(tr("Puyo Puyo VS %1", "Title").arg(PUYOVS_BUILD_STR));
	}
}

// Private slots

void MainWindow::networkError(QString e)
{
	showError(e);
}

void MainWindow::loginResponse(uchar subchannel, QString message)
{
	Settings& settings = pvsApp->settings();
	if (subchannel == SUBCHANNEL_SERVERREQ_LOGIN) {
		// Successful login
		if (message == "ok:0") {
			loggedIn();
		} else if (message == "ok:1" || message == "ok:2") {
			// You are admin or moderator
			loggedIn();
			ui->AdminToolButton->show();
			if (message == "ok:1")
				userLevel = 1;
			else
				userLevel = 2;
		} else if (message == "passfail") {
			QMessageBox(QMessageBox::Critical, windowTitle(), tr("Password incorrect.", "Messages:ErrorIncorrectPassword"), QMessageBox::Ok, this).exec();
			logOut();
		} else if (message == "namefail") {
			if (QMessageBox(QMessageBox::Question, windowTitle(), tr("Account not found. Register this account?", "Messages:Register"), QMessageBox::Yes | QMessageBox::No, this).exec() == QMessageBox::Yes) {
				QString password = settings.string("account", "password", "");
				client->sendMessageToServer(SUBCHANNEL_SERVERREQ_REGISTER, ui->UsernameLineEdit->text() + "|" + password);
			} else {
				logOut();
			}
		} else if (message.startsWith("tempBanned")) {
			QStringList tokens = message.split("|");
			QTime time = QTime::currentTime();
			if (tokens.length() > 1) {
				// 2nd parameter is the time left in seconds
				int secs = tokens.last().toInt();
				time = time.addSecs(secs);
				const QLocale locale;
				const QString timeStr = time.toString(locale.dateTimeFormat(QLocale::LongFormat));
				QMessageBox(QMessageBox::Question, windowTitle(), tr("This account is temporarily blocked. It will be unblocked on %1", "Messages:TempBanned").arg(timeStr), QMessageBox::Ok, this).exec();
				logOut();
			}
		}
	} else // Register
	{
		if (message == "ok") {
			if (QMessageBox(QMessageBox::NoIcon, windowTitle(), tr("Registration success!", "Messages:Registered"), QMessageBox::Ok, this).exec() == QMessageBox::Ok) {
				QString password = settings.string("account", "password", "");
				client->sendMessageToServer(SUBCHANNEL_SERVERREQ_LOGIN, ui->UsernameLineEdit->text() + "|" + password);
			}
		} else if (message == "countfail") {
			QMessageBox(QMessageBox::Question, windowTitle(), tr("Too many accounts.", "Messages:RegistrationTooMany"), QMessageBox::Ok, this).exec();
			logOut();
		} else {
			if (QMessageBox(QMessageBox::Critical, windowTitle(), tr("Registration failed! Name is taken or illegal character.", "Messages:RegistrationFail"), QMessageBox::Ok, this).exec() == QMessageBox::Ok) {
				logOut();
			}
		}
	}
}

void MainWindow::loggedIn() const
{
	client->setUsername(ui->UsernameLineEdit->text());
	ui->statusBar->clearMessage();
}

void MainWindow::connected() const
{
	logIn();
}

void MainWindow::disconnected() const
{
	logOut();
}

void MainWindow::nameSet(QString) const
{
	ui->stackedWidget->setCurrentWidget(ui->LobbyPage);
	ui->ConnectButton->setEnabled(true);
	updateActions();

	// Request motd
	client->sendMessageToServer(SUBCHANNEL_SERVERREQ_MOTD, "");
}

void MainWindow::nameDenied(QString u)
{
	if (u.endsWith("___")) {
		logOut();
		showError(tr("Name not accepted. The name must be at least 3 letters and 32 letters max. It must contain only ASCII characters (0-9, a-z, A-Z or @[]^_`)", "ErrorName"));
	} else {
		client->setUsername(u + '_');
	}
}

void MainWindow::channelJoined(QString channel, NetPeerList peers)
{
	if (channel.startsWith(client->chatRoomPrefix())) {
		QString channelName = channel.mid(CHANNEL_PREFIX_LENGTH);
		ChatroomForm* chat = new ChatroomForm(peers, new NetChannelProxy(channel, client), gameManager, this);
		ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(chat, "#" + channelName));
	} else if (channel.startsWith(client->matchRoomPrefix())) {
		QString channelName = channel.mid(CHANNEL_PREFIX_LENGTH);
	}
}

void MainWindow::channelCreated(NetChannel channel)
{
	if (channel.isMatchRoom()) {
		addMatchRoom(channel);
	} else if (channel.isRankedTsuRoom()) {
		addRankedTsuMatchRoom(channel);
	} else if (channel.isRankedFeverRoom()) {
		addRankedFeverMatchRoom(channel);
	} else if (channel.isChatRoom() && channel.name.startsWith(client->chatRoomPrefix())) {
		if (ui->ChatroomListWidget->findItems(channel.friendlyName(), Qt::MatchExactly).count() == 0) {
			QListWidgetItem* item = new QListWidgetItem(channel.friendlyName());
			ui->ChatroomListWidget->addItem(item);
			item->setData(Qt::ToolTipRole, channel.description);
		}
		updateJoinButton();
	}
}

void MainWindow::channelDestroyed(NetChannel channel) const
{
	if (channel.isMatchRoom()) {
		removeMatchRoom(channel);
	} else if (channel.isRankedTsuRoom()) {
		removeRankedTsuMatchRoom(channel);
	} else if (channel.isRankedFeverRoom()) {
		removeRankedFeverMatchRoom(channel);
	} else if (channel.isChatRoom()) {
		QList<QListWidgetItem*> list = ui->ChatroomListWidget->findItems(channel.friendlyName(), Qt::MatchExactly);

		foreach (QListWidgetItem* item, list)
			delete item;

		updateJoinButton();
	}
}

void MainWindow::channelDescriptionReceived(NetChannel channel)
{
	if (channel.isMatchRoom()) {
		addMatchRoom(channel);
	} else if (channel.isChatRoom()) {
		QList<QListWidgetItem*> list = ui->ChatroomListWidget->findItems(channel.friendlyName(), Qt::MatchExactly);

		list.at(0)->setData(Qt::ToolTipRole, channel.description);
	} else if (channel.isRankedTsuRoom()) {
		removeRankedTsuMatchRoom(channel);
	} else if (channel.isRankedFeverRoom()) {
		removeRankedFeverMatchRoom(channel);
	}
}

void MainWindow::channelListReceived(NetChannelList chanList)
{
	ui->ChatroomListWidget->clear();
	ui->FriendlyMatchesTreeWidget->clear();

	foreach (NetChannel channel, chanList) {
		channelCreated(channel);
	}

	updateJoinButton();
}

void MainWindow::kickMessageReceived(QString message)
{
	QMessageBox(QMessageBox::Critical, windowTitle(), message, QMessageBox::Ok, this).exec();
}

void MainWindow::motdMessageReceived(QString message)
{
	motd = message;

	StartupDialog* dlg = new StartupDialog(motd, this);
	connect(dlg, SIGNAL(finished(int)), this, SLOT(on_StartupDialog_Finished(int)));
	dlg->show();
}

void MainWindow::playerCountMessageReceived(QString message) const
{
	QStringList strings = message.split("|");
	ui->RankedTsuGroupBox->setTitle(tr("Ranked Matches: Tsu", "TitleRankedTsu") + " (" + strings[1] + ")");
	ui->RankedFeverGroupBox->setTitle(tr("Ranked Matches: Fever", "TitleRankedFever") + " (" + strings[2] + ")");
}

void MainWindow::updateRankedCount() const
{
	if (client->isConnected()) {
		client->sendMessageToServer(SUBCHANNEL_SERVERREQ_INFO, QString("getrankedcount|%1").arg(PVSVERSION));
	}
}

void MainWindow::on_ConnectButton_clicked()
{
	connectToServer();
}

void MainWindow::on_ActionLogOut_triggered() const
{
	logOut();
}

void MainWindow::on_SettingsToolButton_clicked()
{
	showSettings();
}

void MainWindow::on_ChatroomListWidget_itemClicked(QListWidgetItem* item) const
{
	ui->ChatroomDescriptionLabel->setText(item->data(Qt::ToolTipRole).value<QString>());
}

void MainWindow::on_CreateChatroomButton_clicked()
{
	CreateChatroomDialog* createChatDlg = new CreateChatroomDialog(this);
	connect(createChatDlg, SIGNAL(createChatroom(CreateChatroomDialog*)), this, SLOT(on_CreateChatroomDialog_Finished(CreateChatroomDialog*)));
	createChatDlg->show();
}

// Private

void MainWindow::showError(QString message)
{
	QMessageBox(QMessageBox::Critical, windowTitle(), message, QMessageBox::Ok, this).exec();
}

void MainWindow::startRankedMatch(bool tsu) const
{
	if (gameManager->rankedMatch())
		return;

	ppvs::GameSettings* settings;
	if (tsu)
		settings = new ppvs::GameSettings(ppvs::RuleSetInfo(ppvs::Rules::TSU_ONLINE));
	else
		settings = new ppvs::GameSettings(ppvs::RuleSetInfo(ppvs::Rules::FEVER_ONLINE));
	settings->rankedMatch = true;
	GameWidget* game = gameManager->createGame(settings, "");

	if (!game)
		return;

	game->show();
	game->raise();
	ChatWindow* chatWindow = game->chatWindow();
	chatWindow->setChatEnabled(false);
}

void MainWindow::spectateRankedMatch(bool tsu) const
{
	NetChannel chan;
	if (tsu)
		chan = selectedRankedTsuMatch();
	else
		chan = selectedRankedFeverMatch();

	if (chan.isNull())
		return;

	GameWidget* existing = gameManager->findGame(chan.name);
	if (existing) {
		existing->raise();
		return;
	}

	ppvs::GameSettings* settings;
	if (tsu)
		settings = new ppvs::GameSettings(ppvs::RuleSetInfo(ppvs::Rules::TSU_ONLINE));
	else
		settings = new ppvs::GameSettings(ppvs::RuleSetInfo(ppvs::Rules::FEVER_ONLINE));

	GameWidget* game = gameManager->createGame(settings, chan.name, true);

	if (!game)
		return;

	game->show();
	game->raise();
	ChatWindow* chatWindow = game->chatWindow();
	chatWindow->setChatEnabled(false);
}

#include "../Puyolib/Game.h"
#include "../Puyolib/GameSettings.h"

void MainWindow::on_OfflineToolButton_clicked()
{
	if (mGameSettings) {
		// Dialog already opened
		return;
	}
	mGameSettings = new ppvs::GameSettings();
	OfflineDialog* dlg = new OfflineDialog(mGameSettings);
	connect(dlg, SIGNAL(finished(int)), this, SLOT(on_OfflineDialog_Finished(int)));
	dlg->show();
}

void MainWindow::on_tabWidget_tabCloseRequested(int index) const
{
	QWidget* tab = ui->tabWidget->widget(index);

	if (!tab)
		return;

	bool closed = tab->close();
	if (closed)
		delete tab;
}

void MainWindow::on_JoinChatroomButton_clicked() const
{
	QListWidgetItem* item = ui->ChatroomListWidget->currentItem();

	if (item)
		client->joinChannel(client->chatRoomPrefix() + item->text());
}

void MainWindow::on_ChatroomListWidget_currentItemChanged() const
{
	updateJoinButton();
}

void MainWindow::on_JoinFriendlyButton_clicked()
{
	NetChannel chan = selectedFriendlyMatch();

	if (chan.isNull())
		return;

	GameWidget* existing = gameManager->findGame(chan.name);
	if (existing) {
		existing->raise();
		return;
	}

	QStringList items = chan.name.split(':');
	if (items.count() > 1) {
		QString item = items.at(1);
		if (!item.isEmpty()) {
			PasswordDialog* dlg = new PasswordDialog(this, gameManager, chan);
			dlg->show();
			dlg->raise();
			dlg->activateWindow();
			return;
		}
	}

	GameWidget* game = gameManager->createGame(chan.description, chan.name);

	if (!game)
		return;

	game->show();
	game->raise();
}

void MainWindow::on_SpectateFriendlyButton_clicked()
{
	NetChannel chan = selectedFriendlyMatch();

	if (chan.isNull())
		return;

	GameWidget* existing = gameManager->findGame(chan.name);
	if (existing) {
		existing->raise();
		return;
	}

	// Ask for password (if any)
	QStringList items = chan.name.split(':');
	if (items.count() > 1) {
		QString item = items.at(1);
		if (!item.isEmpty()) {
			PasswordDialog* dlg = new PasswordDialog(this, gameManager, chan, true);
			dlg->show();
			dlg->raise();
			dlg->activateWindow();
			return;
		}
	}

	GameWidget* game = gameManager->createGame(chan.description, chan.name, true);

	if (!game)
		return;

	game->show();
	game->raise();
}

void MainWindow::on_ReviewRulesFriendlyButton_clicked()
{
	NetChannel chan = selectedFriendlyMatch();

	if (chan.isNull())
		return;

	ppvs::RuleSetInfo rs;
	readRulesetString(chan.description, &rs);
	reviewRulesDialog(rs);
}

void MainWindow::on_FriendlyMatchesTreeWidget_itemSelectionChanged() const
{
	NetChannel chan = selectedFriendlyMatch();
	bool validSelection = !chan.isNull();

	ui->JoinFriendlyButton->setEnabled(validSelection);
	ui->SpectateFriendlyButton->setEnabled(validSelection);
	ui->ReviewRulesFriendlyButton->setEnabled(validSelection);

	if (validSelection) {
		ui->FriendlyMatchScore->setText(ui->FriendlyMatchesTreeWidget->selectedItems().at(0)->data(0, kScoreRole).toString());
	} else {
		ui->FriendlyMatchScore->setText(tr("Click an item to show scores.", "ShowScores"));
	}
}

void MainWindow::on_ReplaysToolButton_clicked()
{
	ReplayDialog* dlg = new ReplayDialog(this, gameManager);
	dlg->show();
	dlg->raise();
	dlg->activateWindow();
}

void MainWindow::reviewRulesDialog(ppvs::RuleSetInfo& rs)
{
	QString ruleStr;
	ruleStr = QString::asprintf(tr("Margin Time: %i\nTarget Point: %i\nRequired Chain: %i\n"
								   "Initial Fever Count: %i\nFever Power: %i\nPuyo To Clear: %i\n"
								   "Quick Drop: %i\nNumber of players: %i\nChoose colors: %i",
									"Messages:ReviewRules")
									.toUtf8()
									.data(),
		rs.marginTime, rs.targetPoint, rs.requiredChain, rs.initialFeverCount, rs.feverPower, rs.puyoToClear, rs.quickDrop, rs.numPlayers, rs.colors);
	QMessageBox* msgBox = new QMessageBox;
	msgBox->setText(ruleStr);
	msgBox->setWindowTitle("Rules");
	msgBox->show();
}

void MainWindow::updateServerList()
{
	// Download list from puyovs.com
	netMan = new QNetworkAccessManager(this);
	QNetworkRequest request;
	request.setUrl(QUrl("https://puyovs.com/files/servers.txt"));
	request.setRawHeader("User-Agent", PUYOVS_USER_AGENT);
	serverListReply = netMan->get(request);
	connect(serverListReply, SIGNAL(finished()), SLOT(getServerList()));
}

void MainWindow::on_PasswordLineEdit_textEdited(const QString& arg1)
{
	Settings& settings = pvsApp->settings();
	if (!settings.string("account", "password", "").isEmpty()) {
		settings.setString("account", "password", "");
	}
	passEdited = true;
}

void MainWindow::on_AdminToolButton_clicked() const
{
	AdminDialog* dlg = new AdminDialog(client);
	dlg->show();
}

void MainWindow::on_RankedTsuPlayButton_clicked() const
{
	startRankedMatch(true);
	for (int i = 0; i < ui->tabWidget->count(); ++i) {
		ChatroomForm* chatroom = qobject_cast<ChatroomForm*>(ui->tabWidget->widget(i));
		if (chatroom) {
			chatroom->setAutoreject();
		}
	}
}

void MainWindow::on_RankedTsuSpectateButton_clicked() const
{
	spectateRankedMatch(true);
}

void MainWindow::on_RankedFeverPlayButton_clicked() const
{
	startRankedMatch(false);
	for (int i = 0; i < ui->tabWidget->count(); ++i) {
		ChatroomForm* chatroom = qobject_cast<ChatroomForm*>(ui->tabWidget->widget(i));
		if (chatroom) {
			chatroom->setAutoreject();
		}
	}
}

void MainWindow::on_RankedFeverSpectateButton_clicked() const
{
	spectateRankedMatch(false);
}

void MainWindow::on_OfflineDialog_Finished(int result)
{
	if (!mGameSettings)
		return;

	mGameSettings->playSound = true;
	mGameSettings->playMusic = true;

	switch (result) {
	case QDialog::Accepted:
		if (mGameSettings) {
			GameWidget* g = gameManager->createGame(mGameSettings, QString());
			g->show();
			mGameSettings = nullptr;
		}
		break;
	default:
	case QDialog::Rejected:
		delete mGameSettings;
		mGameSettings = nullptr;
		break;
	}
}

void MainWindow::on_CreateChatroomDialog_Finished(CreateChatroomDialog* createChatDlg) const
{
	client->createChannel(client->chatRoomPrefix() + createChatDlg->name(), createChatDlg->description(), false, true);
}

void MainWindow::getServerList() const
{
	QByteArray replyArray = serverListReply->readAll();
	QString replyString = QString(replyArray);

	// Parse
	QStringList servers = replyString.split(
		QRegExp("\n|\r\n|\r"),
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
		Qt::SkipEmptyParts
#else
		QString::SkipEmptyParts
#endif
	);

	foreach (QString server, servers) {
		// Add to combo box
		ui->ServerComboBox->addItem(server);
	}
}

void MainWindow::on_ActionSearch_triggered()
{
	if (!showSearchDlg) {
		searchDlg = new SearchDialog(this, client);
		connect(searchDlg, SIGNAL(finished(int)), this, SLOT(on_SearchDialog_Finished(int)));
		searchDlg->show();
		showSearchDlg = true;
	}
}
