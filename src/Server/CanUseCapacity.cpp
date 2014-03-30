//
// CanUseCapacity.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Fri Feb 21 11:08:02 2014 geoffrey bauduin
// Last update Wed Mar 26 10:59:15 2014 geoffrey bauduin
//

#include	"Server/CanUseCapacity.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Protocol.hpp"
#include	"Factory/Server.hpp"
#include	"Factory/Factory.hpp"
#include	"Kernel/Config.hpp"
#include	"Kernel/CapacityData.hpp"

const Server::CanUseCapacity::cUpdatePtrFunc Server::CanUseCapacity::_cUpdatePtrFunc[C_UPDATE_PTR_FUNC] = {
  {::Game::ACTION_START_CAPACITY, &Server::CanUseCapacity::updateStartCapacity},
  {::Game::ACTION_CAST_CAPACITY, &Server::CanUseCapacity::updateCastCapacity}
};

Server::CanUseCapacity::CanUseCapacity(void):
  _mutex(new Mutex), _last(NULL), _castTime(0.0)  {

}

Server::CanUseCapacity::~CanUseCapacity(void) {
  delete this->_mutex;
}

void	Server::CanUseCapacity::init(void) {
  ::Game::CanUseCapacity::init();
  this->_last = NULL;
}

void	Server::CanUseCapacity::destroy(void) {
  if (this->_last) {
    Factory::Clock::remove(this->_last);
    this->_last = NULL;
  }
}

void	Server::CanUseCapacity::setTarget(Kernel::ID::id_t id, ::Game::Type type) {
  ScopeLock s(this->_mutex);
  ::Game::CanUseCapacity::setTarget(id, type);
  this->onSetTarget();
}

void	Server::CanUseCapacity::setTarget(double x, double y, double z) {
  ScopeLock s(this->_mutex);
  ::Game::CanUseCapacity::setTarget(x, y, z);
  this->onSetTargetZone();
}

void	Server::CanUseCapacity::onSetTarget(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->itemAttack(this->getID(), this->_target);
  this->addJob(job);
}

void	Server::CanUseCapacity::onSetTargetZone(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->itemAttackZone(this->getID(), this->_targetZone.x, this->_targetZone.y, this->_targetZone.z);
  this->addJob(job);
}

void	Server::CanUseCapacity::updateStartCapacity(const Clock *clock, double) {
  if (this->_last) {
    Factory::Clock::remove(this->_last);
  }
  this->_last = clock->clone();
  Server::Action *action = Factory::Server::Action::create(::Game::ACTION_CAST_CAPACITY);
  action->useCapacity = this->_action->useCapacity;
  this->_setAction(action);
  ::Game::Capacity *capacity = this->_capacities[this->_action->useCapacity.capacity];
  this->_castTime = capacity->getInfos().timer->creation;
}

void	Server::CanUseCapacity::updateCastCapacity(const Clock *clock, double) {
  if (clock->getElapsedTimeSince(this->_last) >= this->_castTime) {
    Server::Event *event = Factory::Server::Event::create(Server::Event::USE_CAPACITY,
							  Server::Event::SERVER_GAME, NULL);
    event->useCapacity = this->_action->useCapacity;
    this->addEvent(event);
  }
}

bool	Server::CanUseCapacity::update(const Clock *clock, double factor) {
  ScopeLock s(this->_mutex);
  ScopeLock s1(this->_actionMutex);
  if (this->_action) {
    for (unsigned int j = 0 ; j < C_UPDATE_PTR_FUNC ; ++j) {
      if (this->_cUpdatePtrFunc[j].action == this->_action->getType()) {
	(this->*_cUpdatePtrFunc[j].func)(clock, factor);
	break;
      }
    }
  }
  return (true);
}

void	Server::CanUseCapacity::onUseCapacity(const Kernel::Serial &serial) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->itemUseCapacity(this->getID(), serial);
  this->addJob(job);
}

void	Server::CanUseCapacity::onUseCapacity(const Kernel::Serial &serial, double x, double y, double z) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  if (serial == this->getAttackZoneCapacity()) {
    job->itemAttackZone(this->getID(), x, y, z);
  }
  else {
    job->itemUseZoneCapacity(this->getID(), serial, x, y, z);
  }
  this->addJob(job);
}

void	Server::CanUseCapacity::onUseCapacity(const Kernel::Serial &serial, Kernel::ID::id_t target) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  if (serial == this->getAttackCapacity()) {
    job->itemAttack(this->getID(), target);
  }
  else {
    job->itemUseCapacity(this->getID(), target, serial);
  }
  this->addJob(job);
}

bool	Server::CanUseCapacity::_useCapacity(const Kernel::Serial &capacity) {
  const Kernel::ElementRessources &cR = this->_capacities[capacity]->getInfos().cost->personnalRessource;
  return (this->removeRessources(cR.getRessource(), cR.getRessourceType()));
}

void	Server::CanUseCapacity::useCapacityTarget(const Server::AItem *target, const Kernel::Serial &capacity) {
  if (!this->_useCapacity(capacity)) {
    return ;
  }
  const Kernel::CapacityData *cD = this->_capacities[capacity]->getInfos().capacity;
  for (auto it : cD->projectiles) {
    this->createProjectileTarget(it, target);
  }
  for (auto it : cD->effects) {
    (void) it;
  }
  this->onUseCapacity(capacity, target->getID());
}

void	Server::CanUseCapacity::createProjectileTarget(const Kernel::Serial &serial, const Server::AItem *target) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::CREATE_PROJ,
							Server::Event::SERVER_GAME, NULL);
  event->projectile.target.id = target->getID();
  event->projectile.target.type = target->getType();
  event->projectile.targetItem = true;
  event->projectile.source.id = this->getID();
  event->projectile.source.type = this->getType();
  event->projectile.player = this->getPlayer();
  Kernel::assign_serial(event->projectile.serial, serial);
  event->projectile.x = this->getX();
  event->projectile.y = this->getY();
  event->projectile.z = this->getZ();
  event->projectile.o = 0;
  this->addEvent(event);
}

void	Server::CanUseCapacity::createProjectileZone(const Kernel::Serial &serial, double x, double y, double z) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::CREATE_PROJ,
							Server::Event::SERVER_GAME, NULL);
  event->projectile.targetItem = false;
  event->projectile.source.id = this->getID();
  event->projectile.source.type = this->getType();
  event->projectile.destination.x = x;
  event->projectile.destination.y = y;
  event->projectile.destination.z = z;
  event->projectile.player = this->getPlayer();
  Kernel::assign_serial(event->projectile.serial, serial);
  event->projectile.x = this->getX();
  event->projectile.y = this->getY();
  event->projectile.z = this->getZ();
  event->projectile.o = 0;
  this->addEvent(event);
}

void	Server::CanUseCapacity::useCapacityZone(const Kernel::Serial &capacity, double x, double y, double z) {
  if (!this->_useCapacity(capacity)) {
    return ;
  }
  const Kernel::CapacityData *cD = this->_capacities[capacity]->getInfos().capacity;
  for (auto it : cD->projectiles) {
    this->createProjectileZone(it, x, y, z);
  }
  for (auto it : cD->effects) {
    (void) it;
  }
  this->onUseCapacity(capacity, x, y, z);
}

void	Server::CanUseCapacity::useCapacity(const Kernel::Serial &capacity) {
  if (!this->_useCapacity(capacity)) {
    return ;
  }
  const Kernel::CapacityData *cD = this->_capacities[capacity]->getInfos().capacity;
  for (auto it : cD->effects) {
    (void) it;
  }
  this->onUseCapacity(capacity);
}
