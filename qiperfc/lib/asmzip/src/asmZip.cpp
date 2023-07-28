#include <QStringList>
#include <QDebug>

#include "asmZip.h"


asmZip::asmZip( const QString &inFileName )
{
   mFile = unzOpen64( qPrintable( inFileName ) );

   if ( mFile == NULL )
   {
      qDebug() << "asmZip: could not open file" << inFileName;
   }
}

asmZip::~asmZip()
{
   unzClose( mFile );
}
asmZip::eErrCode asmZip::extractFile( const QString &inFileName, QByteArray &outData,
                                      eCaseSensitivity inCaseSensitive ) const
{
   if ( !isValid() )
      return ERR_BADZIPFILE;

   if ( unzLocateFile( mFile, qPrintable( inFileName ), inCaseSensitive ) != UNZ_OK )
   {
      qDebug() << "ERROR: asmZip::extractFile() - file not found in the zipfile" << inFileName;
      return ERR_FILE_NOT_FOUND_IN_ZIP;
   }

   unz_file_info64 file_info;

   int   err = unzGetCurrentFileInfo64( mFile, &file_info, NULL, 0, NULL, 0, NULL, 0 );

   if ( err != NO_ERR)
   {
      qDebug() << "ERROR: asmZip::extractFile() - unzGetCurrentFileInfo64" << err;
      return static_cast<eErrCode>( err );
   }

   err = unzOpenCurrentFile( mFile );
   //unzOpenCurrentFilePassword(mFile, password); // password protect zip file

   if ( err != NO_ERR)
   {
      qDebug() << "ERROR: asmZip::extractFile() - unzOpenCurrentFile" << err;
      return static_cast<eErrCode>( err );
   }

   outData.fill( 0, file_info.uncompressed_size + 1 );

   qDebug() << "asmZip::extractFile() - Extracting" << inFileName << "buffer size" << outData.size();

   err = unzReadCurrentFile( mFile, outData.data(), outData.size() );

   if ( err < 0 )
   {
      qDebug() << "ERROR: asmZip::extractFile() - unzReadCurrentFile" << err;
      return static_cast<eErrCode>( err );
   }

   unzCloseCurrentFile( mFile );

   return static_cast<eErrCode>( err );
}
void asmZip::sOutputListLine( QTextStream &out, const QStringList &inStrings )
{
   if ( inStrings.count() != 8 )
      return;

   out << qSetFieldWidth( 8 ) << inStrings[0];
   out << qSetFieldWidth( 8 ) << inStrings[1];
   out << qSetFieldWidth( 9 ) << inStrings[2];
   out << qSetFieldWidth( 6 ) << inStrings[3];
   out << qSetFieldWidth( 10 ) << inStrings[4];
   out << qSetFieldWidth( 7 ) << inStrings[5];
   out << qSetFieldWidth( 9 ) << inStrings[6];
   out << qSetFieldWidth( 20 ) << inStrings[7];
   out << Qt::endl;
}

asmZip::eErrCode asmZip::listFiles() const
{
   unz_global_info64 gi;

   int   err = unzGetGlobalInfo64( mFile, &gi );

   if ( err != NO_ERR )
   {
      qDebug() << QString( "asmZip::list - error %d with zipfile in unzGetGlobalInfo \n").arg( err );
      return static_cast<eErrCode>( err );
   }

   QTextStream out( stdout );

   out << Qt::right;

   QStringList strList;

   strList << "Length" << "Method" << "Size" << "Ratio" << "Date" << "Time" << "CRC-32" << "Name";
   sOutputListLine( out, strList );

   strList.clear();
   strList << "------" << "------" << "----" << "-----" << "----" << "----" << "------" << "----";
   sOutputListLine( out, strList );

   err = unzGoToFirstFile( mFile );

   if ( err != NO_ERR )
   {
      qDebug() << QString( "asmZip::list - error %d with zipfile in unzGoToFirstFile \n").arg( err );
      return static_cast<eErrCode>( err );
   }

   for ( unsigned int i = 0; i < gi.number_entry; i++ )
   {
      unz_file_info64   file_info;
      QByteArray        filename_inzip;

      filename_inzip.resize( 1024 );

      err = unzGetCurrentFileInfo64( mFile, &file_info,
                                     filename_inzip.data(), filename_inzip.size(),
                                     NULL, 0, NULL, 0 );

      if ( err != NO_ERR)
      {
         qDebug() << QString( "asmZip::list - error %d with zipfile in unzGetCurrentFileInfo \n").arg( err );
         break;
      }

      int ratio = 0;

      if ( file_info.uncompressed_size > 0 )
         ratio = ((file_info.compressed_size*100) / file_info.uncompressed_size);

      QString  method;

      if ( file_info.compression_method == 0 )
      {
         method = "Stored";
      }
      else if ( file_info.compression_method == Z_DEFLATED )
      {
         int iLevel = (file_info.flag & 0x6) / 2;

         if ( iLevel == 0 )
            method = "Defl:N";
         else if ( iLevel == 1 )
            method = "Defl:X";
         else if ( (iLevel == 2) || (iLevel == 3) )
            method = "Defl:F"; /* 2:fast , 3 : extra fast*/
      }
      else if ( file_info.compression_method == Z_BZIP2ED )
      {
         method = "BZip2";
      }
      else
      {
         method = "Unkn.";
      }

      // add a '*' if the file is encrypted
      if ( (file_info.flag & 1) != 0 )
         method += '*';

      strList.clear();
      strList << QString::number( file_info.uncompressed_size );
      strList << method;
      strList << QString::number( file_info.compressed_size );
      strList << QString( "%1%" ).arg( ratio );
      strList << QString( "%1-%2-%3" ).arg( file_info.tmu_date.tm_mon + 1, 2, 10, QChar( '0' ) )
                 .arg( file_info.tmu_date.tm_mday, 2, 10, QChar( '0' ) )
                 .arg( file_info.tmu_date.tm_year % 100, 2, 10, QChar( '0' ) );
      strList << QString( "%1:%2" ).arg( file_info.tmu_date.tm_hour, 2, 10, QChar( '0' ) )
                 .arg( file_info.tmu_date.tm_min, 2, 10, QChar( '0' ) );
      strList << QString::number( file_info.crc, 16 );
      strList << filename_inzip;

      sOutputListLine( out, strList );

      if ( (i+1) < gi.number_entry )
      {
         err = unzGoToNextFile( mFile );

         if ( err != NO_ERR)
         {
            qDebug() << QString( "asmZip::list - error %d with zipfile in unzGoToNextFile \n").arg( err );
            break;
         }
      }
   }

   return NO_ERR;
}
