#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHostAddress>
#include <QNetworkInterface>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pDnsProxy = NULL;

    on_pushButton_refresh_clicked();

    ui->lineEdit_master_dns_ip->setText("8.8.8.8");
    ui->plainTextEdit_debug->setMaximumBlockCount(100);
}

MainWindow::~MainWindow()
{
    if (pDnsProxy != NULL) {
        delete pDnsProxy;
    }

    delete ui;
}

void MainWindow::on_pushButton_start_clicked()
{
    if (pDnsProxy == NULL) {
        pDnsProxy = new DnsProxy(ui->comboBox_to_server->currentText(), ui->comboBox_as_server->currentText(), ui->lineEdit_master_dns_ip->text(), this);
        connect(pDnsProxy, SIGNAL(errorMessage(QString)), this, SLOT(slotOnDebugMessage(QString)));
        connect(pDnsProxy, SIGNAL(warningMessage(QString)), this, SLOT(slotOnDebugMessage(QString)));
        connect(pDnsProxy, SIGNAL(infoMessage(QString)), this, SLOT(slotOnDebugMessage(QString)));
        ui->pushButton_start->setText("Stop DNS Server");
        pDnsProxy->start();
    } else {
        pDnsProxy->stop();
        pDnsProxy->disconnect();
        delete pDnsProxy;
        pDnsProxy = NULL;
        ui->pushButton_start->setText("Start DNS Server");
    }
}

void MainWindow::slotOnDebugMessage(QString msg)
{
    if (ui->plainTextEdit_debug->toPlainText().size() > 100*100)
    {
        ui->plainTextEdit_debug->clear();
    }

    ui->plainTextEdit_debug->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss ") + msg);
    ui->plainTextEdit_debug->moveCursor(QTextCursor::End);
    ui->plainTextEdit_debug->ensureCursorVisible();
}

void MainWindow::on_pushButton_refresh_clicked()
{
    QStringList interfaceAddresses;
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
        {
            interfaceAddresses.append(address.toString());
        }
    }

    ui->comboBox_as_server->clear();
    ui->comboBox_to_server->clear();
    ui->comboBox_as_server->addItems(interfaceAddresses);
    ui->comboBox_to_server->addItems(interfaceAddresses);
    if (interfaceAddresses.size() > 1) {
        ui->comboBox_to_server->setCurrentIndex(0);
        ui->comboBox_as_server->setCurrentIndex(1);
    }
}
