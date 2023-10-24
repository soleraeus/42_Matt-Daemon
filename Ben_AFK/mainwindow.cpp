#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = NULL;
    isSecure = false;
    isLogged = false;
    ui->textLog->setPlaceholderText("<LOG>");
    ui->lineChat->setPlaceholderText("write here");
    ui->linePubKey->setPlaceholderText("Path to public key");
    ui->linePrivateKey->setPlaceholderText("Path to private key");
    pathPubKey = "";
    pathPrivateKey = "";

    connect(ui->buttonStandard, &QPushButton::clicked, this, &MainWindow::standard);
    connect(ui->buttonSecured, &QPushButton::clicked, this, &MainWindow::secured);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::changeTab);
    connect(ui->buttonPubKey, &QPushButton::clicked, this, &MainWindow::changePubPath);
    connect(ui->buttonPrivateKey, &QPushButton::clicked, this, &MainWindow::changePrivatePath);
    connect(ui->buttonValidateKey, &QPushButton::clicked, this, &MainWindow::pressButtonValidateKey);
    connect(ui->buttonSubmit, &QPushButton::clicked, this, &MainWindow::tryLogging);
}

MainWindow::~MainWindow()
{
    if (socket) {
        disconnect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnect);
    }
    delete ui;
}

void MainWindow::standard() {
    qDebug() << "Client choosed an insecure connection";
    this->ui->stackedWidget->setCurrentIndex(PageIndex::Loading);
    this->isSecure = false;
    ui->tabWidget->setTabVisible(1, false);
    createSocket();
}

void MainWindow::secured() {
    qDebug() << "Client choosed a secure connection";
    this->ui->stackedWidget->setCurrentIndex(PageIndex::Loading);
    this->isSecure = true;
    createSocket();
}

void MainWindow::changeTab(int index) {
    if (index == 0) {
        // probably nothing to do here
        qDebug() << "tab chat";
    } else {
        if (isSecure) {
            if (isLogged) {
                ui->stackedWidgetLog->setCurrentIndex(0);
            } else {
                ui->stackedWidgetLog->setCurrentIndex(1);
            }
        } else {
            qDebug() << "you are not allowed to see !";
        }
        // Send request for log and print them in ui->textLog
        qDebug() << "tab log";
    }
}

void MainWindow::onConnect() {
    qDebug() << "socket is connected !";
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnect);
    if (isSecure) {
        ui->stackedWidget->setCurrentIndex(PageIndex::Key);
        // Do the thing to secure the connection
    } else {
        ui->stackedWidget->setCurrentIndex(PageIndex::Chat);
    }
    connect(ui->lineChat, &QLineEdit::returnPressed, this, &MainWindow::sendLine);
    connect(ui->buttonSend, &QPushButton::clicked, this, &MainWindow::sendLine);
}

void MainWindow::onDisconnect() {
    qDebug() << "socket disconnected !";
    disconnect(ui->lineChat, &QLineEdit::returnPressed, this, &MainWindow::sendLine);
    disconnect(ui->buttonSend, &QPushButton::clicked, this, &MainWindow::sendLine);
    ui->stackedWidget->setCurrentIndex(PageIndex::Choice);
    delete socket;
    socket = NULL;
}

void MainWindow::createSocket() {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnect);
    socket->connectToHost(ui->lineIp->text(), isSecure ? 4343 : 4242);
    qDebug() << "Connecting to " << ui->lineIp->text() << ":" << (isSecure ? 4343 : 4242);
    QTimer::singleShot(1000, this, &MainWindow::timeout);
}

void MainWindow::timeout() {
    qDebug() << "Timer timed out";
    if (socket->state() != QAbstractSocket::SocketState::ConnectedState) {
        ui->stackedWidget->setCurrentIndex(PageIndex::Choice);
    }
}

void MainWindow::sendLine() {
    if (isSecure) {
        // Encrypt the message
    } else {
        socket->write(ui->lineChat->text().toUtf8() + "\n");
    }
    ui->lineChat->clear();
}

void MainWindow::changePubPath() {
    pathPubKey = QFileDialog::getOpenFileName(this, tr("Open public key"), "/home");
    ui->linePubKey->setText(pathPubKey);
}

void MainWindow::changePrivatePath() {
    pathPrivateKey = QFileDialog::getOpenFileName(this, tr("Open private key"), "/home");
    ui->linePrivateKey->setText(pathPrivateKey);
}

void MainWindow::pressButtonValidateKey() {
    ui->stackedWidget->setCurrentIndex(PageIndex::Chat);
}

void MainWindow::tryLogging() {
    // try logging, while waiting for response send to PageIndex::Loading, put a timeout and if you do log set isLogged to true;
}
