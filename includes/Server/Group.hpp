//
// Group.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Tue Feb  4 13:49:28 2014 gautier lefebvre
// Last update Thu Feb  6 15:56:46 2014 gautier lefebvre
//

#ifndef		__SERVER_GROUP_HPP__
#define		__SERVER_GROUP_HPP__

#include	<iostream>
#include	<string>
#include	<list>

#include	"Server/HasUsers.hpp"

namespace	Server {
  class		Group :public HasUsers {
  private:
    Server::User*	_leader;

  public:
    Group(Server::User*);
    virtual ~Group();

    bool	isInGroup(const Server::User *) const;
    const Server::User* getLeader() const;
  };
}

#endif
