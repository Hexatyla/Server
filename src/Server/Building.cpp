//
// Building.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 17:19:51 2014 geoffrey bauduin
// Last update Tue Mar 25 11:40:54 2014 geoffrey bauduin
//

#include	"Server/Building.hpp"
#include	"Game/Player.hpp"
#include	"Factory/Factory.hpp"
#include	"Factory/Protocol.hpp"
#include	"Factory/Server.hpp"
#include	"Kernel/Time.hpp"
#include	"Kernel/Manager.hpp"
#include	"Kernel/Config.hpp"

Server::Building::Building(void):
  ::Game::AElement(), ::Game::Building(), Server::AItem(), Server::CanUseCapacity(), Server::CanBeDamaged(),
  _dataMutex(new Mutex), _buildersMutex(new Mutex),
  _timers()  {}

Server::Building::~Building(void) {
  delete this->_dataMutex;
  delete this->_buildersMutex;
}

void	Server::Building::init(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
			       int orientation, double x, double y, double z) {
  ::Game::Building::init(id, serial, player, orientation, x, y, z);
  Server::AItem::init();
  Server::CanUseCapacity::init();
  Server::CanBeDamaged::init();
}

void	Server::Building::destroy(void) {
  this->_timers.clear();
  ::Game::Building::destroy();
  Server::AItem::destroy();
  Server::CanUseCapacity::destroy();
  Server::CanBeDamaged::destroy();
}

void	Server::Building::updateCurrentLife(const Clock *clock) {
  double amount = 0;
  for (auto it : this->_timers) {
    if (it.second == 0.0) {
      this->_timers[it.first] = clock->getElapsedTime() + Kernel::Config::getInstance()->getDouble(Kernel::Config::BUILD_BUILDING_TIME_BETWEEN_TICKS);
    }
    else if (clock->getElapsedTime() >= it.second) {
      amount += Kernel::Config::getInstance()->getUInt(Kernel::Config::BUILD_BUILDERS_AMOUNT_PER_TICK);
      this->_timers[it.first] += Kernel::Config::getInstance()->getDouble(Kernel::Config::BUILD_BUILDING_TIME_BETWEEN_TICKS);
      if (this->getCurrentLife() + amount >= this->getMaxLife()) {
	break;
      }
    }
  }
  if (amount > 0) {
    this->addLife(amount);
  }
  if (this->getCurrentLife() >= this->getMaxLife()) {
    this->setBuilt();
  }

}

bool	Server::Building::update(const Clock *clock, double factor) {
  ScopeLock s(this->_dataMutex);
  Server::CanUseCapacity::update(clock, factor);
  Server::CanBeDamaged::update(clock, factor);
  if (!Server::AItem::update(clock, factor)) {
    return (false);
  }
  if (this->isBuilt() == false) {
    this->updateCurrentLife(clock);
  }
  Kernel::Serial produced;
  Server::HasProductionQueue::Status status = Server::HasProductionQueue::update(clock, &this->_queue, produced);
  if (status == Server::HasProductionQueue::END || status == Server::HasProductionQueue::END_RESTART) {
    this->onFinishedUnitProduction(produced);
  }
  return (true);
}

void	Server::Building::onAddUnitToQueue(int pos) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->unitProduction(this->getID(),
		      pos == -1 ? this->_queue.size() - 1 : pos,
		      pos == -1 ? this->_queue.back() : this->_queue.at(pos));
  this->addJob(job);
}

void	Server::Building::onFinishedUnitProduction(const Kernel::Serial &serial) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::CREATE_ITEM, Server::Event::SERVER_GAME, NULL);
  Kernel::assign_serial(event->create.serial, serial);
  event->create.x = this->getX();
  event->create.y = this->getY();
  event->create.z = this->getZ();
  event->create.o = this->getOrientation();
  event->create.player = this->getPlayer();
  this->addEvent(event);

  Protocol::Job *job = Factory::Protocol::Job::create();
  job->unitHasBeenProduced(this->getID(), serial);
  this->addJob(job);
}

void	Server::Building::onRemoveUnitFromQueue(const Kernel::Serial &serial) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->removeUnitFromProduction(this->getID(), serial);
  this->addJob(job);
}

void	Server::Building::addToQueue(const Kernel::Serial &serial) {
  ::Game::Building::addToQueue(serial);
  this->onAddUnitToQueue();
}

void	Server::Building::placeInQueue(const Kernel::Serial &serial, unsigned int pos) {
  ::Game::Building::placeInQueue(serial, pos);
  this->onAddUnitToQueue(pos);
}

bool	Server::Building::removeFromQueue(const Kernel::Serial &serial) {
  if (::Game::Building::removeFromQueue(serial)) {
    this->onRemoveUnitFromQueue(serial);
    return (true);
  }
  return (false);
}

void	Server::Building::addBuilder(Kernel::ID::id_t unitID) {
  ScopeLock s(this->_buildersMutex);
  ::Game::Building::addBuilder(unitID);
  this->_timers[unitID] = 0.0;
  this->onAddBuilder(unitID);
}

void	Server::Building::removeBuilder(Kernel::ID::id_t unitID) {
  ScopeLock s(this->_buildersMutex);
  ::Game::Building::removeBuilder(unitID);
  this->_timers.erase(unitID);
  this->onRemoveBuilder(unitID);
}

void	Server::Building::onAddBuilder(Kernel::ID::id_t unitID) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->unitCreateBuilding(unitID, this->getID());
  this->addJob(job);
}

void	Server::Building::onRemoveBuilder(Kernel::ID::id_t unitID) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->unitStopBuildingBuilding(unitID, this->getID());
  this->addJob(job);
}

void	Server::Building::onSetBuilt(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->buildingHasBeenBuilt(this->getID());
  this->addJob(job);
}

void	Server::Building::createStopBuildingEvent(Kernel::ID::id_t unitID) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::STOP_BUILDING, Server::Event::SERVER_GAME, NULL);
  event->build.unit = unitID;
  event->build.building = this->getID();
  this->addEvent(event);
}

void	Server::Building::setBuilt(void) {
  ::Game::Building::setBuilt();
  for (auto it : this->_builders) {
    this->createStopBuildingEvent(it);
  }
  this->onSetBuilt();
}

void	Server::Building::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type type) {
  ScopeLock s(this->_buildersMutex);
  if (type == ::Game::UNIT && this->_timers.find(id) != this->_timers.end()) {
    this->_timers.erase(id);
  }
  Server::AItem::onItemDestroyed(id, type);
}
