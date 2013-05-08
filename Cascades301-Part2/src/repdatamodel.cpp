#include "repdatamodel.h"

#include <QtCore/QtConcurrentRun>
#include <bb/cascades/Image>
#include <bb/pim/contacts/ContactService>

#include "util.hpp"

using namespace bb::cascades;

RepDataModel::RepDataModel(QObject *parent) : DataModel(parent)
{
}

RepDataModel::~RepDataModel()
{
}

/*!
 * Append a VCard record to the model
 */
void RepDataModel::appendVCard(const QString &vcard)
{
    // Add the unparsed VCard record to our list
    vcardList_.append(vcard);

    // Notify the addition of a list item
    emit itemAdded(QVariantList() << vcardList_.size() - 1);
}

/*!
 * Clear the data model
 */
void RepDataModel::clear()
{
    // Clear our collections
    vcardList_.clear();
    dataMap_.clear();

    // Notify of the change
    emit itemsChanged();
}

int RepDataModel::childCount(const QVariantList &indexPath)
{
    // Element count is based on the size of the unparsed VCard list
    return indexPath.isEmpty() ? vcardList_.size() : 0;
}

bool RepDataModel::hasChildren(const QVariantList &indexPath)
{
    // Model only supports a single level of elements
    return indexPath.isEmpty();
}

QString RepDataModel::itemType(const QVariantList &indexPath)
{
    Q_UNUSED(indexPath)

    // All items are the same type
    return "item";
}

QVariant RepDataModel::data(const QVariantList &indexPath)
{
    // Validate the indexPath parameter, and then return
    // data for the requested index
    if(indexPath.size() == 1) {
        bool ok;
        int index = indexPath[0].toInt(&ok);
        if(ok && index >= 0 && index < vcardList_.size()) {
            return dataForIndex(index);
        }
    }
    return QVariant();
}

/*!
 * Returns data for the requested index. Assumes the index is valid.
 */
QVariantMap RepDataModel::dataForIndex(int index)
{
    if(dataMap_.contains(index)) {
        // If we have already seen this index, return data from the map
        return dataMap_.value(index);
    }
    else {
        // If this index is new, start a background task to parse its VCard
        // data into something we can display.

        // Create a future watcher to monitor the completion of our task
        QFutureWatcher<QVariantMap> *watcher = new QFutureWatcher<QVariantMap>(this);
        connect(watcher, SIGNAL(finished()), this, SLOT(onContactCardParsed()));

        // Run our parse function on a background thread
        QFuture<QVariantMap> future = QtConcurrent::run(this, &RepDataModel::parseContactCard, vcardList_[index]);

        // Set our watcher to monitor the above task, and also to know which
        // data index it is for
        watcher->setFuture(future);
        watcher->setProperty("dataIndex", index);

        // Create and save a dummy item, which will be visible to the user
        // until our real data has been processed
        QVariantMap map;
        map["loading"] = true;
        dataMap_[index] = map;
        return map;
    }
}

/*!
 * Parse a VCard record, and return a displayable map
 */
QVariantMap RepDataModel::parseContactCard(const QString &vcard)
{
    // The contact service provides a VCard parser
    bb::pim::contacts::ContactService contactService;

    // Convert the VCard into a Contact object
    bb::pim::contacts::Contact contact =
        contactService.contactFromVCard(vcard);
    QVariantMap map;
    if(contact.isValid()) {
        // Convert the Contact object into a displayable map
        map = util::contactToMap(contact, false);
    }
    else {
        qWarning() << "Invalid contact";
    }
    return map;
}

/*!
 * Called when our future watcher emits a signal indicating that
 * parseContactCard() returned.
 */
void RepDataModel::onContactCardParsed()
{
    // Get a pointer to the future watcher, and make sure it is deleted when
    // we are done
    QFutureWatcher<QVariantMap> *watcher = static_cast<QFutureWatcher<QVariantMap> *>(sender());
    watcher->deleteLater();

    // Get the result of parseContactCard()
    QVariantMap map = watcher->result();

    // Get the model index of the parsed data
    int index = watcher->property("dataIndex").toInt();

    // Create a Cascades Image object for the contact photo, as this is
    // the only operation that must be done on the main application thread
    QByteArray photoData = map.value("photoData").toByteArray();
    if(!photoData.isNull()) {
        bb::cascades::Image image(photoData);
        map["photo"] = QVariant(qMetaTypeId<bb::cascades::Image>(), &image);
    }
    else {
        bb::cascades::Image image(QUrl("asset:///images/rep-default-thumb.png"));
        map["photo"] = QVariant(qMetaTypeId<bb::cascades::Image>(), &image);
    }

    // Put the real parsed data into our data map, and notify that we have
    // updated its index
    dataMap_[index] = map;
    qDebug() << "Parsed data for index:" << index;
    emit itemUpdated(QVariantList() << index);
}
