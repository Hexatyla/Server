//
// CanBeDamaged.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Sun Feb 23 00:37:41 2014 geoffrey bauduin
// Last update Tue Mar 25 10:20:04 2014 geoffrey bauduin
//

#include	"Server/CanBeDamaged.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Server/Projectile.hpp"
#include	"Kernel/Config.hpp"
#include	"Factory/Factory.hpp"

Server::CanBeDamaged::CanBeDamaged(void):
  _mutex(new Mutex), _damagedBy(), _lastDamage(NULL),
  _damaged(false) {

}

Server::CanBeDamaged::~CanBeDamaged(void) {
  delete this->_mutex;
}

void	Server::CanBeDamaged::init(void) {
  this->_lastDamage = NULL;
}

void	Server::CanBeDamaged::destroy(void) {
  this->_damagedBy.clear();
  if (this->_lastDamage) {
    Factory::Clock::remove(this->_lastDamage);
    this->_lastDamage = NULL;
  }
}

const std::map<Kernel::ID::id_t, unsigned int> &Server::CanBeDamaged::getAttackers(void) const {
  ScopeLock s(this->_mutex);
  return (this->_damagedBy);
}

void	Server::CanBeDamaged::takeDamage(const Server::AItem *element) {
  ScopeLock s(this->_mutex);
  unsigned int rdm = rand() % 100;
  if (rdm < Kernel::Config::getInstance()->getUInt(Kernel::Config::MISS_CHANCE)) {
    return ;
  }
  this->_damaged = true;
  Kernel::ID::id_t attacker = 0;
  unsigned int damageAmount = 0;
  double critChance = 0.0;
  switch (element->getType()) {
  case ::Game::PROJECTILE:
    damageAmount = static_cast<const Server::Projectile *>(element)->getAttack();
    attacker = static_cast<const Server::Projectile *>(element)->getSourceID();
    critChance = static_cast<const Server::Projectile *>(element)->getCritChance();
    break;
  default:
    break;
  }
  if (attacker != 0) {
    if (rand() % 100 <= static_cast<int>(100.0 * critChance)) {
      damageAmount = static_cast<unsigned int>(Kernel::Config::getInstance()->getDouble(Kernel::Config::CRIT_CHANCE_MULTIPLIER) * static_cast<double>(damageAmount));
    }
    damageAmount -= this->getDefense();
    this->_damagedBy[attacker] += damageAmount;
    this->removeLife(damageAmount);
  }
}

void	Server::CanBeDamaged::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type) {
  ScopeLock s(this->_mutex);
  this->_damagedBy.erase(id);
}

bool	Server::CanBeDamaged::update(const Clock *clock, double) {
  ScopeLock s(this->_mutex);
  if (this->_damaged) {
    if (!this->_lastDamage) {
      this->_lastDamage = Factory::Clock::create();
    }
    this->_lastDamage->clone(clock);
    this->_damaged = false;
  }
  else if (this->_lastDamage &&
	   clock->getElapsedTimeSince(this->_lastDamage) >= Kernel::Config::getInstance()->getDouble(Kernel::Config::TIMER_OUT_OF_COMBAT)) {
    this->_damagedBy.clear();
    Factory::Clock::remove(this->_lastDamage);
    this->_lastDamage = NULL;
  }
  return (true);
}
