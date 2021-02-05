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

#ifndef MANAGEDATADIALOG_HPP
#define MANAGEDATADIALOG_HPP

#include <QDialog>
#include <QtWidgets>
#include <QtSql>
#include <QtNetwork/QtNetwork>

#include "managedata.hpp"
#include "bibleinformationdialog.hpp"
#include "moduledownloaddialog.hpp"
#include "moduleprogressdialog.hpp"

namespace Ui {
class ManageDataDialog;
}

class Module
{
public:
    Module();
    QString name;
    QUrl link;
    int size;
};

class ManageDataDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(ManageDataDialog)
public:
    explicit ManageDataDialog(QWidget *parent = 0);
    virtual ~ManageDataDialog();
    bool reload_bible;
    bool reload_songbook;
    bool reloadThemes;

public slots:

    void setDataDir(QDir &d){dataDir = d;}

protected:
    virtual void changeEvent(QEvent *e);

signals:
    void setMainWaitCursor();
    void setMainArrowCursor();

private:
    QList<Bibles> bible_list;
    BiblesModel *bible_model;
    QNetworkAccessManager downManager;
    QNetworkReply *modDownload;
    QNetworkReply *currentDownload;
    QQueue<Module> downQueue;
    QQueue<QString> modQueue;
    QString downType;
    QString importType;
    QDir dataDir;
    QFile outFile;
    QList<Module> moduleList;
    ModuleProgressDialog *progressDia;
    QTime downTime;
    Ui::ManageDataDialog *ui;

private slots:
    QString get3(int i);
    QString getVerseId(QString book, QString chapter, QString verse);
    void setWaitCursor();
    void setArrowCursor();
    void on_bibleTableView_clicked(QModelIndex index);
    void updateBibleButtons();

    void on_edit_bible_pushButton_clicked();
    void on_delete_bible_pushButton_clicked();
    void on_export_bible_pushButton_clicked();
    void on_import_bible_pushButton_clicked();
    void on_ok_pushButton_clicked();
    void deleteBible(Bibles bilbe);
    void importBible(QString path);
    void exportBible(QString path, Bibles bible);
    void load_bibles();
    void toMultiLine(QString& mline);
    void toSingleLine(QString& sline);


    void on_pushButtonDownBible_clicked();


    void downloadModList(QUrl url);
    void downloadNextMod();
    QString getSaveFileName(QUrl url);
    void saveModFile();
    void downloadModListCompleted();
    void downloadCompleted();
    void dowloadProgress(qint64 recBytes,qint64 totBytes);
    QStringList getModList(QString filepath);
    void importNextModule();
    void importModules();

};

#endif // MANAGEDATADIALOG_HPP
