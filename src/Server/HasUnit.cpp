//
// HasUnit.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 18:02:46 2014 geoffrey bauduin
// Last update Sat Mar 15 15:26:05 2014 geoffrey bauduin
//

#include	"Threading/ScopeLock.hpp"
#include	"Server/HasUnit.hpp"
#include	"Factory/Server.hpp"

Server::HasUnit::HasUnit(void):
  _unitMutex(new Mutex), _units() {}

Server::HasUnit::~HasUnit(void) {
  delete this->_unitMutex;
}

void	Server::HasUnit::init(void) {}

void	Server::HasUnit::destroy(void) {
  for (auto it : this->_units) {
    Factory::Server::Unit::remove(it.second);
  }
  this->_units.clear();
}

void	Server::HasUnit::addUnit(Server::Unit *unit) {
  ScopeLock s(this->_unitMutex);
  this->_units[unit->getID()] = unit;
}

void	Server::HasUnit::removeUnit(Server::Unit *unit) {
  this->removeUnit(unit->getID());
}

void	Server::HasUnit::removeUnit(Kernel::ID::id_t id) {
  ScopeLock s(this->_unitMutex);
  this->_units.erase(id);
}
