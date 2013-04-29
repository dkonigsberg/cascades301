#ifndef REPDATAMODEL_H_
#define REPDATAMODEL_H_

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <bb/cascades/DataModel>
#include <bb/pim/contacts/ContactService>

class RepDataModel : public bb::cascades::DataModel
{
    Q_OBJECT
public:
    RepDataModel(QObject *parent=0);
    virtual ~RepDataModel();
    void appendVCard(const QString &vcard);
    void clear();
    virtual int childCount(const QVariantList &indexPath);
    virtual bool hasChildren(const QVariantList &indexPath);
    virtual QString itemType(const QVariantList &indexPath);
    virtual QVariant data(const QVariantList &indexPath);

private:
    Q_DISABLE_COPY(RepDataModel)
    QVariantMap dataForIndex(int index);
    QVariantMap parseContactCard(const QString &vcard);
    bb::pim::contacts::ContactService contactService_;
    QList<QString> vcardList_;
    QHash<int, QVariantMap> dataMap_;
};

#endif /* REPDATAMODEL_H_ */
