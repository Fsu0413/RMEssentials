
#ifdef RME_USE_QUAZIP

#include "uncompresser.h"
#include "quazipfile.h"

void Uncompresser::run()
{
    for (int i = 0; i < zipNames.length(); ++i) {
        const QString &zipName = zipNames.at(i);
        const QString &fileName = fileNames.at(i);

        QuaZipFile f(zipName, fileName);
        if (f.open(QIODevice::ReadOnly)) {
            QDir dir(QFileInfo(zipName).absolutePath());
            QFile output(dir.absoluteFilePath(fileName));
            output.open(QIODevice::WriteOnly);
            output.write(f.readAll());
            output.close();
            f.close();
            emit signal_file_finished(fileName);
        }
    }
}

#endif
