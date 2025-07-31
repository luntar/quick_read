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
#include "listwidget.h"
#include <QDebug>

ListWidget::ListWidget(QWidget* parent)
  : QListWidget(parent)
{
}

void ListWidget::focusOutEvent(QFocusEvent*)
{
    emit lostFocus();
}

void ListWidget::dropEvent(QDropEvent *event)
{
    QListWidget::dropEvent(event);
    emit reorderItems();
}
