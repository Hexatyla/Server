//
// HasStatistics.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Fri Feb 14 19:42:36 2014 geoffrey bauduin
// Last update Fri Feb 14 19:59:37 2014 geoffrey bauduin
//

#include	"Server/HasStatistics.hpp"
#include	"Threading/ScopeLock.hpp"

Server::HasStatistics::HasStatistics(void):
  _mutex(new Mutex),
  _owKills(), _owDestructions(),
  _built(), _destroyed(),
  _recolted(), _spent() {

}

Server::HasStatistics::~HasStatistics(void) {
  delete this->_mutex;
}

void	Server::HasStatistics::init(const std::vector<Kernel::Serial> &serials) {
  for (auto it : serials) {
    this->_owKills[it] = Quantifier();
    this->_owDestructions[it] = Quantifier();
    this->_built[it] = 0;
    this->_destroyed[it] = 0;
  }
}

void	Server::HasStatistics::init(const ::Game::Race *race) {
  this->init(race->getUnits());
  this->init(race->getBuildings());
  this->init(race->getHeroes());
  this->_recolted.init(0, 0, 0);
  this->_spent.init(0, 0, 0);
}

void	Server::HasStatistics::destroy(void) {
  this->_owKills.clear();
  this->_owDestructions.clear();
  this->_built.clear();
  this->_destroyed.clear();
  this->_recolted.destroy();
  this->_spent.destroy();
}

void	Server::HasStatistics::hasKilledOtherUnit(const Kernel::Serial &lhs, const Kernel::Serial &rhs) {
  ScopeLock s(this->_mutex);
  this->_owKills[lhs][rhs]++;
}

void	Server::HasStatistics::hasDestroyedOtherBuilding(const Kernel::Serial &lhs, const Kernel::Serial &rhs) {
  ScopeLock s(this->_mutex);
  this->_owDestructions[lhs][rhs]++;
}

void	Server::HasStatistics::hasBuiltItem(const Kernel::Serial &item) {
  ScopeLock s(this->_mutex);
  this->_built[item]++;
}

void	Server::HasStatistics::hasLostItem(const Kernel::Serial &item) {
  ScopeLock s(this->_mutex);
  this->_destroyed[item]++;
}

void	Server::HasStatistics::hasSpentRessources(const ::Game::Ressources &ressources) {
  this->_spent += ressources;
}

void	Server::HasStatistics::hasSpentRessources(::Game::Ressources::Type type, unsigned int amount) {
  this->_spent.add(type, static_cast<int>(amount));
}

void	Server::HasStatistics::hasRecoltedRessources(const ::Game::Ressources &ressources) {
  this->_recolted += ressources;
}

void	Server::HasStatistics::hasRecoltedRessources(::Game::Ressources::Type type, unsigned int amount) {
  this->_recolted.add(type, static_cast<int>(amount));
}

void	Server::HasStatistics::onUpdate(void) {
  // A DEFINIR
}
