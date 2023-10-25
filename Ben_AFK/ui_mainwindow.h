/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QStackedWidget *stackedWidget;
    QWidget *pageChoose;
    QGridLayout *gridLayout_4;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QPushButton *buttonStandard;
    QPushButton *buttonSecured;
    QLineEdit *lineIp;
    QWidget *pageLoading;
    QGridLayout *gridLayout_5;
    QLabel *labelMovie;
    QWidget *pageClient;
    QGridLayout *gridLayout_3;
    QTabWidget *tabWidget;
    QWidget *tabChat;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *lineChat;
    QPushButton *buttonSend;
    QWidget *tabLog;
    QGridLayout *gridLayout_2;
    QStackedWidget *stackedWidgetLog;
    QWidget *page;
    QGridLayout *gridLayout_6;
    QTextBrowser *textLog;
    QWidget *page_2;
    QGridLayout *gridLayout_8;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_7;
    QLineEdit *lineUsername;
    QLineEdit *linePassword;
    QPushButton *buttonSubmit;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(361, 266);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName("gridLayout");
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        pageChoose = new QWidget();
        pageChoose->setObjectName("pageChoose");
        gridLayout_4 = new QGridLayout(pageChoose);
        gridLayout_4->setObjectName("gridLayout_4");
        frame = new QFrame(pageChoose);
        frame->setObjectName("frame");
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setObjectName("horizontalLayout");
        buttonStandard = new QPushButton(frame);
        buttonStandard->setObjectName("buttonStandard");
        buttonStandard->setMaximumSize(QSize(16777215, 27));

        horizontalLayout->addWidget(buttonStandard);

        buttonSecured = new QPushButton(frame);
        buttonSecured->setObjectName("buttonSecured");

        horizontalLayout->addWidget(buttonSecured);


        gridLayout_4->addWidget(frame, 1, 0, 1, 1);

        lineIp = new QLineEdit(pageChoose);
        lineIp->setObjectName("lineIp");

        gridLayout_4->addWidget(lineIp, 0, 0, 1, 1);

        stackedWidget->addWidget(pageChoose);
        pageLoading = new QWidget();
        pageLoading->setObjectName("pageLoading");
        gridLayout_5 = new QGridLayout(pageLoading);
        gridLayout_5->setObjectName("gridLayout_5");
        labelMovie = new QLabel(pageLoading);
        labelMovie->setObjectName("labelMovie");

        gridLayout_5->addWidget(labelMovie, 0, 0, 1, 1);

        stackedWidget->addWidget(pageLoading);
        pageClient = new QWidget();
        pageClient->setObjectName("pageClient");
        gridLayout_3 = new QGridLayout(pageClient);
        gridLayout_3->setObjectName("gridLayout_3");
        tabWidget = new QTabWidget(pageClient);
        tabWidget->setObjectName("tabWidget");
        tabChat = new QWidget();
        tabChat->setObjectName("tabChat");
        horizontalLayout_2 = new QHBoxLayout(tabChat);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        lineChat = new QLineEdit(tabChat);
        lineChat->setObjectName("lineChat");

        horizontalLayout_2->addWidget(lineChat);

        buttonSend = new QPushButton(tabChat);
        buttonSend->setObjectName("buttonSend");

        horizontalLayout_2->addWidget(buttonSend);

        tabWidget->addTab(tabChat, QString());
        tabLog = new QWidget();
        tabLog->setObjectName("tabLog");
        gridLayout_2 = new QGridLayout(tabLog);
        gridLayout_2->setObjectName("gridLayout_2");
        stackedWidgetLog = new QStackedWidget(tabLog);
        stackedWidgetLog->setObjectName("stackedWidgetLog");
        page = new QWidget();
        page->setObjectName("page");
        gridLayout_6 = new QGridLayout(page);
        gridLayout_6->setObjectName("gridLayout_6");
        textLog = new QTextBrowser(page);
        textLog->setObjectName("textLog");

        gridLayout_6->addWidget(textLog, 0, 0, 1, 1);

        stackedWidgetLog->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        gridLayout_8 = new QGridLayout(page_2);
        gridLayout_8->setObjectName("gridLayout_8");
        groupBox = new QGroupBox(page_2);
        groupBox->setObjectName("groupBox");
        gridLayout_7 = new QGridLayout(groupBox);
        gridLayout_7->setObjectName("gridLayout_7");
        lineUsername = new QLineEdit(groupBox);
        lineUsername->setObjectName("lineUsername");

        gridLayout_7->addWidget(lineUsername, 0, 0, 1, 1);

        linePassword = new QLineEdit(groupBox);
        linePassword->setObjectName("linePassword");

        gridLayout_7->addWidget(linePassword, 1, 0, 1, 1);


        gridLayout_8->addWidget(groupBox, 0, 0, 1, 1);

        buttonSubmit = new QPushButton(page_2);
        buttonSubmit->setObjectName("buttonSubmit");

        gridLayout_8->addWidget(buttonSubmit, 0, 1, 1, 1);

        stackedWidgetLog->addWidget(page_2);

        gridLayout_2->addWidget(stackedWidgetLog, 0, 0, 1, 1);

        tabWidget->addTab(tabLog, QString());

        gridLayout_3->addWidget(tabWidget, 0, 0, 1, 1);

        stackedWidget->addWidget(pageClient);

        gridLayout->addWidget(stackedWidget, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(0);
        tabWidget->setCurrentIndex(0);
        stackedWidgetLog->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        buttonStandard->setText(QCoreApplication::translate("MainWindow", "standard", nullptr));
        buttonSecured->setText(QCoreApplication::translate("MainWindow", "secured", nullptr));
        lineIp->setText(QCoreApplication::translate("MainWindow", "localhost", nullptr));
        labelMovie->setText(QCoreApplication::translate("MainWindow", "                                                         LOADING", nullptr));
        buttonSend->setText(QCoreApplication::translate("MainWindow", "send", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabChat), QCoreApplication::translate("MainWindow", "chat", nullptr));
        textLog->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Cantarell'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p></body></html>", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "login", nullptr));
        buttonSubmit->setText(QCoreApplication::translate("MainWindow", "submit", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabLog), QCoreApplication::translate("MainWindow", "log", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
