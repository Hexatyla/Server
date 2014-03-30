//
// Unit.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 17:19:51 2014 geoffrey bauduin
// Last update Wed Mar 26 15:19:51 2014 geoffrey bauduin
//

#include	"Server/Unit.hpp"
#include	"Game/Player.hpp"
#include	"Factory/Protocol.hpp"
#include	"Logger.hpp"
#include	"Factory/Server.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Kernel/Config.hpp"

const Server::Unit::uUpdateActionPtrFunc Server::Unit::_uUpdateActionPtrFunc[U_UPDATE_ACTION_PTR_FUNC] = {
  {::Game::ACTION_START_BUILDING, &Server::Unit::updateActionStartBuilding},
  {::Game::ACTION_DEPOSIT, &Server::Unit::updateActionDeposit},
  {::Game::ACTION_PATROL, &Server::Unit::updatePatrolAction},
  {::Game::ACTION_START_HARVEST, &Server::Unit::updateActionStartHarvest}
};

Server::Unit::Unit(void):
  ::Game::AElement(), ::Game::Unit(), Server::AItem(), Server::Movable(), Server::CanUseCapacity(), Server::CanBeDamaged(),
  _mutex(new Mutex), _initialized(false), _targeting(false), _inRangeMutex(new Mutex) {
}

Server::Unit::~Unit(void) {
  delete this->_mutex;
  delete this->_inRangeMutex;
}

void	Server::Unit::init(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
			   int orientation, double x, double y, double z, ::Game::Type type) {
  this->_initialized = false;
  if (type != ::Game::HERO) {
    ::Game::Unit::init(id, serial, player, orientation, x, y, z);
  }
  Server::AItem::init();
  Server::Movable::init();
  Server::CanUseCapacity::init();
  Server::CanBeDamaged::init();
  this->_targeting = false;
}

void	Server::Unit::destroy(void) {
  ::Game::Unit::destroy();
  ::Server::AItem::destroy();
  Server::Movable::destroy();
  Server::CanUseCapacity::destroy();
  Server::CanBeDamaged::destroy();
}

void	Server::Unit::updateActionDeposit(const Clock *, double) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::DEPOSIT_IN_BUILDING,
							Server::Event::SERVER_GAME, NULL);
  event->deposit.unit = this->getID();
  event->deposit.building = this->_action->deposit.building;
  event->deposit.type = this->_action->deposit.type;
  this->_stopAction();
  this->addEvent(event);
}

void	Server::Unit::updateActionStartBuilding(const Clock *, double) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::UNIT_BUILD,
							Server::Event::SERVER_GAME, NULL);
  event->build.unit = this->getID();
  event->build.building = this->_action->building;
  this->_stopAction();
  this->addEvent(event);
}

void	Server::Unit::updateActionStartHarvest(const Clock *, double) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::HARVEST_SPOT,
							Server::Event::SERVER_GAME, NULL);
  event->harvest.unit = this->getID();
  event->harvest.spot = this->_action->spot;
  this->_stopAction();
  this->addEvent(event);
}

void	Server::Unit::updateByAction(const Clock *clock, double factor) {
  for (unsigned int j = 0 ; j < U_UPDATE_ACTION_PTR_FUNC ; ++j) {
    if (this->_uUpdateActionPtrFunc[j].action == this->_action->getType()) {
      (this->*_uUpdateActionPtrFunc[j].func)(clock, factor);
      break;
    }
  }
}

bool	Server::Unit::update(const Clock *clock, double factor) {
  ScopeLock s(this->_mutex);
  this->_initialized = true;
  if (this->mustBeUpdated()) {
    Server::AItem::_mustUpdate = true;
    this->resetUpdateStatus();
  }
  {
    ScopeLock s(this->_actionMutex);
    if (this->_action) {
      this->updateByAction(clock, factor);
    }
  }
  if (!Server::AItem::update(clock, factor)) {
    return (false);
  }
  Server::CanUseCapacity::update(clock, factor);
  Server::Movable::update(clock, factor);
  Server::CanBeDamaged::update(clock, factor);
  return (true);
}

void	Server::Unit::onStanceChange(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->unitStance(this->getID(), this->getStance());
  this->addJob(job);
}

void	Server::Unit::setStance(::Game::Stance::Type stance) {
  if (stance != this->getStance()) {
    ::Game::Unit::setStance(stance);
    this->onStanceChange();
  }
}

void	Server::Unit::addRessources(::Game::Ressources::Type type, int amount) {
  ScopeLock s(this->_mutex);
  ::Game::Unit::addRessources(type, amount);
  if (this->getRessources(type) > Kernel::Config::getInstance()->getUInt(Kernel::Config::HARVESTER_CAPACITY)) {
    this->setRessources(type, Kernel::Config::getInstance()->getUInt(Kernel::Config::HARVESTER_CAPACITY));
  }
  if (this->getRessources(type) == Kernel::Config::getInstance()->getUInt(Kernel::Config::HARVESTER_CAPACITY)) {
    this->stopAction();
    Server::Event *event = Factory::Server::Event::create(Server::Event::STOP_HARVEST_AND_DEPOSIT,
							  Server::Event::SERVER_GAME, NULL);
    event->harvest.unit = this->getID();
    event->harvest.spot = this->_spot;
    this->addEvent(event);
  }
  this->onRessourcesChange();
}

void	Server::Unit::setRessources(::Game::Ressources::Type type, unsigned int amount) {
  ::Game::Unit::setRessources(type, amount);
  this->onRessourcesChange();
}

void	Server::Unit::removeRessources(::Game::Ressources::Type type, int amount) {
  ::Game::Unit::removeRessources(type, amount);
  this->onRessourcesChange();
}

Protocol::Job *Server::Unit::getRessJob(void) const {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->unitPickingUpRessources(this->getID(), this->getRessources(::Game::Ressources::GOLD),
			       this->getRessources(::Game::Ressources::WOOD),
			       this->getRessources(::Game::Ressources::FOOD));
  return (job);
}

void	Server::Unit::onRessourcesChange(void) {
  Protocol::Job *job = this->getRessJob();
  this->addJob(job);
}

unsigned int	Server::Unit::depositRessources(::Game::Ressources::Type type) {
  ScopeLock s(this->_mutex);
  unsigned int amount = this->getRessources(type);
  if (amount > 0) {
    this->removeRessources(type, amount);
  }
  Server::Event *event = Factory::Server::Event::create(Server::Event::UNIT_GO_BACK_TO_SPOT, Server::Event::SERVER_GAME, NULL);
  event->harvest.unit = this->getID();
  event->harvest.spot = this->_spot;
  this->addEvent(event);
  return (amount);
}

void	Server::Unit::itemInRange(const Server::AItem *item) {
  ScopeLock s(this->_inRangeMutex);
  if (this->getStance() == ::Game::Stance::OFFENSIVE && this->_targeting == false) {
    if (item->getTeam() != this->getTeam()) {
      // if (this->getAttackCapacity() != "" || this->getAttackZoneCapacity() != "") {
      // 	Server::Event *event = Factory::Server::Event::create(Server::Event::START_USE_CAPACITY,
      // 							      Server::Event::SERVER_GAME, NULL);
      // 	event->useCapacity.item.id = this->getID();
      // 	event->useCapacity.item.type = this->getType();
      // 	if (this->getAttackCapacity() != "") {
      // 	  event->useCapacity.type = ::Game::Capacity::ITEM;
      // 	  Kernel::assign_serial(event->useCapacity.capacity, this->getAttackCapacity());
      // 	  event->useCapacity.target.id = item->getID();
      // 	  event->useCapacity.target.type = item->getType();
      // 	}
      // 	else {
      // 	  event->useCapacity.type = ::Game::Capacity::ZONE;
      // 	  Kernel::assign_serial(event->useCapacity.capacity, this->getAttackZoneCapacity());
      // 	  event->useCapacity.point.x = item->getX();
      // 	  event->useCapacity.point.y = item->getY();
      // 	  event->useCapacity.point.z = item->getZ();
      // 	}
      // 	this->addEvent(event);
      // 	this->_targeting = true;
      // }
      Server::Event *event = Factory::Server::Event::create(Server::Event::START_MOVE,
							    Server::Event::SERVER_GAME, NULL);
      event->move.id = this->getID();
      event->move.type = this->getType();
      event->move.x = item->getX();
      event->move.y = item->getY();
      event->move.z = item->getZ();
      this->_targeting = true;
      this->addEvent(event);
      this->setGroupMove(NULL);
    }
  }
}

void	Server::Unit::onAttacked(const Server::AItem *item) {
  ScopeLock s(this->_inRangeMutex);
  if (this->getStance() == ::Game::Stance::DEFENSIVE && this->_targeting == false) {
    if (item->getTeam() != this->getTeam()) {
      if (this->getAttackCapacity() != "" || this->getAttackZoneCapacity() != "") {
	Server::Event *event = Factory::Server::Event::create(Server::Event::START_USE_CAPACITY,
							      Server::Event::SERVER_GAME, NULL);
	event->useCapacity.item.id = this->getID();
	event->useCapacity.item.type = this->getType();
	if (this->getAttackCapacity() != "") {
	  event->useCapacity.type = ::Game::Capacity::ITEM;
	  Kernel::assign_serial(event->useCapacity.capacity, this->getAttackCapacity());
	  event->useCapacity.target.id = item->getID();
	  event->useCapacity.target.type = item->getType();
	}
	else {
	  event->useCapacity.type = ::Game::Capacity::ZONE;
	  Kernel::assign_serial(event->useCapacity.capacity, this->getAttackZoneCapacity());
	  event->useCapacity.point.x = item->getX();
	  event->useCapacity.point.y = item->getY();
	  event->useCapacity.point.z = item->getZ();
	}
	this->addEvent(event);
	this->_targeting = true;
      }
    }
  }
}

void	Server::Unit::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type type) {
  ScopeLock s(this->_mutex);
  Server::CanBeDamaged::onItemDestroyed(id, type);
  if (type == ::Game::RESSSPOT && id == this->_spot) {
    this->setSpot(0);
  }
  Server::AItem::onItemDestroyed(id, type);
}

void	Server::Unit::updatePatrolAction(const Clock *, double factor) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::MOVE_TO_POINT,
							Server::Event::SERVER_GAME, NULL);
  event->moveToPoint.item.id = this->getID();
  event->moveToPoint.item.type = this->getType();
  event->moveToPoint.factor = factor;
  if (this->_action->patrol.way == true) {
    event->moveToPoint.point.x = this->_action->patrol.to.x;
    event->moveToPoint.point.y = this->_action->patrol.to.y;
  }
  else {
    event->moveToPoint.point.x = this->_action->patrol.from.x;
    event->moveToPoint.point.y = this->_action->patrol.from.y;
  }
  this->addEvent(event);
}

void	Server::Unit::onDestinationReached(void) {
  {
    ScopeLock s(this->_actionMutex);
    if (this->_action && this->_action->getType() == ::Game::ACTION_PATROL) {
      this->_action->patrol.way = !this->_action->patrol.way;
      return ;
    }
  }
  Server::Movable::onDestinationReached();
}

void	Server::Unit::onBlocked(void) {
  if (this->_action->getType() == ::Game::ACTION_PATROL) {
    this->clearWaypoints();
  }
  else {
    Server::Movable::onBlocked();
  }
}

Protocol::Job	*Server::Unit::createInfosJob(void) const {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->updateItem(this->getID(), this->getX(), this->getY(), this->getZ(), this->getOrientation(),
		  this->getCurrentLife(), this->getCurrentRessources(), this->_xp, this->_level);
  return (job);
}

void	Server::Unit::setXP(unsigned int value) {
  ::Game::Unit::setXP(value);
  Server::AItem::_mustUpdate = true;
}

void	Server::Unit::onStatsChanged(void) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::STATS_CHANGED,
							Server::Event::SERVER_GAME, NULL);
  event->item.id = this->getID();
  event->item.type = this->getType();
  this->addEvent(event);
}

void	Server::Unit::setLevel(unsigned int level) {
  ::Game::Unit::setLevel(level);
  Server::AItem::_mustUpdate = true;
  if (this->_initialized) {
    this->onStatsChanged();
  }
}

void	Server::Unit::addXP(unsigned int amount) {
  ::Game::Unit::addXP(amount);
  Server::AItem::_mustUpdate = true;
}

void	Server::Unit::stopAction(void) {
  Server::AItem::stopAction();
  ScopeLock s(this->_inRangeMutex);
  this->_targeting = false;
}

void	Server::Unit::_stopAction(void) {
  if (this->_action) {
    switch (this->_action->getType()) {
    case ::Game::ACTION_HARVEST:
      {
	Server::Event *event = Factory::Server::Event::create(Server::Event::STOP_ACTION_HARVEST,
							      Server::Event::SERVER_GAME, NULL);
	event->harvest.unit = this->getID();
	event->harvest.spot = this->_action->spot;
	this->addEvent(event);
      }
      break;
    case ::Game::ACTION_BUILDING:
      {
	Server::Event *event = Factory::Server::Event::create(Server::Event::STOP_ACTION_BUILD,
							      Server::Event::SERVER_GAME, NULL);
	event->build.unit = this->getID();
	event->build.building = this->_action->building;
	this->addEvent(event);
      }
    default:
      break;
    }
    Server::AItem::_stopAction();
  }
}
