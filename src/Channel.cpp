/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/21 09:07:40 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Channel.hpp"
#include <cstring>
#include <set>

void print(std::string str);

// ───────────────────────────────────────────────
// ───────────────── CANONICAL ───────────────────
// ───────────────────────────────────────────────

Channel::Channel() :
	_name(""), _topic(""), _key(""), _inviteOnly(false),
	_topicProtection(false), _userLimit(0)
{
}

Channel::Channel(std::string& name) :
	_name(name), _topic(""), _key(""), _inviteOnly(false),
	_topicProtection(false), _userLimit(0)
{
}

Channel::~Channel() {

}

// ───────────────────────────────────────────────
// ─────────────────── HELPER ────────────────────
// ───────────────────────────────────────────────

void	Channel::addUser(int fd) {

	_users.insert(fd);
}

void	Channel::addToInviteList(int fd) {

	_inviteList.insert(fd);
}

void	Channel::addOperator(int fd) {

	_operators.insert(fd);
}

void	Channel::removeUser(int fd) {

	_users.erase(fd);
	if (isOperator(fd))
		_operators.erase(fd);
}

bool	Channel::isMember(int fd) {

	return(_users.find(fd) != _users.end());
}

bool	Channel::isOperator(int fd) {

	return(_operators.find(fd) != _operators.end());
}

bool	Channel::isInvited(int fd) {

	if (_inviteList.find(fd) != _inviteList.end()) {

		_inviteList.erase(fd);
		return true;
	}
	return(false);
}



// ───────────────────────────────────────────────
// ─────────────────── GETTERS ───────────────────
// ───────────────────────────────────────────────

std::string	Channel::getName() const {

	return _name;
}

std::string	Channel::getTopic() const {

	return _topic;
}

std::string	Channel::getKey() const {

	return (_key);
}

bool	Channel::getInviteOnly() const {

	return (_inviteOnly);
}

bool	Channel::getTopicProtection() const {

	return (_topicProtection);
}

size_t	Channel::getUserLimit() const {

	return (_userLimit);
}

const std::set<int>&	Channel::getUsers() const {

	return (_users);
}

const std::set<int>&	Channel::getOperators() const {

	return (_operators);
}

// ───────────────────────────────────────────────
// ────────────────── SETTERS ────────────────────
// ───────────────────────────────────────────────

void	Channel::setTopic(const std::string& topic) {

	_topic = topic;
}

void	Channel::setKey(std::string& key) {

	_key = key;
}

void	Channel::setInviteOnly(bool inviteOnly) {

	_inviteOnly = inviteOnly;
}

void	Channel::setTopicProtection(bool topicProtection) {

	_topicProtection = topicProtection;
}

void	Channel::setUserLimit(size_t userLimit) {

	_userLimit = userLimit;
}



