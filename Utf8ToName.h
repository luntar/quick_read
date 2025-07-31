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
#include "QMap"
#include "TtsTextTools.h"
#ifndef Utf8ToName_h__
#define Utf8ToName_h__
using ByteStringMap_t = QMap<char, QString>;

namespace Utf8Converter
{
static const ByteStringMap_t utf8ToName{

  // clang-format off
    {0x80,"Euro"},
	{0x81," "},
	{0x82,"'"},
	{0x83,"Function"},
	{0x84,R"(")"},
	{0x85,"..."},
	{0x86,"Dagger"},
	{0x87,"Double Dagger"},
	{0x88,"^"},
	{0x89,"Per Mile"},
	{0x8A,"S Caron"},
	{0x8B,"<"},
	{0x8C,"O&E"},
	{0x8D," "},
	{0x8E,"Z Caron"},
	{0x8F," "},
	{0x90," "},
	{0x91,"'"},
	{0x92,"'"},
	{0x93,R"(")"},
	{0x94,R"(")"},
	{0x95,"Bullet"},
    {0x96," dash "}, // En Dash
    {0x97," dash "}, //Em Dash
	{0x98,"~"},
	{0x99,"Trade Mark"},
	{0x9A,"S Caron"},
	{0x9B,">"},
	{0x9C,"OE"},
	{0x9D,""},
	{0x9E,"Z Caron"},
	{0x9F,"Y Diaeresis"},
	{0xA0,"A Caron"},
	{0xA1,"Inverted !"},
	{0xA2,"Cents"},
	{0xA3,"Pounds"},
	{0xA4,"Currency"},
	{0xA5,"Yen"},
	{0xA6,"Symbol"},
	{0xA7,"Section"},
	{0xA8,"diaeresis"},
    {0xA9,"See"},
	{0xAA,"feminine"},
    {0xAB,"<<"},
	{0xAC,"Negation"},
	{0xAD,"A Caron"},
    {0xAE,"Registered Trade Mark"},
	{0xAF,"Macron"},
	{0xB0,"Degree"},
    {0xB1,"plus or minus"},
	{0xB2,"^2"},
	{0xB3,"^3"},
	{0xB4,"`"},
	{0xB5,"Mu"},
	{0xB6,"Paragraph"},
	{0xB7,"Interpunct"},
	{0xB8,"Cedilla"},
	{0xB9,"super 1"},
	{0xBA,"degree"},
	{0xBB,">>"},
	{0xBC,"Quarter"},
	{0xBD,"Half"},
	{0xBE,"three fourths"},
	{0xBF,"Inverted ?"},
	{0xC0,"A Grave"},
	{0xC1,"A Acute"},
	{0xC2,"A Circumflex"},
	{0xC3,"A Tilda"},
	{0xC4,"A Umlaut"},
	{0xC5,"Angstrom"},
	{0xC6,"AESC"},
	{0xC7,"C Cedilla"},
	{0xC8,"E Grave"},
	{0xC9,"E Acute"},
	{0xCA,"A Circumflex"},
	{0xCB,"E Tilda"},
	{0xCC,"I Grave"},
	{0xCD,"I Acute"},
	{0xCE,"I Circumflex"},
	{0xCF,"I Tilda"},
	{0xD0,"Edh"},
	{0xD1,"Enye"},
	{0xD2,"O Grave"},
	{0xD3,"O Acute"},
	{0xD4,"O Circumflex"},
	{0xD5,"O Tilda"},
	{0xD6,"O Umlaut"},
	{0xD7,"Multiply"},
	{0xD8,"NULL"},
	{0xD9,"U Grave"},
	{0xDA,"U Acute"},
	{0xDB,"U Circumflex"},
	{0xDC,"U Tilda"},
	{0xDD,"Y Acute"},
	{0xDE,"Thorn"},
	{0xDF,"Esszett"},
	{0xE0,"a Grave"},
	{0xE1,"a Acute"},
	{0xE2,"a Circumflex"},
	{0xE3,"a Tilda"},
	{0xE4,"a Umlaut"},
	{0xE5,"a ring"},
	{0xE6,"a aesh"},
	{0xE7,"Cedille"},
	{0xE8,"e Grave"},
	{0xE9,"e Acute"},
	{0xEA,"e Circumflex"},
	{0xEB,"e Tilda"},
	{0xEC,"l Grave"},
	{0xED,"l Acute"},
	{0xEE,"l Circumflex"},
	{0xEF,"l Tilda"},
	{0xF0,"eth"},
	{0xF1,"enye"},
	{0xF2,"o Grave"},
	{0xF3,"o Acute"},
	{0xF4,"o Circumflex"},
	{0xF5,"o Tilda"},
	{0xF6,"o Umlaut"},
	{0xF7,"divide"},
	{0xF8,"Empty Set"},
	{0xF9,"u Grave"},
	{0xFA,"u Acute"},
	{0xFB,"u Tilda"},
	{0xFC,"u Umlaut"},
	{0xFD,"y Acute"},
	{0xFE,"Thorn"},
	{0xFF,"y dieresis"}
	};
// clang-format on
QByteArray convertAllUtf8ToNames(const QByteArray& src);
QString toString(char b);
};     // namespace Utf8Converter
#endif // Utf8ToName_h__
