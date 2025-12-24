#include "ftpserver.h"
#include "ftpcontrolconnection.h"
#include "sslserver.h"

#include <QDebug>
#include <QNetworkInterface>
#include <QSslSocket>

FtpServer::FtpServer(QObject *parent, const QString &rootPath, int port,
                     const QString &userName, const QString &password,
                     bool readOnly, bool onlyOneIpAllowed) : QObject(parent)
    , m_pFilter(this)
{
    bool bRet = false;

    server = new SslServer(this);
    // In Qt4, QHostAddress::Any listens for IPv4 connections only, but as of
    // Qt5, it now listens on all available interfaces, and
    // QHostAddress::AnyIPv4 needs to be used if we want only IPv4 connections.
#if QT_VERSION >= 0x050000
    bRet = server->listen(QHostAddress::AnyIPv4, port);
#else
    bRet = server->listen(QHostAddress::Any, port);
#endif
    if(!bRet)
    {
        qCritical() << "Server listen at " << port << " fail:" << server->errorString();
    }
    connect(server, SIGNAL(newConnection()), this, SLOT(startNewControlConnection()));
    this->userName = userName;
    this->password = password;
    this->rootPath = rootPath;
    this->readOnly = readOnly;
    this->onlyOneIpAllowed = onlyOneIpAllowed;
}

bool FtpServer::isListening()
{
    return server->isListening();
}

QString FtpServer::lanIp()
{
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)) {
            return address.toString();
        }
    }
    return "";
}

void FtpServer::startNewControlConnection()
{
    QSslSocket *socket = qobject_cast<QSslSocket*>(server->nextPendingConnection());
    if(!socket) {
        qCritical() << "The socket is nullptr";
        return;
    }

    // If this is not a previously encountered IP emit the newPeerIp signal.
    QString peerIp = socket->peerAddress().toString();
    qDebug() << "connection from" << peerIp;
    bool bFilter = false;
    if(m_pFilter)
        bFilter = m_pFilter->onFilter(socket);
    if(bFilter)
    {
        delete socket;
        return;
    }

    // Create a new FTP control connection on this socket.
    new FtpControlConnection(this, socket, rootPath, userName, password, readOnly);
}

bool FtpServer::onFilter(QSslSocket *socket)
{
    if(!socket) return true;
    QString ip = socket->peerAddress().toString();
    if (!encounteredIps.contains(ip)) {
        // If we don't allow more than one IP for the client, we close
        // that connection.
        if (onlyOneIpAllowed && !encounteredIps.isEmpty()) {
            return true;
        }
        emit newPeerIp(ip);
        encounteredIps.insert(ip);
    }
    return false;
}

void FtpServer::SetFilter(CFtpServerFilter *handler)
{
    m_pFilter = handler;
}
