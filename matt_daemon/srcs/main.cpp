/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:29:26 by bdetune           #+#    #+#             */
/*   Updated: 2023/11/03 19:29:26 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//std
#include <csignal>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <tuple>
#include <variant>

//C
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

//Unix
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>

//Project specific
#include "Matt_daemon.hpp"
#include "Server.hpp"
#include "Tintin_reporter.hpp"

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
		return false;
	if (sigaddset(&set, SIGTERM) || sigaddset(&set, SIGHUP)
		|| sigaddset(&set, SIGINT) || sigaddset(&set, SIGQUIT)
		|| sigaddset(&set, SIGUSR1) || sigaddset(&set, SIGUSR2)
		|| sigaddset(&set, SIGTSTP))
	{
		return false;
	}
	act.sa_handler = handler;
	act.sa_mask = set;
	if (sigaction(SIGTERM, &act, NULL) || sigaction(SIGHUP, &act, NULL)
		|| sigaction(SIGINT, &act, NULL) || sigaction(SIGQUIT, &act, NULL)
		|| sigaction(SIGUSR1, &act, NULL) || sigaction(SIGUSR2, &act, NULL)
		|| sigaction(SIGTSTP, &act, NULL))
	{
		return false;
	}
	return true;
}

static void	exit_procedure(std::shared_ptr<Tintin_reporter>& reporter, int & lock)
{
	if (reporter->log("Quitting.", Tintin_reporter::Loglevel::INFO) != Tintin_reporter::Return::OK) {} 
	if (lock > 0) {
		close(lock);
        unlink(MATT_DAEMON_LOCKFILE_PATH);
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

static bool log(std::shared_ptr<Tintin_reporter>& reporter, const char* message, Tintin_reporter::Loglevel level) {
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
    int lock = open(MATT_DAEMON_LOCKFILE_PATH, O_CREAT | O_RDWR | O_CLOEXEC, 0600);
    if (lock <= 0)
	{
		std::cerr << "Could not open lockfile " << MATT_DAEMON_LOCKFILE_PATH << std::endl;
		log(reporter, "Error could not open lockfile.", Tintin_reporter::Loglevel::ERROR);
		exit_procedure(reporter, lock);
		return false;
	}
	if (flock(lock, LOCK_EX | LOCK_NB) == -1)
	{
		std::cerr << "Could not lock " << MATT_DAEMON_LOCKFILE_PATH << std::endl;
		log(reporter, "Error file locked.", Tintin_reporter::Loglevel::ERROR);
        close(lock);
        lock = 0;
		exit_procedure(reporter, lock);
		return false;
	}
    return lock;
}

std::tuple<bool, bool, int> daemonize(std::shared_ptr<Tintin_reporter>& reporter) {
    pid_t   pid;
    int     fds[2];

    if (pipe(fds) == -1) {
        std::cerr << "Could not enter daemon mode." << std::endl;
        log(reporter, "Could not enter deamon mode.", Tintin_reporter::Loglevel::ERROR);
        return std::tuple<bool, bool, int>(true, true, 1);
    }
    pid = fork();
    if (pid == -1) {
        std::cerr << "Could not enter daemon mode." << std::endl;
        log(reporter, "Could not enter deamon mode.", Tintin_reporter::Loglevel::ERROR);
        close(fds[0]);
        close(fds[1]);
        return std::tuple<bool, bool, int>(true, true, 1);
    }
    if (pid > 0) {
        char    msg[PIPE_BUF];
        
        close(fds[1]);
        ssize_t ret = read(fds[0], msg, PIPE_BUF);
        close(fds[0]);
        if (ret <= 0) {
            std::cerr << "Could not read from pipe." << std::endl;
            return std::tuple<bool, bool, int>(false, true, 1);
        }
        else {
            msg[ret] = '\0';
            std::cout << msg << std::endl;
            if (strncmp("ERROR", msg, 5) == 0)
                return std::tuple<bool, bool, int>(false, true, 1);
            else
                return std::tuple<bool, bool, int>(false, true, 0);
        }
    }
    else {
        close(fds[0]);
        pid = setsid();
        if (pid == -1) {
            log(reporter, "Could not enter deamon mode.", Tintin_reporter::Loglevel::ERROR);
            if (write(fds[1], "ERROR: Could not setsid", strlen("ERROR: Could not setsid"))) {}
            close(fds[1]);
            return std::tuple<bool, bool, int>(true, true, 1); 
        }
        pid = fork();
        if (pid == -1) {
            log(reporter, "Could not enter deamon mode.", Tintin_reporter::Loglevel::ERROR);
            if (write(fds[1], "ERROR: Could not fork", strlen("ERROR: Could not fork"))) {}
            close(fds[1]);
            return std::tuple<bool, bool, int>(true, true, 1); 
        }
        if (pid > 0) {
            close(fds[1]);
            return std::tuple<bool, bool, int>(false, true, 0); 
        }
        int devNull = open("/dev/null", O_RDWR);
        if (devNull <= 0) {
            log(reporter, "Could not enter deamon mode.", Tintin_reporter::Loglevel::ERROR);
            if (write(fds[1], "ERROR: Could not open /dev/null", strlen("ERROR: Could not open /dev/null"))) {}
            close(fds[1]);
            return std::tuple<bool, bool, int>(true, true, 1); 
        } 
        dup2(devNull, 0);
        dup2(devNull, 1);
        dup2(devNull, 2);
        close(devNull);
        pid = getpid();

        std::string info = "INFO: started. PID: ";
        info += std::to_string(pid);
        info += ".";
        if (write(fds[1], info.data(), info.size())) {}
        close(fds[1]);
        log(reporter, info.c_str(), Tintin_reporter::Loglevel::INFO);
        return std::tuple<bool, bool, int>(false, false, 0); 
    }
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
		reporter = std::make_shared<Tintin_reporter, const char*>(MATT_DAEMON_LOGFILE_PATH);
	}
	catch (std::exception const & e) {
		std::cerr << "Could not create reporter: " << e.what() << std::endl;
		return 1;
	}

    if (!log(reporter, "Started.", Tintin_reporter::Loglevel::INFO))
        return 1;

    std::variant<int, bool> lockVar = lockFile(reporter);
    if (std::holds_alternative<bool>(lockVar))
        return 1;
    lock = std::get<int>(lockVar);

	if (!install_signal_handler())
	{
		std::cerr << "Could not install signal handlers" << std::endl;
		log(reporter, "Could not install signal handlers", Tintin_reporter::Loglevel::ERROR);
		exit_procedure(reporter, lock);
		return 1;
	}

    if (!log(reporter, "Creating server.", Tintin_reporter::Loglevel::INFO)) {
        exit_procedure(reporter, lock);
        return 1;
    }

	Server server = Server(reporter);
	if (!server.create_server(serverType, username, password))
	{
		std::cerr << "Could not create server" << std::endl;
		log(reporter, "Could not create server", Tintin_reporter::Loglevel::ERROR);
		exit_procedure(reporter, lock);
		return 1;
	}
    if (!log(reporter, "Server created.", Tintin_reporter::Loglevel::INFO)) {
        exit_procedure(reporter, lock);
        return 1;
    }

    log(reporter, "Entering Daemon mode.", Tintin_reporter::Loglevel::INFO);
    bool    exitProcedure;
    bool    exitProcess;
    int     exitStatus;

    //Since auto is a monstruosity that destroys the beauty of a strongly typed language such as C++
    //I refuse to use structured binding
    std::tie(exitProcedure, exitProcess, exitStatus) = daemonize(reporter);
    if (exitProcedure)
    {
        exit_procedure(reporter, lock);
        return exitStatus;
    }
    else if (exitProcess) {
        close(lock);
        return exitStatus;
    }

	server.serve();
	exit_procedure(reporter, lock);
    close(0);
    close(1);
    close(2);
	return 0;
}
