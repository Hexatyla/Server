//
// HasProjectile.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 18:02:46 2014 geoffrey bauduin
// Last update Sat Mar 15 15:22:37 2014 geoffrey bauduin
//

#include	"Threading/ScopeLock.hpp"
#include	"Server/HasProjectile.hpp"
#include	"Factory/Server.hpp"

Server::HasProjectile::HasProjectile(void):
  _projectileMutex(new Mutex), _projectiles() {}

Server::HasProjectile::~HasProjectile(void) {
  delete this->_projectileMutex;
}

void	Server::HasProjectile::destroy(void) {
  for (auto it : this->_projectiles) {
    Factory::Server::Projectile::remove(it.second);
  }
  this->_projectiles.clear();
}

void	Server::HasProjectile::init(void) {

}

void	Server::HasProjectile::addProjectile(Server::Projectile *projectile) {
  ScopeLock s(this->_projectileMutex);
  this->_projectiles[projectile->getID()] = projectile;
}

void	Server::HasProjectile::removeProjectile(Server::Projectile *projectile) {
  this->removeProjectile(projectile->getID());
}

void	Server::HasProjectile::removeProjectile(Kernel::ID::id_t id) {
  ScopeLock s(this->_projectileMutex);
  this->_projectiles.erase(id);
}
