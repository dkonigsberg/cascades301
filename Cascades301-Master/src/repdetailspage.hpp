#ifndef REPDETAILSPAGE_HPP_
#define REPDETAILSPAGE_HPP_

#include <QtCore/QObject>
#include <QtCore/QVariant>

namespace bb { namespace cascades {
class Page;
}}

class RepDetailsPage : QObject
{
    Q_OBJECT
public:
    RepDetailsPage(QObject *parent=0);
    virtual ~RepDetailsPage();
    bb::cascades::Page *rootNode() const;
    void setRepData(const QVariantMap &data);

private slots:
    void onShowFullPhoto();

private:
    bb::cascades::Page *root_;
    QVariantMap data_;
};

#endif /* REPDETAILSPAGE_HPP_ */
