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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "./ui_mainwindow.h"
#include "dictionarysettings.h"
#include "fileloader.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct ThemeColors
{
    QString text_color = "black";
    QString background_color = "white";
    QString label_background_color = "navajowhite";
};


class MainWindow : public QMainWindow
{
    //Клас головного вікна програми
    Q_OBJECT

public:
    explicit MainWindow(FileLoader* new_fileloader, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void changeEvent(QEvent*);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionExit_triggered();
    void on_actionDicitonary_settings_triggered();
    void on_actionClear_search_history_triggered();
    void on_search_word_line_edit_returnPressed();
    void on_actionShow_Hide_search_history_toggled(bool arg1);
    void on_dict_choice_combobox_currentTextChanged(const QString &arg1);
    void at_dict_groups_amount_changed();
    void on_search_word_line_edit_textEdited(const QString &arg1);
    void on_search_history_list_widget_itemClicked(QListWidgetItem *item);
    void on_actionChoose_font_triggered();
    void at_language_changed(QAction* action);
    void on_actionDefault_font_triggered();
    void on_play_word_button_pressed();
    void on_actionAbout_triggered();
    void on_actionManual_triggered();
    void on_actionDefault_theme_triggered();
    void on_actionPlum_triggered();
    void on_actionKashmir_triggered();

private:
    Ui::MainWindow *ui;
    QPointer<FileLoader> fileloader = {nullptr};
    QPointer<DictionarySettings> dict_settings {nullptr};
    QPointer<QSettings> settings {nullptr};
    QStringList set_of_words {};
    QStringList active_dict_names {};
    QTranslator start_translator;
    QPointer<QActionGroup> languages_group {nullptr};
    QWebEngineSettings* default_webengine_settings {nullptr};
    QString def_font_family;
    int def_font_size;
    QPointer<QTextToSpeech> tts_player {nullptr};
    struct ThemeColors current_theme_colors;

    void add_completer_to_line_edit(const QString& letters);
    void create_set_of_words();
    void load_settings();
    void load_show_history_config();
    void save_settings();
    void save_show_history_config();
    void at_search_history_items_amount_changed();
    void load_dict_groups_choices();
    void save_dict_groups_choices();
    void show_search_results(QString& word);
    QString html_styled_article(const QString& dict_name,
                                const QString& raw_article,
                                const QString& word);
    void create_themes_action_group();
    void create_language_action_group();
    void load_interface_language(const QString& interface_language);
    void switch_translator(QTranslator& translator, const QString& filename);
    void save_interface_language_config();
    void load_default_webengine_settings();
    void set_webengine_settings(QFont& current_font);
    void save_webengine_settings();
    void save_search_history();
    void load_search_history();
    void load_tts_player();
    void save_themecolor_menu_choice();
    void load_themecolor_menu_choice();

};
#endif // MAINWINDOW_H
