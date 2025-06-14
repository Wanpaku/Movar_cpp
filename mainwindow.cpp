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

#include "mainwindow.h"

MainWindow::MainWindow(QPointer<FileLoader> new_fileloader, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow())
    , fileloader(std::move(new_fileloader))
{
    settings = fileloader->get_settings();
    ui->setupUi(this);
    create_language_action_group();
    create_themes_action_group();
    load_settings();
    ui->search_word_line_edit->setFocus();
}

MainWindow::~MainWindow()
{
    qInfo() << tr("Exit from the app");
    delete ui;
}

void MainWindow::create_language_action_group()
{
    //Динамічне створення меню вибору мов інтерфейсу
    languages_group =
        new QActionGroup(ui->menuInterface_language);
    languages_group->setExclusive(true);

    connect(languages_group, SIGNAL(triggered(QAction*)),
            this, SLOT(at_language_changed(QAction*)));

    const QString& default_language {""};
    const QString& current_language =
        settings->value("interface_language", default_language).toString();
    QString default_locale;

    if (current_language == default_language)
    {
        default_locale = QLocale::system().name();
    }
    else
    {
        default_locale = current_language;
    }

    const QString& language_path = QApplication::applicationDirPath();

    const QDir dir(language_path);
    QStringList filenames = dir.entryList(QStringList("Movar_cpp_*.qm"));
    for (const auto& filename : std::as_const(filenames)) {
        QString locale;
        locale = filename;
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.indexOf("p")+3);

        const QString language
            = QLocale::languageToString(QLocale(locale).language());

        const QPointer<QAction> action = new QAction(language, ui->menubar);
        action->setCheckable(true);
        action->setData(locale);
        ui->menuInterface_language->addAction(action);
        languages_group->addAction(action);
        if (default_locale == locale)
        {
            action->setChecked(true);
        }
    }
}

void MainWindow::at_language_changed(QAction* action)
{
    //Слот реагування на зміну мови інтерфейсу
    if (action != nullptr) {
        load_interface_language(action->data().toString());
        ui->dict_choice_combobox->clear();
        load_dict_groups_choices();
        at_search_history_items_amount_changed();
    }
}

void MainWindow::load_interface_language(const QString& interface_language)
{
    //Завантаження нової мови інтерфейсу
    const QString default_language { "" };
    QString current_language =
        settings->value("interface_language", default_language).toString();

    if (current_language != interface_language)
    {
        current_language = interface_language;
        const QLocale locale = QLocale(current_language);
        QLocale::setDefault(locale);
        switch_translator(start_translator,
                          QString("Movar_cpp_%1.qm").arg(interface_language));

        save_interface_language_config();
    }
}

void MainWindow::switch_translator(
    QTranslator& translator, const QString& filename)
{
    //Видалення старого QTranslator і завантаження нового
    qApp->removeTranslator(&translator);

    const QString path = QApplication::applicationDirPath() + '/';

    if (start_translator.load(path + filename))
    {
        qApp->installTranslator(&start_translator);
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    //Оновлення інтерфейсу для показу виставленої нової мови інтерфейсу
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::create_themes_action_group()
{
    const QPointer<QActionGroup> themes
        = new QActionGroup(ui->menuTheme_settings);
    themes->addAction(ui->actionDefault_theme);
    themes->addAction(ui->actionKashmir);
    themes->addAction(ui->actionPlum);
    ui->actionDefault_theme->setChecked(true);
}

void MainWindow::at_dict_groups_amount_changed()
{
    ui->dict_choice_combobox->clear();
    load_dict_groups_choices();
}

void MainWindow::on_actionExit_triggered()
{
    //Закриття програми
    QApplication::quit();
}

void MainWindow::create_set_of_words()
{
    //Створення набору слів, наявних у поточних словниках
    set_of_words.clear();
    active_dict_names.clear();
    const QString& current_dict_group
        = ui->dict_choice_combobox->currentText();
    const QString& default_dict_group =
        ui->dict_choice_combobox->itemText(0);
    const hash_of_mapped_words hash_of_mapped_words
        = fileloader->get_hash_of_mapped_words();
    QMultiMap<QString, std::shared_ptr<QString>> sort_map {};

    if (current_dict_group != default_dict_group)
    {
        const QString& dict_groups =
            settings->value("dictsettings/dict_groups", "").toString();
        QStringList dict_groups_list = dict_groups.split('\n');

        if (!dict_groups_list.isEmpty())
        {
            for (auto item: std::as_const(dict_groups_list))
            {
                const QString& group_name = item.section(":", 0, 0);
                if (group_name == current_dict_group)
                {
                    const int& colon_index
                        = static_cast<int>(item.indexOf(':'));
                    item.remove(0, colon_index+1);
                    QStringList dicts_list =
                        item.split(',', Qt::SkipEmptyParts);
                    if (!dicts_list.isEmpty())
                    {
                        for (const auto& dict: std::as_const(dicts_list))
                        {
                            const QString& dict_name = dict.section(',', 0, 0);
                            active_dict_names.append(dict_name);
                            const auto& word_keys =
                                hash_of_mapped_words.value(dict_name).keys();
                            for (const auto& key : std::as_const(word_keys)) {
                                const QList<std::shared_ptr<QString>> values
                                    = hash_of_mapped_words.value(dict_name)
                                          .values(key);
                                for (const auto& value : values) {
                                    sort_map.insert(key,value);
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    else
    {
        QStringList dict_keys = hash_of_mapped_words.keys();
        active_dict_names.append(dict_keys);

        // auto start = std::chrono::high_resolution_clock::now();
        //  std::vector<std::future<void>> future_vec;
        for (const auto& dict_key : std::as_const(dict_keys)) {

            const auto word_keys = hash_of_mapped_words.value(dict_key).keys();

            for (const auto& word_key : std::as_const(word_keys)) {

                const QList<std::shared_ptr<QString>> values
                    = hash_of_mapped_words.value(dict_key).values(word_key);
                for (const auto& value : values) {
                    sort_map.insert(word_key, value);
                }
            }
        }
        // auto end = std::chrono::high_resolution_clock::now();
        // auto duration =
        // std::chrono::duration_cast<std::chrono::milliseconds>(
        //   end - start);
        // qInfo() << "Def dicts combo: " << duration;
    }

    for (const auto& word: sort_map)
    {
        set_of_words.append(*word);
    }
    set_of_words.removeDuplicates();
}

void MainWindow::add_completer_to_line_edit(const QString &letters)
{
    //Додавання динамічного вибору варіантів слів до поля пошуку слів
    const QPointer<QListView> list_view = new QListView();
    const QPointer<AdaptedCompleter> completer = new AdaptedCompleter(
        set_of_words, list_view, ui->toolbar_hor_layout);
    ui->search_word_line_edit->setCompleter(completer);
}

void MainWindow::on_actionDicitonary_settings_triggered()
{
    //Відкриття вікна налаштувань словників
    const QPointer<DictionarySettings> dict_settings
        = new DictionarySettings(fileloader);
    connect(dict_settings, &DictionarySettings::dict_groups_amount_changed,
            this, &MainWindow::at_dict_groups_amount_changed);
    connect(dict_settings, &DictionarySettings::available_dicts_changed, this,
            &MainWindow::create_set_of_words);
    connect(dict_settings, &DictionarySettings::tts_settings_changed, this,
            &MainWindow::load_tts_player);
    dict_settings->show();
}

void MainWindow::load_settings()
{
    //Завантаження налаштувань програми для поточного вікна
    load_default_webengine_settings();
    load_show_history_config();
    load_dict_groups_choices();
    load_search_history();
    load_tts_player();
    load_themecolor_menu_choice();
    on_actionAbout_triggered();
}

void MainWindow::load_show_history_config()
{
    //Налаштування показу історії пошуку
    const bool& default_show_search_history {true};
    const bool& show_search_history =
        settings->value("menu/show_search_history",
                        default_show_search_history).toBool();
    ui->actionShow_Hide_search_history->setChecked(show_search_history);
    on_actionShow_Hide_search_history_toggled(show_search_history);
}

void MainWindow::load_dict_groups_choices()
{
    //Завантаження переліку груп словників
    //та встановлення поточної групи словників
    const QString& all_dicts = tr("All dictionaries");
    ui->dict_choice_combobox->addItem(all_dicts);
    const QString& dict_groups =
        settings->value("dictsettings/dict_groups", all_dicts).toString();
    QStringList dict_groups_list = dict_groups.split('\n');
    if (!dict_groups_list.isEmpty())
    {
        for (const auto& item: std::as_const(dict_groups_list))
        {
            const QString& dict_name = item.section(":", 0, 0);
            bool exists {false};
            const int& item_count = ui->dict_choice_combobox->count();
            for (int i=0; i<item_count; ++i)
            {
                if (ui->dict_choice_combobox->itemText(i) == dict_name)
                {
                    exists = true;
                }
            }

            if (!exists && !dict_name.isEmpty())
            {
                ui->dict_choice_combobox->addItem(dict_name);
            }
        }
    }

    const QString& current_group_choice =
        settings->value("toolbar/dict_group_choice", all_dicts).toString();

    if (current_group_choice != all_dicts)
    {
        ui->dict_choice_combobox->setCurrentText(current_group_choice);
    }
}

void MainWindow::load_search_history()
{
    //Завантаження історії пошуку слів
    const QString& default_search_history {""};
    const QString& current_search_history =
        settings->value("search_history", default_search_history).toString();
    QStringList history_list = current_search_history.split('\n');

    if (!history_list.empty())
    {
        for (const auto& word : std::as_const(history_list)) {
            if (!word.isEmpty())
            {
                ui->search_history_list_widget->addItem(word);
            }
        }
    }
    at_search_history_items_amount_changed();
}

void MainWindow::load_default_webengine_settings()
{
    //Завантаження стандартних налаштувань вебвіджета
    default_webengine_settings = ui->result_webwindow->settings();
    def_font_family =
        default_webengine_settings->
                    fontFamily(QWebEngineSettings::StandardFont);
    def_font_size =
        default_webengine_settings->
                    fontSize(QWebEngineSettings::DefaultFontSize);
    const QString& current_font_family =
        settings->value("result_window/font_family",
                        def_font_family).toString();
    const int& current_font_size =
        settings->value("result_window/font_size", def_font_size).toInt();

    QFont current_font;
    current_font.setFamily(current_font_family);
    current_font.setPointSize(current_font_size);
    set_webengine_settings(current_font);
}

void MainWindow::load_tts_player()
{
    //Завантаження TTS-програвача
    const QString& default_tts_engine {tr("None")};
    const QString& current_tts_engine =
        settings->value("dictsettings/tts_engine",
                        default_tts_engine).toString();

    tts_player.clear();

    if (current_tts_engine != default_tts_engine)
    {
        tts_player = new QTextToSpeech(current_tts_engine);
    }

    if (tts_player != nullptr) {
        auto available_languages = tts_player->availableLocales();
        const QString& default_tts_language {""};
        const QString& current_tts_language =
            settings->value("dictsettings/tts_language",
                            default_tts_language).toString();

        if (current_tts_language != default_tts_language
            && available_languages.contains(QLocale(current_tts_language)))
        {
            tts_player->setLocale(QLocale(current_tts_language));
        }

        const QString& default_tts_voice {""};
        const QString current_tts_voice
            = settings->value("dictsettings/tts_voice", default_tts_voice)
                  .toString();

        const QString& current_tts_voice_name =
            current_tts_voice.left(current_tts_voice.indexOf(":"));

        auto available_voices = tts_player->availableVoices();
        for (const auto& voice: std::as_const(available_voices))
        {
            if (voice.name() == current_tts_voice_name
                    && current_tts_voice != default_tts_voice)
            {
                tts_player->setVoice(voice);
            }
        }

        const double& default_tts_voice_volume
            = tts_player->volume() * max_voice_volume;
        const int& current_tts_voice_volume =
            settings->value("dictsettings/tts_voice_volume",
                            default_tts_voice_volume).toInt();
        tts_player->setVolume(current_tts_voice_volume / max_voice_volume);

        const double& default_tts_voice_rate
            = tts_player->rate() * max_voice_rate;
        const int& current_tts_voice_rate =
            settings->value("dictsettings/tts_voice_rate",
                            default_tts_voice_rate).toInt();
        tts_player->setRate(current_tts_voice_rate / max_voice_rate);

        const double& default_tts_voice_pitch
            = tts_player->pitch() * max_voice_pitch;
        const int& current_tts_voice_pitch =
            settings->value("dictsettings/tts_voice_pitch",
                            default_tts_voice_pitch).toInt();
        tts_player->setPitch(current_tts_voice_pitch / max_voice_pitch);
    }
}

void MainWindow::save_settings()
{
    //Збереження налаштувань програми для поточного вікна
    save_show_history_config();
    save_dict_groups_choices();
    save_interface_language_config();
    save_webengine_settings();
    save_search_history();
    save_themecolor_menu_choice();
}

void MainWindow::save_show_history_config()
{
    //Налаштування показу історії пошуку
    const bool& show_search_history =
        ui->actionShow_Hide_search_history->isChecked();
    settings->setValue("menu/show_search_history", show_search_history);
}

void MainWindow::save_dict_groups_choices()
{
    //Збереження налаштувань поточної групи словників
    const QString& current_choice =
        ui->dict_choice_combobox->currentText();
    settings->setValue("toolbar/dict_group_choice", current_choice);
}

void MainWindow::save_interface_language_config()
{
    //Збереження поточної мови інтерфейсу
    const QString& current_interface_language =
        languages_group->checkedAction()->data().toString();
    settings->setValue("interface_language", current_interface_language);
}

void MainWindow::save_search_history()
{
    //Збереження історії пошуку слів
    const int& items_amount = ui->search_history_list_widget->count();
    QString words {""};

    if (items_amount > 0)
    {
        for (int i=0; i<items_amount; ++i)
        {
            const QString& word =
                ui->search_history_list_widget->item(i)->text();
            words += word + "\n";
        }
    }
    settings->setValue("search_history", words);
}

void MainWindow::save_webengine_settings()
{
    //Збереження налаштувань QWebEngineSettings
    const QString& current_font_family =
        default_webengine_settings->
        fontFamily(QWebEngineSettings::StandardFont);
    const int& current_font_size =
        default_webengine_settings->
        fontSize(QWebEngineSettings::DefaultFontSize);

    settings->setValue("result_window/font_family", current_font_family);
    settings->setValue("result_window/font_size", current_font_size);
}

void MainWindow::on_actionClear_search_history_triggered()
{
    //Очищення історої пошуку слів
    ui->search_history_list_widget->clear();
    at_search_history_items_amount_changed();
}

void MainWindow::at_search_history_items_amount_changed()
{
    //Зміна напису на ярлиці історії пошуку слів
    //відповідно до кількості слів
    const QString& items_amount =
        QString::number(ui->search_history_list_widget->count());
    const QString& new_label_text = tr("Search history") + "(" + items_amount
        + "/" + QString::number(max_history_words_amount) + ")";
    ui->search_history_box->setTitle(new_label_text);
    save_search_history();
}

void MainWindow::on_search_word_line_edit_returnPressed()
{
    //Реагування на натискання Enter у полі пошуку слів
    QString word = ui->search_word_line_edit->text();
    if (word != "")
    {
        auto word_in_list =
            ui->search_history_list_widget->findItems(word, Qt::MatchExactly);
        if (ui->search_history_list_widget->count() < max_history_words_amount
            && word_in_list.isEmpty()) {
            ui->search_history_list_widget->insertItem(0, word);
            at_search_history_items_amount_changed();
        } else if (ui->search_history_list_widget->count()
                       >= max_history_words_amount
                   && word_in_list.isEmpty()) {
            const int& item_amount = ui->search_history_list_widget->count();
            const QListWidgetItem* last_item
                = ui->search_history_list_widget->takeItem(item_amount - 1);
            delete last_item;
            last_item = nullptr;
            ui->search_history_list_widget->insertItem(0, word);
            at_search_history_items_amount_changed();
        }
        show_search_results(word);
    }
}

void MainWindow::show_search_results(QString &word)
{
    //Вивід результатів пошуку слова до відповідного вебвіджета
    if (!word.isEmpty())
    {
        const QString& word_lower = word.toLower();
        const hash_of_indexes dict_indexes = fileloader->get_hash_of_indexes();
        const hash_of_dicts dicts = fileloader->get_hash_of_dicts();

        QString styled_article;
        bool word_is_exists {false};
        for (const auto& dict_name: std::as_const(active_dict_names))
        {
            for (int i=0; i<dict_indexes.value(dict_name).size(); ++i)
            {
                int start_word { 0 };
                int end_word { 0 };
                const QString& index_word =
                    (*(std::get<0>(dict_indexes.value(dict_name).value(i)))).
                                            toLower();
                if (index_word == word_lower)
                {
                    start_word =
                        *(std::get<2>(dict_indexes.value(dict_name).value(i)));
                    if (i != dict_indexes.value(dict_name).size()-1)
                    {
                        end_word =
                            *(std::get<1>(dict_indexes.value(dict_name).
                                                 value(i+1)));
                    }
                    else
                    {
                        const QString& dict_body =
                            *(std::get<3>(dicts.value(dict_name)));
                        end_word = static_cast<int>(dict_body.length());
                    }

                    const QString& dict_body =
                        *(std::get<3>(dicts.value(dict_name)));
                    const QString& raw_article =
                        dict_body.mid(start_word, end_word-start_word);

                    styled_article +=
                        html_styled_article(dict_name, raw_article, word);
                    word_is_exists = true;
                }
            }
        }
        if (word_is_exists)
        {
            ui->result_webwindow->setHtml(styled_article);
        }
        else
        {
            ui->result_webwindow->setHtml(
                tr("There is no such word in the available dictionaries."));
        }
    }
}

auto MainWindow::html_styled_article(const QString& dict_name,
                                     const QString& raw_article,
                                     const QString& word) -> QString
{
    //Створення html стилю для статті зі словника
    const QString& font_family =
        default_webengine_settings->
                        fontFamily(QWebEngineSettings::StandardFont);
    const double& font_size = default_webengine_settings->fontSize(
                                  QWebEngineSettings::DefaultFontSize)
        / standard_font_size;

    constexpr int font_weight { 400 };
    const QString& text_color = current_theme_colors.text_color;
    const QString& background_color = current_theme_colors.background_color;
    const QString& label_background_color
        = current_theme_colors.label_background_color;
    const QString& html_body_style = QString(
        "style='background-color: black;\
        font-family: %1;\
        font-size: %2em;\
        font-weight: %3;\
        color: %4;'").
        arg(font_family, QString::number(font_size),
            QString::number(font_weight), text_color);

    const QString& html_body_start = QString(
        "<html><body %1>").arg(html_body_style);
    const QString& html_body_end = QString("</body></html>");
    const QString& heading_style = QString(
        "style='background-color: %1;\
        border: 10px ridge %2;\
        border-top-left-radius: 15px;\
        border-top-right-radius:15px;\
        margin-bottom: -1px;\
        width: max-content;'"
        ).arg(label_background_color, label_background_color);

    const QString& heading = QString("<h4 %1>%2</h4>").
        arg(heading_style, dict_name);
    const QString& term = QString("<mark><strong>%1</strong></mark>").arg(word);

    const QString& article_style = QString(
        "style='background-color: %1;\
        border-top: 5px ridge %2;\
        border-left: 5px ridge %3;\
        border-right: 5px ridge %4;\
        border-bottom: 5px ridge %5;\
        line-height: 1.5;\
        text-align: justify;\
        hyphens: auto;\
        padding: 15px;'").arg(
            background_color, label_background_color, label_background_color,
            label_background_color, label_background_color);

    const QString& article_begin =
        QString("<main %1><article>").arg(article_style);
    const QString& article_finish = QString("</article></main>");
    const QString& article_paragraph = article_begin + term +
                               raw_article + article_finish;
    const QString& article_styled = html_body_start + heading +
                               article_paragraph + html_body_end;

    return article_styled;
}

void MainWindow::on_actionShow_Hide_search_history_toggled(bool arg1)
{
    //Показ чи приховування історії пошуку слів
    if (!arg1) {
        ui->search_history_box->hide();
    } else {
        ui->search_history_box->show();
    }
}

void MainWindow::on_dict_choice_combobox_currentTextChanged(
    const QString& current_dict_group)
{
    //Реагування на зміну поточної групи словників
    create_set_of_words();
}

void MainWindow::on_search_word_line_edit_textEdited(const QString &arg1)
{
    //Слот реагування на редагування тексту у віджеті пошуку слова
    if (!arg1.isEmpty())
    {
        add_completer_to_line_edit(arg1);
    }
}

void MainWindow::on_search_history_list_widget_itemClicked(
    QListWidgetItem *item)
{
    //Слот реагування на натискання на слово в історії пошуку
    QString word = item->text();
    ui->search_word_line_edit->clear();
    ui->search_word_line_edit->setText(word);
    show_search_results(word);
}

void MainWindow::on_actionChoose_font_triggered()
{
    //Слот реагування на вибір шрифту
    bool okay { false };
    QFont default_font;
    default_font.setFamily(default_webengine_settings->
                           fontFamily(QWebEngineSettings::StandardFont));
    default_font.setPointSize(default_webengine_settings->
                              fontSize(QWebEngineSettings::DefaultFontSize));
    QFont current_font = QFontDialog::getFont(&okay, default_font, this);

    if (okay) {
        set_webengine_settings(current_font);
    }
}

void MainWindow::set_webengine_settings(QFont& current_font)
{
    //Встановлення налаштувань QWebEngineSettings для вікна результатів пошуку
    default_webengine_settings->
        setFontSize(QWebEngineSettings::DefaultFontSize,
                    current_font.pointSize());
    default_webengine_settings->
        setFontFamily(QWebEngineSettings::StandardFont,
                      current_font.family());
    QString word = ui->search_word_line_edit->text();
    show_search_results(word);
}

void MainWindow::on_actionDefault_font_triggered()
{
    //Реагування на натискання кнопки стандартного шрифту
    default_webengine_settings->
        setFontFamily(QWebEngineSettings::StandardFont, def_font_family);
    default_webengine_settings->
        setFontSize(QWebEngineSettings::DefaultFontSize, def_font_size);
    QString word = ui->search_word_line_edit->text();
    show_search_results(word);
}

void MainWindow::on_play_word_button_pressed()
{
    //Реагування на натискання кнопки програвання слова
    const QString& word = ui->search_word_line_edit->text();
    if (!word.isEmpty() && tts_player != nullptr) {
        tts_player->say(word);
    }
}

void MainWindow::on_actionAbout_triggered()
{
    ui->result_webwindow->setHtml("");
    const QString& about_text = tr("<b><p><center>Movar version 1.0.2</p></b><p><center>"
        "Movar is an application created for work with offline dictionaries "
        "which are saved in '.mvr' format.<p>'.mvr' format is a usual "
        "text format which could be edited in any text editor."
        "<p><b>Overall design:</b> Teg Miles,"
        "<p><b>Icon-logo:</b> "
        "<a href='https://www.flaticon.com/"
        "free-icon/dictionary_7793703?related_id=7793703&origin=pack'>"
        "Design Circle</a>."
        "<p><a href='https://github.com/Wanpaku/Movar_cpp'><b>Website</b></a>"
        "<p>Copyright©2024  Teg Miles(movarocks2@gmail.com)."
        "<p>This program comes with absolutely no warranty. "
        "If you want to know more, please, visit "
        "<a href='https://www.gnu.org/licenses/gpl-3.0.html'> "
        "GNU General Public License GNU (GNU GPL),version 3 or newer.</a>");

    const QString& about_title = tr("Про Movar");
    const QString& styled_about_text =
        html_styled_article(about_title, about_text, "");
    ui->result_webwindow->setHtml(styled_about_text);
}


void MainWindow::on_actionManual_triggered()
{
    const QString& manual_text = tr(
        "<b>You need to perform below mentioned steps "
        "to begin using this application:</b>"
        "<p style='text-indent:1em;'>-Scan paper dictionary "
        "and save it in '.mvr' format "
        "or find already scanned and saved in .txt format dictionary "
        "and change it's extension form txt to mvr. "
        "mvr is an usual text format, which could be edited "
        "in any text editor. It was created for separation "
        "Movar dictionaries from other text files."
        "<p style='text-indent:1em;'>-Edit first four rows "
        "of the dictionary file as described below: "
        "<p style='text-indent:2em;'>*First row — Title of yours dictionary"
        "<p style='text-indent:2em;'>*Second row — "
        "Language pair, for example Eng-Eng"
        "<p style='text-indent:2em;'>*Third row — "
        "Brief description of the dictionary"
        "<p style='text-indent:2em;'>*Fourth row — Pattern from Regular Expressions "
        "for filtering terms from the dictionary, "
        "which must meet demands of Qt RegularExpression. "
        "It's a condition of the Regex engine used in this version of the app. "
        "<p style='text-indent:1em;'>-"
        "Go to the Settings and choose folder with the file. "
        "<p style='text-indent:1em;'>"
        "You can find example of the formatting in the file called  "
        "Webster dictionary(Gutenberg.org).mvr, "
        "which located in the folder named Webster dictionary."
        "<p style='text-indent:1em;'>"
        "This application outputing a dictionary article "
        "through QWebEngineView. Therefore you could use a HTML "
        "markup language for formatting yours dictionaries files.");

    const QString& manual_title = tr("Посібник Movar");
    const QString& styled_manual_text =
        html_styled_article(manual_title, manual_text, "");
    ui->result_webwindow->setHtml(styled_manual_text);

}


void MainWindow::on_actionDefault_theme_triggered()
{
    current_theme_colors.text_color = "black";
    current_theme_colors.background_color = "white";
    current_theme_colors.label_background_color = "navajowhite";
    QString word = ui->search_word_line_edit->text();
    if (word.isEmpty())
    {
        on_actionAbout_triggered();
    }
    else
    {
        show_search_results(word);
    }
}


void MainWindow::on_actionPlum_triggered()
{
    current_theme_colors.text_color = "plum";
    current_theme_colors.background_color = "black";
    current_theme_colors.label_background_color = "SteelBlue";
    QString word = ui->search_word_line_edit->text();
    if (word.isEmpty())
    {
        on_actionAbout_triggered();
    }
    else
    {
        show_search_results(word);
    }
}


void MainWindow::on_actionKashmir_triggered()
{
    current_theme_colors.text_color = "#003300";
    current_theme_colors.background_color = "snow";
    current_theme_colors.label_background_color = "PaleTurquoise";
    QString word = ui->search_word_line_edit->text();
    if (word.isEmpty())
    {
        on_actionAbout_triggered();
    }
    else
    {
        show_search_results(word);
    }
}

void MainWindow::save_themecolor_menu_choice()
{
    const QList<QAction*> actions = ui->menuTheme_settings->actions();
    for (auto action = actions.constBegin();
         action != actions.constEnd(); ++action)
    {
        if ((*action)->isChecked())
        {
            QString action_name = (*action)->objectName();
            const QString& theme_name = action_name.replace("action", "");
            settings->setValue("themes/current_theme", theme_name);
        }
    }
}

void MainWindow::load_themecolor_menu_choice()
{
    const QString& default_theme = ("Default_theme");
    const QString& current_theme =
        settings->value("themes/current_theme", default_theme).toString();
    if (current_theme != default_theme)
    {
        if (current_theme == "Plum")
        {
            current_theme_colors.text_color = "plum";
            current_theme_colors.background_color = "black";
            current_theme_colors.label_background_color = "SteelBlue";
        }
        if (current_theme == "Kashmir")
        {
            current_theme_colors.text_color = "#003300";
            current_theme_colors.background_color = "snow";
            current_theme_colors.label_background_color = "PaleTurquoise";
        }

        const QList<QAction*> actions = ui->menuTheme_settings->actions();
        for (auto action = actions.constBegin();
             action != actions.constEnd(); ++action)
        {
            QString action_name = (*action)->objectName();
            const QString& theme_name = action_name.replace("action", "");
            if (theme_name == current_theme)
            {
                (*action)->setChecked(true);
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    save_settings();
}
