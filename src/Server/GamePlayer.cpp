//
// GamePlayer.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sat Feb  1 20:07:04 2014 geoffrey bauduin
// Last update Tue Mar 25 17:38:44 2014 geoffrey bauduin
//

#include	<algorithm>
#include	"Server/GamePlayer.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Protocol.hpp"
#include	"Factory/Server.hpp"
#include	"Kernel/Manager.hpp"
#include	"Logger.hpp"

Server::GamePlayer::GamePlayer(void):
  ::Game::Player(), Server::HasJobs(), Server::HasProductionQueue(), Server::HasEvent(), Server::HasStatistics(),
  NoDeleteWhileUsedByAThread(),
  _mutex(new Mutex), _ressourcesMutex(new Mutex), _user(NULL),
  _lockedRessources(), _realRessources(),
  _reconnect(), _reconnectMutex(new Mutex) {
}

Server::GamePlayer::~GamePlayer(void) {
  delete this->_mutex;
  delete this->_ressourcesMutex;
  delete this->_reconnectMutex;
}

void	Server::GamePlayer::init(Server::User *user, Kernel::ID::id_t id, const ::Game::Race *race, const ::Game::Team *team) {
  NoDeleteWhileUsedByAThread::init();
  ::Game::Player::init(id, race, team);
  Server::HasJobs::init();
  Server::HasProductionQueue::init();
  Server::HasEvent::init();
  Server::HasStatistics::init(race);
  this->_user = user;
  this->_lockedRessources.init(0, 0, 0);
  this->_realRessources.init(0, 0, 0);
}

void	Server::GamePlayer::destroy(void) {
  ::Game::Player::destroy();
  Server::HasJobs::destroy();
  Server::HasProductionQueue::destroy();
  Server::HasEvent::destroy();
  Server::HasStatistics::destroy();
  this->_lockedRessources.destroy();
  this->_realRessources.destroy();
  this->_reconnect.clear();
}

void	Server::GamePlayer::addRessources(const ::Game::Ressources &rhs) {
  ScopeLock s(this->_ressourcesMutex);
  ::Game::Player::addRessources(rhs);
  this->_realRessources += rhs;
  this->onRessourcesChange();
}

void	Server::GamePlayer::addRessources(::Game::Ressources::Type type, unsigned int amount) {
  ScopeLock s(this->_ressourcesMutex);
  ::Game::Player::addRessources(type, amount);
  this->_realRessources.add(type, static_cast<int>(amount));
  this->onRessourcesChange();
}

void	Server::GamePlayer::removeRessources(const ::Game::Ressources &rhs) {
  ScopeLock s(this->_ressourcesMutex);
  ::Game::Player::removeRessources(rhs);
  this->_realRessources -= rhs;
  this->onRessourcesChange();
}

void	Server::GamePlayer::removeRessources(::Game::Ressources::Type type, unsigned int amount) {
  ScopeLock s(this->_ressourcesMutex);
  ::Game::Player::removeRessources(type, amount);
  this->_realRessources.add(type, -1 * static_cast<int>(amount));
  this->onRessourcesChange();
}

void	Server::GamePlayer::spendRessources(const ::Game::Ressources &rhs) {
  ScopeLock s(this->_ressourcesMutex);
  this->_ressources -= rhs;
  this->_lockedRessources -= rhs;
  this->onRessourcesChange();
}

Protocol::Job	*Server::GamePlayer::onAskRessources(void) const {
  return (this->createRessourcesJob());
}

void	Server::GamePlayer::onRessourcesChange(void) {
  Protocol::Job *job = this->createRessourcesJob();
  this->addJob(job);
}

Protocol::Job	*Server::GamePlayer::createRessourcesJob(void) const {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->playerRessources(this->getID(),
			this->_ressources.get(::Game::Ressources::GOLD),
			this->_ressources.get(::Game::Ressources::WOOD),
			this->_ressources.get(::Game::Ressources::FOOD));
  return (job);
}

Server::User	*Server::GamePlayer::getUser(void) {
  return (this->_user);
}

void	Server::GamePlayer::onAmeliorationProduced(const Kernel::Serial &type, const Kernel::Serial &serial) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::FINISHED_AMELIORATION_PRODUCTION, Server::Event::SERVER_GAME, NULL);
  event->ameliorationFinished.playerID = this->getID();
  Kernel::assign_serial(event->ameliorationFinished.type, type);
  Kernel::assign_serial(event->ameliorationFinished.serial, serial);
  this->addEvent(event);
}

void	Server::GamePlayer::update(const Clock *clock, double) {
  Kernel::Serial produced;
  for (auto it : this->_productionQueues) {
    Server::HasProductionQueue::Status status = Server::HasProductionQueue::update(clock, it.second, produced);
    if (status == Server::HasProductionQueue::END || status == Server::HasProductionQueue::END_RESTART) {
      this->onAmeliorationProduced(it.first, produced);
    }
  }
}

void	Server::GamePlayer::onPing(Kernel::ID::id_t playerID, double x, double y) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->ping(playerID, x, y);
  this->addJob(job);
}

void	Server::GamePlayer::addToQueue(const Kernel::Serial &type, const Kernel::Serial &serial) {
  ::Game::Player::addToQueue(type, serial);
  if (this->_productionQueues[type]) {
    this->onAddToProductionList(type, serial, this->_productionQueues[type]->size());
  }
}

void	Server::GamePlayer::insertInQueue(const Kernel::Serial &type, const Kernel::Serial &serial, unsigned int pos) {
  ::Game::Player::insertInQueue(type, serial, pos);
  this->onAddToProductionList(type, serial, pos);
}

void	Server::GamePlayer::onAddToProductionList(const Kernel::Serial &type, const Kernel::Serial &serial, unsigned int pos) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->ameliorationProduction(this->getID(), type, serial, pos);
}

bool	Server::GamePlayer::removeFromQueue(const Kernel::Serial &type, const Kernel::Serial &serial) {
  bool v = ::Game::Player::removeFromQueue(type, serial);
  if (v) {
    this->onRemoveFromProductionList(type, serial);
  }
  return (v);
}

void	Server::GamePlayer::onRemoveFromProductionList(const Kernel::Serial &type, const Kernel::Serial &serial) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->removeAmeliorationFromProductionQueue(this->getID(), type, serial);
  this->addJob(job);
}

void	Server::GamePlayer::removeItemFromProductionList(const Kernel::Serial &type, const Kernel::Serial &serial) {
  if (this->removeFromQueue(type, serial)) {
    this->addRessources(Kernel::Manager::getInstance()->getCost(serial)->ressources);
  }
}

void	Server::GamePlayer::produceAmelioration(const Kernel::Serial &type, const Kernel::Serial &serial) {
  ScopeLock s(this->_mutex);
  if (!this->isInQueue(type, serial)) {
    this->spendRessources(Kernel::Manager::getInstance()->getCost(serial)->ressources);
    this->addToQueue(type, serial);
  }
}

void	Server::GamePlayer::removeUnit(unsigned int amount) {
  ::Game::Player::removeUnit(amount);
  if (this->hasLost()) {
    this->onLost();
  }
}

void	Server::GamePlayer::removeHero(unsigned int amount) {
  ::Game::Player::removeHero(amount);
  if (this->hasLost()) {
    this->onLost();
  }
}

void	Server::GamePlayer::removeBuilding(unsigned int amount) {
  ::Game::Player::removeBuilding(amount);
  if (this->hasLost()) {
    this->onLost();
  }
}

void	Server::GamePlayer::onLost(void) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::PLAYER_LOST,
							Server::Event::SERVER_GAMEPLAYER, this);
  this->addEvent(event);
}

void	Server::GamePlayer::inGame(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->youAre(this->getID());
  this->addJob(job);
}

const ::Game::Ressources	&Server::GamePlayer::getRealRessources(void) const {
  ScopeLock s(this->_ressourcesMutex);
  return (this->_realRessources);
}

unsigned int	Server::GamePlayer::getRealRessources(::Game::Ressources::Type type) const {
  ScopeLock s(this->_ressourcesMutex);
  return (this->_realRessources.get(type));
}


bool	Server::GamePlayer::lockRessources(const ::Game::Ressources &ressources) {
  ScopeLock s(this->_ressourcesMutex);
  if (this->_realRessources.hasEnough(ressources)) {
    this->_realRessources -= ressources;
    this->_lockedRessources += ressources;
    return (true);
  }
  return (false);
}

bool	Server::GamePlayer::lockRessources(::Game::Ressources::Type type, unsigned int amount) {
  ScopeLock s(this->_ressourcesMutex);
  if (static_cast<unsigned int>(this->_realRessources.get(type)) >= amount) {
    this->_realRessources.add(type, static_cast<int>(amount) * -1);
    this->_lockedRessources.add(type, amount);
    return (true);
  }
  return (false);
}

void	Server::GamePlayer::unlockRessources(const ::Game::Ressources &ressources) {
  ScopeLock s(this->_ressourcesMutex);
  this->_lockedRessources -= ressources;
  this->_realRessources += ressources;
}

void	Server::GamePlayer::unlockRessources(::Game::Ressources::Type type, unsigned int amount) {
  ScopeLock s(this->_ressourcesMutex);
  this->_lockedRessources.add(type, static_cast<int>(amount) * -1);
  this->_realRessources.add(type, amount);
}

bool	Server::GamePlayer::hasEnoughRessources(const ::Game::Ressources &ressources) const {
  ScopeLock s(this->_ressourcesMutex);
  return (this->_realRessources.hasEnough(ressources));
}

void	Server::GamePlayer::addReconnectJobs(const std::vector<Protocol::Job *> &jobs) {
  ScopeLock s(this->_reconnectMutex);
  this->_reconnect.clear();
  for (auto it : jobs) {
    this->_reconnect.push_back(it);
  }
}

bool	Server::GamePlayer::isReconnectJob(Protocol::Job *j) {
  ScopeLock s(this->_reconnectMutex);
  std::list<Protocol::Job *>::iterator it = std::find(this->_reconnect.begin(), this->_reconnect.end(), j);
  if (it != this->_reconnect.end()) {
    this->_reconnect.erase(it);
    return (true);
  }
  return (false);
}
