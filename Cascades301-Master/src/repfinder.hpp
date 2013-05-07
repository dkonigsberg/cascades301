#ifndef REPFINDER_HPP_
#define REPFINDER_HPP_

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtNetwork/QNetworkAccessManager>
#include <bb/pim/contacts/Contact>

namespace bb { namespace cascades {
class Application;
class NavigationPane;
class Page;
class DataModel;
}}

class RepFinder : public QObject
{
    Q_OBJECT
public:
    RepFinder(bb::cascades::Application *app);
    virtual ~RepFinder() {}

private slots:
    void onPopTransitionEnded(bb::cascades::Page *page);
    void onLoadData();
    void onRequestFinished();
    void onShowDetails(const QVariant &dataItem);

private:
    QList<QString> vcardsFromData(const QByteArray &data);
    void populateDataModel(QList<QString> &vcards);
    void populateSparseDataModel(const QVariantList &dataList);
    bb::cascades::NavigationPane *nav_;
    bb::cascades::Page *rootPage_;
    bb::cascades::DataModel *dataModel_;
    QNetworkAccessManager accessManager_;
};


#endif /* REPFINDER_HPP_ */
