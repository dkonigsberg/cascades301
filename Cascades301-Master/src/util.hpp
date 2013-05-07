#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <QtCore/QVariant>
#include <bb/pim/contacts/Contact>

namespace util
{

QVariantMap contactToMap(const bb::pim::contacts::Contact &contact, bool photoToImage = true);

}

#endif /* UTIL_HPP_ */
