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

#include "logger.h"

Logger::Logger() = default;

auto Logger::get_filename() -> QString
{
    // Initialization of the static variable
    static const QString filename { QDir::currentPath() + QDir::separator()
                                    + "log.txt" };
    return filename;
}

auto Logger::get_log_file() -> QSharedPointer<QFile>
{
    // Initialization of the static variable
    static const QSharedPointer<QFile> log_file {
        QSharedPointer<QFile>::create(get_filename())
    };

    return log_file;
}

auto Logger::get_message_types() -> QHash<QtMsgType, QString>
{
    // Initialization of the static variable
    static const QHash<QtMsgType, QString> message_types {
        { QtMsgType::QtDebugMsg,    "Debug"                 },
        { QtMsgType::QtInfoMsg,     QObject::tr("Info")     },
        { QtMsgType::QtWarningMsg,  QObject::tr("Warning")  },
        { QtMsgType::QtCriticalMsg, QObject::tr("Critical") },
        { QtMsgType::QtFatalMsg,    QObject::tr("Fatal")    }
    };

    return message_types;
}

void Logger::init()
{
    // Initialization of qInstallMessageHandler
    if (is_init) {
        return;
    }

    if (!get_log_file()->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qCritical() << tr("Couldn't open file ")
                    << get_log_file()->errorString();
        return;
    }

    qInstallMessageHandler(message_output);

    if (get_log_file()->exists()) {
        get_log_file()->resize(0);
    }

    is_init = true;
}

void Logger::message_output(QtMsgType type, const QMessageLogContext& context,
                            const QString& message)
{
    // Formatting qInstallMessageHandler's output
    if (is_init) {
        const QString log = QDateTime::currentDateTime().toString() + " | "
            + get_message_types().value(type) + " | " + tr("line: ")
            + QString::number(context.line) + " | " + context.function + " | "
            + QString(context.file).section('/', -1) + " | " + tr("message: ")
            + message + '\n';

        QTextStream stream(get_log_file().get());
        stream << log;
        stream.flush();
    }
    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, message);
}
