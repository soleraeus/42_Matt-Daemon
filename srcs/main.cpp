/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:29:26 by bdetune           #+#    #+#             */
/*   Updated: 2023/09/27 18:57:40 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <iostream>
#include "Matt_daemon.hpp"
#include "Tintin_reporter.hpp"

int main(void)
{
	/*if (getuid() != 0)
	{
		std::cerr << "Matt_daemon requires to be started as root" << std::endl;
		return (1);
	}*/
	int				lock;
	Tintin_reporter	reporter("log");

	reporter.log("Started.\n", INFO);
	lock = open("/var/lock/matt_daemon.lock", O_RDWR | O_CLOEXEC);
	if (lock <= 0)
	{
		std::cerr << "Could not open lockfile /var/lock/matt_daemon.lock" << std::endl;
		reporter.log("Error, could no open lockfile /var/lock/matt_daemon.lock.\n", ERROR);
		reporter.log("Quitting.\n", INFO);
		return (1);
	}
	reporter.log("Hello\n", LOG);

	return (0);
}
