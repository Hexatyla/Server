//
// AItem.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Thu Jan 23 15:38:48 2014 geoffrey bauduin
// Last update Tue Mar 25 10:13:33 2014 geoffrey bauduin
//

#include	"Server/AItem.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Protocol.hpp"
#include	"Game/AElement.hpp"
#include	"Server/Unit.hpp"
#include	"Game/Player.hpp"
#include	"Logger.hpp"
#include	"Factory/Factory.hpp"
#include	"Kernel/Config.hpp"
#include	"Server/Effect.hpp"
#include	"Factory/Server.hpp"
#include	"Kernel/Manager.hpp"

const Server::AItem::updateActionPtrFunc Server::AItem::_updateActionPtrFunc[UPDATE_ACTION_PTR_FUNC] = {

};

Server::AItem::AItem(void):
  Server::HasEvent(), Server::HasJobs(), NoDeleteWhileUsedByAThread(),
  _dataMutex(new Mutex), _effectMutex(new Mutex), _moveMutex(new Mutex),
  _grpMove(NULL), _lastUpdate(NULL),
  _lastHpRegen(NULL), _lastRessourceRegen(NULL),
  _destroyed(false), _mustUpdate(false) {

}

Server::AItem::~AItem(void) {
  delete this->_dataMutex;
  delete this->_effectMutex;
  delete this->_moveMutex;
}

void	Server::AItem::lockMove(void) {
  this->_moveMutex->lock();
}

void	Server::AItem::unlockMove(void) {
  this->_moveMutex->unlock();
}

void	Server::AItem::destroy(void) {
  for (auto it : this->_effects) {
    for (auto effect : it.second) {
      Factory::Server::Effect::remove(static_cast<Server::Effect *>(effect));
    }
    it.second.clear();
  }
  this->_effects.clear();

  if (this->_grpMove) {
    this->leaveGroupMove();
  }

  for (auto it : this->_capacities) {
    Factory::Server::Capacity::remove(static_cast<Server::Capacity *>(it.second));
  }
  this->_capacities.clear();
  if (this->_lastHpRegen) {
    Factory::Clock::remove(this->_lastHpRegen);
    this->_lastHpRegen = NULL;
  }
  if (this->_lastRessourceRegen) {
    Factory::Clock::remove(this->_lastRessourceRegen);
    this->_lastRessourceRegen = NULL;
  }
  if (this->_lastUpdate) {
    Factory::Clock::remove(this->_lastUpdate);
    this->_lastUpdate = NULL;
  }
  Server::HasEvent::destroy();
  Server::HasJobs::destroy();
}

void	Server::AItem::init(void) {
  NoDeleteWhileUsedByAThread::init();
  const Kernel::CapacityList *cList = Kernel::Manager::getInstance()->getCapacityList(this->serial());
  if (cList) {
    for (auto it : cList->list) {
      bool isAuto = Kernel::Manager::getInstance()->getCapacityData(it)->isAuto;
      this->_capacities[it] = Factory::Server::Capacity::create(it, isAuto,
								this->getPlayer(), this->getID());
    }
  }

  Server::HasEvent::init();
  Server::HasJobs::init();
  this->_destroyed = false;
  this->onCreate();
  this->_mustUpdate = false;
}

void	Server::AItem::onCreate(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->createItem(this->serial(), this->getID(),
		  this->getX(), this->getY(), this->getZ(), this->getOrientation(),
		  this->getPlayer()->getID());
  this->addJob(job);
}

void	Server::AItem::onDestroy(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->deleteItem(this->getID());
  this->addJob(job);
}

Protocol::Job	*Server::AItem::createInfosJob(void) const {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->updateItem(this->getID(), this->getX(), this->getY(), this->getZ(), this->getOrientation(),
		  this->getCurrentLife(), this->getCurrentRessources(), 0, 0);
  return (job);
}

void	Server::AItem::onUpdate(void) {
  Protocol::Job *job = this->createInfosJob();
  this->addJob(job);
}

Protocol::Job	*Server::AItem::onAskInfos(void) const {
  return (this->createInfosJob());
}

Protocol::Job	*Server::AItem::createItemActionJob(const ::Game::Action *action) const {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->itemAction(this->getID(), action->getType());
  return (job);
}

void	Server::AItem::onSetAction(const ::Game::Action *action) {
  Protocol::Job *job = this->createItemActionJob(action);
  this->addJob(job);
}

Protocol::Job	*Server::AItem::onAskItemAction(void) const {
  return (this->createItemActionJob(this->getAction()));
}

void	Server::AItem::onStopAction(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->itemStopAction(this->getID());
  this->addJob(job);
}

void	Server::AItem::_stopAction(void) {
  if (this->_action) {
    Factory::Server::Action::remove(static_cast<Server::Action *>(this->_action));
    this->_action = NULL;
    //    this->leaveGroupMove();
    this->onStopAction();
  }
}

void	Server::AItem::stopAction(void) {
  ScopeLock s(this->_actionMutex);
  this->_stopAction();
}

void	Server::AItem::endAction(void) {
  ScopeLock s(this->_actionMutex);
  this->_endAction();
}

void	Server::AItem::_endAction(void) {
  if (this->_action) {
    Server::Action *action = static_cast<Server::Action *>(this->_action)->getNextAction();
    this->_stopAction();
    if (action) {
      Logger::getInstance()->logf("Set next action to %d", Logger::DEBUG, action->getType());
      this->_setAction(action);
    }
  }
}

void	Server::AItem::_setAction(::Game::Action *action) {
  ::Game::AElement::_setAction(action);
  this->onSetAction(action);
}

void	Server::AItem::setAction(::Game::Action *action) {
  ScopeLock s(this->_actionMutex);
  this->_setAction(action);
}

void	Server::AItem::updateLife(const Clock *clock, double) {
  if (this->_built && this->getCurrentLife() != this->getMaxLife()) {
    if (!this->_lastHpRegen ||
	clock->getElapsedTimeSince(this->_lastHpRegen) >= Kernel::Config::getInstance()->getDouble(Kernel::Config::HP_REGEN_TIMER)) {
      if (!this->_lastHpRegen) {
	this->_lastHpRegen = clock->clone();
      }
      else {
	this->_lastHpRegen->clone(clock);
      }
      this->_ressources.changeLife(this->getHpRegen());
      this->_mustUpdate = true;
    }
  }
}

void	Server::AItem::updatePersonnalRessources(const Clock *clock, double) {
  if (this->_built && this->getCurrentRessources() != this->getMaxRessources()) {
    if (!this->_lastRessourceRegen ||
	clock->getElapsedTimeSince(this->_lastRessourceRegen) >= Kernel::Config::getInstance()->getDouble(Kernel::Config::RESS_REGEN_TIMER)) {
      if (!this->_lastRessourceRegen) {
	this->_lastRessourceRegen = clock->clone();
      }
      else {
	this->_lastRessourceRegen->clone(clock);
      }
      this->_ressources.changeRessource(this->getRessourcesRegen());
      this->_mustUpdate = true;
    }
  }
}

void	Server::AItem::updateEffects(const Clock *clock, double factor) {
  Server::Effect *effect;
  for (auto it : this->_effects) {
    for (auto e : it.second) {
      effect = static_cast<Server::Effect *>(e);
      if (!effect->update(clock, factor)) {
	this->createEffectTimeoutEvent(effect);
      }
    }
  }
}

void	Server::AItem::updateCapacities(const Clock *clock, double factor) {
  Protocol::Job *job;
  Server::Capacity *capacity;
  for (auto it : this->_capacities) {
    capacity = static_cast<Server::Capacity *>(it.second);
    capacity->update(clock, factor);
    do {
      job = capacity->popJob();
      if (job) {
	this->addJob(job);
      }
    } while (job);
  }
}

bool	Server::AItem::update(const Clock *clock, double factor) {
  ScopeLock s(this->_dataMutex);
  if (this->getCurrentLife() == 0) {
    if (!this->_destroyed) {
      this->onDestroy();
      this->_destroyed = true;
    }
    return (false);
  }
  this->updateLife(clock, factor);
  this->updatePersonnalRessources(clock, factor);
  this->updateEffects(clock, factor);
  this->updateCapacities(clock, factor);
  if (this->_action) {
    this->updateByAction(clock, factor);
  }
  if (this->_mustUpdate || this->_lastUpdate == NULL ||
      clock->getElapsedTimeSince(this->_lastUpdate) >= Kernel::Config::getInstance()->getDouble(Kernel::Config::UPDATE_ITEM_MAX_ELAPSED_TIME)) {
    if (!this->_lastUpdate) {
      this->_lastUpdate = clock->clone();
    }
    else {
      this->_lastUpdate->clone(clock);
    }
    this->onUpdate();
    this->_mustUpdate = false;
  }
  return (true);
}

void	Server::AItem::updateByAction(const Clock *clock, double factor) {
  for (int j = 0 ; j < UPDATE_ACTION_PTR_FUNC ; ++j) {
    if (this->_updateActionPtrFunc[j].action == this->_action->getType()) {
      (this->*_updateActionPtrFunc[j].func)(clock, factor);
      break;
    }
  }
}

void	Server::AItem::createEffectTimeoutEvent(Server::Effect *effect) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::EFFECT_TIMEOUT, Server::Event::SERVER_GAME, NULL);
  event->effectTimeout.item.id = this->getID();
  event->effectTimeout.item.type = this->getType();
  event->effectTimeout.effect = effect->serial();
  this->addEvent(event);
}

void	Server::AItem::onEffectTimeout(const Kernel::Serial &serial) {
  ScopeLock s(this->_effectMutex);
  this->removeEffect(serial);
}

void	Server::AItem::addLife(unsigned int amount) {
  ScopeLock s(this->_dataMutex);
  ::Game::AElement::addLife(amount);
  this->_mustUpdate = true;
}

void	Server::AItem::removeLife(unsigned int amount) {
  ScopeLock s(this->_dataMutex);
  ::Game::AElement::removeLife(amount);
  this->_mustUpdate = true;
}

void	Server::AItem::onAddEffect(const Kernel::Serial &serial) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->itemEffect(this->getID(), serial, 0.0);
  this->addJob(job);
}

void	Server::AItem::onRemoveEffect(const Kernel::Serial &serial) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->stopItemEffect(this->getID(), serial);
  this->addJob(job);
}

void	Server::AItem::addEffect(const Kernel::Serial &serial) {
  ScopeLock s(this->_effectMutex);
  ScopeLock s1(this->_effectsMutex);
  Server::Effect *effect = Factory::Server::Effect::create(serial, this->getPlayer(), this->getID());
  this->_effects[serial].push_back(effect);
  this->onAddEffect(serial);
  if (this->_grpMove) {
    this->_grpMove->update();
  }
}

void	Server::AItem::removeEffect(const Kernel::Serial &serial) {
  ScopeLock s(this->_effectMutex);
  ScopeLock s1(this->_effectsMutex);
  Server::Effect *effect = static_cast<Server::Effect *>(this->_effects[serial].front());
  this->_effects[serial].pop_front();
  this->onRemoveEffect(serial);
  Factory::Server::Effect::remove(effect);
  if (this->_grpMove) {
    this->_grpMove->update();
  }
}

void	Server::AItem::addEffect(const ::Game::Effect *effect) {
  ScopeLock s(this->_effectMutex);
  ::Game::AElement::addEffect(effect);
  this->onAddTemporaryEffect(effect);
  if (this->_grpMove) {
    this->_grpMove->update();
  }
}

void	Server::AItem::removeEffect(const ::Game::Effect *effect) {
  ScopeLock s(this->_effectMutex);
  ::Game::AElement::removeEffect(effect);
  this->onRemoveTemporaryEffect(effect);
  if (this->_grpMove) {
    this->_grpMove->update();
  }
}

void	Server::AItem::onAddTemporaryEffect(const ::Game::Effect *effect) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->temporaryEffect(this->getID(), effect->serial(), effect->getOwner());
  this->addJob(job);
}

void	Server::AItem::onRemoveTemporaryEffect(const ::Game::Effect *effect) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->removeTemporaryEffect(this->getID(), effect->serial(), effect->getOwner());
  this->addJob(job);
}

void	Server::AItem::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type type) {
  ScopeLock s(this->_dataMutex);
  Server::HasEvent::onItemDestroyed(id, type);
  if (this->_action) {
    Server::Action *action = static_cast<Server::Action *>(this->_action);
    if (action->hasReferenceToOtherObject(id, type)) {
      this->_stopAction();
    }
    else if (action->getNextAction() && action->getNextAction()->hasReferenceToOtherObject(id, type)) {
      Factory::Server::Action::remove(action->getNextAction());
      action->setNextAction(NULL);
    }
  }
}

void	Server::AItem::itemInRange(const Server::AItem *) {}
void	Server::AItem::onAttacked(const Server::AItem *) {}

void	Server::AItem::leaveGroupMove(void) {
  ScopeLock s(this->_dataMutex);
  if (this->_grpMove) {
    if (this->_grpMove->removeItem(this)) {
      Factory::Server::GroupMove::remove(this->_grpMove);
    }
    this->_grpMove = NULL;
  }
}

void	Server::AItem::setGroupMove(Server::GroupMove *grpMove) {
  ScopeLock s(this->_dataMutex);
  if (this->_grpMove) {
    this->leaveGroupMove();
  }
  this->_grpMove = grpMove;
}

unsigned int	Server::AItem::getSpeed(void) const {
  ScopeLock s(this->_dataMutex);
  if (this->_grpMove) {
    return (this->_grpMove->getSpeed());
  }
  return (::Game::ABase::getSpeed());
}

bool	Server::AItem::removeRessources(unsigned int amount, Kernel::ElementRessources::Type type) {
  ScopeLock s(this->_dataMutex);
  bool v = ::Game::AElement::removeRessources(amount, type);
  if (v) {
    this->_mustUpdate = true;
  }
  return (v);
}
