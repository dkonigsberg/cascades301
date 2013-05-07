#ifndef REPDATAMODEL_H_
#define REPDATAMODEL_H_

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtNetwork/QNetworkAccessManager>
#include <bb/cascades/DataModel>

class RepDataModel : public bb::cascades::DataModel
{
    Q_OBJECT
public:
    RepDataModel(QObject *parent=0);
    virtual ~RepDataModel();
    void appendSparseElement(const QString &id, const QString &name,
        const QString &state, const QString &district,
        const QString &partyCode);
    void clear();
    virtual int childCount(const QVariantList &indexPath);
    virtual bool hasChildren(const QVariantList &indexPath);
    virtual QString itemType(const QVariantList &indexPath);
    virtual QVariant data(const QVariantList &indexPath);

private slots:
    void onContactCardParsed();
    void onRequestFinished();

private:
    Q_DISABLE_COPY(RepDataModel)
    QVariantMap dataForIndex(int index);
    QVariantMap parseContactCard(const QString &vcard);
    QVariantMap dataForIndexSparse(int index);
    QList<QVariantMap> dataList_;
    QNetworkAccessManager accessManager_;
};

#endif /* REPDATAMODEL_H_ */
