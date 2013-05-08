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
#include <bb/data/JsonDataAccess>
#include <bb/pim/contacts/ContactService>
#include <bb/pim/contacts/Contact>

#include "util.hpp"
#include "repdatamodel.h"
#include "repdetailspage.hpp"

using namespace bb::cascades;

RepFinder::RepFinder(bb::cascades::Application *app)
    : QObject(app)
{
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);

    nav_ = qml->createRootObject<NavigationPane>();

    // Connect to this signal so we can clean up popped pages
    connect(nav_, SIGNAL(popTransitionEnded(bb::cascades::Page*)),
        this, SLOT(onPopTransitionEnded(bb::cascades::Page*)));

    // Get a reference to the main page of the app
    rootPage_ = nav_->findChild<Page *>("repListPage");

    // Set the main scene to the navigation pane
    app->setScene(nav_);

    // Connect to signals on our main page
    connect(rootPage_, SIGNAL(loadData()), this, SLOT(onLoadData()));
    connect(rootPage_, SIGNAL(showDetails(QVariant)), this, SLOT(onShowDetails(QVariant)));

    // Create the custom data model, and assign it to the list view
    RepDataModel *dataModel = new RepDataModel(this);

    ListView *listView = rootPage_->findChild<ListView *>("listView");
    listView->setDataModel(dataModel);
    dataModel_ = dataModel;
}

void RepFinder::onPopTransitionEnded(bb::cascades::Page *page)
{
    // Delete pages that have been popped
    if(page) {
        delete page;
    }
}

/*!
 * Called when the "Load Data" action is clicked, which is essentially the
 * start button of our application.
 */
void RepFinder::onLoadData()
{
    qDebug() << "RepFinder::onLoadData()";
    // Set a property that lets the page know we are loading
    rootPage_->setProperty("loadingData", true);

    // Construct our query URL
    // See supporting lab documentation for more information on the
    // various parameters accepted by our example server.

    QUrl url("http://larkspur.greenviolet.net:9000");

    // Data returned in VCard format
    url.setPath("/vcard");

    // If set, photos are returned as Base64 encoded content inside the VCard
    // It not set, the photos are referenced by URL inside the VCard
    url.addQueryItem("encodeimage", "1");

    // Limit the number of results; useful for testing
    url.addQueryItem("limit", "10");

    // Create an HTTP request, and connect to the completion signal
    QNetworkRequest request(url);
    QNetworkReply *reply = accessManager_.get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onRequestFinished()));
}

/*!
 * Called at the completion of our HTTP request
 */
void RepFinder::onRequestFinished()
{
    qDebug() << "RepFinder::onRequestFinished()";

    // Get a pointer to our network reply, the HTTP status code returned
    // by our server, and make sure we clean up after ourselves
    QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(sender());
    QNetworkRequest networkRequest = networkReply->request();
    int statusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    networkReply->deleteLater();

    if(statusCode == 200 && networkReply->error() == QNetworkReply::NoError) {
        // Read the data successfully returned from our server
        const QByteArray data = networkReply->readAll();

        // Convert the data into a collection of VCards
        QList<QString> vcards = vcardsFromData(data);

        qDebug() << "Retrieved" << vcards.size() << "VCard records";

        // Populate the data model from the VCards
        populateDataModel(vcards);
    }
    else {
        qWarning() << "Network error:" << statusCode << networkReply->errorString();
    }

    // Update our page properties
    rootPage_->setProperty("loadingData", false);
    rootPage_->setProperty("hasData", dataModel_->childCount(QVariantList()) > 0);
}

/*!
 * Read our result data, line by line, and build a list of individual
 * VCard records.
 */
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

/*!
 * Parse each VCard record, and populate the data model with the result.
 */
void RepFinder::populateDataModel(QList<QString> &vcards)
{
    RepDataModel *repModel = qobject_cast<RepDataModel *>(dataModel_);

    // Clear any old data
    repModel->clear();

    foreach(const QString &vcard, vcards) {
        // Add each record to our model, which will handle the parsing itself
        repModel->appendVCard(vcard);
    }
}

/*!
 * Called when the user taps on a list item.
 */
void RepFinder::onShowDetails(const QVariant &dataItem)
{
    qDebug() << "RepFinder::onShowDetails()";
    const QVariantMap map = dataItem.toMap();

    // Create and show a details page for the selected person
    RepDetailsPage *detailsPage = new RepDetailsPage();
    detailsPage->setRepData(map);
    nav_->push(detailsPage->rootNode());
}
