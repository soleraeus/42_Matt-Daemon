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
	secured_client = NULL;
    ui->textLog->setPlaceholderText("<LOG>");
    ui->lineChat->setPlaceholderText("write here");

    connect(ui->buttonStandard, &QPushButton::clicked, this, &MainWindow::standard);
    connect(ui->buttonSecured, &QPushButton::clicked, this, &MainWindow::secured);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::changeTab);
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

enum HandshakeState {
	SendRSA,
	WaitAES,
	Done,
};

char *getPacketSize(std::string buf)
{
    size_t		pos = 0;

    size_t		packetsize = 0;
    for (size_t i = 0; i < buf.size(); ++i)
    {
        if (buf[i] == '\n')
        {
            if (i < 8 || i > 12)
                return NULL;
            if (memcmp((void *)"Length ",(void *)&buf[0], 7))
                return NULL;
            pos = i;
            if (buf[i - 1] == '\r')
                pos = i - 1;
            if (pos == 7)
                return NULL;
            buf[pos] = '\0';
            for (size_t j = 7; j < pos; ++j)
            {
                if (buf[j] < '0' || buf[j] > '9')
                    return NULL;
            }
            packetsize = 0;//std::atoi(buf[7]);
            if (packetsize > PIPE_BUF + 128 + 16 + 16)
                return NULL;
            buf.erase(buf.begin(), buf.begin() + i + 1);
            return NULL;
        }
        if ((buf[i] < ' ' || buf[i] > 'z')
                        && buf[i] != '\r') {
            return NULL;
        }
    }
    if (buf.size() > 12)
        return NULL;
    return buf.data();
}

void MainWindow::Handshake(void) {
	static int status = HandshakeState::SendRSA;

	switch (status) { 
		case SendRSA :
			qDebug() << "Sending pubkey";
			socket->write(secured_client->encryptRSAKey().data());
			status = WaitAES;
			break ;
		
		case WaitAES : 
			qDebug() << "Waiting for AES";
			if (socket->waitForReadyRead(1000)) {
				qDebug() << "Got a response from server";
				char *buf = socket->readAll().data();
				buf += 11;
				qDebug() << "--------------";
				qDebug() << buf;
				qDebug() << "--------------";
				std::string tmp;
				tmp.assign(buf, 512);
				if (secured_client->decryptAESKey(tmp)) {
					qDebug() << "Decrypted AES Key !";
					status = Done;
				} else {
					qDebug() << "Failed to decrypt AES Key ...";
					ui->stackedWidget->setCurrentIndex(PageIndex::Choice);
					return ;
				}
			} else {
				qDebug() << "Timed out ...";
				ui->stackedWidget->setCurrentIndex(PageIndex::Choice);
				return ;
			}
			break ;
	}

	if (status != Done) {
		QMetaObject::invokeMethod(this, "Handshake");
	} else {
        ui->stackedWidget->setCurrentIndex(PageIndex::Chat);
	}
}

void MainWindow::onConnect() {
    qDebug() << "socket is connected !";
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnect);
    if (isSecure) {
		secured_client = new SecuredClient();
		qDebug() << "Starting Handshake";
		this->Handshake();
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
		qDebug() << "In secure mode";
		std::string msg = (ui->lineChat->text().toUtf8() + "\n").data();
		qDebug() << "Will encrypt msg";
		msg = secured_client->encrypt(msg);
		qDebug() << "Will send encrypted msg";
		socket->write(msg.data(), msg.size());
        // Encrypt the message
    } else {
        socket->write(ui->lineChat->text().toUtf8() + "\n");
    }
    ui->lineChat->clear();
}

void MainWindow::tryLogging() {
    // try logging, while waiting for response send to PageIndex::Loading, put a timeout and if you do log set isLogged to true;
}
