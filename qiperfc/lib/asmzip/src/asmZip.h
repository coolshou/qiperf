#ifndef __ASM_ZIP_H__
#define __ASM_ZIP_H__

/*
  Access ZIP files
   - List files contained in ZIP
   - Unzip specific files in memory and put the contents in a QByteArray

  Uses minizip code from here: http://www.winimage.com/zLibDll/minizip.html
*/

#include "unzip.h"
#include <QString>
#include <QByteArray>
#include <QTextStream>

class QByteArray;
class QString;
class QTextStream;

class asmZip
{
   public:
      // enum for case sensitivity
      //   [see unzStringFileNameCompare() in unzip.c]
      enum eCaseSensitivity
      {
         CASE_OS_DEFAULT = 0,
         CASE_SENSITIVE,
         CASE_INSENSITIVE
      };

      // Wrap the minizip error defines from unzip.h in a nice enum and add our own
      enum eErrCode
      {
         NO_ERR = UNZ_OK,

         ERR_END_OF_LIST_OF_FILE = UNZ_END_OF_LIST_OF_FILE,
         ERR_ERRNO = UNZ_ERRNO,
         ERR_EOF = UNZ_EOF,
         ERR_PARAMERROR = UNZ_PARAMERROR,
         ERR_BADZIPFILE = UNZ_BADZIPFILE,
         ERR_INTERNALERROR = UNZ_INTERNALERROR,
         ERR_CRCERROR = UNZ_CRCERROR,

         ERR_FILE_NOT_FOUND_IN_ZIP = -1000
      };

   public:
      asmZip( const QString &inFileName );
      ~asmZip();

      // Is this ZIP file readable and valid?
      bool isValid() const { return mFile != NULL; }

      // Extract a file from the ZIP file and put the contents into a QByteArray
      eErrCode  extractFile( const QString &inFileName, QByteArray &outData,
                             eCaseSensitivity inCaseSensitive = CASE_OS_DEFAULT ) const;

      // List the contents of the ZIP file
      //    Modified version of the do_list() function in miniunz.c
      eErrCode  listFiles() const;

   private:
      static void sOutputListLine( QTextStream &out, const QStringList &inStrings );

      unzFile  mFile;
};

#endif
