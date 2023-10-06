/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tintin_reporter.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:29:21 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/06 20:25:55 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TINTIN_REPORTER_HPP
# define TINTIN_REPORTER_HPP
# include <fstream>
# include <exception>
# include <stdexcept>
# include <string>
# include <iostream>
# include <ctime>
# include "Matt_daemon.hpp"

class Tintin_reporter
{
	public:
		enum class Return {
			NO_FILE,
			FILE_REMOVED,
			SYSTEM_ERROR,
			OK	
		};

		//Constructors
		Tintin_reporter(void) noexcept;
		Tintin_reporter(std::string const & filepath);
		Tintin_reporter(std::string&& filepath);
		Tintin_reporter(Tintin_reporter const & src);
		Tintin_reporter(Tintin_reporter && src);

		//Destructors
		~Tintin_reporter(void);

		//Operator overload
		Tintin_reporter&	operator=(Tintin_reporter const & rhs);
		Tintin_reporter&	operator=(Tintin_reporter && rhs);

		//Member functions
		Tintin_reporter::Return	client_log(std::string const & str);
		Tintin_reporter::Return	client_log(std::string && str);
		Tintin_reporter::Return	log(std::string && str, enum Loglevel level);
		Tintin_reporter::Return	log(std::string const & str, enum Loglevel level);
	
	private:
		std::string			get_time(void) const;

		std::string			_filepath;
		std::fstream		_logfile;		
};

#endif
