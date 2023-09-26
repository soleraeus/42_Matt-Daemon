/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tintin_reporter.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:29:21 by bdetune           #+#    #+#             */
/*   Updated: 2023/09/26 21:44:05 by bdetune          ###   ########.fr       */
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
		//Constructors
		Tintin_reporter(void);
		Tintin_reporter(std::string const & filepath);
		Tintin_reporter(std::string&& filepath);
		Tintin_reporter(Tintin_reporter const & src);
		Tintin_reporter(Tintin_reporter && src);

		//Destructors
		~Tintin_reporter(void);

		//Operator overload
		Tintin_reporter&	operator=(Tintin_reporter const & rhs);

		//Member functions
		void				log(std::string && str, enum Loglevel level);
		void				log(std::string const & str, enum Loglevel level);
	
	private:
		std::string			_filepath;
		std::fstream		_logfile;		

		std::string			get_time(void) const;
};

#endif
