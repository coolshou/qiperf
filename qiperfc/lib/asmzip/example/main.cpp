#include "asmZip.h"
#include <QDebug>

int main( int argc, char *argv[] )
{
   Q_UNUSED( argc );
   Q_UNUSED( argv );

   asmZip   zipFile( "example.zip" );

   if ( zipFile.isValid() )
   {
      zipFile.listFiles();

      QByteArray  data;

      zipFile.extractFile( "asmZip.h", data );
    qDebug() << data << Qt::endl;
      // "data" now contains the contents of "banners.xml"
   }

   return 0;
}
