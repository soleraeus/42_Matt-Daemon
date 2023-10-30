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
#include <QtWidgets/QSpacerItem>
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
    QWidget *pageAuth;
    QGridLayout *gridLayout_11;
    QWidget *widget;
    QGridLayout *gridLayout_10;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_9;
    QLineEdit *lineUsername_2;
    QLineEdit *linePassword_2;
    QPushButton *buttonSubmit_2;
    QWidget *pageLoading;
    QGridLayout *gridLayout_5;
    QSpacerItem *horizontalSpacer;
    QLabel *labelMovie;
    QSpacerItem *horizontalSpacer_2;
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

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(377, 251);
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
        pageAuth = new QWidget();
        pageAuth->setObjectName("pageAuth");
        pageAuth->setMinimumSize(QSize(359, 0));
        pageAuth->setMaximumSize(QSize(359, 16777215));
        gridLayout_11 = new QGridLayout(pageAuth);
        gridLayout_11->setObjectName("gridLayout_11");
        widget = new QWidget(pageAuth);
        widget->setObjectName("widget");
        gridLayout_10 = new QGridLayout(widget);
        gridLayout_10->setObjectName("gridLayout_10");
        groupBox_2 = new QGroupBox(widget);
        groupBox_2->setObjectName("groupBox_2");
        gridLayout_9 = new QGridLayout(groupBox_2);
        gridLayout_9->setObjectName("gridLayout_9");
        lineUsername_2 = new QLineEdit(groupBox_2);
        lineUsername_2->setObjectName("lineUsername_2");

        gridLayout_9->addWidget(lineUsername_2, 0, 0, 1, 1);

        linePassword_2 = new QLineEdit(groupBox_2);
        linePassword_2->setObjectName("linePassword_2");

        gridLayout_9->addWidget(linePassword_2, 1, 0, 1, 1);


        gridLayout_10->addWidget(groupBox_2, 0, 0, 1, 1);

        buttonSubmit_2 = new QPushButton(widget);
        buttonSubmit_2->setObjectName("buttonSubmit_2");

        gridLayout_10->addWidget(buttonSubmit_2, 0, 1, 1, 1);


        gridLayout_11->addWidget(widget, 0, 0, 1, 1);

        stackedWidget->addWidget(pageAuth);
        pageLoading = new QWidget();
        pageLoading->setObjectName("pageLoading");
        gridLayout_5 = new QGridLayout(pageLoading);
        gridLayout_5->setObjectName("gridLayout_5");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer, 0, 0, 1, 1);

        labelMovie = new QLabel(pageLoading);
        labelMovie->setObjectName("labelMovie");
        labelMovie->setWordWrap(false);

        gridLayout_5->addWidget(labelMovie, 0, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_2, 0, 2, 1, 1);

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

        gridLayout_2->addWidget(stackedWidgetLog, 0, 0, 1, 1);

        tabWidget->addTab(tabLog, QString());

        gridLayout_3->addWidget(tabWidget, 0, 0, 1, 1);

        stackedWidget->addWidget(pageClient);

        gridLayout->addWidget(stackedWidget, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(0);
        tabWidget->setCurrentIndex(0);
        stackedWidgetLog->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        buttonStandard->setText(QCoreApplication::translate("MainWindow", "standard", nullptr));
        buttonSecured->setText(QCoreApplication::translate("MainWindow", "secured", nullptr));
        lineIp->setText(QCoreApplication::translate("MainWindow", "localhost", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("MainWindow", "login", nullptr));
        buttonSubmit_2->setText(QCoreApplication::translate("MainWindow", "submit", nullptr));
        labelMovie->setText(QCoreApplication::translate("MainWindow", "LOADING", nullptr));
        buttonSend->setText(QCoreApplication::translate("MainWindow", "send", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabChat), QCoreApplication::translate("MainWindow", "chat", nullptr));
        textLog->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Cantarell'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt;\"><br /></p></body></html>", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabLog), QCoreApplication::translate("MainWindow", "log", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
