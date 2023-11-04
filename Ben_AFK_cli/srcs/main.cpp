//std
#include <cstdio>
#include <exception>
#include <iostream>

//C
#include <unistd.h>

//Project specific
#include "Client.hpp"

int	main(int ac, char **av)
{
	Client			client;
	bool			secure = false;

	if (ac < 2)
	{
		std::cerr << "Usage: `" << av[0] << " [OPTION] IP`" << std::endl;
		std::cerr << "`" << av[0] << " -h` to display help" << std::endl;
	}
	int	opt;
	while ((opt = getopt(ac, av, ":hs")) != -1)
	{
		switch (opt) {
			case 'h':
				std::cout << "Usage: " << std::endl << av[0] << " [OPTION] IP" << std::endl << std::endl;
				std::cout << "Connects to the Matt_daemon service running on port 4242 at the specified IP address. Messages sent are logged on the machine running the service. Contact the administrator for more details" << std::endl;
				std::cout << "Options allow you to connect to a more secure authenticated version of the service on port 4343 using a hybrid handshake and aes256 for encrypted communication." << std::endl;
				std::cout << "Valid options: " << std::endl;
				std::cout << "  -h: Display this help page" << std::endl;
				std::cout << "  -s: Connect to the secure Matt_daemon service running at IP." << std::endl;
				return (0);
				break ;
			case 's':
				secure = true;
				break ;
			default:
				std::cerr << "Unknown option provided. Please see " << av[0] << " -h for more information on usage" << std::endl;
				return (1);
		}
	}
	if (secure && ac == 2)
	{
		std::cerr << "No ip provided" << std::endl;
		return 1;
	}
	try {
		client = Client(av[ac - 1], secure);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return client.run();
}
