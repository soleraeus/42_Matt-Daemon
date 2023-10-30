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
	ui->lineUsername_2->setPlaceholderText("username");
	ui->linePassword_2->setPlaceholderText("password");

    connect(ui->buttonStandard, &QPushButton::clicked, this, &MainWindow::standard);
    connect(ui->buttonSecured, &QPushButton::clicked, this, &MainWindow::secured);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::changeTab);
    connect(ui->buttonSubmit_2, &QPushButton::clicked, this, &MainWindow::tryLogging);
}

MainWindow::~MainWindow()
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
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

int getPacketSize(QByteArray & buf)
{
    size_t		pos = 0;

    size_t		packetsize = 0;
    for (size_t i = 0; i < (size_t)buf.size(); ++i)
    {
        if (buf[i] == '\n')
        {
            if (i < 8 || i > 12) {
				qDebug() << "i is not between 8 and 12";
                return -1;
			}
            if (memcmp((void *)"Length ",(void *)&buf[0], 7)) {
				qDebug() << "buf doesnt start with \"Length \"";
                return -1;
			}
            pos = i;
            if (buf[i - 1] == '\r')
                pos = i - 1;
            if (pos == 7) {
				qDebug() << "Invalid position (?)";
                return -1;
			}
            buf[pos] = '\0';
            for (size_t j = 7; j < pos; ++j)
            {
                if (buf[j] < '0' || buf[j] > '9') {
					qDebug() << "Non numerical in length";
                    return -1;
				}
            }
            packetsize = std::atoi(&buf[7]);
            if (packetsize > PIPE_BUF + 128 + 16 + 16) {
				qDebug() << "Size too large";
                return -1;
			}
            buf.erase(buf.begin(), buf.begin() + i + 1);
			qDebug() << "Post erase";
			qDebug() << buf;
            return packetsize;
        }
        if ((buf[i] < ' ' || buf[i] > 'z')
                        && buf[i] != '\r') {
			qDebug() << "idk but error";
            return -1;
        }
    }
	qDebug() << "buf.size() = " << buf.size();
    if (buf.size() > 12) {
		qDebug() << "Size too big without \\n";
        return -1;
	}
    return packetsize;
}

void MainWindow::Handshake(void) {
	static int status = HandshakeState::SendRSA;

	qDebug() << "MainWindow::Handshake()";
	qDebug() << "status = " << status;
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
				QByteArray buf = socket->readAll();
				qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
				qDebug() << buf;
				qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
				int packetSize = getPacketSize(buf);
				if (packetSize == -1) {
					qDebug() << "Invalid packet ...";
				} else {
					qDebug() << "got packet size...";
				}
				qDebug() << buf;
				if (secured_client->decryptAESKey(buf)) {
					qDebug() << "Decrypted AES Key !";
					status = Done;
				} else {
					qDebug() << "Failed to decrypt AES Key ...";
					ui->stackedWidget->setCurrentIndex(PageIndex::Choice);
					status = SendRSA;
					if (socket)
						socket->disconnectFromHost();
					return ;
				}
			} else {
				qDebug() << "Timed out ...";
				ui->stackedWidget->setCurrentIndex(PageIndex::Choice);
				status = SendRSA;
				if (socket)
					socket->disconnectFromHost();
				return ;
			}
			break ;
	}

	if (status != Done) {
		qDebug() << "Calling Handshake again";
		QMetaObject::invokeMethod(this, "Handshake");
	} else {
        ui->stackedWidget->setCurrentIndex(PageIndex::Auth);
		status = SendRSA;
	}
}

void MainWindow::onConnect() {
    qDebug() << "socket is connected !";
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnect);
	disconnect(socket, &QTcpSocket::connected, this, &MainWindow::onConnect);
    if (isSecure) {
		connect(ui->lineChat, &QLineEdit::returnPressed, this, &MainWindow::sendLine);
		connect(ui->buttonSend, &QPushButton::clicked, this, &MainWindow::sendLine);
		secured_client = new SecuredClient();
		qDebug() << "Starting Handshake";
		this->Handshake();
        // Do the thing to secure the connection
    } else {
        ui->stackedWidget->setCurrentIndex(PageIndex::Chat);
		connect(ui->lineChat, &QLineEdit::returnPressed, this, &MainWindow::sendLine);
		connect(ui->buttonSend, &QPushButton::clicked, this, &MainWindow::sendLine);
    }
}

void MainWindow::onDisconnect() {
    qDebug() << "socket disconnected !";
	disconnect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnect);
    disconnect(ui->lineChat, &QLineEdit::returnPressed, this, &MainWindow::sendLine);
    disconnect(ui->buttonSend, &QPushButton::clicked, this, &MainWindow::sendLine);
    ui->stackedWidget->setCurrentIndex(PageIndex::Choice);
    socket->deleteLater();
    socket = NULL;
	qDebug() << "removed socket";
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
    if (!socket || socket->state() != QAbstractSocket::SocketState::ConnectedState) {
		delete socket;
		socket = NULL;
        ui->stackedWidget->setCurrentIndex(PageIndex::Choice);
    }
}

void MainWindow::sendLine() {
    if (isSecure) {
		qDebug() << "In secure mode";
		QByteArray msg = ui->lineChat->text().toUtf8() + "\n";
		qDebug() << "~~~~~~~~~~~~~~~~~";
		qDebug() << msg;
		qDebug() << "Will encrypt msg";
		msg = secured_client->encrypt(msg);
		qDebug() << "~~~~~~~~~~~~~~~~~";
		qDebug() << msg;
		qDebug() << "Will send encrypted msg";
		std::string header = "Length ";
		header += std::to_string(msg.size());
		header += "\n";
		msg.insert(0, header.data(), header.size());
		socket->write(msg);
        // Encrypt the message
    } else {
        socket->write(ui->lineChat->text().toUtf8() + "\n");
    }
    ui->lineChat->clear();
}

enum AuthState {
	SendInfo,
	WaitResult,
	Authentified,
};

void MainWindow::Auth() {
	static int status = AuthState::SendInfo;

	qDebug() << "MainWindow::Auth()";
	qDebug() << "status = " << status;
	switch (status) {
		case SendInfo : {
			QByteArray msg = ui->lineUsername_2->text().toUtf8() + "\n" + ui->linePassword_2->text().toUtf8() + "\n";
			msg = secured_client->encrypt(msg);
			std::string header = "Length ";
			header += std::to_string(msg.size());
			header += "\n";
			msg.insert(0, header.data(), header.size());
			qDebug() << "Sending info";
			socket->write(msg);
			status = WaitResult;
			break;
		}

		case WaitResult : {
			qDebug() << "Waiting for result";
			if (socket->waitForReadyRead(1000)) {
				qDebug() << "Got a response from server";
				QByteArray buf = socket->readAll();
				int packetSize = getPacketSize(buf);
				if (packetSize == -1) {
					qDebug() << "Invalid packet ...";
				}
				buf = secured_client->decrypt(buf, packetSize);
				qDebug() << buf;
				if (buf == "AUTH KO\n") {
					qDebug() << "Auth KO ...";
					ui->stackedWidget->setCurrentIndex(PageIndex::Auth);
					status = SendInfo;
					return ;
				} else if (buf == "AUTH KO - TOO MANY TRIES\n") {
					qDebug() << "Auth kicked ...";
					ui->stackedWidget->setCurrentIndex(PageIndex::Choice);
					status = SendInfo;
					if (socket)
						socket->disconnectFromHost();
					return ;
				} else if (buf == "AUTH OK\n") {
					this->isLogged = true;
					status = Authentified;
				}
			} else {
				qDebug() << "Timed out ...";
				ui->stackedWidget->setCurrentIndex(PageIndex::Choice);
				status = SendInfo;
				if (socket)
					socket->disconnectFromHost();
				return ;
			}
			break;
		}
	}

	if (status != Authentified) {
		qDebug() << "Calling Auth again";
		QMetaObject::invokeMethod(this, "Auth");
	} else {
		ui->stackedWidget->setCurrentIndex(PageIndex::Chat);
		status = SendInfo;
	}
}

void MainWindow::tryLogging() {
    // try logging, while waiting for response send to PageIndex::Loading, put a timeout and if you do log set isLogged to true;
	this->Auth();
}
