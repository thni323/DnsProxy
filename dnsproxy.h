#ifndef DNSPROXY_H
#define DNSPROXY_H

#include <QObject>
#include <QUdpSocket>

struct Request {
    QByteArray data;
    quint16 id;
    QHostAddress clientAddress;
    quint16 clientPort;
};

class DnsProxy : public QObject
{
    Q_OBJECT

    QHostAddress masterDns;
    QHostAddress dnsBind;
    QHostAddress clientBind;
    QUdpSocket* pSocketAsServer;
    QUdpSocket* pSocketAsClient;

    QVector<Request> requests;

    QByteArray firstNameOfRequest(QByteArray dnsPacket);
    QString convertNameToString(QByteArray name);

private slots:
    void readFromServer();
    void readFromClient();

public:
    explicit DnsProxy(QString toServer, QString asServer, QString dnsServer = "8.8.8.8", QObject *parent = 0);
    void start();
    void stop();
    ~DnsProxy();

signals:
    void errorMessage(QString msg);
    void warningMessage(QString msg);
    void infoMessage(QString msg);

public slots:

};

#endif // DNSPROXY_H
