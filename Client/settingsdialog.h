#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>

namespace Ui {
class SettingsDialog;
}

class LanguageManager;
class PlaylistModel;

class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    enum Rule
    {
        Tsu,
        Fever,
        Fever15,
        EndlessFever
    };

    explicit SettingsDialog(LanguageManager *lm, QWidget *parent = 0);
    ~SettingsDialog();

    void load();
    void save();
    
private slots:
    void languagesModified();
    void updateEnabled(Rule rule);
    void on_BaseRulesComboBox_currentIndexChanged(int index);
    void on_SettingsButtonBox_accepted();
    void on_LanguageComboBox_currentIndexChanged(int index);
    void on_SettingsButtonBox_rejected();
    void on_DefaultRulesCheckbox_clicked();
    void characterSlotIndexChanged(QString indexName);
    void on_DefaultButton_clicked();
    void on_PlaylistComboBox_currentIndexChanged(int index);
    void on_PlaylistAddButton_clicked();
    void on_PlaylistRemoveButton_clicked();
    void on_MusicVolumeHorizontalSlider_sliderMoved(int value);
    void on_SoundVolumeHorizontalSlider_sliderMoved(int value);

private:
    void fetchFileLists();
    void setDefaultRuleSettings();
    void translateDefaultCharacters();

    PlaylistModel *normalPlaylistModel, *feverPlaylistModel;

    QStringList backgroundsList;
    QStringList puyoList;
    QStringList charactersList;
    QStringList soundsList;
    QList<QComboBox*> characterComboBoxList;

    Ui::SettingsDialog *ui;
    LanguageManager *languageManager;
    int oldLanguage;
};

#endif // SETTINGSDIALOG_H
