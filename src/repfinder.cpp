#include "repfinder.hpp"

#include <QtCore/QTextStream>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/NavigationPane>
#include <bb/cascades/Page>
#include <bb/cascades/ListView>
#include <bb/cascades/GroupDataModel>
#include <bb/cascades/Image>
#include <bb/pim/contacts/ContactService>
#include <bb/pim/contacts/Contact>

#include "repdetailspage.hpp"

using namespace bb::cascades;

RepFinder::RepFinder(bb::cascades::Application *app)
    : QObject(app)
{
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);

    nav_ = qml->createRootObject<NavigationPane>();
    connect(nav_, SIGNAL(popTransitionEnded(bb::cascades::Page*)),
        this, SLOT(onPopTransitionEnded(bb::cascades::Page*)));

    rootPage_ = nav_->findChild<Page *>("repListPage");
    app->setScene(nav_);

    connect(rootPage_, SIGNAL(loadData()), this, SLOT(onLoadData()));
    connect(rootPage_, SIGNAL(showDetails(QVariant)), this, SLOT(onShowDetails(QVariant)));

    dataModel_ = new GroupDataModel(this);
    dataModel_->setSortingKeys(QStringList() << "displayName");
    dataModel_->setSortedAscending(true);

    ListView *listView = rootPage_->findChild<ListView *>("listView");
    listView->setDataModel(dataModel_);
}

void RepFinder::onPopTransitionEnded(bb::cascades::Page *page)
{
    if(page) {
        delete page;
    }
}

void RepFinder::onLoadData()
{
    qDebug() << "RepFinder::onLoadData()";
    rootPage_->setProperty("loadingData", true);

    QNetworkRequest request(QUrl("http://larkspur.greenviolet.net:9000/vcard"));
    QNetworkReply *reply = accessManager_.get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onRequestFinished()));
}

void RepFinder::onRequestFinished()
{
    qDebug() << "RepFinder::onRequestFinished()";
    QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(sender());
    QNetworkRequest networkRequest = networkReply->request();
    int statusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    networkReply->deleteLater();

    if(statusCode == 200 && networkReply->error() == QNetworkReply::NoError) {
        const QByteArray data = networkReply->readAll();
        QList<QString> vcards = vcardsFromData(data);
        qDebug() << "Retrieved" << vcards.size() << "VCard records";

        populateDataModel(vcards);
    }
    else {
        qWarning() << "Network error:" << statusCode << networkReply->errorString();
    }
    rootPage_->setProperty("loadingData", false);
}

QList<QString> RepFinder::vcardsFromData(const QByteArray &data)
{
    QList<QString> results;

    QTextStream stream(data, QIODevice::ReadOnly);
    QString buffer;
    while(!stream.atEnd()) {
        QString line = stream.readLine();
        buffer.append(line).append("\r\n");
        if(line == QLatin1String("END:VCARD")) {
            results.append(buffer);
            buffer.clear();
        }
    }

    return results;
}

void RepFinder::populateDataModel(QList<QString> &vcards)
{
    dataModel_->clear();

    bb::pim::contacts::ContactService contactService;
    foreach(const QString &vcard, vcards) {
        bb::pim::contacts::Contact contact =
            contactService.contactFromVCard(vcard);
        if(contact.isValid()) {
            QVariantMap map = contactToMap(contact);
            dataModel_->insert(map);
        }
        else {
            qWarning() << "Invalid contact";
        }
    }
}

QVariantMap RepFinder::contactToMap(const bb::pim::contacts::Contact &contact)
{
    QVariantMap map;

    map["firstName"] = contact.firstName();
    map["lastName"] = contact.lastName();
    map["displayName"] = contact.displayName();
    map["party"] = contact.displayCompanyName();

    bb::pim::contacts::ContactPostalAddress address = contact.postalAddresses().first();
    if(address.isValid()) {
        QStringList addressLines;
        if(!address.line1().isEmpty()) {
            addressLines << address.line1();
        }
        if(!address.line2().isEmpty()) {
            addressLines << address.line2();
        }
        map["address"] = QString("%1\n%2, %3 %4")
            .arg(addressLines.join("\n"))
            .arg(address.city()).arg(address.region())
            .arg(address.postalCode());
    }

    foreach(const bb::pim::contacts::ContactAttribute &attribute, contact.attributes()) {
        //qDebug() << "-->" << attribute.kind() << attribute.subKind() << attribute.attributeDisplayLabel() << attribute.value();
        if(attribute.kind() == bb::pim::contacts::AttributeKind::OrganizationAffiliation) {
            map["role"] = attribute.value();
        }
        if(attribute.kind() == bb::pim::contacts::AttributeKind::Note) {
            const QString prefix = "Committees: ";
            const QString noteValue = attribute.value();
            if(noteValue.startsWith(prefix)) {
                map["committees"] = noteValue.mid(prefix.size());
            } else {
                map["notes"] = attribute.value();
            }
        }
        else if(attribute.kind() == bb::pim::contacts::AttributeKind::Website) {
            map["website"] = attribute.value();
        }
        if(attribute.kind() == bb::pim::contacts::AttributeKind::Phone) {
            map["phoneNumber"] = attribute.value();
        }
    }

    const QString photoPath = contact.smallPhotoFilepath();
    if(!photoPath.isNull()) {
        QFile photoFile(photoPath);
        photoFile.open(QIODevice::ReadOnly);
        const QByteArray photoData = photoFile.readAll();
        if(!photoData.isEmpty()) {
            Image image(photoData);
            map["photo"] = QVariant(qMetaTypeId<bb::cascades::Image>(), &image);
        }
    }

    return map;
}

void RepFinder::onShowDetails(const QVariant &dataItem)
{
    qDebug() << "RepFinder::onShowDetails()";
    const QVariantMap map = dataItem.toMap();

    RepDetailsPage *detailsPage = new RepDetailsPage();
    detailsPage->setRepData(map);
    nav_->push(detailsPage->rootNode());
}
