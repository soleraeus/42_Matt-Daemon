/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Matt_daemon.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 21:04:44 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/31 22:29:29 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MATT_DAEMON_MATT_DAEMON_HPP
# define MATT_DAEMON_MATT_DAEMON_HPP

# define MATT_DAEMON_BASIC_PORT 4242
# define MATT_DAEMON_SECURE_PORT 4343
# define MATT_DAEMON_LOCKFILE_PATH "/var/lock/matt_daemon.lock"
# define MATT_DAEMON_LOGFILE_PATH "/var/log/matt_daemon/matt_daemon.log"

# include <csignal>

extern volatile sig_atomic_t	g_sig;

#endif
