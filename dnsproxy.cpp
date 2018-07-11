#include "dnsproxy.h"

#include <QDebug>

QByteArray DnsProxy::firstNameOfRequest(QByteArray dnsPacket)
{
    qint16 iTotalQuestions = 0;
    QByteArray baName;

    if (dnsPacket.size() < 15) {
        return baName;
    }

    iTotalQuestions = (quint8) dnsPacket.at(4);
    iTotalQuestions = iTotalQuestions << 8;
    iTotalQuestions = iTotalQuestions | (quint8) dnsPacket.at(5);

    if (iTotalQuestions > 0) {
        qint16 iPointer = 12;
        quint16 iPosToReturn = 0;

        if(iPointer+1 >= dnsPacket.size())
        {
            qDebug() << "ERROR Invalid pointer";
            return baName;
        }

        while(iPointer < dnsPacket.size() && ((quint8) dnsPacket.at(iPointer) > 0))
        {
            if ((quint8) dnsPacket.at(iPointer) == 193) {
                qDebug() << "ERROR Unsupported C! pointer";
                baName.clear();
                return baName;
            }

            if((quint8) dnsPacket.at(iPointer) == 192)
            {
                if(iPosToReturn == 0) iPosToReturn = iPointer+2;
                iPointer = (quint8) dnsPacket.at(iPointer+1);
            }

            baName.append(dnsPacket.at(iPointer));
            iPointer++;
        }

        if(iPosToReturn > 0)
        {
            iPointer = iPosToReturn;
        }
        else
        {
            iPointer++;
        }

        baName.resize(baName.size()+1);
        baName[baName.size()-1] = 0x00;
        return baName;
    }

    return baName;
}

QString DnsProxy::convertNameToString(QByteArray name)
{
    QString sName;
    quint16 iPointer = 0;

    if(iPointer+1 >= name.size() ||(quint8) name.at(iPointer) == 192)
    {
        qDebug() << "ERROR Invalid data";
        return sName;
    }

    while(iPointer < name.size() && ((quint8) name.at(iPointer) > 0))
    {
        if((quint8) name.at(iPointer) <= 31)
        {
            sName.append(".");
        }
        else
        {
            sName.append(name.at(iPointer));
        }
        iPointer++;
    }

    sName.remove(0, 1);

    return sName;
}

void DnsProxy::readFromServer()
{
    QByteArray data;
    data.resize(1500);

    data.resize(pSocketAsClient->readDatagram(data.data(), data.size()));

    if (data.size() < 5) {
        emit warningMessage("Received packet is way too small");
    }
    quint16 answerId = (quint8) data.at(0);
    answerId = answerId << 8;
    answerId = answerId | (quint8) data.at(1);

    for (int i=0; i < requests.size(); i++) {
        if (requests.at(i).id == answerId) {
            pSocketAsServer->writeDatagram(data, requests.at(i).clientAddress, requests.at(i).clientPort);

            // Create info message
            QString sId;
            sId.setNum(answerId);
            emit infoMessage("Response " + sId);

            requests.removeAt(i);
            break;
        }
    }

    if (pSocketAsClient->hasPendingDatagrams()) {
        readFromServer();
    }
}

void DnsProxy::readFromClient()
{
    QByteArray data;
    data.resize(1500);
    QHostAddress address;
    quint16 port;

    data.resize(pSocketAsServer->readDatagram(data.data(), data.size(), &address, &port));

    if (data.size() < 5) {
        emit warningMessage("Packet is way too small");
    }
    quint16 requestId = (quint8) data.at(0);
    requestId = requestId << 8;
    requestId = requestId | (quint8) data.at(1);

    // Create info message
    QString sId;
    sId.setNum(requestId);
    emit infoMessage("Request " + sId + ": " + convertNameToString(firstNameOfRequest(data)));

    Request request;
    request.clientAddress = address;
    request.clientPort = port;
    request.data = data;
    request.id = requestId;

    if (requests.size() > 100) {
        emit infoMessage("Clearing request cache");
        requests.remove(0, 50);
    }
    requests.append(request);

    qint64 bytesWritten;
    bytesWritten = pSocketAsClient->writeDatagram(data, masterDns, 53);
    if (bytesWritten != data.size()) {
        emit errorMessage("Failed to forward data to server");
    }

    if (pSocketAsServer->hasPendingDatagrams()) {
        readFromClient();
    }
}

DnsProxy::DnsProxy(QString toServer, QString asServer, QString dnsServer, QObject *parent) :
    QObject(parent)
{
    masterDns = QHostAddress(dnsServer);
    dnsBind = QHostAddress(toServer);
    clientBind = QHostAddress(asServer);

    if (masterDns.isNull() || dnsBind.isNull() || clientBind.isNull())
    {
        emit errorMessage("Invalid address");
    }

    pSocketAsClient = new QUdpSocket(this);
    pSocketAsServer = new QUdpSocket(this);

    connect(pSocketAsClient, SIGNAL(readyRead()), this, SLOT(readFromServer()));
    connect(pSocketAsServer, SIGNAL(readyRead()), this, SLOT(readFromClient()));
}

void DnsProxy::start()
{
    emit infoMessage("Startup DnsProxy v1.0");

    if(!pSocketAsClient->bind(dnsBind, 9888)) {
        emit errorMessage("Could not bind client socket");
    }
    if (!pSocketAsServer->bind(clientBind, 53)) {
        emit errorMessage("Could not bind server socket");
    }
}

void DnsProxy::stop()
{
    emit infoMessage("Shutdown DnsProxy v1.0");

    pSocketAsClient->close();
    pSocketAsServer->close();
}

DnsProxy::~DnsProxy()
{
    pSocketAsClient->abort();
    pSocketAsServer->abort();
    delete pSocketAsClient;
    delete pSocketAsServer;
}
