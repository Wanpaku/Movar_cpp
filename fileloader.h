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

#ifndef FILELOADER_H
#define FILELOADER_H

#include <QObject>
#include <QSettings>
#include <QPointer>
#include <QSharedPointer>
#include <QHash>
#include <QDir>
#include <QRegularExpression>
#include <QFontDialog>
#include <QWebEngineSettings>
#include <QFontDatabase>
#include <QTranslator>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QShortcut>
#include <QDialog>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QListWidget>
#include <QMainWindow>
#include <QCompleter>
#include <QTextToSpeech>
#include <QActionGroup>
#include <QMultiMap>
#include <QMessageBox>

#include <tuple>
#include <memory>
#include <algorithm>
#include <string>

#include "adaptedcompleter.h"

using tuple_of_shared_pointers =
    std::tuple<std::shared_ptr<QString>, std::shared_ptr<QString>,
               std::shared_ptr<QString>, std::shared_ptr<QString>>;
using list_of_tuples_with_shared_pointers =
    QList<std::tuple<std::shared_ptr<QString>,
    std::shared_ptr<int>, std::shared_ptr<int>>>;
using hash_of_dicts = QHash<QString, tuple_of_shared_pointers>;
using hash_of_indexes = QHash<QString, list_of_tuples_with_shared_pointers>;
using hash_of_paths = QHash<QString, std::shared_ptr<QString>>;
using map_of_words = QMultiMap<QString, std::shared_ptr<QString>>;
using hash_of_mapped_words = QHash<QString, map_of_words>;

class FileLoader : public QObject
{
    //Клас, що відповідає за завантаження файлів (словники й підтримка мов)
    Q_OBJECT
public:
    explicit FileLoader(QObject *parent = nullptr);
    auto get_settings() -> QPointer<QSettings>;
    auto get_hash_of_dicts() -> hash_of_dicts;
    auto get_hash_of_indexes() -> hash_of_indexes;
    void download_dictionaries();
    auto get_hash_of_paths() -> hash_of_paths;
    auto get_hash_of_mapped_words() -> hash_of_mapped_words;
    void load_language_support();

private:
    void download_paths_to_dicts();
    void read_from_txt_file(const QString& path);
    void create_dict_indexes();
    auto simplified_word(const QString& word) -> QString;

    QPointer<QSettings> settings {nullptr};
    hash_of_dicts dicts;
    hash_of_indexes dict_indexes;
    QStringList paths_to_dicts;
    hash_of_paths paths_and_dict_names;
    hash_of_mapped_words words;
    QPointer<QProgressDialog> progress_dialog {nullptr};
    QTranslator lang_translator;
    void get_error_message(const QString& message);


};

#endif // FILELOADER_H
