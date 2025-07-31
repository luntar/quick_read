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
#include "readingstatusanimator.h"

ReadingStatusAnimator::ReadingStatusAnimator(QObject* parent)
  : QObject(parent)
  , _trayIcon(nullptr)
{
  connect(&_timer, &QTimer::timeout, this, &ReadingStatusAnimator::animationTimeout);

  for (int i = 0; i < kMaxIconNum; ++i)
  {
    QString iconPath = QString(":/testmode_reading%1.png").arg(i);
    _icons[i] = QIcon(iconPath);
  }
}

void ReadingStatusAnimator::setSystemTrayIcon(QSystemTrayIcon* sti)
{
  _trayIcon = sti;
}

void ReadingStatusAnimator::run()
{
  _timer.setInterval(60);
  setIcon(0);
  _timer.start();
}

void ReadingStatusAnimator::stop()
{
  _timer.stop();
  _trayIcon->setIcon(QIcon("://testmode_on.png"));
}

void ReadingStatusAnimator::showerror()
{
  _trayIcon->setIcon(QIcon("://testmode_err.png"));
  QTimer::singleShot(700, [=]() { _trayIcon->setIcon(QIcon("://testmode_on.png")); });
}

void ReadingStatusAnimator::setIcon(int num)
{
  if (num > 0 && num < kMaxIconNum)
  {
    _trayIcon->setIcon(_icons[num]);
  }
}

void ReadingStatusAnimator::animationTimeout()
{
  if (_trayIcon)
  {
    setIcon(_iconCounter++);
    if (_iconCounter >= kMaxIconNum)
      _iconCounter = 0;
  }
}
