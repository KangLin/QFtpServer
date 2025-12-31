#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <QObject>
#include <QSet>
#include <QHostAddress>
#include <qftpserverlib_global.h>

class SslServer;
class QSslSocket;

class QFTPSERVERLIBSHARED_EXPORT CFtpServerFilter {
public:
    virtual bool onFilter(QSslSocket *socket) = 0;
};

// The ftp server. Listens on a port, and starts a new control connection each
// time it gets connected.

class QFTPSERVERLIBSHARED_EXPORT CFtpServer : public QObject, CFtpServerFilter
{
    Q_OBJECT
public:
    explicit CFtpServer(QObject *parent, const QString &rootPath, quint16 port = 21,
                        const QString &userName = QString(), const QString &password = QString(),
                        bool readOnly = false);
    ~CFtpServer();
    bool Listening(const QHostAddress& addr =
                    // In Qt4, QHostAddress::Any listens for IPv4 connections only, but as of
                    // Qt5, it now listens on all available interfaces, and
                    // QHostAddress::AnyIPv4 needs to be used if we want only IPv4 connections.
                    #if QT_VERSION >= 0x050000
                        QHostAddress::AnyIPv4
                    #else
                        QHostAddress::Any
                    #endif
                   );
    bool Listening(const QList<QHostAddress>& addr);

    // Whether or not the server is listening for incoming connections. If it
    // is not currently listening then there was an error - probably no
    // internet connection is available, or the port address might require root
    // priviledges (on Linux).
    bool isListening();

    // Get the LAN IP of the host, e.g. "192.168.1.10".
    static QString lanIp();

    void SetFilter(CFtpServerFilter* handler);
    virtual bool onFilter(QSslSocket *socket) override;

signals:
    // A connection from a new IP has been established. This signal is emitted
    // when the FTP server is connected by a new IP. The new IP will then be
    // stored and will not cause this FTP server instance to emit this signal
    // any more.
    void newPeerIp(const QString &ip);

private slots:
    // Called by the SSL server when we have received a new connection.
    void startNewControlConnection();

private:
    CFtpServerFilter* m_pFilter;
    quint16 m_nPort;
    // If both username and password are empty, it means anonymous mode - any
    // username and password combination will be accepted.
    QString m_szUserName;
    QString m_szPassword;

    // The root path is the virtual root directory of the FTP. It works like
    // chroot (see http://en.wikipedia.org/wiki/Chroot). The FTP server will
    // not access files or folders that are not in this folder or any of its
    // subfolders.
    QString m_szRootPath;

    // The SSL m_pServer listen for incoming connections.
    QSet<SslServer*> m_pServer;

    // All the IPs that this FTP server object has encountered in its lifetime.
    // See the signal newPeerIp.
    QSet<QString> m_EncounteredIps;

    // Whether or not the server is in read-only mode. In read-only mode the
    // server will not create, modify or delete any files or directories.
    bool m_bReadOnly;
};

#endif // FTPSERVER_H
