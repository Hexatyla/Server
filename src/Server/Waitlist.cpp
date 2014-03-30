//
// Waitlist.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Feb 18 20:08:38 2014 geoffrey bauduin
// Last update Tue Feb 18 22:01:08 2014 geoffrey bauduin
//

#include	<algorithm>
#include	"Server/Waitlist.hpp"
#include	"Threading/ScopeLock.hpp"

Server::Waitlist::Waitlist(void):
  _mutex(new Mutex), _waitlist() {

}

Server::Waitlist::~Waitlist(void) {
  delete this->_mutex;
}

void	Server::Waitlist::init(void) {

}

void	Server::Waitlist::destroy(void) {
  this->_waitlist.clear();
}

void	Server::Waitlist::push(Kernel::ID::id_t id) {
  ScopeLock s(this->_mutex);
  this->_waitlist.push_back(id);
}

void	Server::Waitlist::remove(Kernel::ID::id_t id) {
  ScopeLock s(this->_mutex);
  this->_waitlist.erase(this->_waitlist.begin(), std::remove(this->_waitlist.begin(), this->_waitlist.end(), id));
}

bool	Server::Waitlist::isInWaitlist(Kernel::ID::id_t id) const {
  return (std::find(this->_waitlist.begin(), this->_waitlist.end(), id) != this->_waitlist.end());
}

const std::deque<Kernel::ID::id_t> &Server::Waitlist::getWaitlist(void) const {
  return (this->_waitlist);
}

Kernel::ID::id_t	Server::Waitlist::pop(void) {
  ScopeLock s(this->_mutex);
  Kernel::ID::id_t id = this->_waitlist.front();
  this->_waitlist.pop_front();
  return (id);
}

bool	Server::Waitlist::empty(void) const {
  return (this->_waitlist.empty());
}
