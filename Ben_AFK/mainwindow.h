#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QMovie>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum PageIndex {
    Choice,
    Loading,
    Chat
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void createSocket();

private slots:
    void standard();
    void secured();
    void changeTab(int);
    void onConnect();
    void onDisconnect();
    void timeout();
    void sendLine();
    void tryLogging();

private:
    Ui::MainWindow  *ui;
    bool            isSecure;
    bool            isLogged;
    QTcpSocket      *socket;
};
#endif // MAINWINDOW_H
