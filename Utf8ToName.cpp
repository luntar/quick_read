/*********************************************************************
  Quick Read is a QT Text2Speech Program
  Copyright (C) 2018 John Brinkman

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**********************************************************************/
#include "Utf8ToName.h"
#include <QDebug>
namespace Utf8Converter
{


QByteArray convertAllUtf8ToNames(const QByteArray& src)
{
  QByteArray rtval = src;
  ByteStringMap_t detected;
  ByteStringMap_t::const_iterator i;

  for (i = utf8ToName.constBegin(); i != utf8ToName.constEnd(); ++i)
  {
    char key = i.key();
    QString val = i.value();

    if (rtval.contains(key))
    {
      detected.insert(key, val);
      if (0x92 != static_cast<uint8_t>(key))
      {
        val = ttsMkSilence(val, 700);
      }
      rtval.replace(key, val.toLatin1());
    }
  }
  if (!detected.isEmpty())
  {
    qDebug() << detected;
  }
  return rtval;
}

QString toString(char b)
{
  if (utf8ToName.contains(b))
    return utf8ToName[b];
  else
    return QString(" ");
}
} // namespace Utf8Converter
