/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientWindow.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 19:19:01 by tnaton            #+#    #+#             */
/*   Updated: 2023/10/06 20:37:09 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <QMainWindow>
#include <QtWidgets/QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTcpSocket>

class ClientWindow : public QMainWindow
{
	Q_OBJECT
	public:
		explicit ClientWindow(QWidget *parent = nullptr);
		~ClientWindow();

	private slots:
		void clickSecured(void);
		void clickStandard(void);
		void print(void);


	private:
		QPushButton	secured;
		QPushButton	standard;
		QLineEdit	line;
		QTcpSocket	sock;
		void openChat(void);
};
