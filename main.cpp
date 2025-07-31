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

#include <QApplication>

#include "mainwindow.h"
static const QString kVersionString = "1.0.5";
int main(int argc, char* argv[])
{
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);
  app.setApplicationName("QuickRead");
  app.setApplicationVersion(kVersionString);
  app.setApplicationDisplayName("QuickRead");
  app.setOrganizationDomain("brinko.com");
  app.setOrganizationName("Brinko");

  MainWindow win;
  win.show();
  return app.exec();
}
