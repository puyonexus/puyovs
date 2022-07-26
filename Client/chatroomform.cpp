#include "chatroomform.h"
#include "../Puyolib/GameSettings.h"
#include "chatroomtextedit.h"
#include "common.h"
#include "gamemanager.h"
#include "gamewidget.h"
#include "settings.h"
#include "ui_chatroomform.h"
#include <QCloseEvent>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QTime>

ChatroomForm::ChatroomForm(NetPeerList peers, NetChannelProxy* proxy, GameManager* manager, QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::ChatroomForm)
	, statusIconAvailable(":/icons/blue.png")
	, statusIconAway(":/icons/gray.png")
	, mProxy(proxy)
	, gameManager(manager)
{
	ui->setupUi(this);
	ui->ChallengeIcon->setPixmap(QPixmap("Data/Lobby/none.png"));

	foreach (NetPeer peer, peers) {
		QListWidgetItem* item = new QListWidgetItem(peer.status == 0 ? statusIconAvailable : statusIconAway, peer.username);
		ui->UserListWidget->addItem(item);
		item->setData(Qt::UserRole, peer.status);
	}
	countUsers();

	ui->UserListWidget->sortItems(Qt::AscendingOrder);

	connect(mProxy, SIGNAL(messageReceived(uchar, QString, QString)), SLOT(messageReceived(uchar, QString, QString)));
	connect(mProxy, SIGNAL(peerMessageReceived(uchar, QString, QString)), SLOT(peerMessageReceived(uchar, QString, QString)));
	connect(mProxy, SIGNAL(statusReceived(QString, uchar)), SLOT(statusReceived(QString, uchar)));
	connect(mProxy, SIGNAL(peerJoined(QString)), SLOT(peerJoined(QString)));
	connect(mProxy, SIGNAL(peerParted(QString)), SLOT(peerParted(QString)));
	connect(mProxy, SIGNAL(userInfoReceived(QString)), SLOT(userInfoReceived(QString)));
	connect(ui->ChatTextEdit, SIGNAL(anchorClicked(QString)), SLOT(urlClicked(QString)));

	statusMessage(ChannelJoinedEvent, mProxy->friendlyChannel());

	loadSettings();
	updateChallengeDisplay();
	updateChallengeButton();
}

ChatroomForm::~ChatroomForm()
{
	// Save chat
	Settings& settings = pvsApp->settings();
	if (settings.boolean("launcher", "savechat", false)) {
		QDir dir;
		QString path = QCoreApplication::applicationDirPath() + "/User/Logs/" + mProxy->friendlyChannel() + "/";
		if (dir.mkpath(path)) {
			QDate now = QDate::currentDate();
			QFile file(path + now.toString("yyyy-M-d") + ".html");
			file.open(QIODevice::WriteOnly | QIODevice::Append);
			QTextStream stream(&file);
			stream << ui->ChatTextEdit->toPlainText();
		}
	}
	delete mProxy;
	delete ui;
}

void ChatroomForm::refreshLanguages()
{
	ui->RulesComboBox->blockSignals(true);
	ui->PlayersSpinbox->blockSignals(true);
	ui->retranslateUi(this);
	ui->RulesComboBox->blockSignals(false);
	ui->PlayersSpinbox->blockSignals(false);
	loadSettings();
	countUsers();
}

enum StatusEvent {
	ChannelJoinedEvent,
	ChannelLeftEvent,

	PeerJoinedEvent,
	PeerLeftEvent,

	MatchCancelEvent,
	MatchLeftEvent,
	MatchAutorejectEvent,
	MatchAutorejectSelfEvent,
	MatchDeclineEvent,
	MatchAcceptEvent
};
void ChatroomForm::statusMessage(ChatroomForm::StatusEvent type, QString arg) const
{
	QString message;

	switch (type) {
	case ChannelJoinedEvent:
		message = tr("Chatroom %s joined.", "Messages:ChannelJoined");
		break;
	case ChannelLeftEvent:
		message = tr("Chatroom %s left.", "Messages:ChannelLeft");
		break;
	case PeerJoinedEvent:
		message = tr("%s joined chatroom.", "Messages:PeerJoined");
		break;
	case PeerLeftEvent:
		message = tr("%s left chatroom.", "Messages:PeerLeft");
		break;
	case MatchCancelEvent:
		message = tr("Matchmaking canceled.", "Messages:MatchCancel");
		break;
	case MatchLeftEvent:
		message = tr("Challenger left.", "Messages:MatchLeft");
		break;
	case MatchAutorejectEvent:
		message = tr("User autorejected.", "Messages:MatchAutoreject");
		break;
	case MatchAutorejectSelfEvent:
		message = tr("Autorejected a challenge.", "Messages:MatchAutorejectSelf");
		break;
	case MatchDeclineEvent:
		message = tr("User declined the challenge.", "Messages:MatchDecline");
		break;
	case MatchAcceptEvent:
		message = tr("Get ready to play...", "Messages:MatchAccept");
		break;
	}

	// HACK: should convert properly
	if (!arg.isEmpty())
		message = message.replace("%s", "%1").arg(arg);

#if QT_VERSION < 0x050000
	appendLine(QString("<font color='gray'>%1</font>").arg(Qt::escape(message)));
#else
	appendLine(QString("<font color='gray'>%1</font>").arg(message.toHtmlEscaped()));
#endif
}

void ChatroomForm::appendLine(QString html) const
{
	QTime now = QTime::currentTime();
	ui->ChatTextEdit->moveCursor(QTextCursor::End);
	html.replace("\n", "<br/>");
	ui->ChatTextEdit->appendHtml(QString("<font color='gray'>[%1]</font> ").arg(now.toString("hh:mm")) + html);
	ui->ChatTextEdit->moveCursor(QTextCursor::End);
}

void ChatroomForm::updateChallengeButton() const
{
	QListWidgetItem* current = ui->UserListWidget->currentItem();

	if (current && current->text() != mProxy->nick() && current->data(Qt::UserRole) == 0 && !challenge.beingChallenged && challenge.challengedUser.isEmpty() && !isAutoRejectEnabled())
		ui->ChallengeButton->setEnabled(true);
	else
		ui->ChallengeButton->setEnabled(false);

	// Also check private room button: user must set a room password in settings
	Settings& settings = pvsApp->settings();
	if (settings.string("launcher", "roompassword", "").isEmpty())
		ui->PrivateCheckBox->setEnabled(false);
	else
		ui->PrivateCheckBox->setEnabled(true);
}

void ChatroomForm::updateChallengeDisplay()
{
	if (challenge.beingChallenged || challenge.challenging) {
		ui->ChallengeArea->setVisible(true);

		// Show rules.
		QString challengeRules;
		bool custom = challenge.rules.custom;
		int numPlayers = challenge.rules.numPlayers;
		switch (challenge.rules.ruleSetType) {
		case ppvs::Rules::TSU_ONLINE:
			challengeRules = QString::asprintf(tr("", custom ? "Messages:RulesTsuCustom" : "Messages:RulesTsuDefault").toUtf8().data(), numPlayers);
			ui->ChallengeIcon->setPixmap(QPixmap(!custom ? "Data/Lobby/tsu.png" : "Data/Lobby/tsu_cust.png"));
			break;
		case ppvs::Rules::FEVER_ONLINE:
			challengeRules = QString::asprintf(tr("", custom ? "Messages:RulesFeverCustom" : "Messages:RulesFeverDefault").toUtf8().data(), numPlayers);
			ui->ChallengeIcon->setPixmap(QPixmap(!custom ? "Data/Lobby/fever.png" : "Data/Lobby/fever_cust.png"));
			break;
		case ppvs::Rules::FEVER15_ONLINE:
			challengeRules = QString::asprintf(tr("", custom ? "Messages:RulesFever15Custom" : "Messages:RulesFever15Default").toUtf8().data(), numPlayers);
			ui->ChallengeIcon->setPixmap(QPixmap(!custom ? "Data/Lobby/fever15.png" : "Data/Lobby/fever15_cust.png"));
			break;
		case ppvs::Rules::ENDLESSFEVERVS_ONLINE:
			challengeRules = QString::asprintf(tr("", custom ? "Messages:RulesEndlessFeverCustom" : "Messages:RulesEndlessFeverDefault").toUtf8().data(), numPlayers);
			ui->ChallengeIcon->setPixmap(QPixmap(!custom ? "Data/Lobby/endlessfever.png" : "Data/Lobby/endlessfever_cust.png"));
			break;
		default:
			appendLine("Invalid challenge ruleset.");
			mProxy->sendChallengeMessage(challenge.challenger, "autoreject");
			challenge = ChallengeState();
			ui->ChallengeArea->setVisible(false);
			return;
		}
		ui->ChallengeRulesLabel->setText(challengeRules);

		// Show question.
		QString challengeQuestion;
		if (challenge.beingChallenged) {
			challengeQuestion = QString::asprintf(tr("You've been challenged to a match.\nChallenger: %s\nAccept challenge?", "Messages:GotChallenged").toUtf8().data(),
				challenge.challenger.toUtf8().data());
			ui->ChallengeYesButton->setVisible(true);
			ui->ChallengeNoButton->setVisible(true);
		} else if (challenge.challenging) {
			if (!challenge.challengedUser.isEmpty()) {
				challengeQuestion = tr("Now challenging...\nCancel challenge?", "Messages:ChallengedUser");
				ui->ChallengeYesButton->setVisible(true);
				ui->ChallengeNoButton->setVisible(false);
			} else {
				int n = challenge.challengedList.count();
				int total = challenge.rules.numPlayers - 1;
				challengeQuestion = QString::asprintf(tr("You challenged %i out of %i\nYou can challenge more users.\nOr start game now?", "Messages:ChallengeNumber").toUtf8().data(),
					n, total);
				ui->ChallengeYesButton->setVisible(true);
				ui->ChallengeNoButton->setVisible(false);
			}
		}
		ui->ChallengeQuestionLabel->setText(challengeQuestion);
	} else {
		ui->ChallengeQuestionLabel->setText(QString());
		ui->ChallengeIcon->setPixmap(QPixmap("Data/Lobby/none.png"));
		ui->ChallengeArea->setVisible(false);
	}
}

bool ChatroomForm::isAutoRejectEnabled() const
{
	return ui->AutoRejectCheckBox->isChecked();
}

ppvs::RuleSetInfo ChatroomForm::createRules() const
{
	ppvs::RuleSetInfo rs = gameManager->createRules();
	return rs;
}

void ChatroomForm::closeEvent(QCloseEvent* e)
{
	mProxy->part();
	e->accept();
}

void ChatroomForm::urlClicked(QString url)
{
	// Read rules to determine number of players
	readRulesetString(url, &challenge.rules);

	// Get roomname
	QStringList items = url.split('|');
	foreach (QString item, items) {
		QStringList subitem = item.split(':');

		if (subitem.count() < 1)
			continue;

		if (subitem[0] == "roomName" && subitem.count() != 1)
			challenge.room = subitem[1] + ":" + subitem[2];
	}

	challenge.game = gameManager->createGame(new ppvs::GameSettings(challenge.rules), challenge.room, true);
	challenge.game->show();
	challenge.game->raise();
	challenge = ChallengeState();
}

void ChatroomForm::userInfoReceived(QString msg) const
{
	QStringList items = msg.split("\n");
	QListWidgetItem* item = ui->UserListWidget->currentItem();
	if (items.count() > 0 && item && item->text() == items[0]) {
		ui->UserInfoTextEdit->clear();
		ui->UserInfoTextEdit->setText(msg);
	}
}

void ChatroomForm::messageReceived(uchar subchannel, QString nick, QString msg)
{
	Settings& settings = pvsApp->settings();

	switch (subchannel) {
	case CHANNEL_CHAT:
#if QT_VERSION < 0x050000
		appendLine(QString("<font color='blue'>%1</font>: %2").arg(Qt::escape(nick), Qt::escape(msg)));
#else
		appendLine(QString("<font color='blue'>%1</font>: %2").arg(nick.toHtmlEscaped(), msg.toHtmlEscaped()));
#endif
		if (!QWidget::isActiveWindow() && msg.indexOf(mProxy->nick()) >= 0 && settings.boolean("launcher", "alertname", true)) {
			QApplication::beep();
			QApplication::alert(this);
		}
		break;
	case CHANNEL_CHALLENGE:
		getInvitation(msg);
		break;
	case CHANNEL_CHAT_PRIVATE:
#if QT_VERSION < 0x050000
		appendLine(QString("<font color='green'>%1</font>: %2").arg(Qt::escape(nick), Qt::escape(msg)));
#else
		appendLine(QString("<font color='green'>%1</font>: %2").arg(nick.toHtmlEscaped(), msg.toHtmlEscaped()));
#endif
		if (!QWidget::isActiveWindow()) {
			QApplication::beep();
			QApplication::alert(this);
		}
		break;
	}
}

void ChatroomForm::peerMessageReceived(uchar subchannel, QString nick, QString msg)
{
	switch (subchannel) {
	case CHANNEL_CHALLENGE:
		if (msg == "cancel") {
			challenge = ChallengeState();
			statusMessage(MatchCancelEvent);
		} else if (msg == "autoreject") {
			challenge.challengedUser = QString();

			if (challenge.rules.numPlayers == 2)
				challenge = ChallengeState();
			else if (challenge.rules.numPlayers >= 2 && challenge.challengedList.isEmpty()) {
				delete challenge.game;
				challenge = ChallengeState();
			}

			statusMessage(MatchAutorejectEvent);
		} else {
			if (challenge.beingChallenged || challenge.challenging || isAutoRejectEnabled()) {
				mProxy->sendChallengeMessage(nick, "autoreject");
				statusMessage(MatchAutorejectSelfEvent);

				return;
			}

			QStringList items = msg.split('|');

			if (items.count() == 0)
				return;

			challenge = ChallengeState();
			challenge.beingChallenged = true;
			challenge.challenger = nick;
			challenge.room = items.at(0);

			readRulesetString(msg, &challenge.rules);
			if (!QWidget::isActiveWindow()) {
				QApplication::beep();
				QApplication::alert(this);
			}
		}
		break;
	case CHANNEL_CHALLENGERESPONSE:
		if (!challenge.challenging)
			return;

		if (challenge.challengedUser != nick)
			return;

		if (msg == "reject") {
			statusMessage(MatchDeclineEvent);
			challenge.challengedUser = QString();

			if (challenge.rules.numPlayers == 2)
				challenge = ChallengeState();
			else if (challenge.rules.numPlayers >= 2 && challenge.challengedList.isEmpty()) {
				delete challenge.game;
				challenge = ChallengeState();
			}
		} else if (msg == "accept") {
			challenge.challengedList.push_back(challenge.challengedUser);

			// Maximum number of players
			if (challenge.challengedList.count() >= challenge.rules.numPlayers - 1) {
				challenge.challenging = false;
				statusMessage(MatchAcceptEvent);

				inviteToMatch();
				/*
				QString match("match:");

				if(challenge.rules.Nplayers == 2 && challenge.challengedList.count() > 0)
				    match += challenge.challengedList.first();

				mProxy->sendChallengeMessage((QStringList() << match << challenge.room << createRulesetString(&challenge.rules)).join("|"));
				*/
				challenge.challengedList.clear();
				if (challenge.rules.numPlayers == 2) {
					challenge.game = gameManager->createGame(new ppvs::GameSettings(challenge.rules), challenge.room);
					challenge.game->show();
					challenge.game->raise();
					challenge = ChallengeState();
					// Auto-reject all panels?
					setAutoreject();
				} else {
					if (!challenge.game)
						return;

					challenge.game->show();
					challenge.game->raise();
					setAutoreject();

					challenge = ChallengeState();
				}

				challenge = ChallengeState();
			} else {
				challenge.challengedUser = QString();
			}
		}
		break;
	case CHANNEL_CHAT_PRIVATE:
#if QT_VERSION < 0x050000
		appendLine(QString("<font color='green'>&lt;%1&gt;</font>: %2").arg(Qt::escape(nick), Qt::escape(msg)));
#else
		appendLine(QString("<font color='green'>&lt;%1&gt;</font>: %2").arg(nick.toHtmlEscaped(), msg.toHtmlEscaped()));
#endif
		if (!QWidget::isActiveWindow()) {
			QApplication::beep();
			QApplication::alert(this);
		}
		return;
		break;
	}
	updateChallengeButton();
	updateChallengeDisplay();
}

void ChatroomForm::statusReceived(QString nick, uchar status) const
{
	QList<QListWidgetItem*> list = ui->UserListWidget->findItems(nick, Qt::MatchExactly);

	if (list.count() == 1) {
		list.at(0)->setIcon(status == 0 ? statusIconAvailable : statusIconAway);
		list.at(0)->setData(Qt::UserRole, status);
	}

	updateChallengeButton();
}

void ChatroomForm::peerJoined(QString peer) const
{
	QList<QListWidgetItem*> list = ui->UserListWidget->findItems(peer, Qt::MatchExactly);

	if (list.count() == 0) {
		QListWidgetItem* item = new QListWidgetItem(statusIconAvailable, peer);
		ui->UserListWidget->addItem(item);
		item->setData(Qt::UserRole, 0);
	}
	ui->UserListWidget->sortItems(Qt::AscendingOrder);
	statusMessage(PeerJoinedEvent, peer);
	updateChallengeButton();
	countUsers();
}

void ChatroomForm::peerParted(QString peer)
{
	QList<QListWidgetItem*> list = ui->UserListWidget->findItems(peer, Qt::MatchExactly);

	foreach (QListWidgetItem* item, list)
		delete item;

	statusMessage(PeerLeftEvent, peer);

	if (challenge.beingChallenged && peer == challenge.challenger || challenge.challenging && peer == challenge.challengedUser) {
		if (challenge.beingChallenged) {
			challenge = ChallengeState();
			statusMessage(MatchCancelEvent);
		} else if (challenge.challenging) {
			challenge.challengedUser = QString();
			if (challenge.rules.numPlayers == 2)
				challenge = ChallengeState();
			else if (challenge.rules.numPlayers >= 2 && challenge.challengedList.isEmpty()) {
				delete challenge.game;
				challenge = ChallengeState();
			}
		}
		updateChallengeDisplay();
		updateChallengeButton();
	}
	countUsers();
}

void ChatroomForm::on_EntryTextEdit_returnPressed()
{
	on_SendButton_clicked();
}

void ChatroomForm::on_SendButton_clicked()
{
	QString message = ui->EntryTextEdit->toPlainText();
	ui->EntryTextEdit->clear();

	if (!message.isEmpty()) {
		mProxy->sendMessage(message);
		messageReceived(CHANNEL_CHAT, mProxy->nick(), message);
	}
}

void ChatroomForm::on_ChallengeButton_clicked()
{
	static unsigned int challengeCounter = 0;

	if (challenge.beingChallenged)
		return;

	if (ui->AutoRejectCheckBox->isChecked())
		return;

	challenge.challengedUser = ui->UserListWidget->currentItem()->text();

	Settings& settings = pvsApp->settings();
	QString passHash;
	if (ui->PrivateCheckBox->isChecked() && !settings.string("launcher", "roompassword", "").isEmpty()) {
		passHash = getCryptographicHash(settings.string("launcher", "roompassword", ""));
	}

	// First challenge? Must create rules/room.
	if (!challenge.challenging) {
		challenge.rules = createRules();
		challenge.room = mProxy->client()->matchRoomPrefix() + mProxy->nick()
			+ QString::asprintf("%05i%03i", mProxy->id(), ++challengeCounter % 1000)
			+ ":" + passHash;
	}

	QString msg(challenge.room + "|" + createRulesetString(&challenge.rules));

	mProxy->sendChallengeMessage(challenge.challengedUser, msg);

	challenge.challenging = true;

	// For >2, create game now.
	if (challenge.rules.numPlayers > 2 && challenge.challengedList.empty()) {
		challenge.game = gameManager->createGame(new ppvs::GameSettings(challenge.rules), challenge.room);
		challenge.game->hide();
	}

	updateChallengeButton();
	updateChallengeDisplay();
}

void ChatroomForm::on_AutoRejectCheckBox_toggled(bool checked) const
{
	mProxy->setStatus(checked ? 1 : 0);
}

void ChatroomForm::on_UserListWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous) const
{
	updateChallengeButton();
	if (current) {
		mProxy->sendUserRequestInfo(current->text());
	}
}

void ChatroomForm::on_ChallengeYesButton_clicked()
{
	if (challenge.beingChallenged) {
		mProxy->sendMessage(CHANNEL_CHALLENGERESPONSE, challenge.challenger, "accept");
		challenge.game = gameManager->createGame(new ppvs::GameSettings(challenge.rules), challenge.room);
		challenge.game->show();
		challenge.game->raise();
		challenge = ChallengeState();
		setAutoreject();
	} else if (challenge.challenging) {
		// Cancel challenge.
		if (!challenge.challengedUser.isEmpty()) {
			mProxy->sendChallengeMessage(challenge.challengedUser, "cancel");

			challenge.challengedUser = QString();
			// Only challenged player?
			if (challenge.challengedList.count() == 0) {
				delete challenge.game;
				challenge = ChallengeState();
			}
		}
		// Do not wait for more players.
		else {
			if (challenge.game) {
				challenge.game->show();
				challenge.game->raise();
			}
			inviteToMatch();
			challenge = ChallengeState();
			// TODO: Set global autoreject here.
			setAutoreject();
		}
	}

	updateChallengeDisplay();
	updateChallengeButton();
}

void ChatroomForm::on_ChallengeNoButton_clicked()
{
	if (challenge.beingChallenged) {
		mProxy->sendMessage(CHANNEL_CHALLENGERESPONSE, challenge.challenger, "reject");

		challenge = ChallengeState();
	}

	updateChallengeDisplay();
	updateChallengeButton();
}

void ChatroomForm::on_ReviewRulesButton_clicked()
{
	reviewRulesDialog(challenge.rules);
}

void ChatroomForm::reviewRulesDialog(ppvs::RuleSetInfo& rs)
{
	// MainWindow contains the exact same function
	// Don't know where I can move this function so both classes can call this
	QString ruleStr;
	ruleStr = QString::asprintf(tr("Margin Time: %i\nTarget Point: %i\nRequired Chain: %i\n"
								   "Initial Fever Count: %i\nFever Power: %i\nPuyo To Clear: %i\n"
								   "Quick Drop: %i\nNumber of players: %i\nChoose colors: %i",
									"Messages:ReviewRules")
									.toUtf8()
									.data(),
		rs.marginTime, rs.targetPoint, rs.requiredChain, rs.initialFeverCount, rs.feverPower, rs.puyoToClear, rs.quickDrop, rs.numPlayers, rs.colors);
	QMessageBox* msgBox = new QMessageBox(this);
	msgBox->setText(ruleStr);
	msgBox->setWindowTitle("Rules");
	msgBox->show();
}

void ChatroomForm::inviteToMatch()
{
	QString rules = createRulesetString(&challenge.rules);

	// Check if challenge roomName contains a password
	QStringList items = challenge.room.split(
		':',
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
		Qt::SkipEmptyParts
#else
		QString::SkipEmptyParts
#endif
	);
	if (items.count() > 1 && !items[1].isEmpty()) {
		return;
	}

	// Additional info
	// [roomName:...][namep1:...][namep2...]
	QString add = QString("roomName:") + challenge.room + "|namep1:" + mProxy->nick() + "|namep2:" + challenge.challengedUser + "|";
	QString invite = add + rules;
	mProxy->sendChallengeMessage(invite);
}

void ChatroomForm::getInvitation(QString invite) const
{
	// Read rules to determine number of players
	ppvs::RuleSetInfo rs;
	readRulesetString(invite, &rs);

	// Get additional info out (p1 name and p2 name)
	QString namep1;
	QString namep2;
	QStringList items = invite.split('|');
	foreach (QString item, items) {
		QStringList subitem = item.split(':');

		if (subitem.count() < 1)
			continue;

		if (subitem[0] == "namep1" && subitem.count() != 1)
			namep1 = subitem[1];
		if (subitem[0] == "namep2" && subitem.count() != 1)
			namep2 = subitem[1];
	}
	// Display message
	QString players;
	// Case 2p: namep1 vs namep2, otherwise show number of players
	if (rs.numPlayers == 2)
		players = namep1 + " VS " + namep2;
	else
		players = QString("%1 player match").arg(rs.numPlayers);
	QString mes = tr("A match is starting:", "Messages:MatchStarting") + " (" + players + ")";
#if QT_VERSION < 0x050000
	mes = Qt::escape(mes);
#else
	mes = mes.toHtmlEscaped();
#endif
	mes += "<a href=\"" + invite + "\"> " + tr("Click here to spectate.", "Messages:MatchSpectate") + "</a>";
	appendLine(QString("<i>%1</i>").arg(mes));
}

void ChatroomForm::setAutoreject() const
{
	Settings& settings = pvsApp->settings();
	if (settings.boolean("launcher", "autoreject", true))
		ui->AutoRejectCheckBox->setChecked(true);
}

void ChatroomForm::loadSettings() const
{
	Settings& settings = pvsApp->settings();
	ui->PlayersSpinbox->blockSignals(true);
	ui->RulesComboBox->blockSignals(true);
	ui->PlayersSpinbox->setValue(settings.integer("rules", "numplayers", 2));
	ui->RulesComboBox->setCurrentIndex(settings.integer("rules", "baserules", 0));
	ui->PlayersSpinbox->blockSignals(false);
	ui->RulesComboBox->blockSignals(false);
}

void ChatroomForm::countUsers() const
{
	ui->usersOnlineLabel->setText(tr("Users online: %1", "Launcher:UsersOnline").arg(ui->UserListWidget->count()));
}

void ChatroomForm::on_PlayersSpinbox_valueChanged(int arg1)
{
	Settings& settings = pvsApp->settings();
	settings.setInteger("rules", "numplayers", arg1);
	settings.save();
}

void ChatroomForm::on_RulesComboBox_currentIndexChanged(int index)
{
	Settings& settings = pvsApp->settings();
	settings.setInteger("rules", "baserules", index);
	settings.setBoolean("rules", "default", true);
	settings.save();
}

void ChatroomForm::on_SendPrivateButton_clicked()
{
	QString user;
	if (ui->UserListWidget->currentItem())
		user = ui->UserListWidget->currentItem()->text();
	else
		return;
	if (user.isEmpty())
		return;

	QString message = ui->EntryTextEdit->toPlainText();
	ui->EntryTextEdit->clear();

	if (!message.isEmpty()) {
		mProxy->sendMessage(CHANNEL_CHAT_PRIVATE, user, message);
		messageReceived(CHANNEL_CHAT_PRIVATE, mProxy->nick() + " >> " + user, message);
	}
}

void ChatroomForm::on_UserListWidget_itemChanged(QListWidgetItem* item) const
{
	if (item)
		ui->SendPrivateButton->setEnabled(true);
	else
		ui->SendPrivateButton->setEnabled(false);
}

void ChatroomForm::on_EntryTextEdit_textChanged() const
{
	int max = 240;
	if (ui->EntryTextEdit->toPlainText().length() > max) {
		int diff = ui->EntryTextEdit->toPlainText().length() - max;
		QString newStr = ui->EntryTextEdit->toPlainText();
		newStr.chop(diff);
		ui->EntryTextEdit->setText(newStr);
		QTextCursor cursor(ui->EntryTextEdit->textCursor());
		cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
		ui->EntryTextEdit->setTextCursor(cursor);
	}
}
