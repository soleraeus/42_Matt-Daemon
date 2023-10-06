/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:29:26 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/06 21:11:08 by bdetune          ###   ########.fr       */
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
#include <memory>

volatile sig_atomic_t g_sig = 0;

void	handler(int sig)
{
	g_sig = sig;
}


bool	install_signal_handler(void)
{
	sigset_t			set;
	struct sigaction	act;

	bzero(&act, sizeof(act));
	if (sigemptyset(&set))
		return (false);
	if (sigaddset(&set, SIGTERM) || sigaddset(&set, SIGHUP)
		|| sigaddset(&set, SIGINT) || sigaddset(&set, SIGQUIT)
		|| sigaddset(&set, SIGUSR1) || sigaddset(&set, SIGUSR2)
		|| sigaddset(&set, SIGTSTP))
	{
		return (false);
	}
	act.sa_handler = handler;
	act.sa_mask = set;
	if (sigaction(SIGTERM, &act, NULL) || sigaction(SIGHUP, &act, NULL)
		|| sigaction(SIGINT, &act, NULL) || sigaction(SIGQUIT, &act, NULL)
		|| sigaction(SIGUSR1, &act, NULL) || sigaction(SIGUSR2, &act, NULL)
		|| sigaction(SIGTSTP, &act, NULL))
	{
		return (false);
	}
	return (true);
}

void	exit_procedure(std::shared_ptr<Tintin_reporter>& reporter, int & lock)
{
	if (reporter->log("Quitting.", INFO) != Tintin_reporter::Return::OK) {} 
	if (lock > 0)
		close(lock);
}

int	main(void)
{
	int									lock;
	std::shared_ptr<Tintin_reporter>	reporter;

	if (getuid() != 0)
	{
		std::cerr << "Matt_daemon requires to be started as root" << std::endl;
		return (1);
	}
	try
	{
		reporter = std::make_shared<Tintin_reporter, const char*>("/var/log/matt_daemon/matt_daemon.log");
	}
	catch (std::exception const & e)
	{
		std::cerr << "Could not create reporter: " << e.what() << std::endl;
		return 1;
	}
	switch (reporter->log("Started.", INFO)) {
		case Tintin_reporter::Return::NO_FILE:
			std::cerr << "File is not opened" << std::endl;
			return 1;
			break ;
		case Tintin_reporter::Return::FILE_REMOVED:
			std::cerr << "Log file has been removed" << std::endl;
			return 1;
			break ;
		case Tintin_reporter::Return::SYSTEM_ERROR:
			std::cerr << "System error" << std::endl;
			return 1;
			break ;
		[[likely]] default:
			break ;
	}
	lock = open("/var/lock/matt_daemon.lock", O_CREAT | O_RDWR | O_CLOEXEC, 0600);
	if (lock <= 0)
	{
		std::cerr << "Could not open lockfile /var/lock/matt_daemon.lock" << std::endl;
		if (reporter->log("Error, could no open lockfile /var/lock/matt_daemon.lock.", ERROR) != Tintin_reporter::Return::OK) {}
		exit_procedure(reporter, lock);
		return (1);
	}
	if (flock(lock, LOCK_EX | LOCK_NB) == -1)
	{
		std::cerr << "Could not lock /var/lock/matt_daemon.lock" << std::endl;
		if (reporter->log("Error, could not lock /var/lock/matt_daemon.lock.", ERROR) != Tintin_reporter::Return::OK) {}
		exit_procedure(reporter, lock);
		return (1);
	}
	if (!install_signal_handler())
	{
		std::cerr << "Could not install signal handlers" << std::endl;
		if (reporter->log("Could not install signal handlers", ERROR) != Tintin_reporter::Return::OK) {}
		exit_procedure(reporter, lock);
		return (1);
	}
	switch (reporter->log("Creating server.", INFO)) {
		case Tintin_reporter::Return::NO_FILE:
			std::cerr << "File is not opened" << std::endl;
			return 1;
			break ;
		case Tintin_reporter::Return::FILE_REMOVED:
			std::cerr << "Log file has been removed" << std::endl;
			return 1;
			break ;
		case Tintin_reporter::Return::SYSTEM_ERROR:
			std::cerr << "System error" << std::endl;
			return 1;
			break ;
		[[likely]] default:
			break ;
	}
	Server server = Server(reporter);
	if (!server.create_server())
	{
		std::cerr << "Could not create server" << std::endl;
		if (reporter->log("Could not create server", ERROR) != Tintin_reporter::Return::OK) {}
		exit_procedure(reporter, lock);
		return (1);
	}
	else
	{
		switch (reporter->log("Server created.", INFO)) {
			case Tintin_reporter::Return::NO_FILE:
				std::cerr << "File is not opened" << std::endl;
				return 1;
				break ;
			case Tintin_reporter::Return::FILE_REMOVED:
				std::cerr << "Log file has been removed" << std::endl;
				return 1;
				break ;
			case Tintin_reporter::Return::SYSTEM_ERROR:
				std::cerr << "System error" << std::endl;
				return 1;
				break ;
			[[likely]] default:
				break ;
		}
	}
	server.serve();
	exit_procedure(reporter, lock);
	return (0);
}
