# *******************************************************************
#  Quick Read is a QT Text2Speech Program
#  Copyright (C) 2018 John Brinkman
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.
# ********************************************************************

TEMPLATE = app

TARGET=quick_read

QT += widgets texttospeech
CONFIG += c++17

VERSION=1.0.2

CONFIG(debug, debug|release) {
    DESTDIR = debug
} else {
    DESTDIR = release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
#UI_DIR = $$DESTDIR/.ui

QMAKE_TARGET_COMPANY = Brinko
QMAKE_TARGET_PRODUCT = $$TARGET
QMAKE_TARGET_DESCRIPTION = "Text2Speech Program"
QMAKE_TARGET_COPYRIGHT = "Copyright \251 Brinko 2018"

RC_ICONS = appicon.ico

SOURCES = main.cpp \
          Presigned_Url_Text_File_Downloader.cpp \
          Presigned_Url_Text_File_Uploader.cpp \
          mainwindow.cpp \
          listwidget.cpp \
          TtsTextTools.cpp \
          Utf8ToName.cpp \
          readingstatusanimator.cpp

HEADERS = \
    AR_ASSERT.h \
    Presigned_Url_Text_File_Downloader.h \
    Presigned_Url_Text_File_Uploader.h \
    S3_Download_Url_Client.h \
    S3_Upload_Url_Client.h \
    mainwindow.h \
    listwidget.h \
    TtsTextTools.h \
    Utf8ToName.h \
    readingstatusanimator.h

FORMS = mainwindow.ui

RESOURCES += \
    res.qrc

DISTFILES += \
    text_text.txt
