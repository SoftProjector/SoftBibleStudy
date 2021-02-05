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

#include "settings.hpp"

void saveIndividualSettings(QSqlQuery &sq, QString sId, int tId, QString name, const QVariant &value)
{
    sq.addBindValue(sId);
    if(tId!=-2)
        sq.addBindValue(tId);
    sq.addBindValue(name);
    sq.addBindValue(value);
    sq.exec();
    qDebug()<<"save:"<<name<<sId<<tId<<value;
}

void updateIndividualSettings(QSqlQuery &sq, QString sId, int tId, QString name, const QVariant &value)
{
    sq.addBindValue(value);
    sq.addBindValue(sId);
    if(tId!=-2)
    {
        sq.addBindValue(tId);
        qDebug()<<"TID";
    }
    sq.addBindValue(name);
    sq.exec();
    qDebug()<<"update:"<<name<<sId<<tId<<value;
}

SpSettings::SpSettings()
{
    // Apply main window defaults
    isWindowMaximized = false;
    uiTranslation = "en";
}

BibleVersionSettings::BibleVersionSettings()
{
    bibleOne = "none";
    bibleTwo = "none";
    bibleThree = "none";
    settingsChanged = false;
}

Settings::Settings()
{
    isSpClosing = false;
}

void Settings::loadSettings()
{
    QString t,n,v,s,sets; // type, name, value, userValues
    QStringList set,values;
    bool dataGenOk,dataSpOk,dataB1Ok,dataB2Ok, dataPixOk;
    dataGenOk = dataSpOk = dataB1Ok = dataB2Ok = dataPixOk = false;
    QSqlQuery sq;
    sq.exec(QString("SELECT type, sets FROM Settings "));
    while (sq.next())
    {
        t = sq.value(0).toString();
        sets = sq.value(1).toString();

        if(t == "general") // set general setting values
        {
            dataSpOk = true;
            values = sets.split("\n");
            for(int i(0);i<values.count();++i)
            {
                s = values.at(i);
                set = s.split("=");
                n = set.at(0).trimmed();
                v = set.at(1).trimmed();

                if(n=="spSplitter")
                    spMain.spSplitter = textToByt(v);
                else if(n=="bibleHiddenSplitter")
                    spMain.bibleHiddenSplitter = textToByt(v);
                else if(n=="bibleShowSplitter")
                    spMain.bibleShowSplitter = textToByt(v);
                else if(n=="uiTranslation")
                    spMain.uiTranslation = v;
                else if(n=="isWindowMaximized")
                    spMain.isWindowMaximized = (v=="true");
            }
        }
        else if(t == "bible")
        {
            dataB1Ok = true;
            values = sets.split("\n");
            for(int i(0);i<values.count();++i)
            {
                s = values.at(i);
                set = s.split("=");
                n = set.at(0).trimmed();
                v = set.at(1).trimmed();
                if(n == "bibleOne")
                    bibleVersions.bibleOne = v;
                else if(n == "bibleTwo")
                    bibleVersions.bibleTwo = v;
                else if (n == "bibleThree")
                    bibleVersions.bibleThree = v;
            }
        }
    }

    // if no data exist, then create
    if(!dataGenOk || !dataSpOk || !dataB1Ok || !dataB2Ok || !dataPixOk)
        saveNewSettings();
}

void Settings::saveSettings()
{
    QSqlQuery sq;
    QString gset,bset;//general,bible,song,annouce,spmain


    // **** prepare softProjector main settings
    gset += "spSplitter = " + spMain.spSplitter.toHex();
    gset += "\nbibleHiddenSplitter = " + spMain.bibleHiddenSplitter.toHex();
    gset += "\nbibleShowSplitter = " + spMain.bibleShowSplitter.toHex();
    gset += "\nuiTranslation = " + spMain.uiTranslation;
    if(spMain.isWindowMaximized)
        gset += "\nisWindowMaximized = true";
    else
        gset += "\nisWindowMaximized = false";

    // ***** prepare Bible Versions
    bset += "bibleOne = " + bibleVersions.bibleOne;
    bset += "\nbibleTwo = " + bibleVersions.bibleTwo;
    bset += "\nbibleThree = " + bibleVersions.bibleThree;

    sq.exec(QString("UPDATE Settings SET sets = '%1' WHERE type = 'general'").arg(gset));
    sq.exec(QString("UPDATE Settings SET sets = '%1' WHERE type = 'bible'").arg(bset));
}

void Settings::saveNewSettings()
{
    QSqlQuery sq;
    sq.exec("INSERT OR REPLACE INTO Settings (type, sets) VALUES ('general', 'n=v')");
    sq.exec("INSERT OR REPLACE INTO Settings (type, sets) VALUES ('bible', 'n=v')");

    saveSettings();
}

QByteArray Settings::textToByt(QString text)
{
    QByteArray b;
    b.insert(0,text);
    b = b.fromHex(b);
    return b;
}
