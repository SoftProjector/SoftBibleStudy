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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QtSql>
#include <QPixmap>
#include "spfunctions.hpp"

void saveIndividualSettings(QSqlQuery &sq, QString sId, int tId, QString name, const QVariant &value);
void updateIndividualSettings(QSqlQuery &sq, QString sId, int tId, QString name, const QVariant &value);

class BibleVersionSettings
{
public:
    BibleVersionSettings();
    QString bibleOne;
    QString bibleTwo;
    QString bibleThree;
    bool settingsChanged;
};

class SpSettings
{   // stores main window settings, none user modifiable
public:
    SpSettings();
    QByteArray spSplitter;
    QByteArray bibleHiddenSplitter;
    QByteArray bibleShowSplitter;
    bool isWindowMaximized;
    QString uiTranslation;
};

class Settings
{
public:
    Settings();
    SpSettings spMain;
    BibleVersionSettings bibleVersions;

    bool isSpClosing;

public slots:
    void loadSettings();
    void saveSettings();
    void saveNewSettings();

private slots:
    QByteArray textToByt(QString text);
};

#endif // SETTINGS_HPP
