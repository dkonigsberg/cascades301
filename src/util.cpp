#include "util.hpp"

#include <QtCore/QFile>
#include <bb/cascades/Image>

namespace util
{

QVariantMap contactToMap(const bb::pim::contacts::Contact &contact)
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
            bb::cascades::Image image(photoData);
            map["photo"] = QVariant(qMetaTypeId<bb::cascades::Image>(), &image);
        }
    }

    return map;
}

}
