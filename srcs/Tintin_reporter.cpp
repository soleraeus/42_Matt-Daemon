/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tintin_reporter.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:59:28 by bdetune           #+#    #+#             */
/*   Updated: 2023/09/27 20:29:39 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Tintin_reporter.hpp"

Tintin_reporter::Tintin_reporter(void) noexcept {}

Tintin_reporter::Tintin_reporter(std::string const & filepath): _filepath(filepath), _logfile(filepath, std::ios::out | std::ios::app)
{
	if (!this->_logfile.good())
		throw std::invalid_argument("Could not open logfile");
}

Tintin_reporter::Tintin_reporter(std::string&& filepath): _filepath(filepath), _logfile(std::move(filepath), std::ios::out | std::ios::app)
{
	if (!this->_logfile.good())
		throw std::invalid_argument("Could not open logfile");
}

Tintin_reporter::Tintin_reporter(Tintin_reporter const & src): _filepath(src._filepath), _logfile(src._filepath, std::ios::out | std::ios::app)
{
	if (!this->_logfile.is_open())
	{
		if (!this->_filepath.length())
			return;
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::app);
	}
	else if (!this->_logfile.good())
	{
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::app);
	}
	if (!this->_logfile.good())
		throw std::invalid_argument("Logfile is an invalid state or could not be opened");
}

Tintin_reporter::Tintin_reporter(Tintin_reporter && src): _filepath(std::move(src._filepath)), _logfile(std::move(src._logfile))
{
	if (!this->_logfile.is_open())
	{
		if (!this->_filepath.length())
			return;
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::app);
	}
	else if (!this->_logfile.good())
	{
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::app);
	}
	if (!this->_logfile.good())
		throw std::invalid_argument("Logfile is an invalid state or could not be opened");
}

Tintin_reporter::~Tintin_reporter(void){}

Tintin_reporter& Tintin_reporter::operator=(Tintin_reporter const & rhs)
{
	if (this == &rhs)
		return (*this);
	if (this->_logfile.is_open())
		this->_logfile.close();
	this->_filepath = rhs._filepath;
	if (this->_filepath.length())
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::app);
	else
		return (*this);
	if (!this->_logfile.good())
		throw std::invalid_argument("Could not open logfile");
	return (*this);
}

Tintin_reporter& Tintin_reporter::operator=(Tintin_reporter && rhs)
{
	if (this->_logfile.is_open())
		this->_logfile.close();
	this->_filepath = std::move(rhs._filepath);
	this->_logfile = std::move(rhs._logfile);
	if (!this->_logfile.is_open())
	{
		if (!this->_filepath.length())
			return (*this);
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::app);
	}
	else if (!this->_logfile.good())
	{
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::app);
	}
	if (!this->_logfile.good())
		throw std::invalid_argument("Logfile is an invalid state or could not be opened");
	return (*this);
}


void	Tintin_reporter::client_log(std::string && str)
{
	this->log("User input: " + str, LOG);
}
void	Tintin_reporter::client_log(std::string const & str)
{
	this->log("User input: " + str, LOG);
}

void	Tintin_reporter::log(std::string && str, enum Loglevel level)
{
	std::string	log;

	if (!this->_logfile.is_open())
		throw std::logic_error("Log file is not opened");
	log = this->get_time();
	switch (level)
	{
		case ERROR:
			log += " [ ERROR ] - Matt_daemon: " + str;
			break ;
		case INFO:
			log += " [ INFO ] - Matt_daemon: " + str;
			break ;
		default:
			log += " [ LOG ] - Matt_daemon: " + str;
			break ;
	}
	this->_logfile << log << std::endl;
}

void	Tintin_reporter::log(std::string const & str, enum Loglevel level)
{
	std::string	log;

	if (!this->_logfile.is_open())
		throw std::logic_error("Log file is not opened");
	log = this->get_time();
	switch (level)
	{
		case ERROR:
			log += " [ ERROR ] - Matt_daemon: " + str;
			break ;
		case INFO:
			log += " [ INFO ] - Matt_daemon: " + str;
			break ;
		default:
			log += " [ LOG ] - Matt_daemon: " + str;
			break ;
	}
	this->_logfile << log << std::endl;
}

std::string	Tintin_reporter::get_time(void) const
{
	time_t curr_time;
	tm * curr_tm;
	char buf[50];
	
	if (time(&curr_time) == (time_t) -1)
	{
		throw std::invalid_argument("Could not get time from system");
	}
	curr_tm = localtime(&curr_time);
	if (!curr_tm)
	{
		throw std::overflow_error("Time could not be represented");
	}
	strftime(buf, 50, "[%d/%m/%Y-%R:%S]", curr_tm);
	return (std::string(buf));
}
