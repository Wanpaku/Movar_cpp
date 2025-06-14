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

#include "fileloader.h"

FileLoader::FileLoader(QObject *parent)
    : QObject{parent}
{
    load_language_support();
    qInfo() << tr("Start program Movar.");
    // auto start = std::chrono::high_resolution_clock::now();
    try {
        download_dictionaries();
    } catch (const std::exception& e) {
        const QString& message = e.what();
        get_error_message(message);
        qCritical() << tr("Error happens during downloading files.\n ")
                    << message;
    }
    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration
    //    = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // qInfo() << "Тривалість завантаження словників: " << duration;
}

auto FileLoader::get_settings() -> QPointer<QSettings>
{
    if (settings == nullptr) {
        settings = new QSettings("Movar");
    }
    settings->setFallbacksEnabled(false);

    return settings;
}

auto FileLoader::get_hash_of_dicts() -> hash_of_dicts
{
    return dicts;
}

auto FileLoader::get_hash_of_indexes() -> hash_of_indexes
{
    return dict_indexes;
}

auto FileLoader::get_hash_of_paths() -> hash_of_paths
{
    return paths_and_dict_names;
}

auto FileLoader::get_hash_of_mapped_words() -> hash_of_mapped_words
{
    return words;
}

void FileLoader::load_language_support()
{
    get_settings();
    const QString default_language { "" };
    const QString current_language
        = settings->value("interface_language", default_language).toString();
    const QString translations_path = "";

    if (current_language == default_language)
    {
        const QStringList uiLanguages = QLocale::system().uiLanguages();
        for (const QString &locale : uiLanguages)
        {
            const QString basename = "Movar_cpp_" + QLocale(locale).name();

            if (lang_translator.load(basename, translations_path))
            {
                qApp->installTranslator(&lang_translator);
                break;
            }
        }
    }
    else
    {
        const QString basename = "Movar_cpp_" + current_language;

        if (lang_translator.load(basename, translations_path))
        {
            qApp->installTranslator(&lang_translator);
        }
    }
}

void FileLoader::download_paths_to_dicts()
{
    //Завантаження шляхів до словників
    get_settings();
    if (!paths_to_dicts.isEmpty())
    {
        paths_to_dicts.clear();
    }
    const QString default_paths_to_dicts { "" };
    const QString current_paths
        = settings
              ->value("dictsettings/paths_to_dicts", default_paths_to_dicts)
              .toString();
    if (current_paths != default_paths_to_dicts)
    {
        paths_to_dicts = current_paths.split('\n', Qt::SkipEmptyParts);
    }
}

void FileLoader::get_dict_pair(const QString& path, const QString& filename)
{
    // Додавання ім'я словника та вказівника до його складових
    // до загального контейнера зі словниками
    const QString full_path = path + "/" + filename;
    QFile input_file(full_path);

    if (!input_file.open(QIODevice::ReadOnly)) {
        const std::string& error_message
            = ("Can't open the path: " + full_path + "!").toStdString();
        throw std::runtime_error(error_message);
    }

    QStringList dict_requisites {};
    QString full_dict { "" };
    QTextStream input_stream(&input_file);
    for (int i = 0; i < 4; ++i) {
        const QString title = input_stream.readLine();
        dict_requisites.append(title);
    }
    while (!input_stream.atEnd()) {
        const QString line = input_stream.readLine();
        full_dict += line + "\n";
    }
    const QString full_dict_name
        = dict_requisites[0] + "(" + dict_requisites[1] + ")";
    paths_and_dict_names.insert(path,
                                std::make_shared<QString>(full_dict_name));

    const tuple_of_shared_pointers dict_tuple
        = std::make_tuple(std::make_shared<QString>(dict_requisites[1]),
                          std::make_shared<QString>(dict_requisites[2]),
                          std::make_shared<QString>(dict_requisites[3]),
                          std::make_shared<QString>(full_dict));
    dicts.insert(full_dict_name, dict_tuple);
}

void FileLoader::read_from_txt_file(const QString& path)
{
    // Зчитування словників з текстового файла
    const QDir path_to_dict(path);
    const QStringList selected_formats = QStringList({ "*.mvr" });
    const QStringList files
        = path_to_dict.entryList(selected_formats, QDir::Files);
    for (const auto& filename : files) {
        get_dict_pair(path, filename);
    }
}

void FileLoader::create_sep_dict_index(const QString& dict_name,
                                       const QString& reg_key,
                                       const QString& full_dict)
{
    // Створення індексованого змісту для окремого словника
    QRegularExpression reg;
    reg.setPattern(reg_key);
    reg.setPatternOptions(QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator match_iter = reg.globalMatch(full_dict);
    while (match_iter.hasNext()) {
        const QRegularExpressionMatch match = match_iter.next();

        const QString word = match.captured(0);
        const int word_start = static_cast<int>(match.capturedStart(0));
        const int word_end = static_cast<int>(match.capturedEnd(0));

        dict_indexes[dict_name].append(std::make_tuple(
            std::make_shared<QString>(word), std::make_shared<int>(word_start),
            std::make_shared<int>(word_end)));

        const QString& clean_word_lower = simplified_word(word).toLower();
        words[dict_name].insert(clean_word_lower,
                                std::make_shared<QString>(word));
    }
}

void FileLoader::create_dict_indexes()
{
    // Створення індексованого змісту для словників
    if(!dicts.isEmpty())
    {

        std::vector<std::future<void>> future_vec;
        for(const auto& [lang_pair, descr, regex_key, dict_body]:
             std::as_const(dicts))
        {
            const QString dict_name = dicts.key(
                std::make_tuple(lang_pair, descr, regex_key, dict_body));
            const QString reg_key = *regex_key;
            const QString full_dict = *dict_body;

            future_vec.emplace_back(
                async(std::launch::async, &FileLoader::create_sep_dict_index,
                      this, dict_name, reg_key, full_dict));
        }
        for (auto& future : future_vec) {
            try {
                future.get();
            } catch (const std::exception& e) {
                throw e;
            }
        }
    }
}

void FileLoader::download_dictionaries()
{
    //Завантаження словників
    download_paths_to_dicts();
    if (!dicts.isEmpty())
    {
        dicts.clear();
    }
    if (!dict_indexes.isEmpty())
    {
        dict_indexes.clear();
    }
    if (!paths_and_dict_names.isEmpty())
    {
        paths_and_dict_names.clear();
    }
    if (!words.isEmpty())
    {
        words.clear();
    }

    const int& path_to_dicts_count = static_cast<int>(paths_to_dicts.size());

    if (path_to_dicts_count > 0)
    {

        const QScopedPointer<QProgressDialog> progress_dialog(
            new QProgressDialog("", nullptr, 0, path_to_dicts_count));
        progress_dialog->setWindowModality(Qt::WindowModal);
        progress_dialog->setMinimumDuration(0);
        progress_dialog->setWindowTitle(tr("Download dictionaries..."));

        int path_count { 0 };
        std::vector<std::future<void>> futures_vec;
        for (const auto& path : std::as_const(paths_to_dicts)) {

            progress_dialog->setValue(path_count);
            const int pos
                = static_cast<int>(path.length() - path.lastIndexOf("/") - 1);
            progress_dialog->setLabelText(path.right(pos));

            const QDir path_to_dict(path);
            if (path_to_dict.exists()) {
                futures_vec.emplace_back(
                    std::async(std::launch::async,
                               &FileLoader::read_from_txt_file, this, path));
                path_count += 1;
            } else {
                const std::string& error_message
                    = ("Can't find the path: " + path + "!").toStdString();
                throw std::runtime_error(error_message);
            }
        }
        for (auto& future : futures_vec) {
            try {
                future.get();
            } catch (const std::exception& e) {
                throw e;
            }
        }

        create_dict_indexes();
    }
}

auto FileLoader::simplified_word(const QString& word) -> QString
{
    //Метод, що прибирає діакритичні знаки й апостроф
    //задля правильного сортування слів згідно абетки
    const auto& diacritic_letters =
        QString("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØ"
                "ÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ");
    const QStringList not_diacritic_letters {
        "S", "OE", "Z", "s",  "oe", "z", "Y", "Y", "u", "A",  "A", "A",
        "A", "A",  "A", "AE", "C",  "E", "E", "E", "E", "I",  "I", "I",
        "I", "D",  "N", "O",  "O",  "O", "O", "O", "O", "U",  "U", "U",
        "U", "Y",  "s", "a",  "a",  "a", "a", "a", "a", "ae", "c", "e",
        "e", "e",  "e", "i",  "i",  "i", "i", "o", "n", "o",  "o", "o",
        "o", "o",  "o", "u",  "u",  "u", "u", "y", "y"
    };

    const auto& word_lower = word.toLower();
    QString cleared_string;

    for (const auto &character : std::as_const(word_lower))
    {
        const auto& diacritic_index = diacritic_letters.indexOf(character);
        if (diacritic_index < 0)
        {
            if (character.isLetter())
            {
                cleared_string.push_back(character);
            }
        }
        else
        {
            cleared_string.push_back(not_diacritic_letters.at(diacritic_index));
        }
    }

    return cleared_string;
}

void FileLoader::get_error_message(const QString& message)
{
    QMessageBox error_window;
    error_window.setText(message);
    error_window.setIcon(QMessageBox::Warning);
    error_window.setWindowTitle("Movar");
    error_window.exec();

}
