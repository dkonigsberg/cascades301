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
    connect(nav_, SIGNAL(popTransitionEnded(bb::cascades::Page*)),
        this, SLOT(onPopTransitionEnded(bb::cascades::Page*)));

    rootPage_ = nav_->findChild<Page *>("repListPage");
    app->setScene(nav_);

    connect(rootPage_, SIGNAL(loadData()), this, SLOT(onLoadData()));
    connect(rootPage_, SIGNAL(showDetails(QVariant)), this, SLOT(onShowDetails(QVariant)));

    RepDataModel *dataModel = new RepDataModel(this);

    ListView *listView = rootPage_->findChild<ListView *>("listView");
    listView->setDataModel(dataModel);
    dataModel_ = dataModel;
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

    QUrl url("http://larkspur.greenviolet.net:9000");
    url.setPath("/json");
    url.addQueryItem("minimal", "1");

    QNetworkRequest request(url);
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
        bb::data::JsonDataAccess json;
        QVariantList dataList = json.loadFromBuffer(data).toList();
        qDebug() << "Retrieved" << dataList.size() << "sparse records";
        populateSparseDataModel(dataList);
    }
    else {
        qWarning() << "Network error:" << statusCode << networkReply->errorString();
    }
    rootPage_->setProperty("loadingData", false);
    rootPage_->setProperty("hasData", dataModel_->childCount(QVariantList()) > 0);
}

void RepFinder::populateSparseDataModel(const QVariantList &dataList)
{
    RepDataModel *repModel = qobject_cast<RepDataModel *>(dataModel_);
    repModel->clear();
    foreach(const QVariant &dataElement, dataList) {
        const QVariantMap map = dataElement.toMap();
        repModel->appendSparseElement(
            map["id"].toString(),
            map["name"].toString(),
            map["state"].toString(),
            map["district"].toString(),
            map["party"].toString());
    }
}

void RepFinder::onShowDetails(const QVariant &dataItem)
{
    qDebug() << "RepFinder::onShowDetails()";
    const QVariantMap map = dataItem.toMap();

    RepDetailsPage *detailsPage = new RepDetailsPage();
    detailsPage->setRepData(map);
    nav_->push(detailsPage->rootNode());
}
