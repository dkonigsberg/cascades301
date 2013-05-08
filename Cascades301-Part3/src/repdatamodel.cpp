#include "repdatamodel.h"

#include <QtCore/QtConcurrentRun>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
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
 * Append a sparse element to the data model.
 * This element contains just enough information to show the user a basic
 * summary of the record, to be filterable/sortable if desired, and to
 * form a request for the complete record.
 */
void RepDataModel::appendSparseElement(const QString &id, const QString &name,
    const QString &state, const QString &district,
    const QString &partyCode)
{
    QVariantMap map;
    map["id"] = id;
    map["displayName"] = name;
    map["role"] = QString("%1 District: %2").arg(state).arg(district);

    map["partyCode"] = partyCode;
    if(partyCode == QLatin1String("R")) {
        map["party"] = "Republican";
    }
    else if(partyCode == QLatin1String("D")) {
        map["party"] = "Democrat";
    }

    // Set some state properties
    map["fullRecordRequested"] = false;
    map["loading"] = true;

    dataList_.append(map);

    // Notify the addition of a list item
    emit itemAdded(QVariantList() << dataList_.size() - 1);
}

/*!
 * Clear the data model
 */
void RepDataModel::clear()
{
    // Clear our collections
    dataList_.clear();

    // Notify of the change
    emit itemsChanged();
}

int RepDataModel::childCount(const QVariantList &indexPath)
{
    // Element count is based on the size of the unparsed VCard list
    return indexPath.isEmpty() ? dataList_.size() : 0;
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
        if(ok && index >= 0 && index < dataList_.size()) {
            return dataForIndexSparse(index);
        }
    }
    return QVariant();
}

/*!
 * Returns data for the requested index. Assumes the index is valid.
 */
QVariantMap RepDataModel::dataForIndexSparse(int index)
{
    // Gets the data we already have for this index, which will either be
    // a sparse or complete record.
    QVariantMap data = dataList_[index];

    // Check if we need to start a request for the full record
    bool fullRecordRequested = data["fullRecordRequested"].toBool();
    if(!fullRecordRequested) {
        // Remember that we started a request for the full record
        data["fullRecordRequested"] = true;
        dataList_[index] = data;

        // Get the ID for the full record
        const QString dataId = data["id"].toString();
        qDebug() << "Fetching data for index" << index << "with ID" << dataId;

        // Construct our query URL
        QUrl url("http://larkspur.greenviolet.net:9000");

        // Data returned in VCard format
        url.setPath("/vcard");

        //url.addQueryItem("encodeimage", "1");

        // Return data for the specific record ID
        url.addQueryItem("start", dataId);
        url.addQueryItem("limit", "1");

        // Create an HTTP request, add a custom attribute to track the requested index,
        // and connect to the completion signal
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::Attribute(QNetworkRequest::User + 1), index);
        QNetworkReply *reply = accessManager_.get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    }

    return data;
}

/*!
 * Called at the completion of our HTTP request
 */
void RepDataModel::onRequestFinished()
{
    // Get a pointer to our network reply, the model index for the requested
    // record, the HTTP status code returned by our server, and make sure we
    // clean up after ourselves
    QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(sender());
    QNetworkRequest networkRequest = networkReply->request();
    const int index = networkRequest.attribute(QNetworkRequest::Attribute(QNetworkRequest::User + 1)).toInt();
    const int statusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    networkReply->deleteLater();

    if(statusCode == 200 && networkReply->error() == QNetworkReply::NoError) {
        // Read the data successfully returned from our server
        const QByteArray data = networkReply->readAll();

        // Assume data is a UTF-8 encoded string
        QString stringData = QString::fromUtf8(data);

        qDebug() << "Got data for index:" << index;


        // Create a future watcher to monitor the completion of our task
        QFutureWatcher<QVariantMap> *watcher = new QFutureWatcher<QVariantMap>(this);
        connect(watcher, SIGNAL(finished()), this, SLOT(onContactCardParsed()));

        // Run our parse function on a background thread
        QFuture<QVariantMap> future = QtConcurrent::run(this, &RepDataModel::parseContactCard, stringData);

        // Set our watcher to monitor the above task, and also to know which
        // data index it is for
        watcher->setFuture(future);
        watcher->setProperty("dataIndex", index);
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

    // Since we are replacing the existing record, set this to make sure we
    // don't go into a request loop.
    map["fullRecordRequested"] = true;

    // Put the real parsed data into our data map, and notify that we have
    // updated its index
    dataList_[index] = map;
    qDebug() << "Parsed data for index:" << index;
    emit itemUpdated(QVariantList() << index);
}
