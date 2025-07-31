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

Some of this file was inspired/taken from the Text2Speech example
  included in the Qt C++ framework distribution

**********************************************************************/
#include "mainwindow.h"

#include <QLoggingCategory>

#include <QSettings>

#include "QStringList"
#include "Utf8ToName.h"
#include <QDeadlineTimer>
#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QTimer>
#include <QStandardPaths>


static const int kMaxTextStackSize = 50;
static const int kSameTextTrycountTreshold = 0;

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , m_speech(nullptr)
{
  ui.setupUi(this);

  setWindowTitle(" v" + qApp->applicationVersion());
  QLoggingCategory::setFilterRules(
    QStringLiteral("qt.speech.tts=true \n qt.speech.tts.*=true"));

  createActions();
  createTrayIcon();

  // Populate engine selection list
  ui.engine->addItem("Default", QString("default"));
  foreach (QString engine, QTextToSpeech::availableEngines()) {
    ui.engine->addItem(engine, engine);
  }
  ui.engine->setCurrentIndex(0);
  engineSelected(0);

  connect(ui.speakButton, &QPushButton::clicked, this, &MainWindow::speak);
  connect(ui.pitch, &QSlider::valueChanged, this, &MainWindow::setPitch);
  connect(ui.rate, &QSlider::valueChanged, this, &MainWindow::setRate);
  connect(ui.volume, &QSlider::valueChanged, this, &MainWindow::setVolume);
  connect(ui.engine,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
    &MainWindow::engineSelected);

  connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::saveSystemState);

  m_clipboard = qApp->clipboard();
  connect(m_clipboard, &QClipboard::changed, this, &MainWindow::clipboard_changed);

  restoreSettings();
  trayIcon->setIcon(QIcon("://testmode_on.png"));
  trayIcon->setToolTip("Quick Read, Text2Speech");
  trayIcon->show();


}


QString MainWindow::applyFilterText(const QString &text)
{
  if (text.isEmpty())
    return text;

  QString str = text;

  for(auto & r : m_rules)
  {
    const bool caseInsisitive = ruleIsCaseInsensitive(r);
    const rule_t e = parseRuleString(r);
    QRegularExpression rx{e.first};
    if(caseInsisitive) {
        rx.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }
    str.replace(rx,e.second.toLatin1());
}


  const QString eol = getEOLSymbole();

  // Look for titles, that would be a line each each word in caps
  QStringList lines = str.split(eol);
  QStringList linesMod = lines;
  if (lines.count() > 0)
  {
    int lineIdx = 0;
    int replacementCount = 0;
    foreach (const QString& line, lines)
    {
      static auto rule = QRegularExpression(R"(^[0-9]?\. )");
      if (line.contains(rule))
      {
        linesMod[replacementCount + lineIdx].insert(static_cast<int>(0), QLatin1String(" Number "));
      }
      else
      {
        QStringList words;
        words = line.trimmed().split(" ");
        if (!words.isEmpty())
        {
          bool titleLine = true;
          foreach (const QString& word, words)
          {
            if ((word.length() == 0) || word.at(0).isLower())
            {
              titleLine = false;
            }
            static auto rule = QRegularExpression(R"(^[0-9]?-[0-9]?)");
            if (word.contains(rule))
            {
              linesMod[replacementCount + lineIdx].replace("-", " dash ");
            }
          }

          (void)titleLine;
          // if (titleLine)
          // {
          //   // Insert a pause after a title
          //   linesMod.insert(
          //     replacementCount + lineIdx, QLatin1String(R"(<silence msec="800"/>)"));
          //   linesMod.insert(
          //     replacementCount + lineIdx + 2, QLatin1String(R"(<silence msec="600"/>)"));
          //   replacementCount += 2;
          // }
        }
      }
      lineIdx++;
    }
  }
  QByteArray ba = linesMod.join("\n").toLocal8Bit();
  // Replace all the CRLF with space
  ba.replace(0x0a, QLatin1String(" "));
  ba.replace(0x0d, QLatin1String(" "));

  ba = Utf8Converter::convertAllUtf8ToNames(ba);
  QString rtval(ba);

  return rtval;
}

int MainWindow::populateRulesWidget(const QString &key )
{
    int rtval = -1;
    ui.listWidget->clear();
    int idx = 0;
    for(auto & r : m_rules)
    {
        if(!key.isEmpty() && key == r)
            rtval = idx;
        ui.listWidget->addItem(r);
    }
    ui.listWidget->setCurrentRow(rtval);
    return rtval;
}



bool find_two_words(const QString &text,
                    int start_index,
                    const QRegularExpression &regex,
                    QString &str_one,
                    QString &str_two,
                    int &str_two_index)
{
    QRegularExpressionMatchIterator it_one = regex.globalMatch(text.mid(start_index));
    while (it_one.hasNext()) {
        QRegularExpressionMatch match_one = it_one.next();
        int abs_index_one = match_one.capturedStart() + start_index;
        str_one = match_one.captured();

        QRegularExpressionMatchIterator it_two = regex.globalMatch(text.mid(abs_index_one + str_one.length()));
        if (it_two.hasNext()) {
            QRegularExpressionMatch match_two = it_two.next();
            str_two = match_two.captured();
            str_two_index = match_two.capturedStart() + abs_index_one + str_one.length();
            return true;
        }
    }

    return false;
}


void transform_string(QString &text)
{

    const static auto regex = QRegularExpression(R"(\b\w+: )",QRegularExpression::CaseInsensitiveOption);

    int index = 0;
    int remove_count = 0;
    QString first_two;
    while (index < text.length()) {
        QString str_one, str_two;
        int str_two_index = -1;

        if (!find_two_words(text, index, regex, str_one, str_two, str_two_index))
            break;

        int str_one_index = text.indexOf(str_one, index);
        Q_ASSERT(str_one_index >= 0);

        // Replace str_one with str_one + " says "
        QString prefix = str_one.trimmed(); // remove colon and space
        Q_ASSERT(prefix.length()>0);
        prefix = prefix.mid(0,prefix.length()-1);
        QString replacement = prefix + " says ";
        text.replace(str_one_index, str_one.length()-1, replacement);

        int delta = replacement.length() - str_one.length();
        str_two_index += delta;

        if (str_one == str_two) {
            // Remove str_two and the value after it
            int str_two_end = text.indexOf(QRegularExpression(R"(\r|\n)"), str_two_index+1);
            if (str_two_end == -1)
                str_two_end = text.length();
            Q_ASSERT(str_two_index+1 + str_two.length() < text.length());
            text.remove(str_two_index+1, str_two.length());
            remove_count++;
        }
        else
        {
            remove_count = 0;
            first_two = str_two;
        }

        //index = str_one_index + replacement.length();
        index = str_two_index+1;
    }
}


void MainWindow::speakText(const QString& toSpeak, bool useFilter)
{
  if (m_textBackStack.count() > kMaxTextStackSize)
        m_textBackStack.takeLast();

  QString toSpeakAfterFFXIVFilter = toSpeak;

  // first, delete up to the EOL
  _lastText = toSpeak;
  static const auto not_needs_trime_rule = QRegularExpression(QString("^.*: "));
  const bool need_trim = toSpeak.indexOf(not_needs_trime_rule) > 0;
  if(need_trim)
  {

      static const QString eol = R"(\r|\n)"; // FFXIV NPC EOL text from Eventlog
      const auto eol_rule_str = QString(R"(^.*(%1))").arg(eol);
      const auto eol_rule = QRegularExpression(eol_rule_str);

      if(toSpeak.indexOf(eol_rule) > -1) {
          const int eol_idx = toSpeak.indexOf(QRegularExpression(eol)) + 1;
          toSpeakAfterFFXIVFilter = toSpeakAfterFFXIVFilter.remove(0,eol_idx);
      }
  }

  // if a word ends with ':', replace all but the first
  //    then replace the "word" with "word says"
  // note - the word is realy a name from a FFXIV NPC story
  transform_string(toSpeakAfterFFXIVFilter);


      // QString first_name;
      // QString second_name;
      // int second_name_idx = -1;


      // int firstOrcurence = toSpeakAfterFFXIVFilter.indexOf(rule);
      // const bool is_FFXIV_text = firstOrcurence != -1;
      // QString new_str;
      // if(is_FFXIV_text)
      // {
      //     int len = toSpeakAfterFFXIVFilter.length();
      //     int cur_idx = firstOrcurence;

      //     if(find_two_words(toSpeakAfterFFXIVFilter,cur_idx,rule,
      //                        first_name,
      //                        second_name,
      //                        second_name_idx))
      //     {
      //         if(first_name == second_name)
      //         {
      //             // build new string and don't use the second name
      //             Q_ASSERT(cur_idx < second_name_idx-second_name.length()-1);
      //             int new_len = second_name_idx-second_name.length()-1;
      //             new_str += toSpeakAfterFFXIVFilter.mid(cur_idx,new_len);
      //             new_str += '\r';
      //         }
      //         else
      //         {
      //             QString sub_str = toSpeakAfterFFXIVFilter.mid(cur_idx,new_len)
      //             new_str += QString("%1 says ").arg(first_name);
      //             new_str +=
      //         }


      //     }
      // }



      // if(firstOrcurence > -1) {

      //     int len = toSpeakAfterFFXIVFilter.length();
      //     int cur_idx = firstOrcurence;

      //     QString cur_str = toSpeakAfterFFXIVFilter.mid(cur_idx);
      //     QString name = cur_str.section(':',0,0);
      //     QString next_name = cur_str.section(':',1,1);
      //     if(name == next_name)
      //     {
      //         // replace <name> with "says <name>"
      //     }
      //     else
      //     {
      //         // remove next_name
      //     }


      //     //toSpeakAfterFFXIVFilter.replace(rule,QString("%1 says ").arg(name));
      //     toSpeakAfterFFXIVFilter.remove(rule);
      //     toSpeakAfterFFXIVFilter = QString("1% says %2").arg(name).arg(toSpeakAfterFFXIVFilter);
      //     // inster at index, name + " says "
      // }






  const auto sayThis = useFilter ? applyFilterText(toSpeakAfterFFXIVFilter) : toSpeakAfterFFXIVFilter;
  m_speech->say(sayThis);
}

void MainWindow::speak()
{
    const QString toSpeak = ui.plainTextEdit->toPlainText();
    static bool use_the_filter = true;
    speakText(toSpeak,use_the_filter);
}
void MainWindow::stop()
{
  m_speech->stop();
}

void MainWindow::setRate(int rate)
{
  m_speech->setRate(rate / static_cast<double>(ui.rate->maximum()));
}

void MainWindow::setPitch(int pitch)
{
  m_speech->setPitch(pitch / static_cast<double>(ui.pitch->maximum()));
}

void MainWindow::setVolume(int volume)
{
  m_speech->setVolume(volume / static_cast<double>(ui.volume->maximum()));
}

void MainWindow::stateChanged(QTextToSpeech::State state)
{
  if (state == QTextToSpeech::Speaking)
  {
    ui.statusbar->showMessage("RUNNING");
    _statusAnimator->run();
  }
  else if (state == QTextToSpeech::Ready)
  {
    ui.statusbar->showMessage("STOPPED", 2000);
    _statusAnimator->stop();
  }
  else if (state == QTextToSpeech::Paused)
    ui.statusbar->showMessage("PAUSED");
  else
    ui.statusbar->showMessage("ERROR!");

  ui.pauseButton->setEnabled(state == QTextToSpeech::Speaking);
  ui.resumeButton->setEnabled(state == QTextToSpeech::Paused);
  ui.stopButton->setEnabled(
    state == QTextToSpeech::Speaking || state == QTextToSpeech::Paused);
}

void MainWindow::engineSelected(int index)
{
  QString engineName = ui.engine->itemData(index).toString();
  delete m_speech;
  if (engineName == "default")
    m_speech = new QTextToSpeech(this);
  else
    m_speech = new QTextToSpeech(engineName, this);
  disconnect(ui.language,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
    &MainWindow::languageSelected);
  ui.language->clear();
  // Populate the languages combobox before connecting its signal.
  QVector<QLocale> locales = m_speech->availableLocales();
  QLocale current = m_speech->locale();
  foreach (const QLocale& locale, locales)
  {
    QString name(QString("%1 (%2)")
                   .arg(QLocale::languageToString(locale.language()))
                   .arg(QLocale::territoryToString(locale.territory())));
    QVariant localeVariant(locale);
    ui.language->addItem(name, localeVariant);
    if (locale.name() == current.name())
      current = locale;
  }
  setRate(ui.rate->value());
  setPitch(ui.pitch->value());
  setVolume(ui.volume->value());
   connect(ui.stopButton, &QPushButton::clicked,  m_speech,[this]{m_speech->stop();});
   connect(ui.pauseButton, &QPushButton::clicked, m_speech, [this]{m_speech->pause();});

   connect(ui.resumeButton, &QPushButton::clicked, m_speech, &QTextToSpeech::resume);

   connect(m_speech, &QTextToSpeech::stateChanged, this, &MainWindow::stateChanged);
   connect(m_speech, &QTextToSpeech::localeChanged, this, &MainWindow::localeChanged);

  connect(ui.language,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
    &MainWindow::languageSelected);
  localeChanged(current);
}

void MainWindow::languageSelected(int language)
{
  QLocale locale = ui.language->itemData(language).toLocale();
  m_speech->setLocale(locale);
}

void MainWindow::voiceSelected(int index)
{
  m_speech->setVoice(m_voices.at(index));
}
// FOO BAR
void MainWindow::localeChanged(const QLocale& locale)
{
  QVariant localeVariant(locale);
  ui.language->setCurrentIndex(ui.language->findData(localeVariant));

  disconnect(ui.voice,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
    &MainWindow::voiceSelected);
  ui.voice->clear();

  m_voices = m_speech->availableVoices();
  QVoice currentVoice = m_speech->voice();
  foreach (const QVoice& voice, m_voices)
  {
    ui.voice->addItem(QString("%1 - %2 - %3")
                        .arg(voice.name())
                        .arg(QVoice::genderName(voice.gender()))
                        .arg(QVoice::ageName(voice.age())));
    if (voice.name() == currentVoice.name())
      ui.voice->setCurrentIndex(ui.voice->count() - 1);
  }
  connect(ui.voice,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
    &MainWindow::voiceSelected);
}

void MainWindow::updateHistoryButtons()
{
  ui.backPushButton->setEnabled(m_textBackStack.count() > 0);
  ui.forwardPushButton->setEnabled(m_textForwardStack.count() > 0);
}

void MainWindow::setTextDisplay(QString textToSpeak)
{
  if (textToSpeak.isEmpty())
  {
    return;
  }

  m_textForwardStack.clear();
  m_textBackStack.push_front(ui.plainTextEdit->toPlainText());
  updateHistoryButtons();
  ui.plainTextEdit->setPlainText(textToSpeak);
}

void MainWindow::clipboard_changed()
{
  static int callTimes = 0;

#ifdef VERBOSE_DEBUG
    int cc = cc_num;
  qDebug() << " --------------> CLIP IN ----------" << cc
           << " MINE: " << m_clipboard->mimeData()->formats();

#endif

  if (ui.readClipCheckBox->isChecked())
  {
    QString textToSpeak = m_clipboard->text();

    if (m_speech->state() != QTextToSpeech::Ready)
    {
      if (!_lastText.isEmpty() && !textToSpeak.isEmpty() && textToSpeak != _lastText)
      {
        qDebug().noquote() << callTimes << " CALLING STOP: "
                           << textToSpeak.mid(0, qMin(textToSpeak.length(), 32))
                           << ((textToSpeak.length() > 32) ? "..." : "");
        m_speech->stop();
        callTimes = 0;
        QApplication::processEvents();
      }
    }
#ifdef VERBOSE_DEBUG
    else
    {
        qDebug().nospace() << "SPEECH IS READY "
                         << textToSpeak.mid(0, qMin(textToSpeak.length(), 32))
                         << ((textToSpeak.length() > 32) ? "..." : "");
    }
#endif


    if (!textToSpeak.isEmpty())
    {
      callTimes++;
      if (callTimes == 1)
      {
        QDeadlineTimer deadline(100);
        do
        {
          QApplication::processEvents();
          textToSpeak = m_clipboard->text();

        } while (!deadline.hasExpired());
#ifdef VERBOSE_DEBUG
        qDebug().noquote() << callTimes << " WAIT OVER: "
                           << textToSpeak.mid(0, qMin(textToSpeak.length(), 32))
                           << ((textToSpeak.length() > 32) ? "..." : "");
#endif

        callTimes = 0;

        if (textToSpeak.isEmpty())
        {
          _statusAnimator->showerror();
          trayIcon->showMessage("Quick Read", "Clipboad Empty,Cant Read", QSystemTrayIcon::NoIcon, 1000);
        }
        else if (!textNotSameOrSameCountTrigger(textToSpeak))
        {
            return;
        }

        setTextDisplay(textToSpeak);
        speakText(textToSpeak);
        callTimes = 0;
      }
      else
      {
        bool textTheSame = textToSpeak == m_clipboard->text();
        qDebug() << "CALLTIMES NOT 1, BAIL - calltimes: " << callTimes
                 << " text the same? " << (textTheSame ? "YES" : "NO");
      }
    }
  }


}

int MainWindow::keyExists(const QString &key)
{
    int idx = 0;
    for(auto & r : m_rules)
    {
        rule_t rr = parseRuleString(r);
        if(rr.first == key) {
            return idx;
        }
        idx++;
    }
    return -1;

}
bool MainWindow::textNotSameOrSameCountTrigger(const QString& str)
{
  bool rtval = true;
  if (str == _lastText)
  {
    _sameCount++;

    if (_sameCount < kSameTextTrycountTreshold)
    {
      rtval = false;
    }
    else
    {
      _sameCount = 0;
    }
  }

  return rtval;
}

const QString MainWindow::getEOLSymbole()
{
#ifdef Q_OS_WIN
    return QString("\n\r");
#else
    return QString("\n");
#endif

}
void MainWindow::on_lineEdit_editingFinished()
{
    if(ui.lineEdit->text().isEmpty())
        return;

    if((QApplication::keyboardModifiers()&Qt::ShiftModifier) == Qt::ShiftModifier)
    {
        on_pushButton_clicked();
    }
    else
    {
        QString rule = ui.lineEdit->text();
        rule_t rr = parseRuleString(rule);
        if (rr == rule_t())
            return;
        QString testText = QString("%1    %2").arg(rr.first).arg(rr.second);
        speakText(testText);
    }
}

rule_t MainWindow::parseRuleString(const QString &rule)
{
    rule_t rtval;
    QStringList toks;
    toks = rule.split(",");
    if (toks.count() < 2)
      return rtval;

    if(toks.count() == 3)
    {
        rtval.first = toks.at(1);
        rtval.second = toks.at(2);
    }
    else
    {
        rtval.first = toks.at(0);
        rtval.second = toks.at(1);
    }

    return rtval;
}

bool MainWindow::ruleIsCaseInsensitive(const QString &rule)
{
    QStringList toks;
    toks = rule.split(",");
    if (toks.count() == 3)
    {
        if(toks.at(0) == "i")
        {
            return true;
        }
    }

    return false;
}

bool MainWindow::addRuleString(QString rule)
{
    bool rtval = false;
    auto [key, val]=parseRuleString(rule);

    int i = keyExists(key);
    if(i>-1)
    {
       m_rules.replace(i,rule);
    }
    else
    {
        m_rules << rule;
    }
    populateRulesWidget(rule);
    rtval = true;

  return rtval;
}


// ADD button clicked
void MainWindow::on_pushButton_clicked()
{


  if(addRuleString(ui.lineEdit->text()))
  {
     saveRules();
     ui.lineEdit->clear();
  }
  else
  {
    // BAD RULE STRING
    ui.lineEdit->selectAll();
  }
}

void MainWindow::setSliderValue(const QSettings& settings, QSlider* slider)
{
  int defVal = slider->maximum() / 2;
  slider->setValue(settings.value(slider->objectName(), defVal).toInt());
}

void MainWindow::saveSliderValue(QSettings& settings, QSlider* slider)
{
  int val = slider->value();
  settings.setValue(slider->objectName(), val);
}

void MainWindow::saveRules()
{
  QSettings settings;
  QString defaultRulesPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + QApplication::applicationName());
  QString rulesPath = settings.value("rulesPath",defaultRulesPath).toString();
  QSettings rulessettings(QDir::toNativeSeparators(rulesPath + "/rules.ini"),QSettings::IniFormat);
  rulessettings.remove("rules");
  rulessettings.beginWriteArray("rules");
  for (int i = 0; i < ui.listWidget->count(); ++i)
  {
    rulessettings.setArrayIndex(i);
    rulessettings.setValue("rulestr", ui.listWidget->item(i)->text());
  }
  rulessettings.endArray();
}

void MainWindow::restoreRules()
{
    QSettings settings;

    QString defaultRulesPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + QApplication::applicationName());
    QString rulesPath = settings.value("rulesPath",defaultRulesPath).toString();
    QSettings rulessettings(QDir::toNativeSeparators(rulesPath + "/rules.ini"),QSettings::IniFormat);
    m_rules.clear();
  int size = rulessettings.beginReadArray("rules");
  for (int i = 0; i < size; ++i)
  {
    rulessettings.setArrayIndex(i);
    QString rulestr;
    rulestr = rulessettings.value("rulestr").toString();
    ui.listWidget->addItem(rulestr);
    m_rules << rulestr;
  }
  rulessettings.endArray();
}

QString MainWindow::rulesToString()
{
  QStringList lst;
  for (int i = 0; i < ui.listWidget->count(); ++i)
  {

    lst << ui.listWidget->item(i)->text();
  }
  return lst.join("\n");
}

void MainWindow::rulesFromString(const QString& str)
{
  QStringList lst = str.split("\n");
  ui.listWidget->clear();
  for (const auto& e : lst)
  {
    ui.listWidget->addItem(e);
  }
}

void MainWindow::saveSystemState()
{
  qDebug() << "saveSystemState()";
  QSettings settings;
  saveSliderValue(settings, ui.rate);
  saveSliderValue(settings, ui.pitch);
  saveSliderValue(settings, ui.volume);
  settings.setValue("MainWindow.geometry", geometry());

  settings.setValue("readClickboard", ui.readClipCheckBox->isChecked());
  settings.setValue("autoReadHistory", ui.autoReadHistoryCheckBox->isChecked());
  settings.setValue("MainWindow.visible", isVisible());


  saveRules();
}

void MainWindow::restoreWindowVis()
{
  QSettings settings;
  bool vis = settings.value("MainWindow.visible").toBool();
  if (vis)
  {
    MainWindow::showNormal();
  }
  else
  {
    MainWindow::close();
  }
}

void MainWindow::restoreSettings()
{
  restoreRules();

  QSettings settings;
  setSliderValue(settings, ui.rate);
  setSliderValue(settings, ui.pitch);
  setSliderValue(settings, ui.volume);

  setGeometry(settings.value("MainWindow.geometry", QRect(10, 10, 400, 400)).toRect());

  ui.readClipCheckBox->setChecked(settings.value("readClickboard", true).toBool());
  readClipboad->setChecked(ui.readClipCheckBox->isChecked());
  ui.autoReadHistoryCheckBox->setChecked(
    settings.value("autoReadHistory", true).toBool());
  m_restoreMainWindow = true;
  // QTimer::singleShot(1,this,&MainWindow::restoreWindowVis);
}


void MainWindow::createActions()
{
  minimizeAction = new QAction(tr("Mi&nimize"), this);
  connect(minimizeAction, &QAction::triggered, this, &QMainWindow::hide);

  maximizeAction = new QAction(tr("Ma&ximize"), this);
  connect(maximizeAction, &QAction::triggered, this, &QMainWindow::showMaximized);

  restoreAction = new QAction(tr("&Restore"), this);
  connect(restoreAction, &QAction::triggered, this, &QMainWindow::showNormal);
  connect(restoreAction, &QAction::triggered,
    [=]() { QTimer::singleShot(100, [=]() { this->raise(); }); });

  quitAction = new QAction(tr("&Quit"), this);
  connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

  readClipboad = new QAction(tr("&Read Clipboard"), this);
  readClipboad->setCheckable(true);
  connect(readClipboad, &QAction::triggered,
    [&]() { ui.readClipCheckBox->setChecked(readClipboad->isChecked()); });

  stopReading = new QAction(tr("&Stop"), this);
  connect(stopReading, &QAction::triggered, this, &MainWindow::stop);
}

void MainWindow::createTrayIcon()
{
  trayIconMenu = new QMenu(this);
  trayIconMenu->setTitle("Quick Read " + qApp->applicationVersion());
  trayIconMenu->addAction(minimizeAction);
  trayIconMenu->addAction(maximizeAction);
  trayIconMenu->addAction(restoreAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(readClipboad);
  trayIconMenu->addAction(stopReading);


  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);

  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setContextMenu(trayIconMenu);

  connect(
    trayIcon, &QSystemTrayIcon::activated, [=](QSystemTrayIcon::ActivationReason reason) {
      if (reason == QSystemTrayIcon::DoubleClick)
      {
        qDebug() << "restoreing from dbl";
        QTimer::singleShot(50, [=]() {
          restoreAction->trigger();
          this->setFocus();
          this->raise();
        });
      }
      else
      {
        m_speech->stop();
      }
    });

  _statusAnimator = new ReadingStatusAnimator(this);
  _statusAnimator->setSystemTrayIcon(trayIcon);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
#ifdef Q_OS_OSX
  if (!event->spontaneous() || !isVisible())
  {
    return;
  }
#endif
  if (trayIcon->isVisible())
  {
    hide();
    event->ignore();
  }
}

void MainWindow::setVisible(bool visible)
{
  if (m_restoreMainWindow)
  {
    QSettings settings;
    bool vis = settings.value("MainWindow.visible").toBool();
    QMainWindow::setVisible(vis);
    m_restoreMainWindow = false;
  }
  else
  {
    QMainWindow::setVisible(visible);
  }
}


void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem* item)
{
  speakText(item->text(), false);
  ui.lineEdit->setText(item->text());
}

bool MainWindow::isRuleStrValid(const QString& ruleText)
{
  rule_t rr = parseRuleString(ruleText);
  return rr!=rule_t();
}

void MainWindow::on_listWidget_currentTextChanged(const QString& currentText)
{
  ui.delPushButton->setEnabled(isRuleStrValid(currentText));
}

void MainWindow::on_listWidget_lostFocus()
{
    ui.delPushButton->setEnabled(ui.delPushButton->hasFocus());
}

void MainWindow::on_listWidget_reorderItems()
{
    int cnt = ui.listWidget->count();
    m_rules.clear();
    for(int r=0;r<cnt;r++)
    {
        QString str = ui.listWidget->item(r)->data(Qt::DisplayRole).toString();
        m_rules << str;
    }
    saveRules();
}

void MainWindow::removeRule(const QString& rule)
{
    int i = m_rules.indexOf(rule);
    if(i>-1)
    {
       m_rules.takeAt(i);
    }

    saveRules();
}

void MainWindow::on_delPushButton_clicked()
{
  int row = ui.listWidget->currentRow();
  QListWidgetItem* lwi = ui.listWidget->takeItem(row);
  if (lwi != nullptr)
  {
    m_lastRuleRemoved = lwi->text();
    removeRule(m_lastRuleRemoved);
    ui.restorePushButton->setEnabled(true);
  }
  populateRulesWidget();
  if(ui.listWidget->count() > row)
  {
      ui.listWidget->setCurrentRow(row);
  }
  else
  {
      ui.listWidget->setCurrentRow(ui.listWidget->count()-1);
  }
}

void MainWindow::on_restorePushButton_clicked()
{
  if (!m_lastRuleRemoved.isEmpty())
  {
    addRuleString(m_lastRuleRemoved);
    m_lastRuleRemoved.clear();
    ui.restorePushButton->setEnabled(false);
  }
}


void MainWindow::on_backPushButton_clicked()
{
  QString t = m_textBackStack.takeFirst();
  m_textForwardStack.push_front(ui.plainTextEdit->toPlainText());
  ui.plainTextEdit->setPlainText(t);
  updateHistoryButtons();

  if (ui.autoReadHistoryCheckBox->isChecked())
    speakText(t);
}

void MainWindow::on_forwardPushButton_clicked()
{
  QString t = m_textForwardStack.takeFirst();
  m_textBackStack.push_front(ui.plainTextEdit->toPlainText());
  ui.plainTextEdit->setPlainText(t);

  updateHistoryButtons();
  if (ui.autoReadHistoryCheckBox->isChecked())
    speakText(t);
}


void MainWindow::on_readClipCheckBox_clicked(bool checked)
{
  readClipboad->setChecked(checked);
  if (checked)
  {
    QString clipboardText = m_clipboard->text();
    if (!clipboardText.isEmpty() && clipboardText != ui.plainTextEdit->toPlainText())
    {

      setTextDisplay(clipboardText);
      speakText(clipboardText);
    }
  }
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{

    ui.delPushButton->setEnabled(isRuleStrValid(item->text()));
}



void MainWindow::on_importRulesPB_clicked()
{
    QSettings settings;
    QString lastImportDir = settings.value("lastImportDir").toString();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Rules"),
                                                    lastImportDir,
                                                    tr("Rules(*.ini)"));

    QFileInfo info(fileName);
    if(!info.exists())
        return;

    settings.setValue("lastImportDir",info.path());

  QSettings rulessettings(QDir::toNativeSeparators(fileName),QSettings::IniFormat);

  int size = rulessettings.beginReadArray("rules");
  qDebug() << "Import " << fileName << " has " << size << " rules";
  for (int i = 0; i < size; ++i)
  {
    rulessettings.setArrayIndex(i);
    QString rulestr;
    rulestr = rulessettings.value("rulestr").toString();

    if(!m_rules.contains(rulestr))
        m_rules << rulestr;

  }
  rulessettings.endArray();
  populateRulesWidget();
  saveRules();
}


void MainWindow::on_quitPushButton_clicked()
{
    QApplication::quit();
}


// pops up a message box with some syntax rules
void MainWindow::on_ruleSyntaxPushButton_clicked()
{

    QStringList msgList;

    msgList << tr("<h1>Rule Syntax</h1>");
    msgList << "<pre>";
    msgList << tr("<ul>");
    msgList << tr("<li>JuMp,Yo Jump - replace all instances of <b>JuMp</b> with <b>Yo Jump</b></li>");
    msgList << tr("<li>i,jump,Yo Jump - case insisitive replace all instances of <b>jump</b> (any case) with <b>Yo Jump</b></li>");
    msgList << tr("<li><b>\\b</b>jump<b>\\b</b>,Yo Jump - whole word replace all instances of <b>jump</b> with <b>Yo Jump</b></li>");
    msgList << tr("</ul>");
    msgList << "<br>";
    msgList << "The rule file is in .ini format";
    msgList << "</pre>";


    QMessageBox::information(
        this,
        tr("Rule Syntax"),
        msgList.join("\n"));
}

