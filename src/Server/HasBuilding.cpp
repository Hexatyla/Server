//
// HasBuilding.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 18:02:46 2014 geoffrey bauduin
// Last update Sat Mar 15 15:32:02 2014 geoffrey bauduin
//

#include	"Threading/ScopeLock.hpp"
#include	"Server/HasBuilding.hpp"
#include	"Factory/Server.hpp"

Server::HasBuilding::HasBuilding(void):
  _buildingMutex(new Mutex), _buildings() {}

Server::HasBuilding::~HasBuilding(void) {
  delete this->_buildingMutex;
}

void	Server::HasBuilding::init(void) {}

void	Server::HasBuilding::destroy(void) {
  for (auto it : this->_buildings) {
    Factory::Server::Building::remove(it.second);
  }
  this->_buildings.clear();
}

void	Server::HasBuilding::addBuilding(Server::Building *building) {
  ScopeLock s(this->_buildingMutex);
  this->_buildings[building->getID()] = building;
}

void	Server::HasBuilding::removeBuilding(Server::Building *building) {
  this->removeBuilding(building->getID());
}

void	Server::HasBuilding::removeBuilding(Kernel::ID::id_t id) {
  ScopeLock s(this->_buildingMutex);
  this->_buildings.erase(id);
}
