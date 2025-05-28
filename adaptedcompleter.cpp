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

#include "adaptedcompleter.h"

AdaptedCompleter::AdaptedCompleter(const QStringList& entries,
                                   QPointer<QListView> listView,
                                   QPointer<QObject> parent)
    : QCompleter { parent }
{
    const QPointer<AdaptedStringListModel> model
        = new AdaptedStringListModel(this);
    setModel(model);
    setCompletionRole(model->adapted_role());
    model->setStringList(entries);
    setCompletionMode(QCompleter::PopupCompletion);
    setFilterMode(Qt::MatchStartsWith);
    setPopup(listView);
    setCaseSensitivity(Qt::CaseInsensitive);
    setMaxVisibleItems(max_visible_items);
    setModelSorting(QCompleter::CaseInsensitivelySortedModel);
}

QStringList AdaptedCompleter::splitPath(const QString& path) const
{
    return QStringList() << AdaptedStringListModel::cleared_word(path);
}

QString AdaptedCompleter::pathFromIndex(const QModelIndex& index) const
{
    return index.data().toString();
}
