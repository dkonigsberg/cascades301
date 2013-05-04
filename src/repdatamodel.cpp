#include "repdatamodel.h"

#include <QtCore/QtConcurrentRun>
#include <bb/cascades/Image>
#include <bb/pim/contacts/ContactService>

#include "util.hpp"

using namespace bb::cascades;

//#define USE_FOREGROUND_LOAD
#define USE_BACKGROUND_LOAD

RepDataModel::RepDataModel(QObject *parent) : DataModel(parent)
{
}

RepDataModel::~RepDataModel()
{
}

void RepDataModel::appendVCard(const QString &vcard)
{
    vcardList_.append(vcard);
    emit itemAdded(QVariantList() << vcardList_.size() - 1);
}

void RepDataModel::clear()
{
    vcardList_.clear();
    dataMap_.clear();
    emit itemsChanged();
}

int RepDataModel::childCount(const QVariantList &indexPath)
{
    return indexPath.isEmpty() ? vcardList_.size() : 0;
}

bool RepDataModel::hasChildren(const QVariantList &indexPath)
{
    return indexPath.isEmpty();
}

QString RepDataModel::itemType(const QVariantList &indexPath)
{
    Q_UNUSED(indexPath)
    return "item";
}

QVariant RepDataModel::data(const QVariantList &indexPath)
{
    if(indexPath.size() == 1) {
        bool ok;
        int index = indexPath[0].toInt(&ok);
        if(ok && index >= 0 && index < vcardList_.size()) {
            return dataForIndex(index);
        }
    }
    return QVariant();
}

QVariantMap RepDataModel::dataForIndex(int index)
{
    if(dataMap_.contains(index)) {
        return dataMap_.value(index);
    }
    else {
#ifdef USE_FOREGROUND_LOAD
        QVariantMap map = parseContactCard(vcardList_[index]);
        dataMap_[index] = map;
        return map;
#endif
#ifdef USE_BACKGROUND_LOAD
        QFutureWatcher<QVariantMap> *watcher = new QFutureWatcher<QVariantMap>(this);
        connect(watcher, SIGNAL(finished()), this, SLOT(onContactCardParsed()));
        QFuture<QVariantMap> future = QtConcurrent::run(this, &RepDataModel::parseContactCard, vcardList_[index]);
        watcher->setFuture(future);
        watcher->setProperty("dataIndex", index);

        QVariantMap map;
        map["loading"] = true;
        dataMap_[index] = map;
        return map;
#endif
    }
}

QVariantMap RepDataModel::parseContactCard(const QString &vcard)
{
    bb::pim::contacts::ContactService contactService;
    bb::pim::contacts::Contact contact =
        contactService.contactFromVCard(vcard);
    QVariantMap map;
    if(contact.isValid()) {
        map = util::contactToMap(contact, false);
    }
    else {
        qWarning() << "Invalid contact";
    }
    return map;
}

void RepDataModel::onContactCardParsed()
{
    QFutureWatcher<QVariantMap> *watcher = static_cast<QFutureWatcher<QVariantMap> *>(sender());
    watcher->deleteLater();
    QVariantMap map = watcher->result();
    int index = watcher->property("dataIndex").toInt();

    QByteArray photoData = map.value("photoData").toByteArray();
    if(!photoData.isNull()) {
        bb::cascades::Image image(photoData);
        map["photo"] = QVariant(qMetaTypeId<bb::cascades::Image>(), &image);
    }
    else {
        bb::cascades::Image image(QUrl("asset:///images/rep-default-thumb.png"));
        map["photo"] = QVariant(qMetaTypeId<bb::cascades::Image>(), &image);
    }

    dataMap_[index] = map;
    emit itemUpdated(QVariantList() << index);
}
