/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:29:26 by bdetune           #+#    #+#             */
/*   Updated: 2023/09/26 22:03:55 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/types.h>
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
	Tintin_reporter	reporter("log");

	reporter.log("Hello\n", LOG);

	return (0);
}
