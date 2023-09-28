/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientWindow.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 19:23:09 by tnaton            #+#    #+#             */
/*   Updated: 2023/09/28 21:19:47 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientWindow.hpp"

ClientWindow::ClientWindow(QWidget *parent) : QMainWindow(parent), secured("secured", this), standard("standard", this), line(this) {
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

void ClientWindow::clickStandard(void) {
	this->openChat();
}

void ClientWindow::openChat(void) {
	this->standard.hide();
	this->secured.hide();
	this->setFixedSize(500, 500);
	this->line.setGeometry(0, 500 - 30, 500, 30);
	this->line.show();
}
