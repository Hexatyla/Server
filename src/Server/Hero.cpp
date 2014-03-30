//
// Hero.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 17:19:51 2014 geoffrey bauduin
// Last update Sat Mar 15 16:11:42 2014 geoffrey bauduin
//

#include	"Server/Hero.hpp"
#include	"Game/Player.hpp"
#include	"Factory/Protocol.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Server.hpp"

const Server::Hero::hUpdateActionPtrFunc Server::Hero::_hUpdateActionPtrFunc[H_UPDATE_ACTION_PTR_FUNC] = {
  {::Game::ACTION_RELEASE, &Server::Hero::updateReleaseAction},
  {::Game::ACTION_PICKUP, &Server::Hero::updatePickUpAction}
};

Server::Hero::Hero(void):
  ::Game::AElement(), ::Game::Unit(), ::Game::Hero(),
  _mutex(new Mutex) {

}

Server::Hero::~Hero(void) {
  delete this->_mutex;
}

void	Server::Hero::init(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
			   int orientation, double x, double y, double z) {
  ::Game::Hero::init(id, serial, player, orientation, x, y, z);
  Server::Unit::init(id, serial, player, orientation, x, y, z, ::Game::HERO);
}

void	Server::Hero::updateReleaseAction(const Clock *, double) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::RELEASE_OBJECT,
							Server::Event::SERVER_GAME, NULL);
  event->releaseObject.hero = this->getID();
  event->releaseObject.object = this->_action->release.object;
  event->releaseObject.x = this->_action->release.x;
  event->releaseObject.y = this->_action->release.y;
  event->releaseObject.z = this->_action->release.z;
  this->stopAction();
  this->addEvent(event);
}

void	Server::Hero::updatePickUpAction(const Clock *, double) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::PICK_UP_OBJECT,
							Server::Event::SERVER_GAME, NULL);
  event->pickUpObject.hero = this->getID();
  event->pickUpObject.object = this->_action->object;
  this->stopAction();
  this->addEvent(event);
}

void	Server::Hero::updateByAction(const Clock *clock, double factor) {
  for (unsigned int j = 0 ; j < H_UPDATE_ACTION_PTR_FUNC ; ++j) {
    if (this->_hUpdateActionPtrFunc[j].action == this->_action->getType()) {
      (this->*_hUpdateActionPtrFunc[j].func)(clock, factor);
      break;
    }
  }
}

bool	Server::Hero::update(const Clock *clock, double factor) {
  ScopeLock s(this->_mutex);
  if (this->_action) {
    this->updateByAction(clock, factor);
  }
  if (!Server::Unit::update(clock, factor)) {
    return (false);
  }
  return (true);
}

void	Server::Hero::destroy(void) {
  ::Game::Hero::destroy();
  Server::Unit::destroy();
}

void	Server::Hero::onPickedUpObject(Kernel::ID::id_t id) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->heroPickedUpAnObject(this->getID(), id);
  this->addJob(job);
}

void	Server::Hero::onReleasedObject(Kernel::ID::id_t id) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->heroReleasedObject(this->getID(), id);
  this->addJob(job);
}

void	Server::Hero::addObject(::Game::Object *object) {
  ::Game::Hero::addObject(object);
  this->onPickedUpObject(object->getID());
}

void	Server::Hero::removeObject(::Game::Object *object) {
  ::Game::Hero::removeObject(object);
  this->onReleasedObject(object->getID());
}
