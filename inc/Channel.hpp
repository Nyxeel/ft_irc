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

	public:

		Channel();
		Channel(std::string& name);
		~Channel();

		void			addUser(int fd);
		void			addOperator(int fd);
		void			removeUser(int fd);
		bool			isMember(int fd);
		bool			isOperator(int fd);


		std::string		getName() const;
		std::string		getTopic() const;
		std::string		getKey() const;
		bool			getInviteOnly() const;
		bool			getTopicProtection() const;
		size_t			getUserLimit() const;
		const std::set<int>&	getUsers() const;
		const std::set<int>&	getOperators() const;


		void			setTopic(std::string& topic);
		void			setKey(std::string& password);
		void			setInviteOnly(bool inviteOnly);
		void			setTopicProtection(bool topicProtection);
		void			setUserLimit(size_t userLimit);

};

typedef std::map<std::string, Channel> Channels;

#endif
