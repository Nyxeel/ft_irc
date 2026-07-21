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
#include <map>
#include <set>

#define RMV_MODE	false
#define SET_MODE	true

class Channel {

	private:

		std::string		_name;
		std::string		_topic;
		std::string		_key;				// k
		bool			_inviteOnly;		// i
		bool			_topicProtection;	// t
		size_t			_userLimit;			// l

		std::set<int>	_users;
		std::set<int> 	_operators;
		std::set<int> 	_inviteList;

	public:

		Channel();
		Channel(std::string& name);
		~Channel();

		void			addUser(int fd);
		void			removeUser(int fd);
		void			addToInviteList(int fd);
		void			removeOperator(int fd);
		bool			isMember(int fd);
		bool			isOperator(int fd);
		bool			isInvited(int fd);


		std::string		getName() const;
		std::string		getTopic() const;
		std::string		getKey() const;
		bool			getInviteOnly() const;
		bool			getTopicProtection() const;
		size_t			getUserLimit() const;
		const std::set<int>&	getUsers() const;
		const std::set<int>&	getOperators() const;
		const std::set<int>&	getInviteList() const;


		void			setTopic(const std::string& topic);
		void			setKey(std::string& password);
		void			setInviteOnly(bool mode);
		void			setTopicProtection(bool topicProtection);
		void			setUserLimit(size_t userLimit);
		void			setInviteList(int fd);
		void			setOperator(int fd);

};

typedef std::map<std::string, Channel> Channels;

#endif
