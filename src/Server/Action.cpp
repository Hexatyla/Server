//
// Action.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sun Feb 16 16:20:39 2014 geoffrey bauduin
// Last update Sat Mar 15 14:26:16 2014 geoffrey bauduin
//

#include	"Server/Action.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Server.hpp"

Server::Action::Action(void):
  ::Game::Action(),
  _mutex(new Mutex), _next(NULL) {

}

Server::Action::~Action(void) {
  delete this->_mutex;
}

void	Server::Action::init(::Game::eAction action) {
  ::Game::Action::init(action);
  this->_next = NULL;
}

void	Server::Action::destroy(void) {
  ::Game::Action::destroy();
  if (this->_next) {
    Factory::Server::Action::remove(this->_next);
    this->_next = NULL;
  }
}

void	Server::Action::setNextAction(Server::Action *action) {
  ScopeLock s(this->_mutex);
  this->_next = action;
}

Server::Action	*Server::Action::getNextAction(void) const {
  ScopeLock s(this->_mutex);
  return (this->_next);
}

bool	Server::Action::hasReferenceToOtherObject(Kernel::ID::id_t id, ::Game::Type type) const {
  ScopeLock s(this->_mutex);
  switch (this->_type) {
  case ::Game::ACTION_BUILDING:
  case ::Game::ACTION_START_BUILDING:
  case ::Game::ACTION_DEPOSIT:
    return (id == this->building && type == ::Game::BUILDING);
  case ::Game::ACTION_START_HARVEST:
  case ::Game::ACTION_HARVEST:
    return (id == this->spot && type == ::Game::RESSSPOT);
  case ::Game::ACTION_RELEASE:
    return (id == this->release.object && type == ::Game::OBJECT);
  case ::Game::ACTION_PICKUP:
    return (id == this->object && type == ::Game::OBJECT);
  // case ::Game::ACTION_ATTACK:
  // case ::Game::ACTION_START_ATTACK:
  //   return (this->target.id == id && this->target.type == type);
  default:
    break;
  }
  return (false);
}
