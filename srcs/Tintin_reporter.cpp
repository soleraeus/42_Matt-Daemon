/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tintin_reporter.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:59:28 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/30 21:36:10 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Tintin_reporter.hpp"
#include <climits>
#include <filesystem>

Tintin_reporter::Tintin_reporter(void) noexcept {}

Tintin_reporter::Tintin_reporter(std::string const & filepath): _filepath(filepath), _logfile(filepath, std::ios::out | std::ios::in | std::ios::app)
{
	if (!this->_logfile.good())
		throw std::invalid_argument("Could not open logfile");
}

Tintin_reporter::Tintin_reporter(std::string&& filepath): _filepath(filepath), _logfile(std::move(filepath), std::ios::out | std::ios::in | std::ios::app)
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
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::in | std::ios::app);
	}
	else if (!this->_logfile.good())
	{
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::in | std::ios::app);
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
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::in | std::ios::app);
	}
	else if (!this->_logfile.good())
	{
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::in | std::ios::app);
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
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::in | std::ios::app);
	else
		return (*this);
	if (!this->_logfile.good())
		throw std::invalid_argument("Could not open logfile");
	return (*this);
}

Tintin_reporter& Tintin_reporter::operator=(Tintin_reporter && rhs)
{
	if (this == &rhs)
		return (*this);
	if (this->_logfile.is_open())
		this->_logfile.close();
	this->_filepath = std::move(rhs._filepath);
	this->_logfile = std::move(rhs._logfile);
	if (!this->_logfile.is_open())
	{
		if (!this->_filepath.length())
			return (*this);
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::in | std::ios::app);
	}
	else if (!this->_logfile.good())
	{
		this->_logfile.open(this->_filepath, std::ios::out | std::ios::in | std::ios::app);
	}
	if (!this->_logfile.good())
		throw std::invalid_argument("Logfile is an invalid state or could not be opened");
	return (*this);
}


Tintin_reporter::Return	Tintin_reporter::client_log(std::string && str)
{
	return (this->log("User input: " + str, LOG));
}
Tintin_reporter::Return	Tintin_reporter::client_log(std::string const & str)
{
	return (this->log("User input: " + str, LOG));
}

Tintin_reporter::Return	Tintin_reporter::log(std::string && str, enum Loglevel level)
{
	std::string	log;

	if (!this->_logfile.is_open())
		return Tintin_reporter::Return::NO_FILE;
    if (!std::filesystem::exists(std::filesystem::path(this->_filepath)))
		return Tintin_reporter::Return::FILE_REMOVED;
	try {
		log = this->get_time();
	} catch (const std::exception& e)
	{
		(void)e;
		return Tintin_reporter::Return::SYSTEM_ERROR;
	}
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
	return Tintin_reporter::Return::OK ;
}

Tintin_reporter::Return	Tintin_reporter::log(std::string const & str, enum Loglevel level)
{
	std::string	log;

	if (!this->_logfile.is_open())
		return Tintin_reporter::Return::NO_FILE;
    if (!std::filesystem::exists(std::filesystem::path(this->_filepath)))
		return Tintin_reporter::Return::FILE_REMOVED;
	try {
		log = this->get_time();
	} catch (const std::exception& e)
	{
		(void)e;
		return Tintin_reporter::Return::SYSTEM_ERROR;
	}
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
	return Tintin_reporter::Return::OK ;
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

Tintin_reporter::Return Tintin_reporter::send_logs(std::string & send_buffer) {
    char                    buf[PIPE_BUF + 1];
    std::fstream::pos_type  cursor_pos = 0;

    cursor_pos = this->_logfile.tellg();
    if (cursor_pos == -1) {
        std::cerr << "Could not get position of cursor" << std::endl;
        return Tintin_reporter::Return::SYSTEM_ERROR;
    }
    std::cerr << "Current position: " << cursor_pos << std::endl;
    if (cursor_pos > PIPE_BUF) {
        this->_logfile.seekg(-PIPE_BUF, std::ios_base::end);
        if (!this->_logfile.good()) {
            std::cerr << "Could not move cursor" << std::endl;
            return Tintin_reporter::Return::SYSTEM_ERROR;
        }
        this->_logfile.read(buf, PIPE_BUF);
    }
    else {
        std::cerr << "Limited range" << std::endl;
        this->_logfile.seekg(0, std::ios_base::beg);
        if (!this->_logfile.good()) {
            std::cerr << "Could not move cursor" << std::endl;
            return Tintin_reporter::Return::SYSTEM_ERROR;
        }
        this->_logfile.read(buf, static_cast<int>(cursor_pos));
    }
    if (this->_logfile.fail()) {
        std::cerr << "Could not read from logfile, failbit set" << std::endl;
        return Tintin_reporter::Return::SYSTEM_ERROR;
    }
    else if (this->_logfile.bad()) {
        std::cerr << "Could not read from logfile, badbit set" << std::endl;
        return Tintin_reporter::Return::SYSTEM_ERROR;
    }
    buf[this->_logfile.gcount()] = '\0';
    send_buffer = buf;
    if (cursor_pos > PIPE_BUF)
        send_buffer.erase(0, send_buffer.find('\n') + 1);
    return Tintin_reporter::Return::OK;
}
