#ifndef FTPSTORCOMMAND_H
#define FTPSTORCOMMAND_H

#include "ftpcommand.h"

class QFile;

class FtpStorCommand : public FtpCommand
{
    Q_OBJECT
public:
    explicit FtpStorCommand(QObject *parent, const QString &fileName, bool appendMode = false, qint64 seekTo = 0);
    ~FtpStorCommand();

private slots:
    void acceptNextBlock();

private:
    void startImplementation(QTcpSocket *socket);

    QTcpSocket* socket;
    QString fileName;
    QFile *file;
    bool appendMode;
    qint64 seekTo;
};

#endif // FTPSTORCOMMAND_H