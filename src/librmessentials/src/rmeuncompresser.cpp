
#include "rmeuncompresser.h"

#ifdef RME_USE_QUAZIP
#include "rmedownloader.h"
#include <QDir>
#include <quazip/QuaZipFile>

void RmeUncompresser::run()
{
    for (int i = 0; i < zipNames.length(); ++i) {
        QString zipName = RmeDownloader::downloadPath() + zipNames.at(i);
        const QString &fileName = fileNames.at(i);

        QuaZipFile f(zipName, fileName);
        if (f.open(QIODevice::ReadOnly)) {
            QDir dir(QFileInfo(zipName).absolutePath());
            QFile output(dir.absoluteFilePath(fileName));
            output.open(QIODevice::WriteOnly);
            output.write(f.readAll());
            output.close();
            f.close();
            emit signalFileFinished(fileName);
        }
    }
}

#else

void RmeUncompresser::run()
{
    Q_UNIMPLEMENTED();
}

#endif
