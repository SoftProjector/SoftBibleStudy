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

#ifndef BIBLE_HPP
#define BIBLE_HPP

#include <QtSql>
#include "settings.hpp"

class BibleVerse
{
public:
    // Hold Bible Verse Info and Text
    QString id;
    int book;
    int chapter;
    int number;
    QString text;
};

class ChapterVerse
{
public:
    QString verseTextA;
    QString verseTextB;
    QString verseTextC;
};

class BibleSearch
{
    // For holding search results
public:
    QString book;
    QString chapter;
    QString verse;
    QString verse_text;
    QString verse_id;
    QString display_text;
    int first_v;
    int last_v;
};

class BibleHistory
{
public:
    QString verseIds;
    QString caption;
    QString captionLong;
};

class BibleBook
{
    // For Holding Bible book infromation
public:
    QString book;
    QString bookId;
    int chapterCount;
};

class Bible
{
public:
    Bible();
    QStringList previewIdList; // Verses that are in the preview (chapter) list
    QStringList currentIdList; // Verses that are in the show list
    QList<BibleBook> books;
public slots:
    QList<BibleSearch> searchBible(bool begins, QRegExp searchExp);
    QList<BibleSearch> searchBible(bool allWords, QRegExp searchExp, int book);
    QList<BibleSearch> searchBible(bool allWords, QRegExp searchExp, int book, int chapter);
    QStringList getBooks();
    QString getBookName(int id);
    void getVerseRef(QString vId, QString &book, int &chapter, int &verse);
    int getVerseNumberLast(QString vId);
    BibleVerse getCorrespondingVerse(const QString &verseId, QList<BibleVerse> &bvl);
    const QList<ChapterVerse> getChapter(QString bookName, int chapter);
    void getVerseAndCaption(QString &verse, QString &caption, QString verId, QString &bibId, bool useAbbr);
    int getCurrentBookRow(QString book);
    void setBiblesId(QString const &idA, QString const &idB, QString const &idC);
    QString getBibleName();
    QString getBibleName(QString const &id);
    QList<BibleVerse> loadBibleVersion(QString bibleId, QList<BibleVerse> &bl);
    void loadBible();
    void loadBible(const QString &idA, const QString &idB, const QString &idC);
private:
    QString bibleIdA, bibleIdB, bibleIdC;
    QList<BibleVerse> bibleVerseListA, bibleVerseListB, bibleVerseListC;
    void retrieveBooks();
private slots:
    void addSearchResult(const BibleVerse &bv,QList<BibleSearch> &bsl);
};

#endif // BIBLE_HPP
