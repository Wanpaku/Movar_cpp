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

#ifndef DICTIONARYSETTINGS_H
#define DICTIONARYSETTINGS_H

#include "fileloader.h"

namespace Ui {
class DictionarySettings;
}

class DictionarySettings : public QDialog
{
    //Клас, що створює діалогове вікно налаштувань словників
    Q_OBJECT

public:
    explicit DictionarySettings(FileLoader* new_fileloader,
                                QWidget *parent = nullptr);
    ~DictionarySettings();
    QTabWidget* get_dict_groups_tab();
    QList<QListWidget*> get_dict_groups_list_widgets();

private slots:
    void on_add_path_to_dict_button_clicked();
    void on_del_path_to_dict_button_clicked();
    void on_del_all_paths_to_dicts_button_clicked();
    void on_add_dict_group_button_clicked();
    void on_del_dict_group_button_clicked();
    void on_del_all_dict_groups_button_clicked();
    void on_add_dict_to_group_button_clicked();
    void on_del_dict_from_group_button_clicked();
    void on_show_dict_descr_button_clicked();
    void on_tts_engine_combobox_textActivated(const QString &arg1);
    void on_tts_language_combobox_textActivated(const QString &arg1);
    void on_tts_voice_name_combobox_textActivated(const QString &arg1);
    void on_tts_volume_slider_valueChanged(int value);
    void on_tts_rate_slider_valueChanged(int value);
    void on_tts_pitch_slider_valueChanged(int value);
    void on_tts_play_button_label_pressed();

signals:
    void dict_groups_amount_changed();
    void available_dicts_changed();
    void tts_settings_changed();

private:
    Ui::DictionarySettings* ui_dict_settings;
    QPointer<FileLoader> fileloader {nullptr};
    QPointer<QSettings> settings {nullptr};
    QList<QVBoxLayout*> dict_groups_layouts {};
    QList<QListWidget*> dict_groups_list_widgets {};
    QPointer<QTextToSpeech> text_to_speech {nullptr};

    void add_path_dict(const QString& path);
    void creating_shortcuts();
    void load_settings();
    void load_paths_to_dicts_config();
    void save_settings();
    void save_paths_to_dicts_config();
    void set_available_dicts();
    void load_dict_groups();
    void save_dict_groups();
    void add_dict_group(const QString& new_tab_name);
    void load_tts_engines();
    void load_tts_languages();
    void load_tts_voices();
    void save_current_tts_engine();
    void save_current_tts_language();
    void save_current_tts_voice();
    void load_tts_voice_volume();
    void save_tts_voice_volume();
    void load_tts_voice_rate();
    void save_tts_voice_rate();
    void load_tts_voice_pitch();
    void save_tts_voice_pitch();
};

#endif // DICTIONARYSETTINGS_H
