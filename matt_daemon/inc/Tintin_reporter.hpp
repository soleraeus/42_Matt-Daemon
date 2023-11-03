/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tintin_reporter.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 19:29:21 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/31 22:16:26 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MATT_DAEMON_TINTIN_REPORTER_HPP
# define MATT_DAEMON_TINTIN_REPORTER_HPP

//std
# include <climits>
# include <ctime>
# include <exception>
# include <filesystem>
# include <fstream>
# include <iostream>
# include <stdexcept>
# include <string>

//Project specific
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
        enum class Loglevel {
            LOG,
            INFO,
            ERROR
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
        Tintin_reporter&    operator=(Tintin_reporter const & rhs);
        Tintin_reporter&    operator=(Tintin_reporter && rhs);

        //Member functions
        [[nodiscard]] Tintin_reporter::Return   client_log(std::string const & str);
        [[nodiscard]] Tintin_reporter::Return   client_log(std::string && str);
        [[nodiscard]] Tintin_reporter::Return   log(std::string && str, Tintin_reporter::Loglevel level);
        [[nodiscard]] Tintin_reporter::Return   log(std::string const & str, Tintin_reporter::Loglevel level);
        [[nodiscard]] Tintin_reporter::Return   send_logs(std::string & send_buffer);
    
    private:
        std::string         get_time(void) const;

        std::string         _filepath;
        std::fstream        _logfile;       
};

#endif
