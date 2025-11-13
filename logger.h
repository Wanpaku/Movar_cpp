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

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QSharedPointer>
#include <QHash>
#include <QtLogging>
#include <QDir>
#include <QTextStream>
#include <QDateTime>

class Logger : public QObject {
    // Class responsible for creating the log file
    Q_OBJECT
public:
    Logger();

    static void init();
    static void message_output(QtMsgType type,
                               const QMessageLogContext& context,
                               const QString& message);

private:
    static inline bool is_init { false };
    static inline const qint64 max_log_size = 10 * 1024 * 1024; // 10 Mb
    // Uncomment this variable here and in "message output" method
    // if you wont to see qDebug output in the console
    static inline const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER
        = qInstallMessageHandler(nullptr);

    static auto get_filename() -> QString;
    static auto get_log_file() -> QSharedPointer<QFile>;
    static auto get_message_types() -> QHash<QtMsgType, QString>;
};

#endif // LOGGER_H
