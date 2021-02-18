#ifndef BIBLEMODEL_H
#define BIBLEMODEL_H

#include "QAbstractTableModel"

#include "bible.hpp"

class BibleModel : public QAbstractTableModel
{
public:
    BibleModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void setChapters(const QList<ChapterVerse> &chapter);

private:
    QList<ChapterVerse> chapter;
};

#endif // BIBLEMODEL_H
