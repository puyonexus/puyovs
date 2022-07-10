#pragma once

#include <QWidget>

namespace Ui {
class ChatWindow;
}
namespace ppvs {
    class game;
}

class NetChannelProxy;
class GameWidget;

class ChatWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit ChatWindow(NetChannelProxy *proxy, ppvs::game *game, GameWidget *widget, QWidget *parent = 0);
    ~ChatWindow();

    void setQuickChat(bool);
    void statusMessage(const QString &message);
    void chatMessage(const QString &nick, const QString &message);
    void appendLine(QString html);
    void addUser(QString name,QString type);
    void removeUser(QString name);
    void replaceProxy(NetChannelProxy *newproxy);
    void clearUsers();
    void setChatEnabled(bool);
public slots:
    void on_EntryTextEdit_returnPressed();

signals:
    void messageSubmitted(const QString &message);
    
private slots:
    void on_ShowNameCheckBox_stateChanged(int arg1);

    void on_PlaySoundCheckBox_stateChanged(int arg1);

    void on_PlayMusicCheckBox_stateChanged(int arg1);
    void on_EntryTextEdit_textChanged();

protected:
    void focusInEvent(QFocusEvent *);

private:
    NetChannelProxy *mProxy;
    ppvs::game *mGame;
    GameWidget *mGameWidget;
    bool quickchat;
    Ui::ChatWindow *ui;
};
