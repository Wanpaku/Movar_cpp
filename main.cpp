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
#include "fileloader.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDirIterator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon("./icons/movar.ico"));
    QPointer<FileLoader> fileloader = new FileLoader();
    MainWindow mainwindow(fileloader);
    mainwindow.show();
    return app.exec();
}
