//
// HasHero.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 18:02:46 2014 geoffrey bauduin
// Last update Sat Mar 15 15:18:11 2014 geoffrey bauduin
//

#include	"Threading/ScopeLock.hpp"
#include	"Server/HasHero.hpp"
#include	"Factory/Server.hpp"

Server::HasHero::HasHero(void):
  _heroMutex(new Mutex), _heroes() {

}

Server::HasHero::~HasHero(void) {
  delete this->_heroMutex;
}

void	Server::HasHero::init(void) {}

void	Server::HasHero::destroy(void) {
  for (auto it : this->_heroes) {
    Factory::Server::Hero::remove(it.second);
  }
  this->_heroes.clear();
}

void	Server::HasHero::addHero(Server::Hero *hero) {
  ScopeLock s(this->_heroMutex);
  this->_heroes[hero->getID()] = hero;
}

void	Server::HasHero::removeHero(Server::Hero *hero) {
  this->removeHero(hero->getID());
}

void	Server::HasHero::removeHero(Kernel::ID::id_t id) {
  ScopeLock s(this->_heroMutex);
  this->_heroes.erase(id);
}
