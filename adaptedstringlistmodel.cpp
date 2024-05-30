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

#include "adaptedstringlistmodel.h"

AdaptedStringListModel::AdaptedStringListModel(QObject* parent)
    :QStringListModel(parent)
{
    set_adapted_role(Qt::UserRole + 10);
}

QVariant AdaptedStringListModel::data(const QModelIndex& index, int role) const
{
    return role == adapted_role()
       ? cleared_word(QStringListModel::data(index, Qt::DisplayRole).toString())
       : QStringListModel::data(index, role);
}

void AdaptedStringListModel::set_adapted_role(int role)
{
   current_adapted_role = role;
}

int AdaptedStringListModel::adapted_role() const
{
   return current_adapted_role;
}

QString AdaptedStringListModel::cleared_word(const QString& value)
{
   //Метод, що прибирає діакритичні знаки й апостроф,
   //дозволяючи знайти потрібне слово не вводячи їх.
   const auto& diacritic_letters =
       QString("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØ"
                "ÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ");
   QStringList not_diacritic_letters{"S", "OE", "Z", "s", "oe",
                                  "z", "Y", "Y", "u", "A",
                                  "A", "A", "A", "A", "A",
                                  "AE", "C", "E", "E", "E",
                                  "E", "I", "I", "I", "I", "D",
                                  "N", "O", "O", "O", "O", "O",
                                  "O", "U", "U", "U", "U", "Y",
                                  "s", "a", "a", "a", "a", "a",
                                  "a", "ae", "c", "e", "e", "e",
                                  "e", "i", "i", "i", "i", "o", "n", "o",
                                  "o", "o", "o", "o", "o", "u", "u", "u",
                                  "u", "y", "y"};

   const auto& value_lower = value.toLower();
   QString cleared_string;

   for (const auto &character : std::as_const(value_lower))
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
