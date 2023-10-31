/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:29:26 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/31 21:44:04 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <termios.h>
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
#include <variant>

volatile sig_atomic_t g_sig = 0;

void	handler(int sig) {
	g_sig = sig;
}

static bool	install_signal_handler(void)
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

static void	exit_procedure(std::shared_ptr<Tintin_reporter>& reporter, int & lock)
{
	if (reporter->log("Quitting.", INFO) != Tintin_reporter::Return::OK) {} 
	if (lock > 0) {
		close(lock);
        unlink("/var/lock/matt_daemon.lock");
    }
}

static bool getCredentials(std::string& username, std::string& password) {
    do {
        std::cout << "Username (between 5 and 128 characters): ";
        std::getline(std::cin, username);
        if (username.size() >= 5 && username.size() <= 128)
            break ;
        else
            std::cout << "Invalid username provided" << std::endl;
    } while (!std::cin.eof());
    if (std::cin.eof()) {
        std::cerr << std::endl << "No username provided, leaving Matt Daemon" << std::endl;
        return false;
    }

    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    do {
        std::cout << "Password (between 12 and 128 characters): ";
        std::getline(std::cin, password);
        if (password.size() >= 12 && password.size() <= 128)
            break ;
        else
            std::cout << "Invalid password provided" << std::endl;

    } while (!std::cin.eof());
    if (std::cin.eof()) {
        std::cerr << std::endl << "No password provided, leaving Matt Daemon" << std::endl;
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return false;
    }
    std::cout << std::endl;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return true;
}

static void displayHelp(char** av) {
	std::cout << "Usage: " << std::endl << av[0] << " [OPTION]\n\n";
	std::cout << "Starts a daemon listening on port 4242 which logs every strings sent to it.";
	std::cout << " Options allow you to start a more secure server on port 4343";
    std::cout << " using a hybrid encryption system: RSA for key exchange and AES256GCM";
    std::cout << " for encrypted communication thereafter.";
    std::cout << " This secure port enforces authentication and requires you to set a username and password\n";
	std::cout << "Valid options:\n";
	std::cout << "  -h: Display this help page\n";
	std::cout << "  -s: Start a secure server on port 4343 in addition to the standard server running on port 4242\n";
	std::cout << "  -x: Start the secure server on port 4343 only and do not start the standard one on port 4242.";
	std::cout << " This option is not compatible with -s, only one of these options should be used." << std::endl;
}

static std::variant<Server::ServerType, bool> parseOpt(int ac, char** av) {
	int opt;
    Server::ServerType  serverType = Server::ServerType::STANDARD;

	while ((opt = getopt(ac, av, "hsx")) != -1)
	{
		switch (opt) {
			case 'h':
                displayHelp(av);
				return true;
				break ;
			case 's':
				if (serverType == Server::ServerType::SECURE_ONLY)
				{
					std::cerr << "Cannot combine option -s with option -x. For more information display help with " << av[0] << " -h" << std::endl;
				   return false;
				}
				serverType = Server::ServerType::SECURE;
				break ;
			case 'x':
				if (serverType == Server::ServerType::SECURE)
				{
					std::cerr << "Cannot combine option -s with option -x. For more information display help with " << av[0] << " -h" << std::endl;
					return false;
				}
				serverType = Server::ServerType::SECURE_ONLY;
				break ;
			default:
				std::cerr << "Unkown option provided. Please see " << av[0] << " -h for more information on usage" << std::endl; 
				return false;
		}
	}
    return serverType;
}

static bool log(std::shared_ptr<Tintin_reporter>& reporter, const char* message, enum Loglevel level) {
	switch (reporter->log(message, level)) {
		case Tintin_reporter::Return::NO_FILE:
			std::cerr << "File is not opened" << std::endl;
			return false;
			break ;
		case Tintin_reporter::Return::FILE_REMOVED:
			std::cerr << "Log file has been removed" << std::endl;
			return false;
			break ;
		case Tintin_reporter::Return::SYSTEM_ERROR:
			std::cerr << "System error" << std::endl;
			return false;
			break ;
		[[likely]] default:
            return true;
	}
}

static std::variant<int, bool>  lockFile(std::shared_ptr<Tintin_reporter>& reporter) {
    int lock = open("/var/lock/matt_daemon.lock", O_CREAT | O_RDWR | O_CLOEXEC, 0600);
    if (lock <= 0)
	{
		std::cerr << "Could not open lockfile /var/lock/matt_daemon.lock" << std::endl;
		log(reporter, "Error, could no open lockfile /var/lock/matt_daemon.lock.", ERROR);
		exit_procedure(reporter, lock);
		return false;
	}
	if (flock(lock, LOCK_EX | LOCK_NB) == -1)
	{
		std::cerr << "Could not lock /var/lock/matt_daemon.lock" << std::endl;
		log(reporter, "Error file locked.", ERROR);
        close(lock);
        lock = 0;
		exit_procedure(reporter, lock);
		return false;
	}
    return lock;
}

int	main(int ac, char **av)
{
    Server::ServerType                  serverType;
	int									lock;
	std::shared_ptr<Tintin_reporter>	reporter;
    std::string                         username;
    std::string                         password;

    //Verify if user is root
	if (getuid() != 0) {
		std::cerr << "Matt_daemon requires to be started as root" << std::endl;
		return 1;
	}

    std::variant<Server::ServerType, bool>  variant = parseOpt(ac, av);
    if (std::holds_alternative<bool>(variant))
        return (std::get<bool>(variant) == true ? 0 : 1);
    serverType = std::get<Server::ServerType>(variant);

    if (serverType != Server::ServerType::STANDARD) {
        std::cout << "Secure server requires a username and a password in order for users to connect" << std::endl;
        if(!getCredentials(username, password))
            return 1;
    }

	try {
		reporter = std::make_shared<Tintin_reporter, const char*>("/var/log/matt_daemon/matt_daemon.log");
	}
	catch (std::exception const & e) {
		std::cerr << "Could not create reporter: " << e.what() << std::endl;
		return 1;
	}

    if (!log(reporter, "Started.", INFO))
        return 1;

    std::variant<int, bool> lockVar = lockFile(reporter);
    if (std::holds_alternative<bool>(lockVar))
        return 1;
    lock = std::get<int>(lockVar);

	if (!install_signal_handler())
	{
		std::cerr << "Could not install signal handlers" << std::endl;
		log(reporter, "Could not install signal handlers", ERROR);
		exit_procedure(reporter, lock);
		return 1;
	}

    if (!log(reporter, "Creating server.", INFO)) {
        exit_procedure(reporter, lock);
        return 1;
    }

	Server server = Server(reporter);
	if (!server.create_server(serverType, username, password))
	{
		std::cerr << "Could not create server" << std::endl;
		log(reporter, "Could not create server", ERROR);
		exit_procedure(reporter, lock);
		return 1;
	}
    if (!log(reporter, "Server created.", INFO)) {
        exit_procedure(reporter, lock);
        return 1;
    }

	server.serve();
	exit_procedure(reporter, lock);

	return (0);
}
