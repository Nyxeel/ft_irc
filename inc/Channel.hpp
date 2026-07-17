/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:21:46 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/02 00:25:46 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>



/*
		struct sockaddr_in {
		    sa_family_t    sin_family;   // Adressfamilie
		    in_port_t      sin_port;     // Port (Network Byte Order)
		    struct in_addr sin_addr;     // IPv4-Adresse
		    char           sin_zero[8];  // Padding (ungenutzt)
		};

		struct in_addr {
		    uint32_t s_addr;             // IPv4 als 32-Bit Zahl
		};
*/


class Channel {


	private:
		std::string		_name;

	public:
		Channel(std::string& name);
		Channel(const Channel &other);
		Channel& operator=(const Channel &other);
		~Channel();

};


#endif
