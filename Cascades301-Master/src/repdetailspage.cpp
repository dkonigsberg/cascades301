#include "repdetailspage.hpp"

#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QTemporaryFile>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/Page>
#include <bb/system/InvokeManager>
#include <bb/system/InvokeRequest>
#include <bb/system/InvokeTargetReply>

using namespace bb::cascades;

RepDetailsPage::RepDetailsPage(QObject *parent) : QObject(parent)
{
    qDebug() << "RepDetailsPage::RepDetailsPage()";
    QmlDocument *qml = QmlDocument::create("asset:///RepDetailsPage.qml").parent(this);
    root_ = qml->createRootObject<Page>();
    connect(root_, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(root_, SIGNAL(showFullPhoto()), this, SLOT(onShowFullPhoto()));
}

RepDetailsPage::~RepDetailsPage()
{
    qDebug() << "RepDetailsPage::~RepDetailsPage()";
}

Page *RepDetailsPage::rootNode() const
{
    return root_;
}

void RepDetailsPage::setRepData(const QVariantMap &data)
{
    root_->setProperty("photo", data["photo"]);
    root_->setProperty("displayName", data["displayName"]);
    root_->setProperty("role", data["role"]);
    root_->setProperty("party", data["party"]);
    root_->setProperty("committees", data["committees"]);
    root_->setProperty("notes", data["notes"]);
    root_->setProperty("website", data["website"]);
    root_->setProperty("phoneNumber", data["phoneNumber"]);
    root_->setProperty("address", data["address"]);
    data_ = data;
}

void RepDetailsPage::onShowFullPhoto()
{
    qDebug() << "RepDetailsPage::onShowFullPhoto()";
    const QByteArray photoData = data_["photoData"].toByteArray();
    if(photoData.isNull()) {
        qWarning() << "No photo data";
        return;
    }

    QTemporaryFile *tempFile = new QTemporaryFile("tmp/photo_data.XXXXXX.jpg");
    tempFile->open();
    tempFile->write(photoData);
    tempFile->close();

    bb::system::InvokeManager *invokeManager = new bb::system::InvokeManager();
    tempFile->setParent(invokeManager);
    connect(invokeManager, SIGNAL(childCardDone(bb::system::CardDoneMessage)),
        invokeManager, SLOT(deleteLater()));
    bb::system::InvokeRequest request;
    request.setTarget("sys.pictures.card.previewer");
    request.setAction("bb.action.VIEW");
    request.setMimeType("image/jpeg");
    request.setUri(QLatin1String("file://") + tempFile->fileName());
    bb::system::InvokeTargetReply *reply = invokeManager->invoke(request);
    if(!reply) {
        delete invokeManager;
    }
}
