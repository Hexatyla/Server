//
// Spectator.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sat Mar  8 20:39:32 2014 geoffrey bauduin
// Last update Wed Mar 26 16:51:56 2014 geoffrey bauduin
//

#include	<algorithm>
#include	"Server/Spectator.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Protocol/JobResolver.hpp"
#include	"Factory/Protocol.hpp"
#include	"Factory/Server.hpp"
#include	"Kernel/Config.hpp"
#include	"Logger.hpp"

Server::Spectator::Spectator(void):
  Server::HasJobs(),
  _toSend(), _game(), _spec(), _clock(NULL),
  _sendMutex(new Mutex) {

}

Server::Spectator::~Spectator(void) {
  delete this->_sendMutex;
}

void	Server::Spectator::init(const Clock *clock) {
  this->_clock = clock;
  Server::HasJobs::init();
  this->_spec.init();
  this->_game.init();
}

void	Server::Spectator::destroy(void) {
  this->_spec.destroy();
  this->_game.destroy();
  Server::HasJobs::destroy();
}

Server::Spectator::_data::_data(void):
  _itemMutex(new Mutex), _playerMutex(new Mutex), _gameDataMutex(new Mutex),
  _players(), _items(), _gameData(), _dayNight(NULL), _delete() {

}

Server::Spectator::_data::~_data(void) {
  delete this->_itemMutex;
  delete this->_playerMutex;
  delete this->_gameDataMutex;
}

void	Server::Spectator::_data::init(void) {

}

void	Server::Spectator::_data::destroy(void) {
  for (auto it : this->_players) {
    if (it.second) {
      Factory::Server::Spectator::PlayerData::remove(it.second);
    }
  }
  for (auto it : this->_items) {
    if (it.second) {
      Factory::Server::Spectator::ItemData::remove(it.second);
    }
  }
  this->_items.clear();
  this->_players.clear();
  for (auto it : this->_gameData) {
    if (it) {
      Factory::Protocol::Job::remove(it);
    }
  }
  this->_gameData.assign(0, NULL);
  if (this->_dayNight) {
    Factory::Protocol::Job::remove(this->_dayNight);
    this->_dayNight = NULL;
  }
  this->_delete.clear();
}

void	Server::Spectator::_data::addJob(Kernel::ID::Type type, Kernel::ID::id_t id, Protocol::Job *job) {
  if (job->getCommand() == Command::YOUARE) {
    return ;
  }
  if (type == Kernel::ID::ITEM) {
    if (job->getCommand() == Command::PLAYER) {
      ScopeLock s(this->_gameDataMutex);
      this->_gameData.push_back(job->clone());
    }
    else if (job->getCommand() == Command::DAYNIGHT) {
      ScopeLock s(this->_gameDataMutex);
      if (!this->_dayNight) {
	this->_dayNight = Factory::Protocol::Job::create();
      }
      this->_dayNight->clone(job);
    }
    else {
      ScopeLock s(this->_itemMutex);
      if (!this->_items[id]) {
	this->_items[id] = Factory::Server::Spectator::ItemData::create();
      }
      if (job->getCommand() == Command::ODESTROY &&
	  std::find(this->_delete.begin(), this->_delete.end(), id) != this->_delete.end()) {
	this->_delete.remove(id);
	Factory::Server::Spectator::ItemData::remove(this->_items[id]);
	this->_items.erase(id);
      }
      else {
	this->_items[id]->addJob(job);
      }
    }
  }
  else if (type == Kernel::ID::PLAYER) {
    if (job->getCommand() == Command::YOUARE) {
      return ;
    }
    else {
      ScopeLock s(this->_playerMutex);
      if (!this->_players[id]) {
	this->_players[id] = Factory::Server::Spectator::PlayerData::create();
      }
      this->_players[id]->addJob(job);
    }
  }
}

const std::vector<Protocol::Job *>	Server::Spectator::_data::dump(Kernel::ID::id_t id) {
  std::vector<Protocol::Job *> jobs;
  std::vector<Protocol::Job *> f;
  {
    ScopeLock s(this->_gameDataMutex);
    for (auto it : this->_gameData) {
      jobs.push_back(it->clone());
    }
  }
  {
    ScopeLock s(this->_playerMutex);
    for (auto it : this->_players) {
      if (it.second) {
	f = it.second->dump((id == 0 || id == it.first));
	jobs.insert(jobs.end(), f.begin(), f.end());
      }
    }
  }
  {
    ScopeLock s(this->_itemMutex);
    for (auto it : this->_items) {
      if (it.second) {
	f = it.second->dump();
	jobs.insert(jobs.end(), f.begin(), f.end());
      }
    }
  }
  return (jobs);
}

bool	Server::Spectator::update(const Clock *clock, double) {
  double elapsedTime = clock->getElapsedTime();
  double delay = Kernel::Config::getInstance()->getDouble(Kernel::Config::SPEC_DELAY);
  ScopeLock s(this->_sendMutex);
  std::vector<double> toDelete;
  for (auto it : this->_toSend) {
    if (static_cast<double>(it.first) + delay > elapsedTime) {
      break;
    }
    for (auto data : it.second) {
      Server::HasJobs::addJob(data.job);
      this->_spec.addJob(data.type, data.id, data.job);
    }
    toDelete.push_back(it.first);
  }
  for (auto it : toDelete) {
    this->_toSend.erase(it);
  }
  return (this->_toSend.empty() == false);
}

void	Server::Spectator::addJob(Kernel::ID::Type type, Kernel::ID::id_t id, Protocol::Job *job) {
  this->_game.addJob(type, id, job);
  ScopeLock s(this->_sendMutex);
  this->_toSend[this->_clock->getElapsedTime()].push_back({type, id, job->clone()});
}

void	Server::Spectator::_data::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type) {
  ScopeLock s(this->_itemMutex);
  if (this->_items[id]) {
    Factory::Server::Spectator::ItemData::remove(this->_items[id]);
  }
  this->_items.erase(id);
  this->_delete.push_back(id);
}

void	Server::Spectator::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type type) {
  this->_game.onItemDestroyed(id, type);
  this->_spec.onItemDestroyed(id, type);
}

void	Server::Spectator::onPlayerQuit(Kernel::ID::id_t id) {
  this->_game.onPlayerQuit(id);
}

void	Server::Spectator::_data::onPlayerQuit(Kernel::ID::id_t id) {
  ScopeLock s(this->_playerMutex);
  if (this->_players[id]) {
    Factory::Server::Spectator::PlayerData::remove(this->_players[id]);
  }
  this->_players.erase(id);
}

const std::vector<Protocol::Job *>	Server::Spectator::dump(Kernel::ID::id_t id) {
  if (id == 0) {
    return (this->_spec.dump());
  }
  return (this->_game.dump());
}

Server::Spectator::AData::AData(void):
  _mutex(new Mutex) {

}

Server::Spectator::AData::~AData(void) {
  delete this->_mutex;
}

Server::Spectator::ItemData::ItemData(void):
  AData(),
  _create(NULL), _update(NULL), _effects(), _actions() {

}

Server::Spectator::ItemData::~ItemData(void) {

}

void	Server::Spectator::ItemData::init(void) {
  this->_create = NULL;
  this->_update = NULL;
}

void	Server::Spectator::ItemData::destroy(void) {
  for (auto it : this->_effects) {
    Factory::Protocol::Job::remove(it);
  }
  for (auto it : this->_actions) {
    Factory::Protocol::Job::remove(it);
  }
  Factory::Protocol::Job::remove(this->_create);
  Factory::Protocol::Job::remove(this->_update);
  this->_effects.clear();
  this->_actions.clear();
}

void	Server::Spectator::ItemData::addJob(Protocol::Job *job) {
  ScopeLock s(this->_mutex);
  switch (job->getCommand()) {
  case Command::OCREATE:
  case Command::RESSSPOT:
    this->_create = job->clone();
    break;
  case Command::OMAJ:
  case Command::UPDSPOT:
    if (!this->_update) {
      this->_update = job->clone();
    }
    else {
      this->_update->clone(job);
    }
    break;
  case Command::OEFFECT:
  case Command::OSTOPEFFECT:
  case Command::OTMPEFFECT:
  case Command::OSTOPTMPEFFECT:
    this->addEffectJob(job);
    break;
  case Command::OACT:
  case Command::OSTOP:
  // case Command::OMOVE:
  case Command::OWP:
  case Command::HPUOBJ:
  case Command::HROBJ:
  case Command::UBUILD:
  case Command::USTOPBUILD:
  case Command::UPURESS:
  case Command::UHARVEST:
  case Command::USTOPHARVEST:
  case Command::UWAITHARVEST:
  case Command::USTOPWAITHARVEST:
  // case Command::OATTACK:
  // case Command::OZONEATTACK:
  // case Command::OUSEZONECAP:
  // case Command::OUSECAP:
  // case Command::OUSECAPA:
  // case Command::OACTCAPA:
    this->addActionJob(job);
    break;
  default:
    break;
  }
}

void	Server::Spectator::ItemData::addEffectJob(Protocol::Job *job) {
  switch (job->getCommand()) {
  case Command::OEFFECT:
  case Command::OTMPEFFECT:
    this->_effects.push_back(job->clone());
    break;
  case Command::OSTOPTMPEFFECT:
    {
      Kernel::ID::id_t id, idm, usl;
      Kernel::Serial serial, serialm;
      Protocol::JobResolver::getInstance()->removeTemporaryEffect(job, usl, serial, id);
      for (auto it = this->_effects.begin() ; it != this->_effects.end() ; ++it) {
	if ((*it)->getCommand() == Command::OTMPEFFECT) {
	  Protocol::JobResolver::getInstance()->temporaryEffect(*it, usl, serialm, idm);
	  if (idm == id && serialm == serial) {
	    Factory::Protocol::Job::remove(*it);
	    this->_effects.erase(it);
	    break;
	  }
	}
      }
    }
    break;
  case Command::OSTOPEFFECT:
    {
      Kernel::Serial serial, serialm;
      Kernel::ID::id_t usl;
      double usl2;
      Protocol::JobResolver::getInstance()->stopItemEffect(job, usl, serial);
      for (auto it = this->_effects.begin() ; it != this->_effects.end() ; ++it) {
	if ((*it)->getCommand() == Command::OSTOPEFFECT) {
	  Protocol::JobResolver::getInstance()->itemEffect(*it, usl, serialm, usl2);
	  if (serialm == serial) {
	    Factory::Protocol::Job::remove(*it);
	    this->_effects.erase(it);
	    break;
	  }
	}
      }
    }
    break;
  default:
    break;
  }
}

void	Server::Spectator::ItemData::addActionJob(Protocol::Job *job) {
  switch (job->getCommand()) {
  case Command::OSTOP:
    for (auto it = this->_actions.begin() ; it != this->_actions.end() ; ++it) {
      if ((*it)->getCommand() == Command::OACT) {
	Factory::Protocol::Job::remove(*it);
	this->_actions.erase(it);
	break;
      }
    }
    break;
  case Command::USTOPBUILD:
    for (auto it = this->_actions.begin() ; it != this->_actions.end() ; ++it) {
      if ((*it)->getCommand() == Command::UBUILD) {
	Factory::Protocol::Job::remove(*it);
	this->_actions.erase(it);
	break;
      }
    }
    break;
  case Command::USTOPHARVEST:
    for (auto it = this->_actions.begin() ; it != this->_actions.end() ; ++it) {
      if ((*it)->getCommand() == Command::UHARVEST) {
	Factory::Protocol::Job::remove(*it);
	this->_actions.erase(it);
	break;
      }
    }
    break;
  case Command::USTOPWAITHARVEST:
    for (auto it = this->_actions.begin() ; it != this->_actions.end() ; ++it) {
      if ((*it)->getCommand() == Command::UWAITHARVEST) {
	Factory::Protocol::Job::remove(*it);
	this->_actions.erase(it);
	break;
      }
    }
    break;
  case Command::HROBJ:
    {
      Kernel::ID::id_t id, idm, oid, oidm;
      Protocol::JobResolver::getInstance()->heroReleasedObject(job, id, oid);
      for (auto it = this->_actions.begin() ; it != this->_actions.end() ; ++it) {
	if ((*it)->getCommand() == Command::HPUOBJ) {
	  Protocol::JobResolver::getInstance()->heroPickedUpAnObject(*it, idm, oidm);
	  if (id == idm && oidm == oid) {
	    Factory::Protocol::Job::remove(*it);
	    this->_actions.erase(it);
	    break;
	  }
	}
      }
    }
    break;
  case Command::OACTCAPA:
    {
      Kernel::ID::id_t id, idm;
      bool s, sm;
      Kernel::Serial cs, csm;
      Protocol::JobResolver::getInstance()->activateCapacity(job, id, cs, s);
      for (auto it = this->_actions.begin() ; it != this->_actions.end() ; ++it) {
	if ((*it)->getCommand() == Command::OACTCAPA) {
	  Protocol::JobResolver::getInstance()->activateCapacity(*it, idm, csm, sm);
	  if (id == idm && cs == csm) {
	    Factory::Protocol::Job::remove(*it);
	    this->_actions.erase(it);
	    break;
	  }
	}
      }
    }
    break;
  case Command::OWP:
    for (auto it = this->_actions.begin() ; it != this->_actions.end() ; ++it) {
      if ((*it)->getCommand() == Command::OWP) {
	Factory::Protocol::Job::remove(*it);
	this->_actions.erase(it);
	break;
      }	
    }
    this->_actions.push_back(job->clone());
    break;
  case Command::UPURESS:
    for (auto it = this->_actions.begin() ; it != this->_actions.end() ; ++it) {
      if ((*it)->getCommand() == Command::UPURESS) {
	Factory::Protocol::Job::remove(*it);
	this->_actions.erase(it);
	break;
      }
    }
    this->_actions.push_back(job->clone());
    break;
  default:
    this->_actions.push_back(job->clone());
  }
}

const std::vector<Protocol::Job *>	Server::Spectator::ItemData::dump(void) {
  ScopeLock s(this->_mutex);
  std::vector<Protocol::Job *>	jobs;
  if (this->_create) {
    jobs.push_back(this->_create->clone());
  }
  if (this->_update) {
    jobs.push_back(this->_update->clone());
  }
  for (auto it : this->_effects) {
    jobs.push_back(it->clone());
  }
  for (auto it : this->_actions) {
    jobs.push_back(it->clone());
  }
  return (jobs);
}

Server::Spectator::PlayerData::PlayerData(void):
  AData(), _ressources(NULL), _ameliorations() {

}

Server::Spectator::PlayerData::~PlayerData(void) {

}

void	Server::Spectator::PlayerData::init(void) {
  this->_ressources = NULL;
}

void	Server::Spectator::PlayerData::destroy(void) {
  if (this->_ressources) {
    Factory::Protocol::Job::remove(this->_ressources);
  }
  for (auto it : this->_ameliorations) {
    Factory::Protocol::Job::remove(it);
  }
  this->_ameliorations.clear();
}

void	Server::Spectator::PlayerData::addJob(Protocol::Job *job) {
  ScopeLock s(this->_mutex);
  switch (job->getCommand()) {
  case Command::PRESS:
    if (this->_ressources) {
      this->_ressources->clone(job);
    }
    else {
      this->_ressources = job->clone();
    }
    break;
  case Command::PAMELIO:
    this->_ameliorations.push_back(job->clone());
    break;
  case Command::PPRODAMELIO:
    {
      Kernel::ID::id_t usl;
      unsigned int usl2;
      Kernel::Serial itemS, itemSm, aS, aSm;
      Protocol::JobResolver::getInstance()->playerHasJustProducedAmelioration(job, usl, itemS, aS);
      for (auto it = this->_ameliorations.begin() ; it != this->_ameliorations.end() ; ++it) {
	if ((*it)->getCommand() == Command::PRODAMELIO) {
	  Protocol::JobResolver::getInstance()->ameliorationProduction(*it, usl, itemSm, aSm, usl2);
	  if (itemSm == itemS && aSm == aS) {
	    Factory::Protocol::Job::remove(*it);
	    this->_ameliorations.erase(it);
	    break;
	  }
	}
      }
      this->_ameliorations.push_back(job->clone());
    }
    break;
  case Command::PRODAMELIO:
    this->_ameliorations.push_back(job->clone());
    break;
  case Command::DAMELIO:
    {
      Kernel::ID::id_t usl;
      unsigned int usl2;
      Kernel::Serial itemS, itemSm, aS, aSm;
      Protocol::JobResolver::getInstance()->removeAmeliorationFromProductionQueue(job, usl, itemS, aS);
      for (auto it = this->_ameliorations.begin() ; it != this->_ameliorations.end() ; ++it) {
	if ((*it)->getCommand() == Command::PRODAMELIO) {
	  Protocol::JobResolver::getInstance()->ameliorationProduction(*it, usl, itemSm, aSm, usl2);
	  if (itemSm == itemS && aSm == aS) {
	    Factory::Protocol::Job::remove(*it);
	    this->_ameliorations.erase(it);
	    break;
	  }
	}
      }
    }
    break;
  default:
    break;
  }
}

const std::vector<Protocol::Job *>	Server::Spectator::PlayerData::dump(bool ress) {
  ScopeLock s(this->_mutex);
  std::vector<Protocol::Job *>	jobs;
  if (ress && this->_ressources) {
    jobs.push_back(this->_ressources->clone());
  }
  for (auto it : this->_ameliorations) {
    jobs.push_back(it->clone());
  }
  return (jobs);
}
