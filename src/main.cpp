#include <qapplication.h>
#include "reglauncher.h"
#include "unistd.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    RegLauncher *w = new RegLauncher;
    w->show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
