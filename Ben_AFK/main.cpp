/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 17:48:56 by tnaton            #+#    #+#             */
/*   Updated: 2023/09/28 19:53:35 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientWindow.hpp"
#include <QtGui>
#include <QApplication>
#include <QLabel>

int main(int ac, char **av) {
	QApplication ClientApp(ac, av);
	ClientWindow client;
	client.show();
	return ClientApp.exec();
}
