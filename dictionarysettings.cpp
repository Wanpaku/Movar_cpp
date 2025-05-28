/*Copyright (C) 2024  Teg Miles

This file is part of Movar.

Movar is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Movar is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Movar. If not, see <https://www.gnu.org/licenses/>.*/

#include "dictionarysettings.h"
#include "ui_dictionarysettings.h"
#include <iostream>

DictionarySettings::DictionarySettings(
    std::shared_ptr<FileLoader> new_fileloader, QPointer<QWidget> parent)
    : QDialog(parent)
    , fileloader(std::move(new_fileloader))
    , ui_dict_settings(std::make_shared<Ui::DictionarySettings>())
{
    ui_dict_settings->setupUi(this);
    creating_shortcuts();
    set_available_dicts();
    load_settings();
}

auto DictionarySettings::get_dict_groups_list_widgets()
    -> QList<QPointer<QListWidget>>
{
    return dict_groups_list_widgets;
}

void DictionarySettings::creating_shortcuts()
{
    const QPointer<QShortcut> shortcut
        = new QShortcut(QKeySequence("F2"), this);
    connect(shortcut, &QShortcut::activated, this, &DictionarySettings::close);
}

void DictionarySettings::load_settings()
{
    //Завантаження налаштувань програми для поточного вікна
    settings = fileloader->get_settings();
    ui_dict_settings->buttonBox->
        button(QDialogButtonBox::Close)->setText(tr("Close"));
    load_paths_to_dicts_config();
    load_dict_groups();
    load_tts_engines();
    load_tts_languages();
    load_tts_voices();
    load_tts_voice_volume();
    load_tts_voice_rate();
    load_tts_voice_pitch();
}

void DictionarySettings::load_paths_to_dicts_config()
{
    //Завантаження шляхів до словників
    const QString& default_paths_to_dicts {""};
    const QString paths_to_dicts
        = settings
              ->value("dictsettings/paths_to_dicts", default_paths_to_dicts)
              .toString();
    if (paths_to_dicts != default_paths_to_dicts)
    {
        QStringList paths = paths_to_dicts.split('\n', Qt::SkipEmptyParts);
        for (const auto& path: std::as_const(paths))
        {
            ui_dict_settings->path_to_dict_list->addItem(path);
        }
    }
}

void DictionarySettings::load_dict_groups()
{
    //Завантаження груп словників
    const QString& default_dict_groups {""};
    const QString current_dict_groups
        = settings->value("dictsettings/dict_groups", default_dict_groups)
              .toString();

    if (current_dict_groups != default_dict_groups)
    {
        const int& tab_amount
            = static_cast<int>(current_dict_groups.count('\n'));
        QStringList dict_groups = current_dict_groups.split('\n');
        for (int i=0; i<tab_amount; ++i)
        {
            const int& dicts_amount
                = static_cast<int>(dict_groups[i].count(','));
            const QString& group_name = dict_groups[i].section(':', 0, 0);
            add_dict_group(group_name);

            if (dicts_amount > 0)
            {
                const int& colon_index
                    = static_cast<int>(dict_groups[i].indexOf(':'));
                dict_groups[i].remove(0, colon_index+1);
                for (int j=0; j<dicts_amount; ++j)
                {
                    const QString& dict_name =
                        dict_groups[i].section(',', j, j);
                    dict_groups_list_widgets[i]->addItem(dict_name);
                }
            }
        }
    }
}

void DictionarySettings::load_tts_engines()
{
    //Завантаження рушіїв синтезу мови
    const QString& default_tts_engine {tr("None")};
    const QString& current_tts_engine =
        settings->value("dictsettings/tts_engine",
                        default_tts_engine).toString();

    text_to_speech.clear();

    if (ui_dict_settings->tts_engine_combobox->count() != 0) {
        ui_dict_settings->tts_engine_combobox->clear();
    }

    ui_dict_settings->tts_engine_combobox->addItem(default_tts_engine);
    const auto engines = QTextToSpeech::availableEngines();
    for (const QString& engine : std::as_const(engines))
    {
        ui_dict_settings->tts_engine_combobox->addItem(engine);
    }

    if (current_tts_engine != default_tts_engine)
    {
        ui_dict_settings->tts_engine_combobox->
            setCurrentText(current_tts_engine);
        text_to_speech = new QTextToSpeech(current_tts_engine);
    }
}

void DictionarySettings::load_tts_languages()
{
    //Завантаження доступних для поточного TTS-рушія мов
    QVector<QLocale> available_languages;
    if (ui_dict_settings->tts_language_combobox->count() != 0) {
        ui_dict_settings->tts_language_combobox->clear();
    }

    if (text_to_speech != nullptr) {
        available_languages = text_to_speech->availableLocales();
        for (const QLocale& language: std::as_const(available_languages))
        {
            ui_dict_settings->tts_language_combobox->
                addItem(language.name());
        }
    }
    const QString& default_tts_language {""};
    const QString& current_tts_language =
        settings->value("dictsettings/tts_language",
                        default_tts_language).toString();

    if (current_tts_language != default_tts_language
        && available_languages.contains(QLocale(current_tts_language)))
    {
        ui_dict_settings->tts_language_combobox->
            setCurrentText(current_tts_language);
        text_to_speech->setLocale(QLocale(current_tts_language));
    }
}

void DictionarySettings::load_tts_voices()
{
    //Завантаження доступних для поточного TTS-рушія голосів
    QVector<QVoice> available_voices;
    if (ui_dict_settings->tts_voice_name_combobox->count() != 0) {
        ui_dict_settings->tts_voice_name_combobox->clear();
    }

    const QString& default_tts_voice {""};
    const QString current_tts_voice
        = settings->value("dictsettings/tts_voice", default_tts_voice)
              .toString();

    const QString& current_tts_voice_name =
        current_tts_voice.left(current_tts_voice.indexOf(":"));

    bool voice_is_exists {false};

    if (text_to_speech != nullptr) {
        available_voices = text_to_speech->availableVoices();
        for (const auto& voice: std::as_const(available_voices))
        {
            if (voice.name() == current_tts_voice_name)
            {
                voice_is_exists = true;
            }
            const QString& voice_name = voice.name() + ":"
                + voice.ageName(voice.age()) + " "
                + voice.genderName(voice.gender());
            ui_dict_settings->tts_voice_name_combobox->addItem(voice_name);
        }
    }

    if (current_tts_voice != default_tts_voice && voice_is_exists)
    {
        ui_dict_settings->tts_voice_name_combobox->
            setCurrentText(current_tts_voice);
        for (const auto& voice: std::as_const(available_voices))
        {
            if (voice.name() == current_tts_voice_name)
            {
                text_to_speech->setVoice(voice);
            }
        }

    }
}

void DictionarySettings::load_tts_voice_volume()
{
    //Завантаження поточної гучності голосу
    if (text_to_speech != nullptr) {
        const double default_tts_voice_volume
            = text_to_speech->volume() * max_volume;
        const int current_tts_voice_volume
            = settings
                  ->value("dictsettings/tts_voice_volume",
                          default_tts_voice_volume)
                  .toInt();
        ui_dict_settings->tts_volume_slider->setValue(current_tts_voice_volume);
        text_to_speech->setVolume(current_tts_voice_volume / max_volume);
    }
}

void DictionarySettings::load_tts_voice_rate()
{
    //Завантаження поточної темпу голосу
    if (text_to_speech != nullptr) {
        const double default_tts_voice_rate
            = text_to_speech->rate() * max_volume;
        const int current_tts_voice_rate
            = settings
                  ->value("dictsettings/tts_voice_rate",
                          default_tts_voice_rate)
                  .toInt();
        ui_dict_settings->tts_rate_slider->setValue(current_tts_voice_rate);
        text_to_speech->setRate(current_tts_voice_rate / max_volume);
    }
}

void DictionarySettings::load_tts_voice_pitch()
{
    //Завантаження поточної висоти голосу
    if (text_to_speech != nullptr) {
        const double default_tts_voice_pitch
            = text_to_speech->pitch() * max_volume;
        const int current_tts_voice_pitch
            = settings
                  ->value("dictsettings/tts_voice_pitch",
                          default_tts_voice_pitch)
                  .toInt();
        ui_dict_settings->tts_pitch_slider->setValue(current_tts_voice_pitch);
        text_to_speech->setPitch(current_tts_voice_pitch / max_volume);
    }
}

void DictionarySettings::save_paths_to_dicts_config()
{
    //Збереження налаштувань шляхів до словників
    const auto& path_widget = ui_dict_settings->path_to_dict_list;
    const int& items_amount = path_widget->count();
    QList<QListWidgetItem*> paths_items;
    for (int i=0; i<items_amount; ++i)
    {
        paths_items.append(path_widget->item(i));
    }
    QString paths {""};
    if (!paths_items.empty())
    {
        for (const auto& new_path: std::as_const(paths_items))
        {
            paths += new_path->text() + QString("\n");
        }
    }
    settings->setValue("dictsettings/paths_to_dicts", paths);
}

void DictionarySettings::save_dict_groups()
{
    //Збереження груп словників
    const auto& tabs_amount = ui_dict_settings->dict_groups_tab->count();
    QString dict_groups {""};
    if (tabs_amount > 0)
    {
        for (int i=0; i<tabs_amount; ++i)
        {
            const QString& tab_name =
                ui_dict_settings->dict_groups_tab->tabText(i);
            dict_groups += tab_name + ":";
            const int& dicts_amount = dict_groups_list_widgets[i]->count();

            if (dicts_amount > 0)
            {
                for (int j=0; j<dicts_amount; ++j)
                {
                    const QString& dict_name =
                        dict_groups_list_widgets[i]->item(j)->text();
                    dict_groups += dict_name + ",";
                }
            }
            dict_groups += "\n";
        }
    }
    settings->setValue("dictsettings/dict_groups", dict_groups);
}

void DictionarySettings::save_current_tts_engine(
    const QString& current_engine_name)
{
    // Збереження поточного TTS-рушія
    settings->setValue("dictsettings/tts_engine", current_engine_name);
}

void DictionarySettings::save_current_tts_language(
    const QString& current_language_name)
{
    //Збереження поточної мови TTS-рушія
    settings->setValue("dictsettings/tts_language", current_language_name);
}

void DictionarySettings::save_current_tts_voice(
    const QString& current_voice_name)
{
    //Збереження поточного голосу TTS-рушія
    settings->setValue("dictsettings/tts_voice", current_voice_name);
}

void DictionarySettings::save_tts_voice_volume(int volume_value)
{
    //Збереження гучності голосу TTS-рушія
    settings->setValue("dictsettings/tts_voice_volume", volume_value);
}

void DictionarySettings::save_tts_voice_rate(int rate_value)
{
    //Збереження темпу голосу TTS-рушія
    settings->setValue("dictsettings/tts_voice_rate", rate_value);
}

void DictionarySettings::save_tts_voice_pitch(int pitch_value)
{
    //Збереження висоти голосу TTS-рушія
    settings->setValue("dictsettings/tts_voice_pitch", pitch_value);
}

void DictionarySettings::on_add_path_to_dict_button_clicked()
{
    //Додавння шляху до словника
    const QString& path = QDir::currentPath();
    const QString& dir = QFileDialog::getExistingDirectory(
                        this, tr("Open Directory"),
                        path, QFileDialog::ShowDirsOnly|
                        QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
    {
        QDir::setCurrent(dir);
        add_path_dict(dir);
    }
}

void DictionarySettings::add_path_dict(const QString& path)
{
    //Вставка шляху до словника до відповідного віджета
    QList<QListWidgetItem*> dict_paths = ui_dict_settings->
                       path_to_dict_list->findItems(path, Qt::MatchExactly);
    QList<QString> dict_paths_text;
    for (const auto& dict_path : std::as_const(dict_paths)) {
        dict_paths_text.append(dict_path->text());
    }
    if (!dict_paths_text.contains(path))
    {
        ui_dict_settings->path_to_dict_list->addItem(path);
    }
    save_paths_to_dicts_config();
    fileloader->download_dictionaries();
    set_available_dicts();
    emit available_dicts_changed();
}


void DictionarySettings::on_del_path_to_dict_button_clicked()
{
    //Видалення шляху до словника
    const int& current_row = ui_dict_settings->path_to_dict_list->currentRow();
    if (current_row >= 0)
    {
        const QString path_name
            = ui_dict_settings->path_to_dict_list->takeItem(current_row)
                  ->text();
        const QString dict_name
            = *(fileloader->get_hash_of_paths()[path_name]);

        const int& dict_groups_amount
            = ui_dict_settings->dict_groups_tab->count();
        if (dict_groups_amount > 0)
        {
            for (int i=0; i<dict_groups_amount; ++i)
            {
               int dicts_amount = dict_groups_list_widgets[i]->count();

                if (dicts_amount > 0)
                {
                    for (int j=0; j<dicts_amount; ++j)
                    {
                        const QString& item_name =
                            dict_groups_list_widgets[i]->item(j)->text();
                        if ( item_name == dict_name)
                        {
                            dict_groups_list_widgets[i]
                                ->takeItem(j)
                                ->listWidget()
                                ->clear();

                            j -= 1;
                            dicts_amount -= 1;
                        }
                    }
                }
            }
        }

    }
    save_paths_to_dicts_config();
    fileloader->download_dictionaries();
    set_available_dicts();
    emit available_dicts_changed();
}

void DictionarySettings::on_del_all_paths_to_dicts_button_clicked()
{
    //Видалення всіх шляхів до словників
    ui_dict_settings->path_to_dict_list->clear();
    const int& dict_groups_amount =
        ui_dict_settings->dict_groups_tab->count();
    if (dict_groups_amount > 0)
    {
        for (int i=0; i<dict_groups_amount; ++i)
        {
            const int dicts_amount = dict_groups_list_widgets[i]->count();

            if (dicts_amount > 0)
            {
                dict_groups_list_widgets[i]->clear();
            }
        }
    }
    save_paths_to_dicts_config();
    fileloader->download_dictionaries();
    set_available_dicts();
    emit available_dicts_changed();
}

void DictionarySettings::on_add_dict_group_button_clicked()
{
    //Створення нової групи словників
    bool okay { false };
    const QString new_tab_name = QInputDialog::getText(
        this, tr("Add dictionary group"), tr("Enter dictionary group name:"),
        QLineEdit::Normal, "", &okay);
    if (okay) {
        add_dict_group(new_tab_name);
        save_dict_groups();
        emit dict_groups_amount_changed();
    }
}

void DictionarySettings::add_dict_group(const QString& new_tab_name)
{
    if (!new_tab_name.isEmpty())
    {
        bool is_tab_exists {false};
        const int tab_amount = ui_dict_settings->dict_groups_tab->count();
        for (int i=0; i<tab_amount; ++i)
        {
            if (new_tab_name == ui_dict_settings->dict_groups_tab->tabText(i))
            {
                is_tab_exists = true;
            }
        }

        if (!is_tab_exists)
        {
            const QPointer<QWidget> new_tab = new QWidget();
            ui_dict_settings->dict_groups_tab->addTab(new_tab, new_tab_name);
            dict_groups_layouts.append(new QVBoxLayout(
                ui_dict_settings->dict_groups_tab->widget(tab_amount)));
            dict_groups_layouts[tab_amount]->setContentsMargins(0, 0, 0, 0);
            dict_groups_list_widgets.append(new QListWidget());
            dict_groups_layouts[tab_amount]->addWidget(
                dict_groups_list_widgets[tab_amount]);
        }
    }
}

void DictionarySettings::on_del_dict_group_button_clicked()
{
    //Видалення групи словників
    const int current_tab = ui_dict_settings->dict_groups_tab->currentIndex();
    if (current_tab >= 0)
    {
        ui_dict_settings->dict_groups_tab->removeTab(current_tab);
        save_dict_groups();
        emit dict_groups_amount_changed();
    }
}


void DictionarySettings::on_del_all_dict_groups_button_clicked()
{
    //Видалення всіх груп словників
    ui_dict_settings->dict_groups_tab->clear();
    save_dict_groups();
    emit dict_groups_amount_changed();
}

void DictionarySettings::set_available_dicts()
{
    //Завантаження назв доступних словників до відповідного віджета
    ui_dict_settings->avail_dicts_list->clear();
    if(!fileloader->get_hash_of_dicts().isEmpty())
    {
        QList dicts = fileloader->get_hash_of_dicts().keys();
        for(const auto& dict: std::as_const(dicts))
        {
            ui_dict_settings->avail_dicts_list->addItem(dict);
        }
    }
}

void DictionarySettings::on_add_dict_to_group_button_clicked()
{
    //Додавання словника до обраної групи словників
    const int& current_dict = ui_dict_settings->avail_dicts_list->currentRow();
    const int& tabs_amount = ui_dict_settings->dict_groups_tab->count();
    if (current_dict >= 0)
    {
        QListWidgetItem* selected_dict_item =
            ui_dict_settings->avail_dicts_list->currentItem();
        const QString& selected_dict = selected_dict_item->text();

        if (tabs_amount > 0)
        {
            const int& current_tab_index =
                ui_dict_settings->dict_groups_tab->currentIndex();
            const int& dicts_amount =
                dict_groups_list_widgets[current_tab_index]->count();
            QStringList dicts_in_tab;
            for (int i=0; i<dicts_amount; ++i)
            {
                dicts_in_tab.append(
                    dict_groups_list_widgets[current_tab_index]->item(i)->text()
                    );
            }
            if (!dicts_in_tab.contains(selected_dict))
            {
                dict_groups_list_widgets[current_tab_index]->
                    addItem(selected_dict);
                save_dict_groups();
                emit available_dicts_changed();

            }

        }
        save_dict_groups();
    }
}


void DictionarySettings::on_del_dict_from_group_button_clicked()
{
    //Видалення словника з групи словників
    const int& tabs_amount = ui_dict_settings->dict_groups_tab->count();

    if (tabs_amount > 0)
    {
        const int& current_tab_index =
            ui_dict_settings->dict_groups_tab->currentIndex();
        const int& items_count =
            dict_groups_list_widgets[current_tab_index]->count();
        const int& selected_item =
            dict_groups_list_widgets[current_tab_index]->currentRow();

        if ( (items_count>0) && (selected_item>=0))
        {
            QListWidgetItem* dict_to_del
                = dict_groups_list_widgets[current_tab_index]->takeItem(
                    selected_item);
            if (dict_to_del != nullptr) {
                delete dict_to_del;
                dict_to_del = nullptr;
            }

            save_dict_groups();
            emit available_dicts_changed();
        }

        save_dict_groups();
    }
}

void DictionarySettings::on_show_dict_descr_button_clicked()
{
    //Реагування на натискання кнопки опису словника
    const int& avail_dicts_amount = ui_dict_settings->avail_dicts_list->count();
    const int& selected_item = ui_dict_settings->avail_dicts_list->currentRow();

    if (avail_dicts_amount > 0 && selected_item >= 0)
    {
        const QString& dict_name =
            ui_dict_settings->avail_dicts_list->item(selected_item)->text();

        hash_of_dicts current_dicts = fileloader->get_hash_of_dicts();
        QList<QString> dict_keys = current_dicts.keys();

        for (const auto& key : std::as_const(dict_keys)) {
            if (key == dict_name)
            {
                ui_dict_settings->dict_descr_text_edit->clear();
                const QString& dict_descr =
                    *(std::get<1>(current_dicts[dict_name]));
                ui_dict_settings->dict_descr_text_edit->append(dict_descr);
            }
        }
    }
}

void DictionarySettings::on_tts_engine_combobox_textActivated(
    const QString& current_engine_name)
{
    // Реагування на зміну поточного TTS-рушія
    save_current_tts_engine(current_engine_name);
    load_tts_engines();
    load_tts_languages();
    load_tts_voices();
    emit tts_settings_changed();
}

void DictionarySettings::on_tts_language_combobox_textActivated(
    const QString& current_language_name)
{
    //Реагування на зміну поточної мови TTS-рушія
    save_current_tts_language(current_language_name);
    load_tts_languages();
    load_tts_voices();
    emit tts_settings_changed();
}

void DictionarySettings::on_tts_voice_name_combobox_textActivated(
    const QString& current_voice_name)
{
    //Реагування на зміну поточного голосу TTS-рушія
    save_current_tts_voice(current_voice_name);
    load_tts_voices();
    emit tts_settings_changed();
}

void DictionarySettings::on_tts_volume_slider_valueChanged(int volume_value)
{
    //Реагування на зміну гучності голосу
    save_tts_voice_volume(volume_value);
    emit tts_settings_changed();
}

void DictionarySettings::on_tts_rate_slider_valueChanged(int rate_value)
{
    //Реагування на зміну темпу голосу
    save_tts_voice_rate(rate_value);
    emit tts_settings_changed();
}

void DictionarySettings::on_tts_pitch_slider_valueChanged(int pitch_value)
{
    //Реагування на зміну висоти голосу
    save_tts_voice_pitch(pitch_value);
    emit tts_settings_changed();
}

void DictionarySettings::on_tts_play_button_label_pressed()
{
    if (text_to_speech != nullptr) {
        const QString& test_text =
            ui_dict_settings->tts_test_word_line_edit->text();
        if (!test_text.isEmpty() && text_to_speech != nullptr) {
            text_to_speech->say(test_text);
        }
    }
}

