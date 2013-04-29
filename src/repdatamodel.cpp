#include "repdatamodel.h"

#include "util.hpp"

using namespace bb::cascades;

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
        QVariantMap map = parseContactCard(vcardList_[index]);
        dataMap_[index] = map;
        return map;
    }
}

QVariantMap RepDataModel::parseContactCard(const QString &vcard)
{
    bb::pim::contacts::Contact contact =
        contactService_.contactFromVCard(vcard);
    QVariantMap map;
    if(contact.isValid()) {
        map = util::contactToMap(contact);
    }
    else {
        qWarning() << "Invalid contact";
    }
    return map;
}
