#include "util.hpp"

#include <QtCore/QFile>
#include <bb/cascades/Image>

namespace util
{

/*!
 * Converts a Contact object, created by parsing a Rep VCard record, into
 * something more generic and suitable for display in a ListView.
 *
 * Creating an Image is optional, because it can only be create on the main
 * thread of the application. Creating an Image on another thread will cause
 * the application to crash.
 *
 * @param contact The contact object to convert
 * @param photoToImage true, if a displayable Cascades Image object should be
 *     created as part of the conversion process
 * @return the converted Rep
 */
QVariantMap contactToMap(const bb::pim::contacts::Contact &contact, bool photoToImage)
{
    QVariantMap map;

    map["firstName"] = contact.firstName();
    map["lastName"] = contact.lastName();
    map["displayName"] = contact.displayName();
    map["party"] = contact.displayCompanyName();

    if(contact.displayCompanyName().startsWith("Republican")) {
        map["partyCode"] = "R";
    }
    else if(contact.displayCompanyName().startsWith("Democrat")) {
        map["partyCode"] = "D";
    }
    else {
        map["partyCode"] = "I";
    }

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

    // Iterate through the attributes of the Contact, picking out specific
    // fields we want to display.
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

    // The Contact photo is provided as a path to an image file. Since this
    // may be temporary in the case of VCard parsing, and we need access to
    // it later, read it into a byte array.
    const QString photoPath = contact.smallPhotoFilepath();
    if(!photoPath.isNull()) {
        QFile photoFile(photoPath);
        photoFile.open(QIODevice::ReadOnly);
        const QByteArray photoData = photoFile.readAll();
        if(!photoData.isEmpty()) {
            if(photoToImage) {
                bb::cascades::Image image(photoData);
                map["photo"] = QVariant(qMetaTypeId<bb::cascades::Image>(), &image);
            }
            map["photoData"] = photoData;
        }
        else {
            if(photoToImage) {
                bb::cascades::Image image(QUrl("asset:///images/rep-default-thumb.png"));
                map["photo"] = QVariant(qMetaTypeId<bb::cascades::Image>(), &image);
            }
        }
    }

    return map;
}

}
