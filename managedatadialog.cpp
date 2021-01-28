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

#include "managedatadialog.hpp"
#include "ui_managedatadialog.h"

Module::Module()
{

}

ManageDataDialog::ManageDataDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::ManageDataDialog)
{
    ui->setupUi(this);

    // Set tables
    bible_model = new BiblesModel;
    themeModel = new ThemeModel;

    // Set Bible Table
    load_bibles();
    ui->bibleTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->bibleTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->bibleTableView->verticalHeader()->hide();
    ui->bibleTableView->setColumnWidth(0, 395);


    //Set reload cariers to false
    reload_bible = false;

    //  Progress Dialog
    progressDia = new ModuleProgressDialog(this);

    // Temporary disable "Download & Import" until server will be figured out.
//    ui->pushButtonDownBible->setEnabled(false);
//    ui->pushButtonDownSong->setEnabled(false);
//    ui->pushButtonDownTheme->setEnabled(false);
}

ManageDataDialog::~ManageDataDialog()
{
    delete bible_model;
    delete themeModel;
    delete progressDia;
    delete ui;
}

void ManageDataDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ManageDataDialog::load_bibles()
{
    Database db;
    // Set Bible Table
    bible_list = db.getBibles();
    bible_model->setBible(bible_list);
    ui->bibleTableView->setModel(bible_model);
    updateBibleButtons();
}


void ManageDataDialog::updateBibleButtons()
{
    bool enable_edit;
    bool enable_export;
    bool enable_delete;

    if (ui->bibleTableView->hasFocus())
    {
        enable_edit = true;
        enable_export = true;
        if (bible_model->rowCount()>=2)
            enable_delete = true;
        else
            enable_delete = false;
    }
    else
    {
        enable_edit = false;
        enable_export = false;
        enable_delete = false;
    }

    ui->edit_bible_pushButton->setEnabled(enable_edit);
    ui->export_bible_pushButton->setEnabled(enable_export);
    ui->delete_bible_pushButton->setEnabled(enable_delete);
}

void ManageDataDialog::on_ok_pushButton_clicked()
{
    close();
}

void ManageDataDialog::on_import_bible_pushButton_clicked()
{
    QString file_path = QFileDialog::getOpenFileName(this,
                                                     tr("Select Bible file to import"),
                                                     ".",tr("SoftProjector Bible file ") + "(*.spb)");

    importType = "local";
    // if file_path exits or "Open" is clicked, then import Bible
    if( !file_path.isNull() )
        importBible(file_path);
}

void ManageDataDialog::importBible(QString path)
{
    setWaitCursor();
    QFile file;
    file.setFileName(path);
    QString version;
    QString line , title, abbr, info, rtol, id;
    QStringList split;
    QSqlQuery sq;
    int row(0);

    if (file.open(QIODevice::ReadOnly))
    {
        line = QString::fromUtf8(file.readLine()); // read version
        // check file format and version
        if (!line.startsWith("##spData")) // Old bible format verison
        {
            QString errorm = tr("The Bible format you are importing is of an usupported file version.\n"
                                "Your current SoftProjector version does not support this format.");
            if(importType == "down")
                progressDia->appendText(errorm);
            else
            {
                QMessageBox mb(this);
                mb.setWindowTitle(tr("Unsupported Bible file format"));
                mb.setText(errorm);
                mb.setIcon(QMessageBox::Critical);
                mb.exec();
            }
            file.close();
            return;
        }
        else if (line.startsWith("##spData")) //New bible format version
        {
            split = line.split("\t");
            version = split.at(1);
            if (version.trimmed() != "1") // Version 1
            {
                QString errorm = tr("The Bible format you are importing is of an new version.\n"
                                    "Your current SoftProjector does not support this format.\n"
                                    "Please upgrade SoftProjector to latest version.");
                if(importType == "down")
                    progressDia->appendText(errorm);
                else
                {
                    QMessageBox mb(this);
                    mb.setWindowTitle(tr("New Bible file format"));
                    mb.setText(errorm);
                    mb.setIcon(QMessageBox::Critical);
                    mb.exec();
                }
                file.close();
                return;
            }
        }
        else
        {
            file.close();
            return;
        }

        line = QString::fromUtf8(file.readLine()); // read title
        split = line.split("\t");
        title = split.at(1);
        line = QString::fromUtf8(file.readLine()); // read abbreviation
        split = line.split("\t");
        abbr = split.at(1);
        line = QString::fromUtf8(file.readLine()); // read info
        split = line.split("\t");
        info = split.at(1);
        // Convert bible information from single line to multiple line
        QStringList info_list = info.split("@%");
        info.clear();
        for(int i(0); i<info_list.size();++i)
            info += info_list[i] + "\n";

        line = QString::fromUtf8(file.readLine()); // read right to left
        split = line.split("\t");
        rtol = split.at(1);

        QProgressDialog progress(tr("Importing..."), tr("Cancel"), 0, 31200, this);
        if(importType == "down")
        {
            progress.close();
            progressDia->setCurrentMax(31200);
            progressDia->setCurrentValue(row);
        }
        else
            progress.setValue(row);

        // add Bible Name and information
        QSqlDatabase::database().transaction();
        sq.prepare("INSERT INTO BibleVersions (bible_name, abbreviation, information, right_to_left) VALUES (?,?,?,?)");
        sq.addBindValue(title.trimmed());
        sq.addBindValue(abbr.trimmed());
        sq.addBindValue(info.trimmed());
        sq.addBindValue(rtol.trimmed());
        sq.exec();
        QSqlDatabase::database().commit();
        sq.clear();

        // get Bible id of newly added Bible
        sq.exec("SELECT seq FROM sqlite_sequence WHERE name = 'BibleVersions'");
        sq.first() ;
        id = sq.value(0).toString();
        sq.clear();

        // If this bible is the first bible, reload bibles
        if (id.trimmed() == "1")
            reload_bible = true;

        // add Bible book names
        line = QString::fromUtf8(file.readLine());
        QSqlDatabase::database().transaction();
        sq.prepare("INSERT INTO BibleBooks (bible_id, id, book_name, chapter_count) VALUES (?,?,?,?)");
        while (!line.startsWith("---"))
        {
            QString bk_id, bk_name, ch_count;
            split = line.split("\t");
            bk_id = split.at(0);
            bk_name = split.at(1);
            ch_count = split.at(2);

            sq.addBindValue(id);
            sq.addBindValue(bk_id.trimmed());
            sq.addBindValue(bk_name.trimmed());
            sq.addBindValue(ch_count.trimmed());
            sq.exec();

            line = QString::fromUtf8(file.readLine());
            ++row;
            if(importType == "down")
                progressDia->setCurrentValue(row);
            else
                progress.setValue(row);
        }
        QSqlDatabase::database().commit();
        sq.clear();

        // add bible verses
        QSqlDatabase::database().transaction();
        sq.prepare("INSERT INTO BibleVerse (verse_id, bible_id, book, chapter, verse, verse_text)"
                   "VALUES (?,?,?,?,?,?)");
        while (!file.atEnd())
        {
            if (progress.wasCanceled() && importType == "local")
                break;

            line = QString::fromUtf8(file.readLine());
            split = line.split("\t");
            sq.addBindValue(split.at(0));
            sq.addBindValue(id);
            sq.addBindValue(split.at(1));
            sq.addBindValue(split.at(2));
            sq.addBindValue(split.at(3));
            sq.addBindValue(split.at(4));
            sq.exec();

            ++row;
            if(importType == "down")
                progressDia->setCurrentValue(row);
            else
                progress.setValue(row);
        }
        QSqlDatabase::database().commit();

        file.close();
    }

    if(importType == "local")
        load_bibles();
    setArrowCursor();
    importModules();
}

void ManageDataDialog::on_export_bible_pushButton_clicked()
{
    int row = ui->bibleTableView->currentIndex().row();
    Bibles bible = bible_model->getBible(row);

    QString file_path = QFileDialog::getSaveFileName(this,tr("Save exported Bible as:"),
                                                     clean(bible.title),
                                                     tr("SoftProjector Bible file ") + "(*.spb)");
    if(!file_path.isEmpty())
    {
        if(!file_path.endsWith(".spb"))
            file_path = file_path + ".spb";
        exportBible(file_path,bible);
    }
}

void ManageDataDialog::exportBible(QString path, Bibles bible)
{
    setWaitCursor();
    QProgressDialog progress(tr("Exporting..."), tr("Cancel"), 0, 31000, this);
    int p(1);
    progress.setValue(p);

    QSqlQuery sq;
    QString id = bible.bibleId;
    QString to_file = "##spDataVersion:\t1\n"; // SoftProjector bible file version number is 1 as of 2/26/2011

    QString title, abbr, info, rtol;

    // get Bible version information
    sq.exec("SELECT bible_name, abbreviation, information, right_to_left FROM BibleVersions WHERE id = " + id );
    sq.first();
    title = sq.value(0).toString().trimmed();
    abbr = sq.value(1).toString().trimmed();
    info = sq.value(2).toString().trimmed();
    rtol = sq.value(3).toString().trimmed();
    sq.clear();

    // Convert bible information from multiline to single line
    QStringList info_list = info.split("\n");
    info = info_list[0];
    qDebug()<< QString::number(info_list.size());
    for(int i(1); i<info_list.size();++i)
        info += "@%" + info_list[i];

    to_file += "##Title:\t" + title + "\n" +
            "##Abbreviation:\t" + abbr + "\n" +
            "##Information:\t" + info.trimmed() + "\n" +
            "##RightToLeft:\t" + rtol + "\n";


    // get Bible books information
    sq.exec("SELECT id, book_name, chapter_count FROM BibleBooks WHERE bible_id = " + id );
    while (sq.next())
    {
        ++p;
        progress.setValue(p);

        to_file += sq.value(0).toString().trimmed() + "\t" +    //book id
                sq.value(1).toString().trimmed() + "\t" +    //book name
                sq.value(2).toString().trimmed() + "\n";     //chapter count
    }

    // get Bible verses
    to_file += "-----";
    sq.exec("SELECT verse_id, book, chapter, verse, verse_text FROM BibleVerse WHERE bible_id = " + id );
    while (sq.next())
    {
        ++p;
        progress.setValue(p);

        to_file += "\n" + sq.value(0).toString().trimmed() + "\t" + //verse id
                sq.value(1).toString().trimmed() + "\t" +        //book
                sq.value(2).toString().trimmed() + "\t" +        //chapter
                sq.value(3).toString().trimmed() + "\t" +        //verse
                sq.value(4).toString().trimmed();                //verse text
    }

    QFile ofile;
    ofile.setFileName(path);
    if (ofile.open(QIODevice::WriteOnly))
    {
        QTextStream out(&ofile);
        out.setCodec("UTF8");
        out << to_file;
    }
    ofile.close();
    setArrowCursor();

    QMessageBox mb(this);
    mb.setWindowTitle(tr("Bible has been exported"));
    mb.setText(tr("Bible:\n     ") + bible.title + tr("\nHas been saved to:\n     " )+ path);
    mb.setIcon(QMessageBox::Information);
    mb.exec();
}

void ManageDataDialog::on_delete_bible_pushButton_clicked()
{
    int row = ui->bibleTableView->currentIndex().row();
    Bibles bible = bible_model->getBible(row);
    QString name = bible.title;

    QMessageBox ms(this);
    ms.setWindowTitle(tr("Delete Bible?"));
    ms.setText(tr("Are you sure that you want to delete: ")+ name);
    ms.setInformativeText(tr("This action will permanently delete this Bible"));
    ms.setIcon(QMessageBox::Question);
    ms.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    ms.setDefaultButton(QMessageBox::Yes);
    int ret = ms.exec();

    switch (ret) {
    case QMessageBox::Yes:
        // Delete a Bible
        deleteBible(bible);
        break;
    case QMessageBox::No:
        // Cancel was clicked
        break;
    default:
        // should never be reached
        break;
    }
}

void ManageDataDialog::deleteBible(Bibles bible)
{
    setWaitCursor();
    reload_bible = true;
    QSqlQuery sq;
    QString id = bible.bibleId.trimmed();

    // Delete from BibleBooks Table
    sq.exec("DELETE FROM BibleBooks WHERE bible_id = '" + id + "'");
    sq.clear();

    // Delete form BibleVerse Table
    sq.exec("DELETE FROM BibleVerse WHERE bible_id = '" + id +"'");
    sq.clear();

    // Delete from BibleVersions Table
    sq.clear();
    sq.exec("DELETE FROM BibleVersions WHERE id = '" + id +"'");

    load_bibles();
    setArrowCursor();
}

void ManageDataDialog::on_edit_bible_pushButton_clicked()
{
    int row = ui->bibleTableView->currentIndex().row();
    Bibles bible = bible_model->getBible(row);
    QSqlTableModel sq;
    QSqlRecord sr;
    int r(0);

    BibleInformationDialog bible_info;
    bible_info.setBibleIformation(bible.title,bible.abbr,bible.info,bible.isRtoL);

    int ret = bible_info.exec();
    switch(ret)
    {
    case BibleInformationDialog::Accepted:
        sq.setTable("BibleVersions");
        sq.setFilter("id = " + bible.bibleId.trimmed());
        sq.select();
        sr = sq.record(0);
        sr.setValue(1,bible_info.title.trimmed());
        sr.setValue(2,bible_info.abbr.trimmed());
        sr.setValue(3,bible_info.info.trimmed());
        if(!bible_info.isRtoL)
            r = 0;
        else if (bible_info.isRtoL)
            r = 1;
        sr.setValue(4,r);
        sq.setRecord(0,sr);
        sq.submitAll();
        break;
    case BibleInformationDialog::Rejected:
        break;
    }

    load_bibles();
}

void ManageDataDialog::on_bibleTableView_clicked(QModelIndex index)
{
    updateBibleButtons();
}

void ManageDataDialog::setArrowCursor()
{
    this->setCursor(Qt::ArrowCursor);
    emit setMainArrowCursor();
}

void ManageDataDialog::setWaitCursor()
{
    this->setCursor(Qt::WaitCursor);
    emit setMainWaitCursor();
}

QString ManageDataDialog::getVerseId(QString book, QString chapter, QString verse)
{
    QString id;
    id = "B" + get3(book.toInt()) + "C" + get3(chapter.toInt()) + "V" + get3(verse.toInt());
    return id;
}

QString ManageDataDialog::get3(int i)
{
    QString st;
    if (i>=100)
    {
        st = st.number(i);
    }
    else if (i>=10)
    {
        st = "0" + st.number(i);
    }
    else
    {
        st = "00" + st.number(i);
    }
    return st;
}

void ManageDataDialog::toMultiLine(QString &mline)
{
    QStringList line_list = mline.split("@%");
    mline.clear();
    for(int i(0); i<line_list.size();++i)
        mline += line_list[i] + "\n";

    mline = mline.trimmed();
}

void ManageDataDialog::toSingleLine(QString &sline)
{
    QStringList line_list = sline.split("\n");
    sline = line_list[0];
    for(int i(1); i<line_list.size();++i)
        sline += "@%" + line_list[i];

    sline = sline.trimmed();
}



void ManageDataDialog::on_pushButtonDownBible_clicked()
{
    downType = "bible";
    importType = "down";
    downloadModList(QUrl("http://softprojector.org/bibles/bible.xml"));
}

void ManageDataDialog::downloadModList(QUrl url)
{
    QString filename = getSaveFileName(url);
    outFile.setFileName(filename);
    if(!outFile.open(QIODevice::WriteOnly))
    {
        //Fixme: need error message
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Error opening module list."));
        mb.setIcon(QMessageBox::Critical);
        mb.setText(tr("Failed to open mod list"));
        mb.exec();
        currentDownload->deleteLater();
        return;
    }

    QNetworkRequest request(url);
    currentDownload = downManager.get(request);
    connect(currentDownload,SIGNAL(finished()),this,SLOT(downloadModListCompleted()));
    connect(currentDownload,SIGNAL(readyRead()),this,SLOT(saveModFile()));
}

void ManageDataDialog::downloadNextMod()
{
    if(downQueue.empty())
    {
        progressDia->setSpeed("");
        importModules();
        return;
    }
    Module mod;
    mod = downQueue.dequeue();
    progressDia->appendText(tr("\nDownloading: %1\nFrom: %2").arg(mod.name).arg(mod.link.toString()));
    progressDia->setCurrentMax(mod.size);
    progressDia->setCurrentValue(0);
    QString filename = getSaveFileName(mod.link);
    outFile.setFileName(filename);
    if(!outFile.open(QIODevice::WriteOnly))
    {
        progressDia->appendText(tr("Error opening save file %1 for download %2\nError: %3")
                                .arg(filename).arg(mod.name).arg(outFile.errorString()));
        downloadNextMod();
        return;
    }

    QNetworkRequest request(mod.link);
    currentDownload = downManager.get(request);
    connect(currentDownload,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(dowloadProgress(qint64,qint64)));
    connect(currentDownload,SIGNAL(finished()),this,SLOT(downloadCompleted()));
    connect(currentDownload,SIGNAL(readyRead()),this,SLOT(saveModFile()));
    downTime.start();
}

QString ManageDataDialog::getSaveFileName(QUrl url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if (basename.isEmpty())
        basename = "download";

    QDir dir(dataDir);
    if(downType == "bible")
    {
        if(!dir.cd("BibleModules"))
        {
            if(dir.mkdir("BibleModules"))
                dir.cd("BibleModules");
        }
        path = dir.absolutePath() + dir.separator() + basename;
    }
    else if(downType == "song")
    {
        if(!dir.cd("SongModules"))
        {
            if(dir.mkdir("SongModules"))
                dir.cd("SongModules");
        }
        path = dir.absolutePath() + dir.separator() + basename;
    }
    else if(downType == "theme")
    {
        if(!dir.cd("ThemeModules"))
        {
            if(dir.mkdir("ThemeModules"))
                dir.cd("ThemeModules");
        }
        path = dir.absolutePath() + dir.separator() + basename;
    }
    else
        path = dir.absolutePath() + dir.separator() + basename;

    if (QFile::exists(path))
    {
        // File already exist, overwrite it
        if(!QFile::remove(path))
        {
            QRegExp rx("(.sps|.spb|.spt|.xml)");
            rx.indexIn(basename);
            basename = basename.remove(rx);

            int i(1);
            while(QFile::exists(QString("%1%2%3_%4%5").arg(dir.absolutePath()).arg(dir.separator())
                                .arg(basename).arg(i).arg(rx.cap(1))))
                ++i;

            path = QString("%1%2%3_%4%5").arg(dir.absolutePath()).arg(dir.separator())
                    .arg(basename).arg(i).arg(rx.cap(1));
        }
    }

    return path;
}

void ManageDataDialog::saveModFile()
{
    outFile.write(currentDownload->readAll());
}

void ManageDataDialog::downloadModListCompleted()
{
    outFile.close();

    if(currentDownload->error())
    {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Error downloading module list."));
        mb.setIcon(QMessageBox::Critical);
        mb.setText(currentDownload->errorString());
        mb.exec();
        currentDownload->deleteLater();
        return;
    }

    currentDownload->deleteLater();

    QStringList modlist;
    modlist = getModList(outFile.fileName());
    if(modlist.count()<=0)
        return;

    ModuleDownloadDialog modDia(this);
    if(downType == "bible")
        modDia.setWindowTitle(tr("Bible Module Download"));
    else if(downType == "song")
        modDia.setWindowTitle(tr("Songbook Module Download"));
    else if(downType == "theme")
        modDia.setWindowTitle(tr("Theme Module Download"));

    modDia.setList(modlist);
    int ret = modDia.exec();
    QList<int> mods;
    switch (ret)
    {
    case ModuleDownloadDialog::Accepted:
        mods = modDia.getSelected();
        if(mods.count()<=0)
            break;
        foreach(const int &modrow, mods)
            downQueue.enqueue(moduleList.at(modrow));
        progressDia->clearAll();
        progressDia->setTotalMax((mods.count()*2) +1);
        progressDia->show();
        downloadNextMod();
        break;
    case ModuleDownloadDialog::Rejected:
        break;
    }
}

void ManageDataDialog::downloadCompleted()
{
    outFile.close();

    if(currentDownload->error())
        progressDia->appendText(tr("Download Error: %1").arg(currentDownload->errorString()));
    else
    {
        modQueue.enqueue(outFile.fileName());
        progressDia->appendText(tr("Saved to: %1").arg(outFile.fileName()));
        progressDia->increaseTotal();
    }
    currentDownload->deleteLater();
    downloadNextMod();
}

void ManageDataDialog::dowloadProgress(qint64 recBytes, qint64 totBytes)
{
    progressDia->setCurrentValue(recBytes);

    // calculate the download speed
    double speed = recBytes * 1000.0 / downTime.elapsed();
    QString unit;
    if (speed < 1024)
        unit = "bytes/sec";
    else if (speed < 1024*1024)
    {
        speed /= 1024;
        unit = "kB/s";
    }
    else
    {
        speed /= 1024*1024;
        unit = "MB/s";
    }

    progressDia->setSpeed(QString("%1 %2").arg(speed, 3, 'f', 1).arg(unit));
}

QStringList ManageDataDialog::getModList(QString filepath)
{
    moduleList.clear();
    QStringList modList;
    QString name,link;
    int size(0);
    QFile file(filepath);
    Module mod;
    if(file.open(QIODevice::ReadOnly))
    {
        QXmlStreamReader xml(&file);
        while(!xml.atEnd())
        {
            xml.readNext();
            if(xml.StartElement && xml.name() == "Modules")
            {
                xml.readNext();
                while(xml.tokenString() != "EndElement" && xml.name() != "Modules")
                {
                    xml.readNext();
                    if(xml.StartElement && xml.name() == "Module")
                    {
                        xml.readNext();
                        while(xml.tokenString() != "EndElement")
                        {
                            //                            qDebug()<<"loop2";
                            xml.readNext();
                            if(xml.StartElement && xml.name() == "name")
                            {
                                name = xml.readElementText();
                                xml.readNext();
                            }
                            else if(xml.StartElement && xml.name() == "link")
                            {
                                link = xml.readElementText();
                                xml.readNext();
                            }
                            else if(xml.StartElement && xml.name() == "size")
                            {
                                size = xml.readElementText().toInt();
                                xml.readNext();
                            }
                        }

                        mod.name = name;
                        mod.link = QUrl(link);
                        mod.size = size;
                        moduleList.append(mod);
                        modList.append(name);
                        xml.readNext();
                    }
                }
                xml.readNext();
            }
        }
    }
    return modList;
}

void ManageDataDialog::importNextModule()
{
    if(modQueue.isEmpty())
    {
        progressDia->enableCloseButton(true);
        progressDia->setToMax();
        if(downType == "bible")
            load_bibles();


        return;
    }

    QString filePath = modQueue.dequeue();
    progressDia->appendText(tr("\nImporting: %1").arg(filePath));
    if(downType == "bible")
        importBible(filePath);

}

void ManageDataDialog::importModules()
{
    if(importType == "down")
    {
        progressDia->increaseTotal();
        importNextModule();
    }
}
