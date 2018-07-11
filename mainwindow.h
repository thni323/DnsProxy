#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <dnsproxy.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    DnsProxy* pDnsProxy;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slotOnDebugMessage(QString msg);

private slots:
    void on_pushButton_clicked(); // Start

    void on_pushButton_refresh_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
