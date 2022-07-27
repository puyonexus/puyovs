#pragma once

#include <QComboBox>
#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class LanguageManager;
class PlaylistModel;

class SettingsDialog : public QDialog {
	Q_OBJECT

public:
	enum Rule {
		Tsu,
		Fever,
		Fever15,
		EndlessFever
	};

	explicit SettingsDialog(LanguageManager* lm, QWidget* parent = nullptr);
	~SettingsDialog() override;

	void load();
	void save();

private slots:
	void languagesModified();
	void updateEnabled(Rule rule) const;
	void on_BaseRulesComboBox_currentIndexChanged(int index) const;
	void on_SettingsButtonBox_accepted();
	void on_LanguageComboBox_currentIndexChanged(int index) const;
	void on_SettingsButtonBox_rejected() const;
	void on_DefaultRulesCheckbox_clicked() const;
	void characterSlotIndexChanged(QString indexName) const;
	void on_DefaultButton_clicked() const;
	void on_PlaylistComboBox_currentIndexChanged(int index) const;
	void on_PlaylistAddButton_clicked();
	void on_PlaylistRemoveButton_clicked() const;
	void on_MusicVolumeHorizontalSlider_sliderMoved(int value);
	void on_SoundVolumeHorizontalSlider_sliderMoved(int value);

private:
	void fetchFileLists();
	void setDefaultRuleSettings() const;
	void translateDefaultCharacters() const;

	PlaylistModel *normalPlaylistModel, *feverPlaylistModel;

	QStringList backgroundsList;
	QStringList puyoList;
	QStringList charactersList;
	QStringList soundsList;
	QList<QComboBox*> characterComboBoxList;

	Ui::SettingsDialog* ui;
	LanguageManager* languageManager;
	int oldLanguage;
};
