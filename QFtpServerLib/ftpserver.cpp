#include <QLoggingCategory>
#include <QDebug>
#include <QNetworkInterface>
#include <QSslSocket>

#include "ftpserver.h"
#include "ftpcontrolconnection.h"
#include "sslserver.h"

static Q_LOGGING_CATEGORY(log, "FtpServer")
CFtpServer::CFtpServer(QObject *parent, const QString &rootPath, quint16 port,
                       const QString &userName, const QString &password,
                       bool readOnly) : QObject(parent)
    , m_pFilter(this)
    , m_nPort(port)
    , m_szUserName(userName)
    , m_szPassword(password)
    , m_szRootPath(rootPath)
    , m_bReadOnly(readOnly)
{
}

CFtpServer::~CFtpServer()
{
    qDebug(log) << Q_FUNC_INFO;
    foreach (auto s, m_pServer) {
        if(s) {
            s->close();
            delete s;
        }
    }
}

bool CFtpServer::Listening(const QHostAddress &addr)
{
    bool bRet = false;
    auto pServer = new SslServer(this);
    if(!pServer) {
        qCritical(log) << "new SslServer fail. no memery";
        return false;
    }

    bRet = pServer->listen(addr, m_nPort);
    if(!bRet)
    {
        qCritical(log) << "Server listen at" << m_nPort << " fail:" << pServer->errorString();
        delete pServer;
        return false;
    }
    qDebug(log) << "Server listen at" << addr.toString() << "Port:" << m_nPort;
    bRet = connect(pServer, SIGNAL(newConnection()), this, SLOT(startNewControlConnection()));
    Q_ASSERT(bRet);
    m_pServer.insert(pServer);
    return true;
}

bool CFtpServer::Listening(const QList<QHostAddress> &addr)
{
    bool bRet = false;
    foreach (auto a, addr) {
        bool b = Listening(a);
        if(b)
            bRet = b;
    }
    return bRet;
}

bool CFtpServer::isListening()
{
    foreach (auto s, m_pServer) {
        if(s->isListening())
            return true;
    }
    return false;
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
    SslServer* pServer = qobject_cast<SslServer*>(sender());
    QSslSocket* pSocket = qobject_cast<QSslSocket*>(pServer->nextPendingConnection());
    if(!pSocket) {
        qCritical() << "The socket is nullptr";
        return;
    }

    // If this is not a previously encountered IP emit the newPeerIp signal.
    QString peerIp = pSocket->peerAddress().toString();
    qDebug() << "connection from" << peerIp;
    bool bFilter = false;
    if(m_pFilter)
        bFilter = m_pFilter->onFilter(pSocket);
    if(bFilter)
    {
        qDebug(log) << "Filte connection from IP:" << peerIp << "Port:" << pSocket->peerPort();
        delete pSocket;
        return;
    }

    // Create a new FTP control connection on this socket.
    new FtpControlConnection(this, pSocket, m_szRootPath, m_szUserName, m_szPassword, m_bReadOnly);
}

bool CFtpServer::onFilter(QSslSocket *socket)
{
    if(!socket) return true;
    QString ip = socket->peerAddress().toString();
    if (!m_EncounteredIps.contains(ip)) {
        emit newPeerIp(ip);
        m_EncounteredIps.insert(ip);
    }
    return false;
}

void CFtpServer::SetFilter(CFtpServerFilter *handler)
{
    m_pFilter = handler;
}
