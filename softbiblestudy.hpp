/***************************************************************************
//
//    softProjector - an open source media projection software
//    Copyright (C) 2017  Vladislav Kobzar
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation version 3 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
***************************************************************************/

#ifndef SOFTBIBLESTUDY_H
#define SOFTBIBLESTUDY_H

#include <QMainWindow>
#include "biblewidget.hpp"
#include "bible.hpp"
#include "managedatadialog.hpp"
#include "settingsdialog.hpp"
#include "settings.hpp"
#include "helpdialog.hpp"

class QActionGroup;

namespace Ui
{
class SoftBibleStudyClass;
}

class SoftBibleStudy : public QMainWindow
{
    Q_OBJECT

public:
    SoftBibleStudy(QWidget *parent = 0);
    ~SoftBibleStudy();
    BibleWidget *bibleWidget;
    ManageDataDialog *manageDialog;
    QDesktopWidget *desktop;

    bool showing; // whether we are currently showing to the projector
    Verse current_verse;
    QString version_string;
    Theme theme;
    Settings mySettings;

    SoftBibleStudy *softProjector;

public slots:
    void updateSetting(GeneralSettings &g,Theme &t, SlideShowSettings &ssets,
                       BibleVersionSettings &bsets, BibleVersionSettings &bsets2);
    void saveSettings();
    void setWaitCursor();
    void setArrowCursor();
    void setAppDataDir(QDir d){appDataDir = d;}

private:
    QDir appDataDir;
    Ui::SoftBibleStudyClass *ui;
    SettingsDialog *settingsDialog;
    HelpDialog *helpDialog;
    bool new_list;
    QActionGroup *languageGroup;
    QString languagePath;
    QTranslator translator;

    //For saving and opening schedule files
    //QString project_file_path;
    QString schedule_file_path;
    bool is_schedule_saved;
    QString cur_locale;
    bool isSingleScreen;
    bool hasDisplayScreen2;

    // shortcuts
    QShortcut *shpgUP;
    QShortcut *shpgDwn;
    QShortcut *shSart1;
    QShortcut *shSart2;

private slots:
    void applySetting(GeneralSettings &g, Theme &t, SlideShowSettings &s,
                      BibleVersionSettings &b1, BibleVersionSettings &b2);
    void on_projectTab_currentChanged(int index);
    void updateEditActions();
    void on_actionNew_triggered();
    void on_actionEdit_triggered();
    void on_actionCopy_triggered();
    void on_actionDelete_triggered();

    void retranslateUis();
    void createLanguageActions();
    void switchLanguage(QAction *action);
    void on_action_Help_triggered();
    void on_actionManage_Database_triggered();
    void on_actionAbout_triggered();
    //void on_actionSettings_triggered();
    void on_actionClose_triggered();
    // void setChapterList(QStringList chapter_list, QString caption, QItemSelection selectedItems);

    void on_actionPrint_triggered();

    //void on_actionClear_triggered();


protected:
    void closeEvent(QCloseEvent *event);
    //void keyPressEvent(QKeyEvent *event);
};

#endif // SOFTBIBLESTUDY_H
