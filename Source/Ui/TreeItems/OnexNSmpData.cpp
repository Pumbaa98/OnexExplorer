#include "OnexNSmpData.h"
#include "../Previews/MultiImagePreview.h"
#include "OnexNSmpFrame.h"

OnexNSmpData::OnexNSmpData(const QString &name, QByteArray content, NosZlibOpener *opener,
                           int id, int creationDate, bool compressed)
        : OnexTreeZlibItem(name, opener, content, id, creationDate, compressed) {
    if (content.isEmpty())
        this->content = QByteArray(0x0);
    if (id == -1)
        return;
    int amount = content.at(0);
    for (int i = 0; i < amount; i++) {
        int width = opener->getLittleEndianConverter()->fromShort(content.mid(1 + i * 12, 2));
        int height = opener->getLittleEndianConverter()->fromShort(content.mid(3 + i * 12, 2));
        int xOrigin = opener->getLittleEndianConverter()->fromShort(content.mid(5 + i * 12, 2));
        int yOrigin = opener->getLittleEndianConverter()->fromShort(content.mid(7 + i * 12, 2));
        int offset = opener->getLittleEndianConverter()->fromInt(content.mid(9 + i * 12, 4));
        QByteArray subContent = content.mid(offset, (width * 2 * height));
        addFrame(subContent, width, height, xOrigin, yOrigin);
    }
}

OnexNSmpData::OnexNSmpData(QJsonObject jo, NosZlibOpener *opener, const QString &directory) : OnexTreeZlibItem(jo["ID"].toString(), opener) {
    this->content = QByteArrayLiteral("\x00");
    setId(jo["ID"].toInt(), true);
    setCreationDate(jo["Date"].toString(), true);
    setCompressed(jo["isCompressed"].toBool(), true);

    QJsonArray contentArray = jo["content"].toArray();

    for (auto &&i : contentArray) {
        QJsonObject frameJo = i.toObject();
        this->addChild(new OnexNSmpFrame(name + "_" + QString::number(this->childCount()), frameJo, opener, directory));
    }
}

OnexNSmpData::~OnexNSmpData() = default;

QWidget *OnexNSmpData::getPreview() {
    if (!hasParent())
        return nullptr;
    auto *images = new QList<QImage>();
    for (int i = 0; i != this->childCount(); ++i) {
        auto *item = dynamic_cast<OnexNSmpFrame *>(this->child(i));
        images->append(item->getImage());
    }
    auto *imagePreview = new MultiImagePreview(images);
    connect(this, SIGNAL(replaceSignal(QList<QImage> * )), imagePreview, SLOT(onReplaced(QList<QImage> * )));
    return imagePreview;
}

QByteArray OnexNSmpData::getContent() {
    int amount = childCount();
    if (!hasParent() || amount <= 0)
        return content;
    QByteArray offsetArray;
    offsetArray.push_back((uint8_t) amount);
    int sizeOfOffsetArray = 1 + amount * 12;
    QByteArray contentArray;
    for (int i = 0; i < amount; i++) {
        int currentFileOffset = sizeOfOffsetArray + contentArray.size();
        auto *currentItem = dynamic_cast<OnexNSmpFrame *>(this->child(i));
        offsetArray.push_back(opener->getLittleEndianConverter()->toShort(currentItem->getWidth()));
        offsetArray.push_back(opener->getLittleEndianConverter()->toShort(currentItem->getHeight()));
        offsetArray.push_back(opener->getLittleEndianConverter()->toShort(currentItem->getXOrigin()));
        offsetArray.push_back(opener->getLittleEndianConverter()->toShort(currentItem->getYOrigin()));
        offsetArray.push_back(opener->getLittleEndianConverter()->toInt(currentFileOffset));
        contentArray.push_back(currentItem->getContent());
    }
    this->content = QByteArray();
    content.push_back(offsetArray);
    content.push_back(contentArray);
    return content;
}

void OnexNSmpData::setName(QString name) {
    OnexTreeZlibItem::setName(name);
    QList<QTreeWidgetItem *> childList = takeChildren();
    for (int i = 0; i < childList.size(); i++) {
        auto *item = static_cast<OnexNSmpFrame *>(childList.at(i));
        item->OnexTreeItem::setName(name + "_" + QString::number(i));
    }
    addChildren(childList);
}

int OnexNSmpData::afterReplace(QByteArray content) {
    auto *images = new QList<QImage>();
    for (int i = 0; i != this->childCount(); ++i) {
        auto *item = dynamic_cast<OnexNSmpFrame *>(this->child(i));
        images->append(item->getImage());
    }
    emit replaceSignal(images);
    emit replaceInfo(generateInfos());
    return 1;
}

OnexTreeItem *OnexNSmpData::addFrame(QByteArray content, short width, short height, short xOrigin, short yOrigin) {
    OnexTreeItem *frame = new OnexNSmpFrame(name + "_" + QString::number(this->childCount()), content, width, height, xOrigin, yOrigin,
                                            (NosZlibOpener *) opener, id, creationDate, compressed);
    this->addChild(frame);
    return frame;
}
