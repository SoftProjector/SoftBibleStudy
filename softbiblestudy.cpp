/***************************************************************************
//
//    SoftProjector - an open source media projection software
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

#include <QDesktopWidget>
#include "softbiblestudy.hpp"
#include "ui_softbiblestudy.h"
#include "aboutdialog.hpp"
#include "printpreviewdialog.hpp"

SoftBibleStudy::SoftBibleStudy(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SoftBibleStudyClass)
{
    // Load settings
    mySettings.loadSettings();
    theme.setThemeId(mySettings.general.currentThemeId);
    theme.loadTheme();
    // Reset current theme id if initial was 0
    mySettings.general.currentThemeId = theme.getThemeId();

    // Update Themes Bible Versions
    theme.bible.versions = mySettings.bibleSets;
    theme.bible2.versions = mySettings.bibleSets2;

    //Setting up the Display Screen
    desktop = new QDesktopWidget();
    // NOTE: With virtual desktop, desktop->screen() will always return the main screen,
    // so this will initialize the Display1 widget on the main screen:

    bibleWidget = new BibleWidget;
    settingsDialog = new SettingsDialog(this);
    helpDialog = new HelpDialog();
    manageDialog = new ManageDataDialog(this);

    ui->setupUi(this);

    // Create action group for language slections
    languagePath = qApp->applicationDirPath()+QString(QDir::separator())+"translations"+QString(QDir::separator());
    createLanguageActions();

    // Always place the "Settings" menu item under the application
    // menu, even if the item is translated (Mac OS X):
    ui->actionSettings->setMenuRole(QAction::PreferencesRole);
    // FIXME Make the Preferences menu appear in the menu bar even for the
    // display window (Mac OS X)

    // Apply Settings
    applySetting(mySettings.general, theme, mySettings.slideSets, mySettings.bibleSets, mySettings.bibleSets2);


    showing = false;

    ui->projectTab->clear();
    ui->projectTab->addTab(bibleWidget,QIcon(":/icons/icons/book.png"), tr("Bible (F6)"));
    ui->projectTab->setCurrentIndex(0);


//    connect(bibleWidget, SIGNAL(setArrowCursor()), this, SLOT(setArrowCursor()));
//    connect(bibleWidget, SIGNAL(setWaitCursor()), this, SLOT(setWaitCursor()));
//    connect(manageDialog, SIGNAL(setMainArrowCursor()), this, SLOT(setArrowCursor()));
//    connect(manageDialog, SIGNAL(setMainWaitCursor()), this, SLOT(setWaitCursor()));
    connect(languageGroup, SIGNAL(triggered(QAction*)), this, SLOT(switchLanguage(QAction*)));

    // Add tool bar actions
    ui->toolBarFile->addAction(ui->actionNewSchedule);
    ui->toolBarFile->addAction(ui->actionOpenSchedule);
    ui->toolBarFile->addAction(ui->actionSaveSchedule);
    ui->toolBarFile->addSeparator();
    ui->toolBarFile->addAction(ui->actionPrint);

    ui->toolBarSchedule->addAction(ui->actionMoveScheduleTop);
    ui->toolBarSchedule->addAction(ui->actionMoveScheduleUp);
    ui->toolBarSchedule->addAction(ui->actionMoveScheduleDown);
    ui->toolBarSchedule->addAction(ui->actionMoveScheduleBottom);
    ui->toolBarSchedule->addSeparator();
    ui->toolBarSchedule->addAction(ui->actionScheduleAdd);
    ui->toolBarSchedule->addAction(ui->actionScheduleRemove);
    ui->toolBarSchedule->addAction(ui->actionScheduleClear);

    ui->toolBarEdit->addAction(ui->actionNew);
    ui->toolBarEdit->addAction(ui->actionEdit);
    ui->toolBarEdit->addAction(ui->actionCopy);
    ui->toolBarEdit->addAction(ui->actionDelete);
    ui->toolBarEdit->addSeparator();
    ui->toolBarEdit->addAction(ui->actionSettings);
    ui->toolBarEdit->addSeparator();
    ui->toolBarEdit->addAction(ui->actionSong_Counter);
    ui->toolBarEdit->addSeparator();
    ui->toolBarEdit->addAction(ui->action_Help);

    ui->toolBarShow->addAction(ui->actionShow);
    ui->toolBarShow->addAction(ui->actionClear);
    ui->toolBarShow->addAction(ui->actionHide);
    ui->toolBarShow->addAction(ui->actionCloseDisplay);

    ui->actionShow->setEnabled(false);
    ui->actionHide->setEnabled(false);
    ui->actionClear->setEnabled(false);


    version_string = "2.2";
    this->setWindowTitle("SoftProjector " + version_string);
}

SoftBibleStudy::~SoftBibleStudy()
{
    saveSettings();
    delete bibleWidget;
    delete manageDialog;
    delete desktop;
    delete languageGroup;
    delete settingsDialog;
    delete shpgUP;
    delete shpgDwn;
    delete shSart1;
    delete shSart2;
    delete helpDialog;
    delete ui;
}

void SoftBibleStudy::saveSettings()
{
    // Save splitter states
    mySettings.spMain.spSplitter = ui->splitter->saveState();
    mySettings.spMain.bibleHiddenSplitter = bibleWidget->getHiddenSplitterState();
    mySettings.spMain.bibleShowSplitter = bibleWidget->getShownSplitterState();

    // Save window maximized state
    mySettings.spMain.isWindowMaximized = this->isMaximized();

    // save translation settings
    QList<QAction*> languageActions = ui->menuLanguage->actions();

    for(int i(0);i < languageActions.count();++i)
    {
        if(languageActions.at(i)->isChecked())
        {
            if(i < languageActions.count())
                mySettings.spMain.uiTranslation = languageActions.at(i)->data().toString();
            else
                mySettings.spMain.uiTranslation = "en";
        }
    }

    // save settings
    mySettings.saveSettings();
    theme.saveThemeUpdate();
}

void SoftBibleStudy::updateSetting(GeneralSettings &g, Theme &t, SlideShowSettings &ssets,
                                  BibleVersionSettings &bsets, BibleVersionSettings &bsets2)
{
    mySettings.general = g;
    mySettings.slideSets = ssets;
    mySettings.bibleSets = bsets;
    mySettings.bibleSets2 = bsets2;
    mySettings.saveSettings();
    theme = t;
    bibleWidget->setSettings(mySettings.bibleSets);

    theme.bible.versions = mySettings.bibleSets;
    theme.bible2.versions = mySettings.bibleSets2;
}

void SoftBibleStudy::applySetting(GeneralSettings &g, Theme &t, SlideShowSettings &s,
                                 BibleVersionSettings &b1, BibleVersionSettings &b2)
{
    updateSetting(g,t,s,b1,b2);

    // Apply splitter states
    ui->splitter->restoreState(mySettings.spMain.spSplitter);
    bibleWidget->setHiddenSplitterState(mySettings.spMain.bibleHiddenSplitter);
    bibleWidget->setShownSplitterState(mySettings.spMain.bibleShowSplitter);

    // Apply window maximized
    if(mySettings.spMain.isWindowMaximized)
        this->setWindowState(Qt::WindowMaximized);

    // Apply current translation
    QList<QAction*> la = ui->menuLanguage->actions();
    QString splocale;
    for(int i(0);i < la.count(); ++i)
    {
        if(la.at(i)->data().toString() == mySettings.spMain.uiTranslation)
        {
            if(i < la.count())
            {
                ui->menuLanguage->actions().at(i)->setChecked(true);
                splocale = mySettings.spMain.uiTranslation;
            }
            else
            {
                ui->menuLanguage->actions().at(0)->setChecked(true);//default
                splocale = "en";
            }
        }
    }
    cur_locale = splocale;
    retranslateUis();
}

void SoftBibleStudy::closeEvent(QCloseEvent *event)
{
    if(is_schedule_saved || schedule_file_path.isEmpty())
    {
        QCoreApplication::exit(0);
        event->accept();
    }
    else
    {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Schedule not saved"));
        mb.setText(tr("Do you want to save current schedule?"));
        mb.setIcon(QMessageBox::Question);
        mb.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard);
        mb.setDefaultButton(QMessageBox::Save);
        int ret = mb.exec();

        switch (ret)
        {
        case QMessageBox::Save:
            // Save Schedule and close
            QCoreApplication::exit(0);
            event->accept();
            break;
        case QMessageBox::Cancel:
            // Cancel was clicked, do nothing
            event->ignore();
            break;
        case QMessageBox::Discard:
            // Close without saving
            QCoreApplication::exit(0);
            event->accept();
            break;
        default:
            // should never be reached
            break;
        }
    }
}
void SoftBibleStudy::on_actionClose_triggered()
{
    close();
}


void SoftBibleStudy::on_projectTab_currentChanged(int index)
{
    updateEditActions();
}

void SoftBibleStudy::updateEditActions()
{
    int ctab = ui->projectTab->currentIndex();
    // ctab - 0=bible, 1=songs, 2=pix, 3=media, 4=annouce
    if(ctab == 0)
    {
        ui->actionNew->setText("");
        ui->actionEdit->setText("");
        ui->actionCopy->setText("");
        ui->actionDelete->setText(tr("&Clear Bible History List"));
        ui->actionNew->setIcon(QIcon());
        ui->actionEdit->setIcon(QIcon());
        ui->actionCopy->setIcon(QIcon());
        ui->actionDelete->setIcon(QIcon(":/icons/icons/bibleHistoryDelete.png"));
    }
}

void SoftBibleStudy::on_actionNew_triggered()
{
//    int ctab = ui->projectTab->currentIndex();
//    if(ctab == 1)
//        //newSong();
//    else if(ctab == 2)
//        newSlideShow();
//    else if(ctab == 3)
//        addMediaToLibrary();
//    else if(ctab == 4)
//        newAnnouncement();
}

void SoftBibleStudy::on_actionEdit_triggered()
{
//    int ctab = ui->projectTab->currentIndex();
//    if(ctab == 1)
//        editSong();
//    else if(ctab == 2)
//        editSlideShow();
//    else if(ctab == 4)
//        editAnnouncement();
}

void SoftBibleStudy::on_actionCopy_triggered()
{
//    int ctab = ui->projectTab->currentIndex();
//    if(ctab == 1)
//        copySong();
//    else if(ctab == 4)
//        copyAnnouncement();
}

void SoftBibleStudy::on_actionDelete_triggered()
{
//    int ctab = ui->projectTab->currentIndex();
//    if(ctab == 0)

//    else if(ctab == 1)
//        deleteSong();
//    else if(ctab == 2)
//        deleteSlideShow();
//    else if(ctab == 3)
//        removeMediaFromLibrary();
//    else if(ctab == 4)
//        deleteAnnoucement();
}

void SoftBibleStudy::on_actionManage_Database_triggered()
{
    QSqlQuery sq;

    manageDialog->setDataDir(appDataDir);
    manageDialog->exec();

    // Relaod themes if a theme has been deleted
    if (manageDialog->reloadThemes)
    {
        // Check if current theme has been deleted
        sq.exec("SELECT * FROM Themes WHERE id = " + QString::number(theme.getThemeId()));
        if(!sq.first())
        {
            GeneralSettings g = mySettings.general;
            Theme t;
            sq.exec("SELECT id FROM Themes");
            sq.first();
            t.setThemeId(sq.value(0).toInt());
            t.loadTheme();
            g.currentThemeId = t.getThemeId();
            updateSetting(g,t,mySettings.slideSets,mySettings.bibleSets,mySettings.bibleSets2);
        }
    }

    // Reload Bibles if Bible has been deleted
    if (manageDialog->reload_bible)
    {
        // check if Primary bible has been removed
        sq.exec("SELECT * FROM BibleVersions WHERE id = " + mySettings.bibleSets.primaryBible);
        if (!sq.first())
        {
            // If original primary bible has been removed, set first bible in the list to be primary
            sq.clear();
            sq.exec("SELECT id FROM BibleVersions");
            sq.first();
            mySettings.bibleSets.primaryBible = sq.value(0).toString();
        }
        sq.clear();

        // check if secondary bible has been removed, if yes, set secondary to "none"
        sq.exec("SELECT * FROM BibleVersions WHERE id = " + mySettings.bibleSets.secondaryBible);
        if (!sq.first())
            mySettings.bibleSets.secondaryBible = "none";
        sq.clear();

        // check if trinary bible has been removed, if yes, set secondary to "none"
        sq.exec("SELECT * FROM BibleVersions WHERE id = " + mySettings.bibleSets.trinaryBible);
        if (!sq.first())
            mySettings.bibleSets.trinaryBible = "none";
        sq.clear();

        // check if operator bible has been removed, if yes, set secondary to "same"
        sq.exec("SELECT * FROM BibleVersions WHERE id = " + mySettings.bibleSets.operatorBible);
        if (!sq.first())
            mySettings.bibleSets.operatorBible = "same";
        bibleWidget->setSettings(mySettings.bibleSets);
    }
}

void SoftBibleStudy::on_actionAbout_triggered()
{
    AboutDialog *aboutDialog;
    aboutDialog = new AboutDialog(this, version_string);
    aboutDialog->exec();
    delete aboutDialog;
}

void SoftBibleStudy::on_action_Help_triggered()
{
    //helpDialog->show();
     QDesktopServices::openUrl(QUrl("http://softprojector.org/help/index.html"));
}

void SoftBibleStudy::setArrowCursor()
{
    this->setCursor(Qt::ArrowCursor);
}

void SoftBibleStudy::setWaitCursor()
{
    this->setCursor(Qt::WaitCursor);
}

void SoftBibleStudy::createLanguageActions()
{
    // find all *.qm files at language folder
     // and create coresponding action in language menu

        languageGroup = new QActionGroup(this);
        //default language and flag
        QAction *englishAction = new QAction(QIcon(":/icons/icons/flag_uk.png"), "English", this);
        englishAction->setCheckable(true);
        englishAction->setChecked(true);
        englishAction->setIconVisibleInMenu(true);
        languageGroup->addAction(englishAction);
        ui->menuLanguage->addAction(englishAction);

        QDir languageDir(languagePath);
        //all available languages
        QStringList languagesList = languageDir.entryList(QStringList("softpro_*.qm"), QDir::Files);
        //all available flags
        QStringList flagsList = languageDir.entryList(QStringList("flag_*.png"), QDir::Files);

        foreach(QString agent, languagesList)
        {
            // local translator for taken original language's name
            QTranslator tmpTranslator;
            tmpTranslator.load(agent, languageDir.absolutePath());
            // this string are used for detection language' name
            // this is one of translated strings

            QString fullLanguageName = tmpTranslator.translate("Native language name", "English","Do not change");
            QAction *tmpAction = new QAction(fullLanguageName, this);

            QString splocale = agent.remove(0, agent.indexOf('_')+1);
            splocale.chop(3);

            // flag's file name
            QString flagFileName = "flag_"+splocale+".png";
            if(flagsList.contains(flagFileName))//  if flag is available
            {
                tmpAction->setIcon(QIcon(languageDir.absolutePath() + QDir::separator() + flagFileName));
                tmpAction->setIconVisibleInMenu(true);
            }

            tmpAction->setData(splocale);// information abount localization
            tmpAction->setCheckable(true);
            languageGroup->addAction(tmpAction);
            ui->menuLanguage->addAction(tmpAction);
        }
}

void SoftBibleStudy::switchLanguage(QAction *action)
{
    cur_locale = action->data().toString();
    retranslateUis();
}

void SoftBibleStudy::retranslateUis()
{
    qApp->removeTranslator(&translator);
    if(cur_locale != "en")
    {
        translator.load("softpro_"+cur_locale+".qm", QDir(languagePath).absolutePath());
        // qt libs translator must be add there,
        // but where are qt_locale.qm files?
        qApp->installTranslator(&translator);
    }

    ui->retranslateUi(this);
    ui->projectTab->setTabText(0, tr("Bible (F6)"));
    ui->projectTab->setTabText(1, tr("Songs (F7)"));
    ui->projectTab->setTabText(2, tr("Pictures"));
//    ui->projectTab->setTabText(3, tr("Media"));
    ui->projectTab->setTabText(3, tr("Announcements (F8)"));
    updateEditActions();
}

void SoftBibleStudy::on_actionPrint_triggered()
{
    PrintPreviewDialog* p;
    p = new PrintPreviewDialog(this);
    if(ui->projectTab->currentIndex() == 0)
    {
        p->setText(mySettings.bibleSets.operatorBible + "," + mySettings.bibleSets.primaryBible,
                   bibleWidget->getCurrentBook(),bibleWidget->getCurrentChapter());
        p->exec();
    }
    else if (ui->projectTab->currentIndex() == 1)
    {

        {
            QMessageBox ms(this);
            ms.setWindowTitle(tr("No song selected"));
            ms.setText(tr("No song has been selected to be printed."));
            ms.setInformativeText(tr("Please select a song to be printed."));
            ms.setIcon(QMessageBox::Information);
            ms.exec();
        }
    }
    else if (ui->projectTab->currentIndex() == 4)
    {

        {
            QMessageBox ms(this);
            ms.setWindowTitle(tr("No announcement selected"));
            ms.setText(tr("No announcement has been selected to be printed."));
            ms.setInformativeText(tr("Please select a announcement to be printed."));
            ms.setIcon(QMessageBox::Information);
            ms.exec();
        }
    }
    delete p;
}
