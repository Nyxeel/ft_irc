/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/17 11:50:15 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Channel.hpp"
#include <string>

void print(std::string str);

// ───────────────────────────────────────────────
// ────────────────── CANONICAL ──────────────────
// ───────────────────────────────────────────────


Channel::Channel(std::string& name) : _name(name) {

}


Channel::Channel(const Channel &other) : _name(other._name) {

}

Channel &Channel::operator=(const Channel &other) {

	if (this != &other) {
		_name = other._name;

  	}
  	return *this;
}

Channel::~Channel() {

}
