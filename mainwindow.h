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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/qmainwindow.h>

#include "readingstatusanimator.h"
#include "ui_mainwindow.h"

#include <QClipboard>
#include <QElapsedTimer>
#include <QMap>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTextToSpeech>
#include <QPair>


using ruleMap_t = QHash<QString, QString>;
using rule_t = QPair<QString, QString>;
using ruleVec_t = QVector<rule_t>;
using ruleStr_t = QVector<QString>;


class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget* parent = nullptr);

  void speakText(const QString& toSpeak, bool useFilter = true);

public slots:
  void setVisible(bool visible) override;
  void restoreWindowVis();
  void saveSystemState();
  void speak();
  void stop();

  void setRate(int);
  void setPitch(int);
  void setVolume(int volume);

  void stateChanged(QTextToSpeech::State state);
  void engineSelected(int index);
  void languageSelected(int language);
  void voiceSelected(int index);

  void localeChanged(const QLocale& locale);
  void clipboard_changed();
  int keyExists(const QString& key);


private slots:
  void on_lineEdit_editingFinished();
  void on_pushButton_clicked();
  void on_listWidget_itemDoubleClicked(QListWidgetItem* item);
  void on_listWidget_currentTextChanged(const QString& currentText);
  void on_listWidget_lostFocus();
  void on_listWidget_reorderItems();
  void on_restorePushButton_clicked();
  void on_delPushButton_clicked();
  void on_backPushButton_clicked();
  void on_forwardPushButton_clicked();
  void on_readClipCheckBox_clicked(bool checked);

  // void on_exportPushButton_clicked();

  void on_listWidget_itemClicked(QListWidgetItem *item);

  void on_importRulesPB_clicked();



  void on_quitPushButton_clicked();


  void on_ruleSyntaxPushButton_clicked();

  private:
  void setTextDisplay(QString textToSpeak);
  void updateHistoryButtons();
  bool isRuleStrValid(const QString& ruleText);
  QPair<QString,QString> parseRuleString(const QString& rule);
  bool ruleIsCaseInsensitive(const QString& rule);
  bool addRuleString(QString rule);
 bool addRule(const QString &rule);
  void removeRule(const QString& rule);

  Ui::MainWindow ui;

  ReadingStatusAnimator* _statusAnimator;
  QTextToSpeech* m_speech;
  QVector<QVoice> m_voices;
  QClipboard* m_clipboard;
  ruleStr_t m_rules;
  QString m_lastRuleRemoved;

  QStringList m_textBackStack;
  QStringList m_textForwardStack;
  int m_textHistoryIdx = 0;
  QMenu* trayIconMenu;
  QSystemTrayIcon* trayIcon;

  QAction* minimizeAction;
  QAction* maximizeAction;
  QAction* restoreAction;
  QAction* quitAction;

  QAction* readClipboad;
  QAction* stopReading;
  bool m_restoreMainWindow;
  QString _lastText;

  void saveSliderValue(QSettings& settings, QSlider* slider);
  void setSliderValue(const QSettings& settings, QSlider* slider);
  void restoreSettings();
  QString applyFilterText(const QString& text);
  int populateRulesWidget(const QString& key = QString());

  void restoreRules();
  QString rulesToString();
  void rulesFromString(const QString& str);
  void saveRules();
  void createTrayIcon();
  void createActions();

  QElapsedTimer etimer;
  int _sameCount = 0;

  bool textNotSameOrSameCountTrigger(const QString& str);

  const QString getEOLSymbole();
  protected:
  void closeEvent(QCloseEvent* event) override;
};

#endif
