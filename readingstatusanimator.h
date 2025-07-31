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

#ifndef READINGSTATUSANIMATOR_H
#define READINGSTATUSANIMATOR_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>

#define kMaxIconNum 4
class ReadingStatusAnimator : public QObject
{
  Q_OBJECT
public:
  explicit ReadingStatusAnimator(QObject* parent = nullptr);
  void setSystemTrayIcon(QSystemTrayIcon* sti);
  void run();
  void stop();
  void showerror();


  void setIcon(int num);

signals:

public slots:
  void animationTimeout();

private:
  QTimer _timer;
  int _iconCounter{0};
  QSystemTrayIcon* _trayIcon = nullptr;
  QIcon _icons[kMaxIconNum];
};

#endif // READINGSTATUSANIMATOR_H
