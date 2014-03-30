//
// HasRessourcesSpot.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 18:02:46 2014 geoffrey bauduin
// Last update Sat Mar 15 15:25:21 2014 geoffrey bauduin
//

#include	"Threading/ScopeLock.hpp"
#include	"Server/HasRessourcesSpot.hpp"
#include	"Factory/Server.hpp"

Server::HasRessourcesSpot::HasRessourcesSpot(void):
  _ressourcesSpotMutex(new Mutex), _ressourcesSpots() {}

Server::HasRessourcesSpot::~HasRessourcesSpot(void) {
  delete this->_ressourcesSpotMutex;
}

void	Server::HasRessourcesSpot::init(void) {}

void	Server::HasRessourcesSpot::destroy(void) {
  for (auto it : this->_ressourcesSpots) {
    Factory::Server::RessourcesSpot::remove(it.second);
  }
  this->_ressourcesSpots.clear();
}

void	Server::HasRessourcesSpot::addRessourcesSpot(Server::RessourcesSpot *ressourcesSpot) {
  ScopeLock s(this->_ressourcesSpotMutex);
  this->_ressourcesSpots[ressourcesSpot->getID()] = ressourcesSpot;
}

void	Server::HasRessourcesSpot::removeRessourcesSpot(Server::RessourcesSpot *ressourcesSpot) {
  this->removeRessourcesSpot(ressourcesSpot->getID());
}

void	Server::HasRessourcesSpot::removeRessourcesSpot(Kernel::ID::id_t id) {
  ScopeLock s(this->_ressourcesSpotMutex);
  this->_ressourcesSpots.erase(id);
}
