#ifndef INOSFILEOPENER_H
#define INOSFILEOPENER_H
#include <QFile>
#include <QtEndian>
#include "../Ui/OnexTreeItem.h"

class INosFileOpener
{
protected:
    int readNextInt(QFile& file);
public:
    virtual OnexTreeItem* decrypt(QFile& file) = 0;
};

#endif // INOSFILEOPENER_H
