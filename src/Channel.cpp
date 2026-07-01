/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/02 01:00:16 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Channel.hpp"
#include <arpa/inet.h> // htons()...
#include <arpa/inet.h> // htons(), inet_ntop()
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h> // fcntl(), O_NONBLOCK
#include <poll.h>  // poll(), struct pollfd
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <unistd.h>     // close()
#include <iostream>     // close()

void print(std::string str);

// ───────────────────────────────────────────────
// ────────────────── CANONICAL ──────────────────
// ───────────────────────────────────────────────

Channel::Channel() {

}

Channel::Channel(const Channel &other)
{
	(void) other;

}

Channel &Channel::operator=(const Channel &other) {

(void) other;
  if (this != &other) {

  }
  return *this;
}

Channel::~Channel() {


}
