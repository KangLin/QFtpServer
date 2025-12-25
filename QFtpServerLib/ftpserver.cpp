#include "ftpserver.h"
#include "ftpcontrolconnection.h"
#include "sslserver.h"

#include <QDebug>
#include <QNetworkInterface>
#include <QSslSocket>

CFtpServer::CFtpServer(QObject *parent, const QString &rootPath, int port,
                     const QString &userName, const QString &password,
                     bool readOnly, bool onlyOneIpAllowed) : QObject(parent)
    , m_pFilter(this)
{
    bool bRet = false;

    m_pServer = new SslServer(this);
    // In Qt4, QHostAddress::Any listens for IPv4 connections only, but as of
    // Qt5, it now listens on all available interfaces, and
    // QHostAddress::AnyIPv4 needs to be used if we want only IPv4 connections.
#if QT_VERSION >= 0x050000
    bRet = m_pServer->listen(QHostAddress::AnyIPv4, port);
#else
    bRet = server->listen(QHostAddress::Any, port);
#endif
    if(!bRet)
    {
        qCritical() << "Server listen at " << port << " fail:" << m_pServer->errorString();
    }
    connect(m_pServer, SIGNAL(newConnection()), this, SLOT(startNewControlConnection()));
    this->m_szUserName = userName;
    this->m_szPassword = password;
    this->m_szRootPath = rootPath;
    this->m_bReadOnly = readOnly;
    this->m_bOnlyOneIpAllowed = onlyOneIpAllowed;
}

bool CFtpServer::isListening()
{
    return m_pServer->isListening();
}

QString CFtpServer::lanIp()
{
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)) {
            return address.toString();
        }
    }
    return "";
}

void CFtpServer::startNewControlConnection()
{
    QSslSocket *socket = qobject_cast<QSslSocket*>(m_pServer->nextPendingConnection());
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
    new FtpControlConnection(this, socket, m_szRootPath, m_szUserName, m_szPassword, m_bReadOnly);
}

bool CFtpServer::onFilter(QSslSocket *socket)
{
    if(!socket) return true;
    QString ip = socket->peerAddress().toString();
    if (!m_EncounteredIps.contains(ip)) {
        // If we don't allow more than one IP for the client, we close
        // that connection.
        if (m_bOnlyOneIpAllowed && !m_EncounteredIps.isEmpty()) {
            return true;
        }
        emit newPeerIp(ip);
        m_EncounteredIps.insert(ip);
    }
    return false;
}

void CFtpServer::SetFilter(CFtpServerFilter *handler)
{
    m_pFilter = handler;
}
