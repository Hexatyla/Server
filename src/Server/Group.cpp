//
// Group.cpp for  in /home/deleme/pfa
// 
// Made by anthony delemer
// Login   <deleme_a@epitech.net>
// 
// Started on  Tue Feb  4 14:11:22 2014 anthony delemer
// Last update Thu Feb  6 15:57:26 2014 gautier lefebvre
//

#include	"Server/User.hpp"
#include	"Server/Group.hpp"

Server::Group::Group(Server::User* l):
  Server::HasUsers(),
  _leader(l)
{}

Server::Group::~Group() {}

bool		Server::Group::isInGroup(const Server::User *u) const {
  for (auto it : _users) {
    if (it == u) {
      return (true);
    }
  }
  return (false);
}

const Server::User* Server::Group::getLeader() const {
  return (this->_leader);
}
