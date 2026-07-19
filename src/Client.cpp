/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/19 19:49:02 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Client.hpp"
#include <string>
#include <sys/socket.h>

#include <string>

// ───────────────────────────────────────────────
// ────────────────── CANONICAL ──────────────────
// ───────────────────────────────────────────────

Client::Client() :
	_clientSocket(-1), _nickname(""), _username(""),
	_authenticate(false), _passOK(false), _userOK(false), _nickOK(false)
{
}

Client::Client(int clientSocket) :
	_clientSocket(clientSocket), _nickname(""), _username(""),
	_authenticate(false), _passOK(false), _userOK(false), _nickOK(false)
{
}

Client::~Client() {

}


// ───────────────────────────────────────────────
// ─────────────────── GETTERS ───────────────────
// ───────────────────────────────────────────────

int	Client::getClientSocket() const {
	return _clientSocket;
}

std::string	Client::getNickname() const		{
	return _nickname;
}

std::string	Client::getUsername() const		{
	return _username;
}

std::string	Client::getHostAdresse() const {

	return _ip ;
}

// ───────────────────────────────────────────────
// ─────────────────── SETTERS ───────────────────
// ───────────────────────────────────────────────

void Client::setPassOK() {
	_passOK = true;
}

void Client::setNickname(const std::string& nickname) {
	_nickname = nickname;
}

void Client::setUsername(const std::string& username) {
	_username = username;
}

void Client::setAuthenticate() {
	_authenticate = true;
}

void Client::setNickOK() {
	_nickOK = true;
}

void Client::setUserOK() {
	_userOK = true;
}

bool Client::isPassOK() const {
	return _passOK;
}

bool Client::isNickOK() const {
	return _nickOK;
}

bool Client::isUserOK() const {
	return _userOK;
}

void	Client::setHostAdresse(char ip[INET_ADDRSTRLEN]) {

	_ip = ip;
}

bool Client::isAuthenticated() const {
	return _passOK && _nickOK && _userOK;
}

void	Client::addChannel(const std::string& name) {

	_joinedChannels.insert(name);
}

void 	Client::removeChannel(const std::string& name) {

	
	_joinedChannels.erase(name);
}



