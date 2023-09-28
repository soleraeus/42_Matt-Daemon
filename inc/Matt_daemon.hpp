/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Matt_daemon.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 21:04:44 by bdetune           #+#    #+#             */
/*   Updated: 2023/09/28 21:19:50 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MATT_DAEMON_HPP
# define MATT_DAEMON_HPP
# define MATT_DAEMON_BASIC_PORT 8080
# include <signal.h>

extern volatile sig_atomic_t	g_sig;

enum Loglevel { LOG = 0, INFO = 1, ERROR = 2 };

#endif
