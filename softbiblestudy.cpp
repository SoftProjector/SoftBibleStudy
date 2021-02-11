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

    //Setting up the Display Screen
    desktop = new QDesktopWidget();
    // NOTE: With virtual desktop, desktop->screen() will always return the main screen,
    // so this will initialize the Display1 widget on the main screen:

    settingsDialog = new SettingsDialog(this);
    helpDialog = new HelpDialog();
    manageDialog = new ManageDataDialog(this);



    ui->setupUi(this);

    chapter_validator = new QIntValidator(1, 1, ui->chapter_ef);

    ui->chapter_ef->setValidator( chapter_validator );

    // Create action group for language slections
    languagePath = qApp->applicationDirPath()+QString(QDir::separator())+"translations"+QString(QDir::separator());
    createLanguageActions();

    // Always place the "Settings" menu item under the application
    // menu, even if the item is translated (Mac OS X):
    ui->actionSettings->setMenuRole(QAction::PreferencesRole);
    // FIXME Make the Preferences menu appear in the menu bar even for the
    // display window (Mac OS X)

    // Apply Settings
    applySetting();
    loadBibles();

    showing = false;

    connect(languageGroup, SIGNAL(triggered(QAction*)), this, SLOT(switchLanguage(QAction*)));


    ui->toolBarEdit->addAction(ui->actionNew);
    ui->toolBarEdit->addAction(ui->actionEdit);
    ui->toolBarEdit->addAction(ui->actionCopy);
    ui->toolBarEdit->addAction(ui->actionDelete);
    ui->toolBarEdit->addSeparator();
    ui->toolBarEdit->addAction(ui->actionSettings);
    ui->toolBarEdit->addSeparator();
    ui->toolBarEdit->addSeparator();
    ui->toolBarEdit->addAction(ui->action_Help);

    on_hide_result_button_clicked();



    highlight = new HighlighterDelegate(ui->search_results_list);
    ui->search_results_list->setItemDelegate(highlight);

    version_string = "0.0.1";
    this->setWindowTitle("SoftBibleStudy" + version_string);
}

SoftBibleStudy::~SoftBibleStudy()
{
    saveSettings();
    delete manageDialog;
    delete desktop;
    delete languageGroup;
    delete settingsDialog;
    delete shpgUP;
    delete shpgDwn;
    delete shSart1;
    delete shSart2;
    delete helpDialog;

    delete chapter_validator;
    delete ui;
}

void SoftBibleStudy::saveSettings()
{
    // Save splitter states
    mySettings.spMain.bibleHiddenSplitter = getHiddenSplitterState();
    mySettings.spMain.bibleShowSplitter = getShownSplitterState();

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
}

void SoftBibleStudy::updateSetting()
{
    mySettings.saveSettings();
}

void SoftBibleStudy::applySetting()
{

    qDebug()<<mySettings.bibleVersions.bibleOne<<mySettings.bibleVersions.bibleTwo<<mySettings.bibleVersions.bibleThree;
    updateSetting();

    // Apply splitter states
    setHiddenSplitterState(mySettings.spMain.bibleHiddenSplitter);
    setShownSplitterState(mySettings.spMain.bibleShowSplitter);

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

    Database db;
    biblesInDatabase = db.getBibles();
    QStringList bl;

    foreach(Bibles b, biblesInDatabase){
        bl.append(b.title);
    }

    ui->comboBoxBibleOne->addItems(bl);
    ui->comboBoxBibleTwo->addItems(bl);
    ui->comboBoxBibleThree->addItems(bl);

    ui->comboBoxBibleOne->setCurrentText(biblesInDatabase.at(mySettings.bibleVersions.bibleOne.toInt()-1).title);
    ui->comboBoxBibleTwo->setCurrentText(biblesInDatabase.at(mySettings.bibleVersions.bibleTwo.toInt()-1).title);
    ui->comboBoxBibleThree->setCurrentText(biblesInDatabase.at(mySettings.bibleVersions.bibleThree.toInt()-1).title);


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


void SoftBibleStudy::updateEditActions()
{
    int ctab = 0;
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


    // Reload Bibles if Bible has been deleted
    if (manageDialog->reload_bible)
    {
        /*
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
        bibleSettings = mySettings.bibleSets;
        */
    }
}

void SoftBibleStudy::on_actionAbout_triggered()
{
    AboutDialog *aboutDialog;
    aboutDialog = new AboutDialog(this, version_string);
    aboutDialog->exec();
    delete aboutDialog;
}

void SoftBibleStudy::on_actionSettings_triggered()
{
    //settingsDialog->loadSettings(mySettings.general,theme,mySettings.slideSets, mySettings.bibleSets,mySettings.bibleSets2);
    //settingsDialog->exec();
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
    updateEditActions();
}

void SoftBibleStudy::on_actionPrint_triggered()
{
    PrintPreviewDialog* p;
    p = new PrintPreviewDialog(this);

        p->setText(mySettings.bibleVersions.bibleOne ,
                   getCurrentBook(),getCurrentChapter());
        p->exec();

    delete p;
}


/**************************************************************/

//void SoftBibleStudy::changeEvent(QEvent *e)
//{
//    QWidget::changeEvent(e);
//    switch (e->type()) {
//    case QEvent::LanguageChange:
//        ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
//}

void SoftBibleStudy::loadBibles()
{
    bible1.loadBible(mySettings.bibleVersions.bibleOne);
    bible2.loadBible(mySettings.bibleVersions.bibleTwo);
    bible3.loadBible(mySettings.bibleVersions.bibleThree);

    ui->listBook->clear();
    ui->listBook->addItems(bible1.getBooks());
    ui->listBook->setCurrentRow(0);
}

void SoftBibleStudy::on_listBook_currentTextChanged(QString currentText)
{
    int s = ui->listBook->currentRow();
    if( s != -1 )
    {
        int max_chapter = bible1.books.at(bible1.getCurrentBookRow(currentText)).chapterCount;
        ui->listChapterNum->clear();
        for(int i=0; i<max_chapter; ++i)
            ui->listChapterNum->addItem(QString::number(i+1));
        chapter_validator->setTop(max_chapter);
        if( ui->listChapterNum->currentRow() != 0 )
            ui->listChapterNum->setCurrentRow(0);
    }
    else
    {
        // No bible book selected
        chapter_validator->setTop(1);
        ui->listChapterNum->clear();
    }
}

void SoftBibleStudy::on_listChapterNum_currentTextChanged(QString currentText)
{
    int s = ui->listChapterNum->currentRow();
    if( s != -1 )
    {
        // This optimization is required in order for the bible filter entry field to work fast:
        if( currentBook != getCurrentBook() || currentChapter != currentText.toInt() )
        {
            currentBook = getCurrentBook();
            currentChapter = currentText.toInt();
            currentChapterList = bible1.getChapter(bible1.books.at(bible1.getCurrentBookRow(currentBook)).bookId.toInt(), currentChapter);
        }

        ui->chapter_preview_list->clear();
        ui->chapter_preview_list->addItems(currentChapterList);
        ui->chapter_ef->setText(currentText);
        ui->chapter_preview_list->setCurrentRow(0);
    }
    else
    {
        ui->chapter_preview_list->clear();
    }
}

QString SoftBibleStudy::getCurrentBook()
{
    return ui->listBook->currentItem()->text();
}

int SoftBibleStudy::getCurrentChapter()
{
    return ui->listChapterNum->currentItem()->text().toInt();
}

/*
bool SoftBibleStudy::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->spinVerse && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::KeyDown) {
            qDebug() << "DOWN KEY";
            // Special tab handling
            return true;
        } else
            return false;
    }
    return false;
}
*/

void SoftBibleStudy::on_chapter_preview_list_currentRowChanged(int currentRow)
{

}

void SoftBibleStudy::on_chapter_preview_list_doubleClicked(QModelIndex index)
{
    // Called when a chapter or verse is double clicked

}



void SoftBibleStudy::on_lineEditBook_textChanged(QString text)
{
    // Called when the bible book filter field is modified.
    QStringList all_books = bible1.getBooks();

    // Remove trailing spaces:
    text = text.trimmed();

    int chapter = 0;
    int verse = 0;

    // Check whether the user entered a search string that include book, chapter,
    // and verse. For example: "1King 3 13"
    QStringList search_words = text.split(" ");

    // Allows the user to use more than one space as a seperator:
    search_words.removeAll("");

    if( search_words.count() > 1 )
    {
        bool ok;
        int num1 = search_words.last().toInt(&ok);
        if( ok )
        {
            chapter = num1;
            search_words.removeLast();
            if( search_words.count() > 1 )
            {
                bool ok2;
                int num2 = search_words.last().toInt(&ok2);
                if( ok2 )
                {
                    search_words.removeLast();
                    chapter = num2;
                    verse = num1;
                }
            }
            text = search_words.join(" ");
        }
    }

    // Now search all books to find the matching book:
    if( text.isEmpty() )
    {
        // Show all bible books
        if( ui->listBook->count() != all_books.count() )
        {
            // This is an important optimization
            ui->listBook->clear();
            ui->listBook->addItems(all_books);
        }
    }
    else
    {
        // Show only the bible books that match the filter
        QStringList filtered_books;
        if( text.at(0).isDigit() )
        {
            // First character of filter text is a number. Special search, where the
            // first character must be the first character of the first word of the book;
            // while the rest of the filter must be the beginning of the second book word.
            QString num_str(text.at(0));
            QString name_str = text.remove(0, 1);
            // Allow for space inbetween num_str and name_str e.g. "1 Peter"
            name_str = name_str.trimmed();
            for(int i=0; i<all_books.count(); i++)
            {
                QString book = all_books.at(i);
                QStringList book_words = book.split(" ");

                if( ! book_words.at(0).startsWith(num_str) )
                    continue;
                if( !book_words.at(1).startsWith(name_str, Qt::CaseInsensitive) )
                    continue;

                filtered_books.append(book);
            }
        }
        else
            filtered_books = all_books.filter(text, Qt::CaseInsensitive);

        if( ui->listBook->count() != filtered_books.count() )
        {
            // This is an important optimization
            // FIXME don't just check the count; check values
            ui->listBook->clear();
            ui->listBook->addItems(filtered_books);
        }
    }

    if( ui->listBook->count() > 0 )
        // Select the first row. This will take a longer time only if it will cause
        // a new chapter to be loaded into the preview
        ui->listBook->setCurrentRow(0);

    if( chapter != 0 && chapter <= ui->listChapterNum->count() )
    {
        if( ui->listChapterNum->currentRow() != (chapter-1) )
            ui->listChapterNum->setCurrentRow(chapter-1);
        if( verse != 0 && verse <= ui->chapter_preview_list->count() )
            ui->chapter_preview_list->setCurrentRow(verse-1);
    }
}

void SoftBibleStudy::on_chapter_ef_textChanged(QString new_string)
{
    int value = new_string.toInt();
    ui->listChapterNum->setCurrentRow(value-1);
}

void SoftBibleStudy::on_search_button_clicked()
{
    QString search_text = ui->search_ef->text();
    search_text = clean(search_text); // remove all none alphanumeric charecters

    // Make sure that there is some text to do a search for, if none, then return
    if(search_text.count()<1)
    {
        ui->search_ef->clear();
        ui->search_ef->setPlaceholderText(tr("Please enter search text"));
        return;
    }

    emit setWaitCursor();
    int type = ui->comboBoxSearchType->currentIndex();
    int range = ui->comboBoxSearchRange->currentIndex();

    QRegExp rx, rxh;
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    search_text.replace(" ","\\W*");
    if(type == 0)
    {
        // Search text phrase
        rx.setPattern(search_text);
        rxh.setPattern(search_text);
    }
    else if(type == 1)
    {
        // Search whole word exsact phrase only
        rx.setPattern("\\b"+search_text+"\\b");
        rxh.setPattern("\\b"+search_text+"\\b");
    }
    else if(type == 2)
    {
        // Search begining of every line
        rx.setPattern("^"+search_text);
        rxh.setPattern(search_text);
    }
    else if(type == 3 || type == 4)
    {
        // Search for any of the search words
        search_text.replace("\\W*","|");
        rx.setPattern("\\b("+search_text+")\\b");
        rxh.setPattern("\\b("+search_text+")\\b");
    }

    highlight->highlighter->setHighlightText(rxh.pattern()); // set highlighting rule

    if(range == 0) // Search entire Bible
        search_results = bible1.searchBible((type == 4),rx);
    else if(range == 1) // Search current book only
        search_results = bible1.searchBible((type == 4),rx,
                                           bible1.books.at(bible1.getCurrentBookRow(ui->listBook->currentItem()->text())).bookId.toInt());
    else if (range == 2) // Search current chapter only
        search_results = bible1.searchBible((type == 4),rx,
                                           bible1.books.at(bible1.getCurrentBookRow(ui->listBook->currentItem()->text())).bookId.toInt(),
                                           ui->listChapterNum->currentItem()->text().toInt());

    ui->search_results_list->clear();

    if (!search_results.isEmpty()) // If have results, then show them
    {
        if( not ui->result_label->isVisible() )
        {
            ui->lineEditBook->clear();
            hidden_splitter_state = ui->results_splitter->saveState();
            ui->result_label->show();
            ui->result_count_label->show();
            ui->search_results_list->show();
            ui->hide_result_button->show();
            ui->search_layout->addItem(ui->results_layout);
            ui->results_splitter->restoreState(shown_splitter_state);
        }
        QStringList verse_list;
        int count = search_results.count();

        ui->result_count_label->setText(tr("Total\nresutls:\n%1").arg(count));

        for(int i(0);i<count;i++)
            verse_list.append(search_results.at(i).verse_text);
        ui->search_results_list->addItems(verse_list);
    }
    else // If no relust, notify the user and hide result list
        ui->result_count_label->setText(tr("No search\nresults."));

    emit setArrowCursor();
}

void SoftBibleStudy::on_hide_result_button_clicked()
{
    shown_splitter_state = ui->results_splitter->saveState();
    ui->result_label->hide();
    ui->result_count_label->hide();
    ui->search_results_list->hide();
    ui->hide_result_button->hide();
    ui->search_layout->removeItem(ui->results_layout);
    ui->results_splitter->restoreState(hidden_splitter_state);
}

void SoftBibleStudy::on_search_results_list_currentRowChanged(int currentRow)
{
    if (currentRow >=0)
    {
        QStringList all_books = bible1.getBooks();

        if(ui->listBook->count() != all_books.count())
        {
            ui->listBook->clear();
            ui->listBook->addItems(all_books);
        }

        int row = all_books.indexOf(search_results.at(currentRow).book);
        ui->listBook->setCurrentRow(row);

        ui->chapter_ef->setText(search_results.at(currentRow).chapter);
    }
}

void SoftBibleStudy::on_search_results_list_doubleClicked(QModelIndex index)
{
    on_search_results_list_currentRowChanged(index.row());

}

QByteArray SoftBibleStudy::getHiddenSplitterState()
{
    if(ui->hide_result_button->isHidden())
        hidden_splitter_state = ui->results_splitter->saveState();
    return hidden_splitter_state;
}

QByteArray SoftBibleStudy::getShownSplitterState()
{
    if(!ui->hide_result_button->isHidden())
        shown_splitter_state = ui->results_splitter->saveState();
    return shown_splitter_state;
}

void SoftBibleStudy::setHiddenSplitterState(QByteArray& state)
{
    hidden_splitter_state = state;
    ui->results_splitter->restoreState(hidden_splitter_state);
}

void SoftBibleStudy::setShownSplitterState(QByteArray& state)
{
    shown_splitter_state = state;
}

BibleHistory SoftBibleStudy::getCurrentVerse()
{
    BibleHistory b;
    QString selected_ids;

    QString book = ui->listBook->currentItem()->text();
    QString chapter = ui->chapter_ef->text();

    int first_selected(-1),last_selected(-1);
    for(int i(0);i<ui->chapter_preview_list->count();++i)
    {
        if(ui->chapter_preview_list->item(i)->isSelected())
        {
            if(first_selected == -1)
                first_selected = i;
            last_selected = i;
            selected_ids += bible1.previewIdList.at(i) + ",";
        }
    }
    selected_ids.chop(1);

    QString verse_text = ui->chapter_preview_list->item(first_selected)->text().trimmed();
    b.verseIds = selected_ids;

    if(first_selected==last_selected)
    {
        b.caption = book + " " + chapter + ":" + QString::number(first_selected+1);
        b.captionLong = book + " " + chapter + ":" + verse_text;
    }
    else
    {   // Create multi verse caption for display
        int f(first_selected+1), l(last_selected+1),j(0);
        QString v=verse_text,p=".";
        while(v.at(j)!=p.at(0))
            ++j;
        v = v.remove(0,j);

        b.caption = book + " " + chapter + ":" + QString::number(f) + "-" + QString::number(l);
        b.captionLong = book + " " + chapter + ":" + QString::number(f) + "-" + QString::number(l) + v + "...";
    }

    return b;
}

bool SoftBibleStudy::isVerseSelected()
{
    if(ui->chapter_preview_list->selectedItems().count() >= 1)
        return true;
    else
        return false;
}

void SoftBibleStudy::setBibleBookActive()
{
    ui->lineEditBook->setFocus();
    ui->lineEditBook->selectAll();
}

void SoftBibleStudy::setBibleSearchActive()
{
    ui->search_ef->setFocus();
    ui->search_ef->selectAll();
}


void SoftBibleStudy::on_comboBoxBibleOne_activated(int index)
{
    mySettings.bibleVersions.bibleOne = biblesInDatabase.at(index).bibleId;
    bible1.loadBible(mySettings.bibleVersions.bibleOne);
    loadBibles();
}

void SoftBibleStudy::on_comboBoxBibleTwo_activated(int index)
{
     mySettings.bibleVersions.bibleTwo = biblesInDatabase.at(index).bibleId;
     bible2.loadBible(mySettings.bibleVersions.bibleTwo);
}

void SoftBibleStudy::on_comboBoxBibleThree_activated(int index)
{
     mySettings.bibleVersions.bibleThree = biblesInDatabase.at(index).bibleId;
     bible3.loadBible(mySettings.bibleVersions.bibleThree);
}
