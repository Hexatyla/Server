//
// Effect.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Feb 11 21:49:57 2014 geoffrey bauduin
// Last update Sat Mar 15 15:05:05 2014 geoffrey bauduin
//

#include	<cstddef>
#include	"Server/Effect.hpp"
#include	"Factory/Factory.hpp"
#include	"Threading/ScopeLock.hpp"

Server::Effect::Effect(void):
  ::Game::Effect(),
  _mutex(new Mutex),
  _start(NULL) {}

Server::Effect::~Effect(void) {
  delete this->_mutex;
}

void	Server::Effect::init(const Kernel::Serial &serial, const Game::Player *player, Kernel::ID::id_t itemID) {
  ::Game::Effect::init(serial, player, itemID);
  this->_start = NULL;
}

void	Server::Effect::destroy(void) {
  ::Game::Effect::destroy();
  if (this->_start) {
    Factory::Clock::remove(this->_start);
  }
  this->_start = NULL;
}

bool	Server::Effect::update(const Clock *clock, double) {
  ScopeLock s(this->_mutex);
  if (!this->_start) {
    this->_start = clock->clone();
  }
  else if (clock->getElapsedTimeSince(this->_start) >= this->getInfos().timer->duration) {
    return (false);
  }
  else {
    this->setPercentage(clock->getElapsedTimeSince(this->_start) / this->getInfos().timer->duration);
  }
  return (true);
}


