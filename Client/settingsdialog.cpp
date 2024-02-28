#include "settingsdialog.h"
#include "../Puyolib/AssetManager.h"
#include "language.h"
#include "playlist.h"
#include "pvsapplication.h"
#include "settings.h"
#include "ui_settingsdialog.h"
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <alib/audiolib.h>

SettingsDialog::SettingsDialog(LanguageManager* lm, ppvs::AssetManager* am, QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::SettingsDialog)
	, am_template(am)
{
	ui->setupUi(this);
	connect(ui->SettingsButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(ui->SettingsButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	normalPlaylistModel = new PlaylistModel(&pvsApp->playlist(), this);
	feverPlaylistModel = new PlaylistModel(&pvsApp->feverPlaylist(), this);
	ui->PlaylistView->setModel(normalPlaylistModel);

	characterComboBoxList = QList<QComboBox*>()
		<< ui->Char0ComboBox << ui->Char1ComboBox << ui->Char2ComboBox << ui->Char3ComboBox
		<< ui->Char4ComboBox << ui->Char5ComboBox << ui->Char6ComboBox << ui->Char7ComboBox
		<< ui->Char8ComboBox << ui->Char9ComboBox << ui->Char10ComboBox << ui->Char11ComboBox
		<< ui->Char12ComboBox << ui->Char13ComboBox << ui->Char14ComboBox << ui->Char15ComboBox
		<< ui->Char16ComboBox << ui->Char17ComboBox << ui->Char18ComboBox << ui->Char19ComboBox
		<< ui->Char20ComboBox << ui->Char21ComboBox << ui->Char22ComboBox << ui->Char23ComboBox;

	languageManager = lm;
	languagesModified();
	oldLanguage = lm->currentLanguageIndex();
	connect(languageManager, &LanguageManager::languagesModified, this, &SettingsDialog::languagesModified);
	foreach (QComboBox* combobox, characterComboBoxList)
		connect(combobox, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
			this, &SettingsDialog::characterSlotIndexChanged);
	translateDefaultCharacters();

	load();
	updateEnabled(Rule(ui->BaseRulesComboBox->currentIndex()));

	ui->PlaylistView->viewport()->setAcceptDrops(true);
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::load()
{
	Settings& settings = pvsApp->settings();

	// General
	ui->LanguageComboBox->setCurrentIndex(languageManager->currentLanguageIndex());

	ui->EnableMusicCheckBox->setChecked(settings.boolean("launcher", "enablemusic", true));
	ui->EnableSoundCheckBox->setChecked(settings.boolean("launcher", "enablesound", true));
	ui->AutosaveReplaysCheckBox->setChecked(settings.boolean("launcher", "savereplays", true));
	ui->SaveLogsCheckBox->setChecked(settings.boolean("launcher", "savechat", true));
	ui->AutorejectCheckBox->setChecked(settings.boolean("launcher", "autoreject", true));
	ui->AlertNameCheckBox->setChecked(settings.boolean("launcher", "alertname", true));
	ui->DefaultCharacterComboBox->setCurrentIndex(settings.integer("account", "defaultcharacter", 2));
	ui->MusicVolumeHorizontalSlider->setSliderPosition(settings.integer("music", "musicvolume", 100));
	ui->SoundVolumeHorizontalSlider->setSliderPosition(settings.integer("music", "soundvolume", 100));
	ui->RoomPasswordLineEdit->setText(settings.string("launcher", "roompassword", ""));
	ui->AllowCrashDumpsCheckBox->setChecked(settings.boolean("telemetry", "collectdebugdata", false));
	ui->AllowUsageStatisticsCheckBox->setChecked(settings.boolean("telemetry", "collectusagedata", false));

	// Rules
	ui->BaseRulesComboBox->setCurrentIndex(settings.integer("rules", "baserules", 0));
	ui->DefaultRulesCheckbox->setChecked(settings.boolean("rules", "default", true));
	ui->MarginTimeSpinBox->setValue(settings.integer("rules", "margintime", 192));
	ui->TargetPointsSpinBox->setValue(settings.integer("rules", "targetpoints", 70));
	ui->RequiredChainSpinBox->setValue(settings.integer("rules", "requiredchain", 0));
	ui->PuyoToClearSpinBox->setValue(settings.integer("rules", "puyotoclear", 4));
	ui->InitialFeverCountSpinBox->setValue(settings.integer("rules", "initialfevercount", 0));
	ui->FeverPowerSpinBox->setValue(settings.integer("rules", "feverpower", 100));
	ui->NumberOfColorsSpinBox->setValue(settings.integer("rules", "colors", 4));
	ui->NumberOfColorsSelectCheckBox->setChecked(settings.boolean("rules", "colorselect", false));
	ui->NumberOfPlayersSpinBox->setValue(settings.integer("rules", "numplayers", 2));
	ui->QuickDropCheckBox->setChecked(settings.boolean("rules", "quickdrop", false));

	// Controls
	ui->UpInput->setText(settings.string("controlsp1", "up", "up"));
	ui->DownInput->setText(settings.string("controlsp1", "down", "down"));
	ui->LeftInput->setText(settings.string("controlsp1", "left", "left"));
	ui->RightInput->setText(settings.string("controlsp1", "right", "right"));
	ui->AInput->setText(settings.string("controlsp1", "a", "x"));
	ui->BInput->setText(settings.string("controlsp1", "b", "z"));
	ui->StartInput->setText(settings.string("controlsp1", "start", "return"));
	ui->SwapABConfirmCheckBox->setChecked(settings.boolean("controlsp1", "swapabconfirm", false));

	// Customization
	fetchFileLists();
	ui->BackgroundComboBox->setCurrentIndex(ui->BackgroundComboBox->findText(settings.string("custom", "background", "Forest")));
	ui->PuyoComboBox->setCurrentIndex(ui->PuyoComboBox->findText(settings.string("custom", "puyo", "Default")));
	ui->SoundComboBox->setCurrentIndex(ui->SoundComboBox->findText(settings.string("custom", "sound", "Default")));
	ui->ApplyCharacterCheckBox->setCheckState(settings.boolean("custom", "characterfield", true) ? Qt::Checked : Qt::Unchecked);
	QStringList charmap = settings.charMap();
	for (int i = 0; i < qMin(characterComboBoxList.count(), charmap.count()); i++) {
		QComboBox* combobox = characterComboBoxList.at(i);
		QString currentChar = charmap.at(i);
		combobox->setCurrentIndex(combobox->findText(currentChar));
	}

	// Music
	ui->MusicAdvanceAtRoundStartCheckbox->setChecked(settings.boolean("music", "advance", true));
	ui->MusicAtLeastOnceCheckBox->setChecked(settings.boolean("music", "looponce", true));
	ui->MusicStopAfterRoundCheckBox->setChecked(settings.boolean("music", "stopmusicafterround", false));
	ui->MusicRandomizeOrder->setChecked(settings.boolean("music", "randomorder", true));
	ui->MusicLoopModeComboBox->setCurrentIndex(settings.integer("music", "loopmode", 0));
}

void SettingsDialog::save()
{
	Settings& settings = pvsApp->settings();

	// General
	QString langFn = languageManager->currentLanguageFileName();
	if (langFn.startsWith("Language/"))
		langFn.remove(0, 9);
	settings.setString("launcher", "language", langFn);

	settings.setBoolean("launcher", "enablemusic", ui->EnableMusicCheckBox->isChecked());
	settings.setBoolean("launcher", "enablesound", ui->EnableSoundCheckBox->isChecked());
	settings.setBoolean("launcher", "savereplays", ui->AutosaveReplaysCheckBox->isChecked());
	settings.setBoolean("launcher", "savechat", ui->SaveLogsCheckBox->isChecked());
	settings.setBoolean("launcher", "autoreject", ui->AutorejectCheckBox->isChecked());
	settings.setBoolean("launcher", "alertname", ui->AlertNameCheckBox->isChecked());

	settings.setInteger("account", "defaultcharacter", ui->DefaultCharacterComboBox->currentIndex());
	settings.setInteger("music", "musicvolume", ui->MusicVolumeHorizontalSlider->sliderPosition());
	settings.setInteger("music", "soundvolume", ui->SoundVolumeHorizontalSlider->sliderPosition());
	settings.setString("launcher", "roompassword", ui->RoomPasswordLineEdit->text());

	settings.setBoolean("telemetry", "collectdebugdata", ui->AllowCrashDumpsCheckBox->isChecked());
	settings.setBoolean("telemetry", "collectusagedata", ui->AllowUsageStatisticsCheckBox->isChecked());

	// Rules
	settings.setInteger("rules", "baserules", ui->BaseRulesComboBox->currentIndex());
	settings.setBoolean("rules", "default", ui->DefaultRulesCheckbox->isChecked());
	settings.setInteger("rules", "margintime", ui->MarginTimeSpinBox->value());
	settings.setInteger("rules", "targetpoints", ui->TargetPointsSpinBox->value());
	settings.setInteger("rules", "requiredchain", ui->RequiredChainSpinBox->value());
	settings.setInteger("rules", "puyotoclear", ui->PuyoToClearSpinBox->value());
	settings.setInteger("rules", "initialfevercount", ui->InitialFeverCountSpinBox->value());
	settings.setInteger("rules", "feverpower", ui->FeverPowerSpinBox->value());
	settings.setInteger("rules", "colors", ui->NumberOfColorsSpinBox->value());
	settings.setBoolean("rules", "colorselect", ui->NumberOfColorsSelectCheckBox->isChecked());
	settings.setInteger("rules", "numplayers", ui->NumberOfPlayersSpinBox->value());
	settings.setBoolean("rules", "quickdrop", ui->QuickDropCheckBox->isChecked());

	// Controls
	settings.setString("controlsp1", "up", ui->UpInput->text());
	settings.setString("controlsp1", "down", ui->DownInput->text());
	settings.setString("controlsp1", "left", ui->LeftInput->text());
	settings.setString("controlsp1", "right", ui->RightInput->text());
	settings.setString("controlsp1", "a", ui->AInput->text());
	settings.setString("controlsp1", "b", ui->BInput->text());
	settings.setString("controlsp1", "start", ui->StartInput->text());
	settings.setBoolean("controlsp1", "swapabconfirm", ui->SwapABConfirmCheckBox->isChecked());

	// Customization
	settings.setString("custom", "background", ui->BackgroundComboBox->currentText());
	settings.setString("custom", "puyo", ui->PuyoComboBox->currentText());
	settings.setString("custom", "sound", ui->SoundComboBox->currentText());
	QStringList characters;
	foreach (QComboBox* cb, characterComboBoxList)
		characters.append(cb->currentText());
	settings.setCharMap(characters);
	settings.setBoolean("custom", "characterfield", ui->ApplyCharacterCheckBox->isChecked());

	// Music
	settings.setBoolean("music", "advance", ui->MusicAdvanceAtRoundStartCheckbox->isChecked());
	settings.setBoolean("music", "looponce", ui->MusicAtLeastOnceCheckBox->isChecked());
	settings.setBoolean("music", "stopmusicafterround", ui->MusicStopAfterRoundCheckBox->isChecked());
	settings.setBoolean("music", "randomorder", ui->MusicRandomizeOrder->isChecked());
	settings.setInteger("music", "loopmode", ui->MusicLoopModeComboBox->currentIndex());

	settings.save();
}

void SettingsDialog::languagesModified()
{
	int defaultchar = ui->DefaultCharacterComboBox->currentIndex();
	ui->BaseRulesComboBox->blockSignals(true);
	ui->LanguageComboBox->blockSignals(true);
	ui->LanguageComboBox->clear();
	int languageCount = languageManager->languageCount();

	for (int i = 0; i < languageCount; ++i)
		ui->LanguageComboBox->addItem(languageManager->language(i)->realName(), i);

	int language = languageManager->currentLanguageIndex();
	if (language != -1 && language != ui->LanguageComboBox->currentIndex())
		ui->LanguageComboBox->setCurrentIndex(language);
	ui->LanguageComboBox->blockSignals(false);
	ui->retranslateUi(this);
	ui->BaseRulesComboBox->blockSignals(false);
	ui->DefaultCharacterComboBox->setCurrentIndex(defaultchar);
}

void SettingsDialog::updateEnabled(SettingsDialog::Rule rule) const
{
	bool enabled = !ui->DefaultRulesCheckbox->isChecked();
	ui->DefaultButton->setEnabled(enabled);
	ui->MarginTimeSpinBox->setEnabled(enabled);
	ui->TargetPointsSpinBox->setEnabled(enabled);
	ui->RequiredChainSpinBox->setEnabled(enabled);
	ui->PuyoToClearSpinBox->setEnabled(enabled);
	ui->InitialFeverCountSpinBox->setEnabled(enabled);
	ui->FeverPowerSpinBox->setEnabled(enabled);
	ui->NumberOfColorsSpinBox->setEnabled(enabled);
	ui->NumberOfColorsSelectCheckBox->setEnabled(enabled);
	ui->NumberOfPlayersSpinBox->setEnabled(enabled);
	ui->QuickDropCheckBox->setEnabled(enabled);

	if (!enabled)
		return;

	switch (rule) {
	case Rule::Tsu:
		ui->InitialFeverCountSpinBox->setEnabled(false);
		ui->FeverPowerSpinBox->setEnabled(false);
		break;
	case Rule::Fever:
		ui->PuyoToClearSpinBox->setEnabled(false);
		break;
	case Rule::Fever15:
		// ...Everything can be enabled?
		break;
	case Rule::EndlessFever:
		ui->PuyoToClearSpinBox->setEnabled(false);
		ui->InitialFeverCountSpinBox->setEnabled(false);
		break;
	}
}

void SettingsDialog::on_BaseRulesComboBox_currentIndexChanged(int index) const
{
	setDefaultRuleSettings();
	updateEnabled(Rule(index));
}

void SettingsDialog::on_SettingsButtonBox_accepted()
{
	save();

	languageManager->setCurrentLanguageIndex(ui->LanguageComboBox->currentIndex());
}

void SettingsDialog::on_SettingsButtonBox_rejected() const
{
	languageManager->setCurrentLanguageIndex(oldLanguage);
}

void SettingsDialog::on_LanguageComboBox_currentIndexChanged(int index) const
{
	if (languageManager->currentLanguageIndex() != index)
		languageManager->setCurrentLanguageIndex(ui->LanguageComboBox->currentIndex());
	translateDefaultCharacters();
}

void SettingsDialog::on_DefaultRulesCheckbox_clicked() const
{
	updateEnabled(Rule(ui->BaseRulesComboBox->currentIndex()));
}

void SettingsDialog::characterSlotIndexChanged(const QString&) const
{
	QComboBox* comboBox = qobject_cast<QComboBox*>(sender());
	QString senderName = comboBox->objectName();
	// Retrieve number
	// TODO: something supposed to go here...?
}

//
QStringList convertToQStringList(const std::set<std::string>& inputSet)
{
	QStringList outputList;
	outputList.reserve(inputSet.size());

	for (const auto& str : inputSet) {
		outputList.append(QString::fromStdString(str));
	}

	return outputList;
}

void SettingsDialog::fetchFileLists()
{
	assert(am_template != nullptr);

	// HACK: possible infinite loop, this will wait until the template AM is set to be initialized
	// This is to prevent race conditions if a different thread manipulates the same object
	while (!am_template->is_initialized())
	{}

	// Backgrounds
	ui->BackgroundComboBox->addItems(convertToQStringList(am_template->listBackgrounds()));

	// Puyo
	ui->PuyoComboBox->addItems(convertToQStringList(am_template->listPuyoSkins()));

	// Characters
	foreach (QComboBox* combobox, characterComboBoxList) {
		combobox->addItem("");
		combobox->addItems(convertToQStringList(am_template->listCharacterSkins()));
	}

	// Sound
	ui->SoundComboBox->addItems(convertToQStringList(am_template->listSfx()));
}

void SettingsDialog::setDefaultRuleSettings() const
{
	int index = ui->BaseRulesComboBox->currentIndex();
	switch (Rule(index)) {
	case Rule::Tsu:
		ui->TargetPointsSpinBox->setValue(70);
		break;
	case Rule::Fever15:
		ui->TargetPointsSpinBox->setValue(120);
		break;
	case Rule::Fever:
		ui->TargetPointsSpinBox->setValue(120);
		break;
	case Rule::EndlessFever:
		ui->TargetPointsSpinBox->setValue(120);
		break;
	}
	ui->MarginTimeSpinBox->setValue(192);
	ui->RequiredChainSpinBox->setValue(0);
	ui->PuyoToClearSpinBox->setValue(4);
	ui->InitialFeverCountSpinBox->setValue(0);
	ui->FeverPowerSpinBox->setValue(100);
	ui->NumberOfColorsSpinBox->setValue(4);
	ui->NumberOfColorsSelectCheckBox->setChecked(false);
	ui->QuickDropCheckBox->setChecked(false);
}

void SettingsDialog::translateDefaultCharacters() const
{
	for (int i = 0; i < ui->DefaultCharacterComboBox->count(); i++) {
		QString c = QString("Char%1").arg(i + 1);
		ui->DefaultCharacterComboBox->setItemText(i, languageManager->translate("Launcher", c));
	}
}

void SettingsDialog::on_DefaultButton_clicked() const
{
	setDefaultRuleSettings();
}

void SettingsDialog::on_PlaylistComboBox_currentIndexChanged(int index) const
{
	switch (index) {
	case 0:
		ui->PlaylistView->setModel(normalPlaylistModel);
		break;
	case 1:
		ui->PlaylistView->setModel(feverPlaylistModel);
		break;
	}
}

void SettingsDialog::on_PlaylistAddButton_clicked()
{
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Music Files"), QString(),
		"Audio Files (*.ogg *.logg *.wav *.vag *.dsp *.stm *.adx *.xa);;"
		"Ogg Vorbis (*.ogg *.logg);;"
		"Microsoft WAV (*.wav);;"
		"Sony PS2 VAG (*.vag);;"
		"GameCube ADPCM (*.dsp *.stm);;"
		"CRI ADX (*.adx);;"
		"Sony PSX XA (*.xa);;"
		"All files (*.*)");

	Playlist* playlist = &pvsApp->playlist();
	switch (ui->PlaylistComboBox->currentIndex()) {
	case 0:
		playlist = &pvsApp->playlist();
		break;
	case 1:
		playlist = &pvsApp->feverPlaylist();
		break;
	}

	foreach (QString file, files)
		playlist->add(PlaylistEntry("", file));
}

void SettingsDialog::on_PlaylistRemoveButton_clicked() const
{
	ui->PlaylistView->model()->removeRow(ui->PlaylistView->currentIndex().row());
}

void SettingsDialog::on_MusicVolumeHorizontalSlider_sliderMoved(int value)
{
	alib::open()->setMusicVolume(value / 100.0f);
}

void SettingsDialog::on_SoundVolumeHorizontalSlider_sliderMoved(int value)
{
	alib::open()->setSoundVolume(value / 100.0f);
}
