//
// RessourcesSpot.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Mon Feb 10 13:44:36 2014 geoffrey bauduin
// Last update Mon Mar 24 18:52:30 2014 geoffrey bauduin
//

#include	"Server/RessourcesSpot.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Protocol.hpp"
#include	"Factory/Server.hpp"
#include	"Kernel/Config.hpp"

Server::RessourcesSpot::RessourcesSpot(void):
  ::Game::AElement(), ::Game::RessourcesSpot(), Server::AItem(),
  _mutex(new Mutex), _harvestersClock(), _hCapa(), _waitlist() {

}

Server::RessourcesSpot::~RessourcesSpot(void) {
  delete this->_mutex;
}

void	Server::RessourcesSpot::init(Kernel::ID::id_t id, double x, double y, double z, ::Game::Ressources::Type type, unsigned int amount) {
  ::Game::RessourcesSpot::init(id, x, y, z, type, amount);
  Server::AItem::init();
  this->_waitlist.init();
}

void	Server::RessourcesSpot::destroy(void) {
  ::Game::RessourcesSpot::destroy();
  Server::AItem::destroy();
  this->_waitlist.destroy();
  this->_harvestersClock.clear();
  this->_hCapa.clear();
}

void	Server::RessourcesSpot::onUnitAddedToHarvesterList(Kernel::ID::id_t unitID) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->unitHarvest(unitID, this->getID());
  this->addJob(job);
}

void	Server::RessourcesSpot::onUnitRemovedFromHarvesterList(Kernel::ID::id_t unitID) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->unitStopHarvest(unitID);
  this->addJob(job);
}

void	Server::RessourcesSpot::onAddToWaitlist(Kernel::ID::id_t id) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->unitWaitHarvest(id);
  this->addJob(job);
}

void	Server::RessourcesSpot::addHarvester(Kernel::ID::id_t id, unsigned int amount) {
  ScopeLock s(this->_mutex);
  if (this->canBeHarvested()) {
    ::Game::RessourcesSpot::addHarvester(id);
    this->_harvestersClock[id] = 0.0;
    this->onUnitAddedToHarvesterList(id);
  }
  else {
    this->_waitlist.push(id);
    this->onAddToWaitlist(id);
  }
  this->_hCapa[id] = amount;
}

void	Server::RessourcesSpot::moveOneFromWaitlist(void) {
  Kernel::ID::id_t unit = this->_waitlist.pop();
  Server::Event *event = Factory::Server::Event::create(Server::Event::WAITLIST_TO_HARVEST,
							Server::Event::SERVER_GAME, NULL);
  event->harvest.unit = unit;
  event->harvest.spot = this->getID();
  this->addEvent(event);
  this->onRemoveFromWaitlist(unit);
}

void	Server::RessourcesSpot::onRemoveFromWaitlist(Kernel::ID::id_t id) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->unitStopWaitHarvest(id);
  this->addJob(job);
}

bool	Server::RessourcesSpot::_removeHarvester(Kernel::ID::id_t id) {
  if (this->isHarvesting(id)) {
    ::Game::RessourcesSpot::removeHarvester(id);
    this->_harvestersClock.erase(id);
    this->_hCapa.erase(id);
    if (this->_waitlist.empty() == false) {
      this->moveOneFromWaitlist();
    }
    return (true);
  }
  this->_waitlist.remove(id);
  return (false);
}

void	Server::RessourcesSpot::removeHarvester(Kernel::ID::id_t id) {
  ScopeLock s(this->_mutex);
  if (this->_removeHarvester(id)) {
    this->onUnitRemovedFromHarvesterList(id);
  }
  else {
    this->onRemoveFromWaitlist(id);
  }
}

void	Server::RessourcesSpot::createUnitHarvestEvent(Kernel::ID::id_t id, unsigned int amount) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::UNIT_ADD_RESS, Server::Event::SERVER_GAME, NULL);
  event->harvestAmount.unit = id;
  event->harvestAmount.spot = this->getID();
  event->harvestAmount.amount = amount;
  event->harvestAmount.type = this->_type;
  this->addEvent(event);
}

bool	Server::RessourcesSpot::update(const Clock *clock, double) {
  ScopeLock s(this->_mutex);
  bool upd = false;
  for (auto it : this->_harvestersClock) {
    if (it.second == 0.0) {
      this->_harvestersClock[it.first] = clock->getElapsedTime() + Kernel::Config::getInstance()->getDouble(Kernel::Config::HARVEST_TIME_BETWEEN_TICKS);
    }
    else if (clock->getElapsedTime() >= it.second) {
      if (this->_hCapa[it.first] < Kernel::Config::getInstance()->getUInt(Kernel::Config::HARVESTER_CAPACITY)) {
	upd = true;
	this->_harvestersClock[it.first] += Kernel::Config::getInstance()->getDouble(Kernel::Config::HARVEST_TIME_BETWEEN_TICKS);
	this->_capacity.add(this->_type, -Kernel::Config::getInstance()->getUInt(Kernel::Config::HARVEST_AMOUNT_PER_TICK));
	this->createUnitHarvestEvent(it.first, Kernel::Config::getInstance()->getUInt(Kernel::Config::HARVEST_AMOUNT_PER_TICK));
	this->_hCapa[it.first]++;
      }
      if (this->_hCapa[it.first] >= Kernel::Config::getInstance()->getUInt(Kernel::Config::HARVESTER_CAPACITY)) {
	this->_removeHarvester(it.first);
      }
    }
  }
  if (upd) {
    this->onUpdate();
  }
  return (true);
}

void	Server::RessourcesSpot::onUpdate(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->updateSpot(this->getID(), this->_capacity.get(this->_type));
  this->addJob(job);
}

Protocol::Job	*Server::RessourcesSpot::getRessJob(void) const {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->ressourceSpot(this->getID(), this->getX(), this->getY(), this->getZ(), this->_type, this->_capacity.get(this->_type));
  return (job);
}

void	Server::RessourcesSpot::onCreate(void) {
  Protocol::Job *job = this->getRessJob();
  this->addJob(job);
}

void	Server::RessourcesSpot::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type type) {
  ScopeLock s(this->_mutex);
  if (type == ::Game::UNIT && this->_harvestersClock.find(id) != this->_harvestersClock.end()) {
    this->_removeHarvester(id);
  }
  Server::AItem::onItemDestroyed(id, type);
}
