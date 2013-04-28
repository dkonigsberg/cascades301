#include "repdetailspage.hpp"

#include <QtCore/QDebug>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/Page>

using namespace bb::cascades;

RepDetailsPage::RepDetailsPage(QObject *parent) : QObject(parent)
{
    qDebug() << "RepDetailsPage::RepDetailsPage()";
    QmlDocument *qml = QmlDocument::create("asset:///RepDetailsPage.qml").parent(this);
    root_ = qml->createRootObject<Page>();
    connect(root_, SIGNAL(destroyed()), this, SLOT(deleteLater()));
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
}
