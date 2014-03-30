//
// GroupMove.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Thu Feb 27 00:07:00 2014 geoffrey bauduin
// Last update Sat Mar 15 15:06:35 2014 geoffrey bauduin
//

#include	"Server/GroupMove.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Server/AItem.hpp"

Server::GroupMove::GroupMove(void):
  _mutex(new Mutex), _items(), _currentSpeed(0.0) {

}

Server::GroupMove::~GroupMove(void) {
  delete this->_mutex;
}

void	Server::GroupMove::init(const std::list<const Server::AItem *> &items) {
  this->_items = items;
  this->update();
}

void	Server::GroupMove::destroy(void) {
  this->_items.clear();
}

void	Server::GroupMove::addItem(const Server::AItem *item) {
  ScopeLock s(this->_mutex);
  this->_items.push_back(item);
  this->update();
}

bool	Server::GroupMove::removeItem(const Server::AItem *item) {
  ScopeLock s(this->_mutex);
  this->_items.remove(item);
  if (this->_items.empty() == false) {
    this->update();
    return (false);
  }
  return (true);
}

unsigned int	Server::GroupMove::getSpeed(void) const {
  ScopeLock s(this->_mutex);
  return (this->_currentSpeed);
}

void	Server::GroupMove::update(void) {
  ScopeLock s(this->_mutex);
  this->_currentSpeed = static_cast<unsigned int>(-1);
  unsigned int speed;
  for (auto it : this->_items) {
    speed = it->::Game::ABase::getSpeed();
    if (this->_currentSpeed == static_cast<unsigned int>(-1) || this->_currentSpeed > speed) {
      this->_currentSpeed = speed;
    }
  }
}
