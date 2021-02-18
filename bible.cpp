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

#include "bible.hpp"

Bible::Bible()
{
}

void Bible::setBiblesId(const QString &idA, const QString &idB, const QString &idC)
{
    bibleIdA = idA;
    bibleIdB = idB;
    bibleIdC = idC;
    retrieveBooks();
}

QString Bible::getBibleName()
{
    return getBibleName(bibleIdA);
}

QString Bible::getBibleName(QString const &id)
{
    if(id.isEmpty()){
        return "";
    }
    QSqlQuery sq;
    sq.exec("SELECT bible_name FROM BibleVersions WHERE id = "+ id );
    sq.first();
    QString b = sq.value(0).toString().trimmed();
    return b;
}


void Bible::retrieveBooks()
{
    BibleBook book;
    QSqlQuery sq;
    books.clear();
    sq.exec("SELECT book_name, id, chapter_count FROM BibleBooks WHERE bible_id = "+ bibleIdA );
    while (sq.next())
    {
        book.book = sq.value(0).toString().trimmed();
        book.bookId = sq.value(1).toString();
        book.chapterCount = sq.value(2).toInt();
        books.append(book);
    }
}

QStringList Bible::getBooks()
{
    QStringList bookList;
    if( books.count() == 0 ){
        retrieveBooks();
    }

    foreach(const BibleBook &b, books){
        bookList.append(b.book);
    }

    return bookList;
}

QString Bible::getBookName(int id)
{
    QString book;
    foreach (const BibleBook bk, books)
    {
        if(bk.bookId.toInt() == id)
        {
            book = bk.book;
            break;
        }
    }
    return book;
}

void Bible::getVerseRef(QString vId, QString &book, int &chapter, int &verse)
{
    if(vId.contains(","))
    {
        vId = vId.split(",").first();
    }

    foreach(const BibleVerse &bv, bibleVerseListA)
    {
        if(bv.id == vId)
        {
            book = QString::number(bv.book);
            chapter = bv.chapter;
            verse = bv.number;
            break;
        }
    }

    foreach (const BibleBook bk, books)
    {
        if(bk.bookId == book)
        {
            book = bk.book;
            break;
        }
    }
}

int Bible::getVerseNumberLast(QString vId)
{
    int vernum(0);
    if(vId.contains(","))
    {
        vId = vId.split(",").last();
    }

    foreach(const BibleVerse &bv, bibleVerseListA)
    {
        if(bv.id == vId)
        {
            vernum = bv.number;
            break;
        }
    }
    return vernum;
}

int Bible::getCurrentBookRow(QString book)
{
    int chapters(0);
    for(int i(0); books.count()>i;++i)
    {
        if(books.at(i).book==book)
        {
            chapters = i;
            break;
        }
    }
    return chapters;
}

BibleVerse Bible::getCorrespondingVerse(const QString &verseId, QList<BibleVerse> &bvl)
{
    foreach (const BibleVerse &bv, bvl)
    {
        if(bv.id == verseId){
            return bv;
        }
    }

    return BibleVerse();
}

const QList<ChapterVerse> Bible::getChapter(QString bookName, int chapter)
{
    QList<ChapterVerse> verseList;
    QString id;
    ChapterVerse verseText;
    int verse(0), verse_old(0), verseOldB(0), verseOldC(0);
    bool chapterStarted = false;
    int book = books.at(getCurrentBookRow(bookName)).bookId.toInt();

    previewIdList.clear();
    foreach (const BibleVerse &bvA, bibleVerseListA)
    {
        BibleVerse bvB, bvC;
        if(bvA.book == book && bvA.chapter == chapter)
        {
            chapterStarted = true;
            bvB = getCorrespondingVerse(bvA.id, bibleVerseListB);
            bvC = getCorrespondingVerse(bvA.id, bibleVerseListC);
            verse  = bvA.number;
            if(verse==verse_old)
            {
                verseText.verseTextA = verseText.verseTextA.simplified() + " " + bvA.text.simplified();

                if (bvB.number == verseOldB){
                    verseText.verseTextB = verseText.verseTextB.simplified() + " " + bvB.text.simplified();
                } else {
                    verseText.verseTextB = QString("%1 (%2) %3").arg(verseText.verseTextB.simplified())
                            .arg(bvB.number).arg(bvB.text.simplified());
                }

                if (bvC.number == verseOldC){
                    verseText.verseTextC = verseText.verseTextC.simplified() + " " + bvC.text.simplified();
                } else {
                    verseText.verseTextC = QString("%1 (%2) %3").arg(verseText.verseTextC.simplified())
                            .arg(bvC.number).arg(bvC.text.simplified());
                }

                id += "," + bvA.id;
                verseList.removeLast();
                previewIdList.removeLast();
            }
            else
            {
                verseText.verseTextA = QString("(%1) %2").arg(bvA.number).arg(bvA.text);
                verseText.verseTextB = QString("(%1) %2").arg(bvB.number).arg(bvB.text);
                verseText.verseTextC = QString("(%1) %2").arg(bvC.number).arg(bvC.text);
                id = bvA.id;
            }
            verseList.append(verseText);
            previewIdList << id;
            verse_old = verse;
            verseOldB = bvB.number;
            verseOldC = bvC.number;

            continue;
        }
        else if (chapterStarted){
            break;
        }
    }

    return verseList;
}

void Bible::getVerseAndCaption(QString& verse, QString& caption, QString verId, QString& bibId, bool useAbbr)
{
    QString verse_old, verse_show, verse_n, verse_nold, verse_nfirst, chapter;
    QString book;
    QStringList ids;
    QSqlQuery sq;

    // clean old verses
    verse.clear();
    caption.clear();

    if (verId.contains(","))// Run if more than one database verse items exist or show muliple verses
    {
        ids = verId.split(",");
        verId.replace(",", "' OR verse_id = '");
        sq.exec("SELECT book,chapter,verse,verse_text FROM BibleVerse WHERE ( verse_id = '"
                + verId +"' ) AND bible_id = " + bibId);
        while (sq.next())
        {
            book = sq.value(0).toString();
            chapter = sq.value(1).toString();
            verse_n = sq.value(2).toString();
            verse = sq.value(3).toString().trimmed();

            // Set first verse number
            if (verse_nfirst.isEmpty())
                verse_nfirst = verse_n;

            // If second(nold) verse number is the same or it is empty, then create a regular sigle verse
            // Else create a single display of muliple verses
            if (verse_n==verse_nold)
            {
                // If current verse number is same as first verse number,
                // then remove verse number from verse text and caption
                // shows only current verse number
                if(verse_n == verse_nfirst)
                {
                    int j(0);
                    for(int i(0);i<verse_show.count(); ++i)
                    {
                        j = 1 + i;
                        QString p =verse_show.at(i);
                        if(p==(")"))
                            break;
                    }
                    verse_show = verse_show.remove(0,j);

                    caption = " " + chapter + ":" + verse_n;
                }
                else
                    // Else if current verse number does match first verse number,
                    // then show bigening and eding verse numbers in caption
                    caption = " " + chapter + ":" + verse_nfirst + "-" + verse_n;

                verse_show += " " + verse;
            }
            else
            {
                caption =" " + chapter + ":" + verse_nfirst + "-" + verse_n;
                verse = " (" + verse_n + ") " + verse;
                verse_show += verse;
                if(!verse_show.startsWith(" ("))
                    verse_show = " ("+ verse_nfirst + ") " + verse_show;
            }
            verse_old = verse;
            verse_nold = verse_n;
        }
        verse = verse_show.simplified();
    }
    else // Run as standard single verse item from database
    {
        sq.exec("SELECT book,chapter,verse,verse_text FROM BibleVerse WHERE verse_id = '"
                +verId+"' AND bible_id = " + bibId);

        sq.first();
        verse = sq.value(3).toString().trimmed();// Remove the empty line at the end using .trimmed()

        book = sq.value(0).toString();
        caption =" " + sq.value(1).toString() + ":" + sq.value(2).toString();
        sq.clear();
    }

    // Add book name to caption
    sq.exec("SELECT book_name FROM BibleBooks WHERE id = "
            + book + " AND bible_id = " + bibId);
    sq.first();
    caption = sq.value(0).toString() + caption;
    sq.clear();

    // Add bible abbreveation if to to use it
    if(useAbbr)
    {
        sq.exec("SELECT abbreviation FROM BibleVersions WHERE id = " + bibId);
        sq.first();
        QString abr = sq.value(0).toString().trimmed();
        if (!abr.isEmpty())
            caption = QString("%1 (%2)").arg(caption).arg(abr);
    }

    verse = verse.simplified();
    caption = caption.simplified();
}

QList<BibleSearch> Bible::searchBible(bool allWords, QRegExp searchExp)
{   ///////// Search entire Bible //////////

    QList<BibleSearch> return_results;

    QString sw = searchExp.pattern();
    sw.remove("\\b(");
    sw.remove(")\\b");

    foreach(const BibleVerse &bv,bibleVerseListA)
    {
        if(bv.text.contains(searchExp))
        {
            if(allWords)
            {
                QStringList stl = sw.split("|");
                bool hasAll = false;
                for (int j(0);j<stl.count();++j)
                {
                    hasAll = bv.text.contains(QRegExp("\\b"+stl.at(j)+"\\b",Qt::CaseInsensitive));
                    if(!hasAll)
                        break;
                }
                if(hasAll)
                    addSearchResult(bv,return_results);
            }
            else
                addSearchResult(bv,return_results);
        }

    }

    return return_results;
}

QList<BibleSearch> Bible::searchBible(bool allWords, QRegExp searchExp, int book)
{   ///////// Search in selected book //////////

    QList<BibleSearch> return_results;

    QString sw = searchExp.pattern();
    sw.remove("\\b(");
    sw.remove(")\\b");

    foreach(const BibleVerse &bv,bibleVerseListA)
    {
        if(bv.text.contains(searchExp) && bv.book == book)
        {
            if(allWords)
            {
                QStringList stl = sw.split("|");
                bool hasAll = false;
                for (int j(0);j<stl.count();++j)
                {
                    hasAll = bv.text.contains(QRegExp("\\b"+stl.at(j)+"\\b",Qt::CaseInsensitive));
                    if(!hasAll)
                        break;
                }
                if(hasAll)
                    addSearchResult(bv,return_results);
            }
            else
                addSearchResult(bv,return_results);
        }
    }

    return return_results;
}

QList<BibleSearch> Bible::searchBible(bool allWords, QRegExp searchExp, int book, int chapter)
{   ///////// Search in selected chapter //////////

    QList<BibleSearch> return_results;

    QString sw = searchExp.pattern();
    sw.remove("\\b(");
    sw.remove(")\\b");

    foreach(const BibleVerse &bv,bibleVerseListA)
    {
        if(bv.text.contains(searchExp) && bv.book == book && bv.chapter == chapter)
        {
            if(allWords)
            {
                QStringList stl = sw.split("|");
                bool hasAll = false;
                for (int j(0);j<stl.count();++j)
                {
                    hasAll = bv.text.contains(QRegExp("\\b"+stl.at(j)+"\\b",Qt::CaseInsensitive));
                    if(!hasAll)
                        break;
                }
                if(hasAll)
                    addSearchResult(bv,return_results);
            }
            else
                addSearchResult(bv,return_results);
        }
    }

    return return_results;
}

void Bible::addSearchResult(const BibleVerse &bv, QList<BibleSearch> &bsl)
{
    BibleSearch  results;
    foreach (const BibleBook &bk,books)
    {
        if(bk.bookId == QString::number(bv.book))
        {
            results.book = bk.book;
            break;
        }
    }
    results.chapter = QString::number(bv.chapter);
    results.verse = QString::number(bv.number);
    results.verse_text = QString("%1 %2:%3 %4").arg(results.book).arg(results.chapter).arg(results.verse).arg(bv.text);

    bsl.append(results);
}

QList<BibleVerse> Bible::loadBibleVersion(QString bibleId, QList<BibleVerse> &bl)
{
    bl.clear();
    BibleVerse bv;
    QSqlQuery sq;
    sq.exec("SELECT verse_id, book, chapter, verse, verse_text FROM BibleVerse WHERE bible_id = '"+bibleId+"'");
    while(sq.next())
    {
        bv.id = sq.value(0).toString().trimmed();
        bv.book = sq.value(1).toInt();
        bv.chapter = sq.value(2).toInt();
        bv.number = sq.value(3).toInt();
        bv.text = sq.value(4).toString().trimmed();

        bl.append(bv);

    }
    return bl;
}

void Bible::loadBible()
{

    qDebug()<<"Loading Bible Version id:"<<bibleIdA;
    loadBibleVersion(bibleIdA, bibleVerseListA);
    qDebug()<<"Loading Bible Version id:"<<bibleIdB;
    loadBibleVersion(bibleIdB, bibleVerseListB);
    qDebug()<<"Loading Bible Version id:"<<bibleIdC;
    loadBibleVersion(bibleIdC, bibleVerseListC);
    qDebug()<<"*** Completed loading bibles ****";
}

void Bible::loadBible(QString const &idA, QString const &idB, QString const &idC)
{
    setBiblesId(idA, idB, idC);
    loadBible();
}
