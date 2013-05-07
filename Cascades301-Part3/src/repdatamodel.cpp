#include "repdatamodel.h"

#include <QtCore/QtConcurrentRun>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <bb/cascades/Image>
#include <bb/pim/contacts/ContactService>

#include "util.hpp"

using namespace bb::cascades;

#define USE_BACKGROUND_LOAD
#define USE_SPARSE_FETCH

RepDataModel::RepDataModel(QObject *parent) : DataModel(parent)
{
}

RepDataModel::~RepDataModel()
{
}

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

    map["fullRecordRequested"] = false;
    map["loading"] = true;

    dataList_.append(map);
    emit itemAdded(QVariantList() << dataList_.size() - 1);
}

void RepDataModel::clear()
{
    dataList_.clear();
    emit itemsChanged();
}

int RepDataModel::childCount(const QVariantList &indexPath)
{
    return indexPath.isEmpty() ? dataList_.size() : 0;
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
        if(ok && index >= 0 && index < dataList_.size()) {
            return dataForIndexSparse(index);
        }
    }
    return QVariant();
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

    map["fullRecordRequested"] = true;
    dataList_[index] = map;
    qDebug() << "Parsed data for index:" << index;
    emit itemUpdated(QVariantList() << index);
}

QVariantMap RepDataModel::dataForIndexSparse(int index)
{
    QVariantMap data = dataList_[index];
    bool fullRecordRequested = data["fullRecordRequested"].toBool();
    if(!fullRecordRequested) {
        data["fullRecordRequested"] = true;
        dataList_[index] = data;
        const QString dataId = data["id"].toString();
        qDebug() << "Fetching data for index" << index << "with ID" << dataId;

        QUrl url("http://larkspur.greenviolet.net:9000");
        url.setPath("/vcard");
        //url.addQueryItem("encodeimage", "1");
        url.addQueryItem("start", dataId);
        url.addQueryItem("limit", "1");

        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::Attribute(QNetworkRequest::User + 1), index);
        QNetworkReply *reply = accessManager_.get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    }

    return data;
}

void RepDataModel::onRequestFinished()
{
    QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(sender());
    QNetworkRequest networkRequest = networkReply->request();
    const int index = networkRequest.attribute(QNetworkRequest::Attribute(QNetworkRequest::User + 1)).toInt();
    const int statusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    networkReply->deleteLater();

    if(statusCode == 200 && networkReply->error() == QNetworkReply::NoError) {
        const QByteArray data = networkReply->readAll();
        QString stringData = QString::fromUtf8(data);

        qDebug() << "Got data for index:" << index;
        QFutureWatcher<QVariantMap> *watcher = new QFutureWatcher<QVariantMap>(this);
        connect(watcher, SIGNAL(finished()), this, SLOT(onContactCardParsed()));
        QFuture<QVariantMap> future = QtConcurrent::run(this, &RepDataModel::parseContactCard, stringData);
        watcher->setFuture(future);
        watcher->setProperty("dataIndex", index);
    }
}
