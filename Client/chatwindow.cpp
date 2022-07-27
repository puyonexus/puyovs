#include "chatwindow.h"
#include "../Puyolib/Game.h"
#include "gamewidget.h"
#include "netclient.h"
#include "settings.h"
#include "ui_chatwindow.h"
#include <QTime>

ChatWindow::ChatWindow(NetChannelProxy* proxy, ppvs::Game* game, GameWidget* widget, QWidget* parent)
	: QWidget(parent)
	, mProxy(proxy)
	, mGame(game)
	, mGameWidget(widget)
	, quickchat(false)
	, ui(new Ui::ChatWindow)
{
	ui->setupUi(this);
	ui->ShowNameCheckBox->setChecked(mGame->m_forceStatusText);
	ui->PlayMusicCheckBox->setChecked(pvsApp->settings().boolean("launcher", "enablemusic", true));
	ui->PlaySoundCheckBox->setChecked(pvsApp->settings().boolean("launcher", "enablesound", true));
	setFocusPolicy(Qt::ClickFocus);
}

ChatWindow::~ChatWindow()
{
	delete ui;
}

void ChatWindow::setQuickChat(bool quick)
{
	if (quick) {
		quickchat = true;
		QPalette p = ui->EntryTextEdit->palette();
		p.setColor(QPalette::Base, Qt::cyan);
		ui->EntryTextEdit->setPalette(p);
		raise();
		activateWindow();
		showNormal();
		ui->EntryTextEdit->setFocus();
	} else {
		quickchat = false;
		QPalette p = ui->EntryTextEdit->palette();
		p.setColor(QPalette::Base, Qt::white);
		ui->EntryTextEdit->setPalette(p);
		mGameWidget->raise();
		mGameWidget->activateWindow();
		mGameWidget->setFocus();
	}
}

void ChatWindow::statusMessage(const QString& message) const
{
#if QT_VERSION < 0x050000
	appendLine(QString("<font color='gray'>%1</font>").arg(Qt::escape(message)));
#else
	appendLine(QString("<font color='gray'>%1</font>").arg(message.toHtmlEscaped()));
#endif
}

void ChatWindow::chatMessage(const QString& nick, const QString& message) const
{
#if QT_VERSION < 0x050000
	appendLine(QString("<font color='blue'>%1</font>: %2").arg(Qt::escape(nick), Qt::escape(message)));
#else
	appendLine(QString("<font color='blue'>%1</font>: %2").arg(nick.toHtmlEscaped(), message.toHtmlEscaped()));
#endif
}

void ChatWindow::appendLine(QString html) const
{
	QTime now = QTime::currentTime();
	ui->ChatTextView->moveCursor(QTextCursor::End);
	ui->ChatTextView->appendHtml(QString("<font color='gray'>[%1]</font> ").arg(now.toString("hh:mm")) + html);
	ui->ChatTextView->moveCursor(QTextCursor::End);
}

void ChatWindow::addUser(QString name, QString type) const
{
	QStringList itemStrings;
	itemStrings.append(name);
	itemStrings.append(type);
	QTreeWidgetItem* item = new QTreeWidgetItem(itemStrings);
	ui->UsersTreeWidget->addTopLevelItem(item);
}

void ChatWindow::removeUser(QString name) const
{
	QList<QTreeWidgetItem*> oldItems = ui->UsersTreeWidget->findItems(name, Qt::MatchCaseSensitive, 0);

	if (!oldItems.isEmpty())
		qDeleteAll(oldItems);
}

void ChatWindow::replaceProxy(NetChannelProxy* newproxy)
{
	mProxy = newproxy;
}

void ChatWindow::clearUsers() const
{
	ui->UsersTreeWidget->clear();
}

void ChatWindow::setChatEnabled(bool enabled) const
{
	ui->EntryTextEdit->setEnabled(enabled);
}

void ChatWindow::on_EntryTextEdit_returnPressed()
{
	if (quickchat)
		setQuickChat(false);

	QString message = ui->EntryTextEdit->toPlainText();
	if (message.isEmpty())
		return;
	ui->EntryTextEdit->clear();

	mProxy->sendMessage(message);
	chatMessage(mProxy->nick(), message);
}

void ChatWindow::on_ShowNameCheckBox_stateChanged(int arg1) const
{
	mGame->m_forceStatusText = arg1;
}

void ChatWindow::on_PlaySoundCheckBox_stateChanged(int arg1) const
{
	if (mGame->m_data)
		mGame->m_data->playSounds = arg1;
}

void ChatWindow::on_PlayMusicCheckBox_stateChanged(int arg1) const
{
	if (mGame->m_data)
		mGame->m_data->playMusic = arg1;
	if (arg1 == 0) {
		pvsApp->settings().setBoolean("launcher", "enablemusic", false);
		pvsApp->setMusicMode(PVSApplication::MusicPause);
	} else {
		pvsApp->settings().setBoolean("launcher", "enablemusic", true);
		mGameWidget->playMusic();
	}
}

void ChatWindow::focusInEvent(QFocusEvent* ev)
{
	// Reload global music settings
	ui->PlayMusicCheckBox->setChecked(pvsApp->settings().boolean("launcher", "enablemusic", true));
	QWidget::focusInEvent(ev);
}

void ChatWindow::on_EntryTextEdit_textChanged() const
{
	int max = 140;
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
