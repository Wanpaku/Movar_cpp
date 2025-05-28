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

#ifndef ADAPTEDSTRINGLISTMODEL_H
#define ADAPTEDSTRINGLISTMODEL_H

#include <QObject>
#include <QStringListModel>
#include <QRegularExpression>
#include <QDebug>

class AdaptedStringListModel : public QStringListModel
{
    //Клас, що переписує допоміжний клас,
    //у якому відображається перелік наявних слів для вибору при пошуку
    Q_OBJECT
public:
    explicit AdaptedStringListModel(QObject* parent);
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role) const override;
    void set_adapted_role (int role);
    [[nodiscard]] int adapted_role() const;
    static QString cleared_word(const QString &value);


private:
    int current_adapted_role { 0 };
};

#endif // ADAPTEDSTRINGLISTMODEL_H
