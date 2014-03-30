//
// Movable.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Fri Feb 14 21:31:43 2014 geoffrey bauduin
// Last update Wed Mar 26 16:30:06 2014 geoffrey bauduin
//

#include	<math.h>
#include	"Server/Movable.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Protocol.hpp"
#include	"Factory/Server.hpp"
#include	"Logger.hpp"

#define	DEG(x)	(180.0 * (x) / M_PI)

const Server::Movable::blockedPtrFunc Server::Movable::_blockedPtrFunc[BLOCKED_PTR_FUNC] = {
  {::Game::ACTION_RELEASE, &Server::Movable::blockedNextActionRelease},
  {::Game::ACTION_PICKUP, &Server::Movable::blockedPickUp},
  {::Game::ACTION_BUILDING, &Server::Movable::blockedBuilding},
  {::Game::ACTION_DEPOSIT, &Server::Movable::blockedDeposit},
  {::Game::ACTION_START_HARVEST, &Server::Movable::blockedStartHarvest}
};

const Server::Movable::mUpdatePtrFunc Server::Movable::_mUpdatePtrFunc[M_UPDATE_PTR_FUNC] = {
  {::Game::ACTION_MOVE, &Server::Movable::updateMoveAction},
  {::Game::ACTION_PROJECTILE_FOLLOW, &Server::Movable::updateProjectileFollowAction}
};

const Server::Movable::destinationReachedPtrFunc Server::Movable::_destinationReachedPtrFunc[DESTINATION_REACHED_PTR_FUNC] = {
  {::Game::ACTION_MOVE, &Server::Movable::endActualAction},
  {::Game::ACTION_FOLLOW, &Server::Movable::endActualAction}
};

Server::Movable::Movable(void):
  _mutex(new Mutex), _mustUpdate(false),
  _oldFollowingPos(-1.0, -1.0, -1.0), _stopCondition(NONE), _recalc(0) {

}

Server::Movable::~Movable(void) {
  delete this->_mutex;
}

void	Server::Movable::init(void) {
  this->_mustUpdate = false;
  this->_oldFollowingPos.x = -1.0;
  this->_oldFollowingPos.y = -1.0;
  this->_oldFollowingPos.z = -1.0;
  this->_recalc = 0;
}

void	Server::Movable::destroy(void) {

}

bool	Server::Movable::mustBeUpdated(void) const {
  return (this->_mustUpdate);
}

void	Server::Movable::resetUpdateStatus(void) {
  ScopeLock s(this->_mutex);
  this->_mustUpdate = false;
}

void	Server::Movable::setX(double x) {
  ScopeLock s(this->_mutex);
  ::Game::Movable::setX(x);
  this->_mustUpdate = true;
}

void	Server::Movable::setY(double y) {
  ScopeLock s(this->_mutex);
  ::Game::Movable::setY(y);
  this->_mustUpdate = true;
}

void	Server::Movable::setZ(double z) {
  ScopeLock s(this->_mutex);
  ::Game::Movable::setZ(z);
  this->_mustUpdate = true;
}

void	Server::Movable::setOrientation(int orientation) {
  ScopeLock s(this->_mutex);
  this->_setOrientation(orientation);
}

void	Server::Movable::_setOrientation(int orientation) {
  ::Game::Movable::setOrientation(orientation);
  this->_mustUpdate = true;
}

void	Server::Movable::setPosition(double x, double y, double z) {
  ScopeLock s(this->_mutex);
  ::Game::Movable::setPosition(x, y, z);
  this->_mustUpdate = true;
}

void	Server::Movable::waypointJob(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->itemWaypoint(this->getID(), 0, this->_waypoints[0].x, this->_waypoints[0].y, this->_waypoints[0].z, this->getSpeed());
  this->addJob(job);
}

void	Server::Movable::onChangeWaypoint(void) {
  Vector3d dest = this->_waypoints.front() - this->getPosition();
  dest.z = 0.0;
  Vector3d normal(1.0, 0.0, 0.0);
  double scalar = dest.x * normal.x + dest.y * normal.y;
  int angle = static_cast<int>(DEG(acos(scalar / (sqrt(dest.sqLength()) * normal.sqLength()))));
  this->_setOrientation(angle);
}

void	Server::Movable::onStartMove(double x, double y, double z) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->itemMove(this->getID(), x, y, z);
}

void	Server::Movable::setWaypoints(const std::vector<Vector3d> &waypoints) {
  ScopeLock s(this->_mutex);
  ::Game::Movable::setWaypoints(waypoints);
  this->onChangeWaypoint();
  this->waypointJob();
  this->setStopCondition(NONE);
}

void	Server::Movable::updateMoveAction(const Clock *, double factor) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::MOVE,
							Server::Event::SERVER_GAME, NULL);
  event->moveItem.id = this->getID();
  event->moveItem.type = this->getType();
  event->moveItem.factor = factor;
  this->addEvent(event);
}

void	Server::Movable::updateProjectileFollowAction(const Clock *, double factor) {
  ::Game::Projectile *projectile = static_cast<Server::Projectile *>(this);
  Server::Event *event = Factory::Server::Event::create(Server::Event::FOLLOW,
							Server::Event::SERVER_GAME, NULL);
  event->follow.source.id = this->getID();
  event->follow.source.type = this->getType();
  event->follow.target.id = projectile->getTargetID();
  event->follow.target.type = projectile->getTargetType();
  event->follow.factor = factor;
  this->addEvent(event);
}

void	Server::Movable::updateByAction(const Clock *clock, double factor) {
  for (unsigned int j = 0 ; j < M_UPDATE_PTR_FUNC ; ++j) {
    if (this->_mUpdatePtrFunc[j].action == this->_action->getType()) {
      (this->*_mUpdatePtrFunc[j].func)(clock, factor);
    }
  }
}

bool	Server::Movable::update(const Clock *clock, double factor) {
  {
    ScopeLock s(this->_actionMutex);
    if (this->_action) {
      this->updateByAction(clock, factor);
    }
  }
  return (true);
}

void	Server::Movable::blockedStartHarvest(Server::Action *action, Server::Event **ev) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_SPOT_AND_HARVEST,
							Server::Event::SERVER_GAME, NULL);
  event->harvest.unit = this->getID();
  event->harvest.spot = action->spot;
  *ev = event;
}

void	Server::Movable::blockedNextActionRelease(Server::Action *action, Server::Event **ev) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_POINT_AND_RELEASE_OBJECT,
							Server::Event::SERVER_GAME, NULL);
  event->releaseObject.hero = this->getID();
  event->releaseObject.object = action->release.object;
  event->releaseObject.x = action->release.x;
  event->releaseObject.y = action->release.y;
  event->releaseObject.z = action->release.z;
  *ev = event;
}

// void	Server::Movable::blockedStartAttack(Server::Action *action, Server::Event **ev) {
//   Server::Event *event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_ITEM_AND_ATTACK,
// 							Server::Event::SERVER_GAME, NULL);
//   event->attack.item1.id = this->getID();
//   event->attack.item1.type = this->getType();
//   event->attack.item2.id = action->target.id;
//   event->attack.item2.type = action->target.type;
//   *ev = event;
// }

void	Server::Movable::blockedPickUp(Server::Action *action, Server::Event **ev) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_OBJECT_AND_PICK_UP_OBJECT,
							Server::Event::SERVER_GAME, NULL);
  event->pickUpObject.hero = this->getID();
  event->pickUpObject.object = action->object;
  *ev = event;
}

void	Server::Movable::blockedBuilding(Server::Action *action, Server::Event **ev) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_ITEM_AND_BUILD,
							Server::Event::SERVER_GAME, NULL);
  event->build.unit = this->getID();
  event->build.building = action->building;
  *ev = event;
}

void	Server::Movable::blockedDeposit(Server::Action *action, Server::Event **ev) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_BUILDING_AND_DEPOSIT,
							Server::Event::SERVER_GAME, NULL);
  event->deposit.unit = this->getID();
  event->deposit.building = action->deposit.building;
  event->deposit.type = action->deposit.type;
  *ev = event;
}

void	Server::Movable::onBlocked(void) {
  Server::Event *event = NULL;
  {
    ScopeLock s(this->_actionMutex);
    Server::Action *action = static_cast<Server::Action *>(this->_action);
    if (action && action->getType() == ::Game::ACTION_MOVE) {
      action = action->getNextAction();
    }
    const Vector3d end = this->_waypoints.back();
    if (action) {
    for (unsigned int j = 0 ; j < BLOCKED_PTR_FUNC ; ++j) {
      if (this->_blockedPtrFunc[j].action == action->getType()) {
	(this->*_blockedPtrFunc[j].func)(action, &event);
	break;
      }
    }
    }
    if (event == NULL) {
      this->_recalc++;
      if (this->_recalc < 3) {
    	event = Factory::Server::Event::create(Server::Event::START_MOVE,
    					       Server::Event::SERVER_GAME, NULL);
    	event->move.id = this->getID();
    	event->move.type = this->getType();
    	event->move.x = end.x;
    	event->move.y = end.y;
    	event->move.z = end.z;
      }
      else {
    	this->_recalc = 0;
      }
    }
  }
  this->stopAction();
  if (event) {
    this->addEvent(event);
  }
}

void	Server::Movable::waypointReached(void) {
  ScopeLock s(this->_mutex);
  if (this->hasWaypoints()) {
    this->popWaypoints();
    this->waypointJob();
  }
}

bool	Server::Movable::mustGeneratePath(const Server::AItem *target) {
  ScopeLock s(this->_mutex);
  if (target->getPosition() != this->_oldFollowingPos) {
    this->_oldFollowingPos = target->getPosition();
    return (true);
  }
  return (false);
}

bool	Server::Movable::mustGeneratePath(void) const {
  return (this->_waypoints.empty());
}

void	Server::Movable::onDestinationReached(void) {
  ScopeLock s(this->_actionMutex);
  if (this->_action) {
    for (unsigned int j = 0 ; j < DESTINATION_REACHED_PTR_FUNC ; ++j) {
      if (this->_action->getType() == this->_destinationReachedPtrFunc[j].action) {
	(this->*_destinationReachedPtrFunc[j].func)();
	return ;
      }
    }
    Logger::getInstance()->logf("Action >%d< has no binding when destination is reached", Logger::FATAL, this->_action->getType());
  }
}

void	Server::Movable::endActualAction(void) {
  // Server::Event *event = Factory::Server::Event::create(Server::Event::END_ACTION,
  // 							Server::Event::SERVER_GAME, NULL);
  // event->itemInfos.id = this->getID();
  // event->itemInfos.type = this->getType();
  // this->addEvent(event);
  this->_endAction();
}

void	Server::Movable::setStopCondition(int c) {
  this->_stopCondition = c;
}

int	Server::Movable::getStopCondition(void) const {
  return (this->_stopCondition);
}

void	Server::Movable::_stopAction(void) {

}
