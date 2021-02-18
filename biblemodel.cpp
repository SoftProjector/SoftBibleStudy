#include "biblemodel.hpp"

BibleModel::BibleModel()
{

}

int BibleModel::rowCount(const QModelIndex &parent) const
{
    return chapter.count();
}

int BibleModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant BibleModel::data(const QModelIndex &index, int role) const
{
    if( index.isValid() && role == Qt::DisplayRole )
    {
        if (index.column() == 0 ) {
            return QVariant(chapter.at(index.row()).verseTextA);
        } else if (index.column() == 1 ) {
            return QVariant(chapter.at(index.row()).verseTextB);
        } else if (index.column() == 2 ) {
            return QVariant(chapter.at(index.row()).verseTextC);
        }
    }
    return QVariant();
}

void BibleModel::setChapters(const QList<ChapterVerse>  &ch)
{
    emit layoutAboutToBeChanged();
    chapter = ch;
    emit layoutChanged();
}
