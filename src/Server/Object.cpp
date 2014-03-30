//
// Object.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Feb  5 13:41:24 2014 geoffrey bauduin
// Last update Sat Mar 15 15:29:11 2014 geoffrey bauduin
//

#include	"Server/Object.hpp"
#include	"Threading/ScopeLock.hpp"

Server::Object::Object(void):
  ::Game::AElement(), ::Game::Object(), Server::AItem(),
  _mutex(new Mutex) {

}

Server::Object::~Object(void) {
  delete this->_mutex;
}

void	Server::Object::init(Kernel::ID::id_t id, const Kernel::Serial &serial, int orientation, double x, double y, double z) {
  ::Game::Object::init(id, serial, orientation, x, y, z);
  Server::AItem::init();
}

void	Server::Object::destroy(void) {
  ::Game::Object::destroy();
  Server::AItem::destroy();
}

void	Server::Object::setPosition(double x, double y, double z) {
  ::Game::Object::setPosition(x, y ,z);
  this->_mustUpdate = true;
}

void	Server::Object::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type type) {
  ScopeLock s(this->_mutex);
  if (type == ::Game::HERO && id == this->_ownerID) {
    this->_ownerID = 0;
  }
  Server::AItem::onItemDestroyed(id, type);
}
