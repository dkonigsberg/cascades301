#include <bb/cascades/Application>

#include <QLocale>
#include <QTranslator>
#include "repfinder.hpp"

#include <Qt/qdeclarativedebug.h>

using namespace bb::cascades;

Q_DECL_EXPORT int main(int argc, char **argv)
{
    Application app(argc, argv);

    QTranslator translator;
    QString locale_string = QLocale().name();
    QString filename = QString("AdvCascadesLab_%1").arg(locale_string);
    if (translator.load(filename, "app/native/qm")) {
        app.installTranslator(&translator);
    }

    new RepFinder(&app);

    return Application::exec();
}
