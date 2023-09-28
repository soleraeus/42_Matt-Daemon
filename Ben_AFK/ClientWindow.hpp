/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientWindow.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 19:19:01 by tnaton            #+#    #+#             */
/*   Updated: 2023/09/28 21:15:13 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <QMainWindow>
#include <QtWidgets/QLabel>
#include <QPushButton>
#include <QLineEdit>

class ClientWindow : public QMainWindow
{
	Q_OBJECT
	public:
		explicit ClientWindow(QWidget *parent = nullptr);
		~ClientWindow();

	private slots:
		void clickSecured(void);
		void clickStandard(void);

	private:
		QPushButton	secured;
		QPushButton	standard;
		QLineEdit	line;
		void openChat(void);
};
