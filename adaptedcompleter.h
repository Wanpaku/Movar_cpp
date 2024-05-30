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

#ifndef ADAPTEDCOMPLETER_H
#define ADAPTEDCOMPLETER_H

#include <QCompleter>
#include <QObject>
#include <QListView>
#include <QPointer>
#include <QDebug>
#include "adaptedstringlistmodel.h"

class AdaptedCompleter : public QCompleter
{
    //Клас, що переписує основу QCompleter задля пошуку слів
    //без введення діакритичних знаків й апострофу.
    Q_OBJECT
public:
    explicit AdaptedCompleter(const QStringList &entries, QListView *listView,
                              QObject *parent = nullptr);
    QStringList splitPath (const QString &path) const override;
    QString pathFromIndex (const QModelIndex &index) const override;
};

#endif // ADAPTEDCOMPLETER_H
