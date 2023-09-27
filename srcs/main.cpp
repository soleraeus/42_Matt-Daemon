/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:29:26 by bdetune           #+#    #+#             */
/*   Updated: 2023/09/27 21:33:10 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <iostream>
#include <unistd.h>
#include "Matt_daemon.hpp"
#include "Tintin_reporter.hpp"
#include "Server.hpp"

void	exit_procedure(Tintin_reporter & reporter, int & lock)
{
	reporter.log("Quitting.", INFO);
	if (lock > 0)
		close(lock);
}

int	main(void)
{
	/*if (getuid() != 0)
	{
		std::cerr << "Matt_daemon requires to be started as root" << std::endl;
		return (1);
	}*/
	int				lock;
	Tintin_reporter	reporter("log");
	Server			server;

	reporter.log("Started.", INFO);
	lock = open("/var/lock/matt_daemon.lock", O_CREAT | O_RDWR | O_CLOEXEC, 0600);
	if (lock <= 0)
	{
		std::cerr << "Could not open lockfile /var/lock/matt_daemon.lock" << std::endl;
		reporter.log("Error, could no open lockfile /var/lock/matt_daemon.lock.", ERROR);
		exit_procedure(reporter, lock);
		return (1);
	}
	if (flock(lock, LOCK_EX | LOCK_NB) == -1)
	{
		std::cerr << "Could not lock /var/lock/matt_daemon.lock" << std::endl;
		reporter.log("Error, could not lock /var/lock/matt_daemon.lock.", ERROR);
		exit_procedure(reporter, lock);
		return (1);
	}
	reporter.log("Creating server.", INFO);
	server = Server(reporter);
	if (!server.create_server())
	{
		std::cerr << "Could not create server" << std::endl;
		reporter.log("Could not create server", ERROR);
		exit_procedure(reporter, lock);
		return (1);
	}
	else
	{
		reporter.log("Server created.", INFO);
	}
	exit_procedure(reporter, lock);
	return (0);
}
