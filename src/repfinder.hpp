// Default empty project template
#ifndef REPFINDER_HPP_
#define REPFINDER_HPP_

#include <QtCore/QObject>

namespace bb { namespace cascades { class Application; }}

class RepFinder : public QObject
{
    Q_OBJECT
public:
    RepFinder(bb::cascades::Application *app);
    virtual ~RepFinder() {}
};


#endif /* REPFINDER_HPP_ */
