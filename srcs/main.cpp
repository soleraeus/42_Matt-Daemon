/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:29:26 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/26 19:23:26 by bdetune          ###   ########.fr       */
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

int	main(int ac, char **av)
{
	bool								secure = false;
	bool								secure_only = false;
	int									lock;
	int									opt;
	std::shared_ptr<Tintin_reporter>	reporter;
    std::string                         username;
    std::string                         password;

	if (getuid() != 0)
	{
		std::cerr << "Matt_daemon requires to be started as root" << std::endl;
		return (1);
	}
	while ((opt = getopt(ac, av, "hsx")) != -1)
	{
		switch (opt) {
			case 'h':
				std::cout << "Usage: " << std::endl << av[0] << " [OPTION]" << std::endl << std::endl;
				std::cout << "Starts a daemon listening on port 4242 which logs every strings sent to it.";
				std::cout << " Options allow you to start a more secure server on port 4343 using aes256 for encrypted communication" << std::endl;
				std::cout << "Valid options: " << std::endl;
				std::cout << "  -h: Display this help page" << std::endl;
				std::cout << "  -s: Start a secure server on port 4343 which relies on aes256 encryption" << std::endl;
				std::cout << "  -x: Start the secure server only and do not start the standard one on port 4242.";
				std::cout << " This option is not compatible with -s." << std::endl;
				return (0);
				break ;
			case 's':
				if (secure_only)
				{
					std::cerr << "Cannot combine option -s with option -x. For more information display help with " << av[0] << " -h" << std::endl;
				   return (1);	
				}
				secure = true;
				break ;
			case 'x':
				if (secure)
				{
					std::cerr << "Cannot combine option -s with option -x. For more information display help with " << av[0] << " -h" << std::endl;
					return (1);
				}
				secure_only = true;
				break ;
			default:
				std::cerr << "Unkown option provided. Please see " << av[0] << " -h for more information on usage" << std::endl; 
				return (1);
		}
	}
    if (secure || secure_only) {
        std::cout << "Secure server requires a username and a password in order for users to connect" << std::endl;
        do {
            std::cout << "Username (between 5 and 128 characters): ";
            std::getline(std::cin, username);
            if (username.size() >= 5 && username.size() <= 128)
                break ;
            else
                std::cout << "Invalid username provided" << std::endl;
        } while (!std::cin.eof());
        if (std::cin.eof()) {
            std::cerr << "No username provided, leaving Matt Daemon" << std::endl;
            return 1;
        }
        std::cout << std::endl;
        do {
            std::cout << "Password (between 12 and 128 characters): ";
            std::getline(std::cin, password);
            if (password.size() >= 12 && password.size() <= 128)
                break ;
            else
                std::cout << "Invalid password provided" << std::endl;

        } while (!std::cin.eof());
        if (std::cin.eof()) {
            std::cerr << "No password provided, leaving Matt Daemon" << std::endl;
            return 1;
        }
        std::cout << std::endl;
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
	if (!server.create_server(secure ? Server::ServerType::SECURE : (secure_only ? Server::ServerType::SECURE_ONLY : Server::ServerType::STANDARD)))
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
