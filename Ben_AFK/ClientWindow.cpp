/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientWindow.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 19:23:09 by tnaton            #+#    #+#             */
/*   Updated: 2023/10/06 20:36:50 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientWindow.hpp"

ClientWindow::ClientWindow(QWidget *parent) : QMainWindow(parent), secured("secured", this), standard("standard", this), line(this), sock(this) {
	this->line.hide();
	this->secured.setGeometry(this->standard.geometry().right(), this->standard.y(), this->standard.width(), this->standard.height());
	this->setFixedSize(this->secured.geometry().right(), this->secured.height());
	connect(&this->standard, SIGNAL (clicked()), this, SLOT (clickStandard()));
	connect(&this->secured, SIGNAL (clicked()), this, SLOT (clickSecured()));
}

ClientWindow::~ClientWindow() {
}

void ClientWindow::clickSecured(void) {
	this->openChat();
}

void ClientWindow::print(void) {
	std::cout << "CONNECTED" << std::endl;
}

void ClientWindow::clickStandard(void) {
	this->sock.connectToHost("10.0.2.2", 4343);
	connect(&this->sock, SIGNAL (connected()), this, SLOT (print()));
	this->openChat();
}

void ClientWindow::openChat(void) {
	this->standard.hide();
	this->secured.hide();
	this->setFixedSize(500, 500);
	this->line.setGeometry(0, 500 - 30, 500, 30);
	this->line.show();
}
