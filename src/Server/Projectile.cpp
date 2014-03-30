//
// Projectile.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 17:19:51 2014 geoffrey bauduin
// Last update Tue Mar 25 10:19:31 2014 geoffrey bauduin
//

#include	"Server/Projectile.hpp"
#include	"Game/Player.hpp"
#include	"Threading/ScopeLock.hpp"

Server::Projectile::Projectile(void):
  Game::AElement(), Game::Projectile(), Server::AItem(),
  __mutex(new Mutex) {
}

Server::Projectile::~Projectile(void) {
  delete this->__mutex;
}

void	Server::Projectile::init(Kernel::ID::id_t id, const Kernel::Serial &serial, const Game::Player *player,
				 int orientation, double x, double y, double z) {
  Game::Projectile::init(id, serial, player, orientation, x, y, z);
  Server::AItem::init();
  Server::Movable::init();
}

bool	Server::Projectile::update(const Clock *clock, double factor) {
  ScopeLock s(this->__mutex);
  if (this->mustBeUpdated()) {
    Server::AItem::_mustUpdate = true;
    this->resetUpdateStatus();
  }
  if (!Server::AItem::update(clock, factor)) {
    return (false);
  }
  Server::Movable::update(clock, factor);
  return (true);
}

void	Server::Projectile::destroy(void) {
  ::Game::Projectile::destroy();
  Server::AItem::destroy();
  Server::Movable::destroy();
}

void	Server::Projectile::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type type) {
  {
    ScopeLock s(this->__mutex);
    ScopeLock s2(Game::Projectile::_mutex);
    if (id == this->_sourceID && type == this->_sourceType) {
      this->_sourceID = 0;
      this->_sourceType = ::Game::NONE;
    }
    if (id == this->_targetID && type == this->_targetType) {
      this->_targetID = 0;
      this->_sourceType = ::Game::NONE;
    }
  }
  Server::AItem::onItemDestroyed(id, type);
}

void	Server::Projectile::_stopAction(void) {
  Server::Movable::_stopAction();
  Server::AItem::_stopAction();
}
