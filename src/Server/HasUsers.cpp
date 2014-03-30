//
// HasUsers.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 29 16:04:39 2014 geoffrey bauduin
// Last update Wed Mar  5 15:10:03 2014 gautier lefebvre
//

#include	"Network/Socket.hpp"
#include	"Server/HasUsers.hpp"
#include	"Threading/ScopeLock.hpp"

Server::HasUsers::HasUsers(void):
  _uMutex(new Mutex), _users() {

}

Server::HasUsers::~HasUsers(void) {
  delete this->_uMutex;
}

void	Server::HasUsers::init(void) {}

void	Server::HasUsers::destroy(void) {
  this->_users.clear();
}

void	Server::HasUsers::init(const Server::HasUsers::Container &list) {
  this->init();
  this->addUsers(list);
}

const Server::HasUsers::Container	&Server::HasUsers::getUsers(void) const {
  return (this->_users);
}

Server::HasUsers::Container	&Server::HasUsers::getUsers(void) {
  return (this->_users);
}

void	Server::HasUsers::addUser(Server::User *user) {
  ScopeLock s(this->_uMutex);
  this->_users.push_back(user);
}

void	Server::HasUsers::removeUser(Server::User *user) {
  ScopeLock s(this->_uMutex);
  this->_users.remove(user);
}

void	Server::HasUsers::addUsers(const Server::HasUsers::Container &list) {
  ScopeLock s(this->_uMutex);
  this->_users.insert(this->_users.end(), list.begin(), list.end());
}

Server::User*	Server::HasUsers::findUser(const std::string &nick) {
  for (auto it : this->_users) {
    if (it->getNick() == nick) {
      return (it);
    }
  }
  return (NULL);
}

Server::User*	Server::HasUsers::findUser(const Network::SSocket *ss) {
  for (auto it : this->_users) {
    if (*it == ss) {
      return (it);
    }
  }
  return (NULL);
}
