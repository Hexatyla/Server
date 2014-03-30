//
// Game.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 14:20:07 2014 geoffrey bauduin
// Last update Wed Mar 26 16:35:16 2014 geoffrey bauduin
//

#include	<math.h>
#include	<limits>
#include	<iostream>
#include	<algorithm>
#include	"Server/Game.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Server.hpp"
#include	"Factory/Factory.hpp"
#include	"Factory/Protocol.hpp"
#include	"Kernel/Config.hpp"
#include	"Logger.hpp"
#include	"Kernel/Manager.hpp"
#include	"Game/Controller.hpp"
#include	"Protocol/Job.hpp"
#include	"Factory/Game.hpp"
#include	"Parser/Map.hpp"
#include	"Network/Manager/Server.hpp"

#define	LOCK_UNIT	ScopeLock s_unit(this->_mutexes[::Game::UNIT])
#define	LOCK_BUILDING	ScopeLock s_building(this->_mutexes[::Game::BUILDING])
#define	LOCK_OBJECT	ScopeLock s_object(this->_mutexes[::Game::OBJECT])
#define	LOCK_RESSSPOT	ScopeLock s_ressspot(this->_mutexes[::Game::RESSSPOT])
#define	LOCK_HERO	ScopeLock s_hero(this->_mutexes[::Game::HERO])
#define	LOCK_PROJECTILE	ScopeLock s_projectile(this->_mutexes[::Game::PROJECTILE])
#define	LOCK_PLAYER	ScopeLock s_player(this->_playerMutex)
#define ABS(x)	((x) >= 0 ? (x) : -(x))
#define	IS_BETWEEN(a, b, c)	((a) <= (b) && (b) <= (c))
#define	IS_BETWEEN_B(a, b, c)	((IS_BETWEEN(a, b, c)) || (IS_BETWEEN(c, b, a)))


#define	DEBUG_LOL
#define DEBUG_LOLZ	{ std::string ffff = __PRETTY_FUNCTION__; int llllll = __LINE__; Logger::getInstance()->logf("LOCK MUTEX %s %d", Logger::DEBUG, &ffff, llllll); }

Server::Game::Game(void):
  HasID(), HasEvent(), HasJobs(), HasUsers(),
  HasHero(), HasProjectile(), HasBuilding(), HasUnit(), HasObject(), HasRessourcesSpot(), HasPlayers(),
  _mutex(new Mutex), _clock(NULL), _map(NULL), _pathfinding(), _lastDayNightSwitch(NULL), _day(true),
  _removeMutex(new Mutex), _loader(NULL), _mutexes(),
  _teamMutex(new Mutex), _teams(), _spectator(NULL), _end(false),
  _specJobsMutex(new Mutex), _firstSpecJob(false), _specJobs() {
  this->_mutexes[::Game::OBJECT] = this->_objectMutex;
  this->_mutexes[::Game::UNIT] = this->_unitMutex;
  this->_mutexes[::Game::HERO] = this->_heroMutex;
  this->_mutexes[::Game::PROJECTILE] = this->_projectileMutex;
  this->_mutexes[::Game::RESSSPOT] = this->_ressourcesSpotMutex;
  this->_mutexes[::Game::BUILDING] = this->_buildingMutex;
}

Server::Game::~Game(void) {
  delete this->_mutex;
  delete this->_removeMutex;
  delete this->_teamMutex;
}

void	Server::Game::destroy(void) {
  Logger::getInstance()->logf("Game #%d shutting down ...", Logger::INFORMATION, this->getID());
  HasEvent::destroy();
  HasID::destroy();
  HasProjectile::destroy();
  HasBuilding::destroy();
  HasUnit::destroy();
  HasHero::destroy();
  HasJobs::destroy();
  HasUsers::destroy();
  HasPlayers::destroy();
  if (this->_loader) {
    Factory::Server::GameLoader::remove(this->_loader);
    this->_loader = NULL;
  }
  if (this->_clock) {
    Factory::Clock::remove(this->_clock);
    this->_clock = NULL;
  }
  if (this->_lastDayNightSwitch) {
    Factory::Clock::remove(this->_lastDayNightSwitch);
    this->_lastDayNightSwitch = NULL;
  }
  if (this->_map) {
    Factory::Server::Map::remove(this->_map);
    this->_map = NULL;
  }
  for (auto it : this->_teams) {
    Factory::Game::Team::remove(it.first);
  }
  this->_teams.clear();
  Factory::Server::Spectator::remove(this->_spectator);
  this->_spectator = NULL;
  while (this->_specJobs.empty() == false) {
    Factory::Protocol::Job::remove(this->_specJobs.front().second);
    this->_specJobs.pop();
  }
  Logger::getInstance()->logf("Game has been shutted down.", Logger::INFORMATION);
}

void	Server::Game::init(Kernel::ID::id_t id, const std::vector<Server::GamePlayer *> &players,
			   const Server::HasUsers::Container &users, const std::vector< ::Game::Team *> &teams,
			   const std::string &map, Server::GameLoader *gl) {
  Logger::getInstance()->logf("Initializing Game #%d", Logger::INFORMATION, id);
  Logger::getInstance()->addDecalage();
  HasEvent::init();
  HasID::init(id);
  HasProjectile::init();
  HasBuilding::init();
  HasUnit::init();
  HasHero::init();
  HasJobs::init();
  HasUsers::init(users);
  HasPlayers::init(players);
  this->_loader = gl;
  this->_clock = Factory::Clock::create();
  this->_lastDayNightSwitch = Factory::Clock::create();
  this->_clock->init();
  this->_day = true;
  this->_end = false;

  if (!this->initMap(map)) {
    this->_end = true;
  }
  this->_pathfinding.setMap(this->_map);
  this->initTeams(teams, players);

  this->_spectator = Factory::Server::Spectator::create(this->_clock);

  this->_firstSpecJob = true;
  this->_clock->update();
  this->_lastDayNightSwitch->clone(this->_clock);

  ::Game::Ressources rs;
  Kernel::Config *config = Kernel::Config::getInstance();
  rs.init(config->getUInt(Kernel::Config::START_GOLD),
	  config->getUInt(Kernel::Config::START_WOOD),
	  config->getUInt(Kernel::Config::START_FOOD));
  for (auto it : this->_players) {
    this->initPlayer(it.second);
    Protocol::Job *job;
    job = Factory::Protocol::Job::create();
    job->playerInformation(it.second->getID(),
			   std::find(teams.begin(), teams.end(), it.second->getTeam()) - teams.begin(),
			   it.second->getRace()->serial());
    this->addJob(job);
    this->addSpectatorJob(Kernel::ID::ITEM, 0, job);
    it.second->inGame();
    it.second->addRessources(rs);
  }
  Logger::getInstance()->removeDecalage();

  this->onDayNightSwitch();

  Protocol::Job *job = Factory::Protocol::Job::create();
  job->gameLaunch();
  this->addJob(job);

  if (this->_end == false) {
    Logger::getInstance()->logf("Game #%d has been initialized.", Logger::INFORMATION, id);
  }
  else {
    Logger::getInstance()->logf("Game #%d failed to initialize.", Logger::FATAL, id);
  }
}

bool	Server::Game::initMap(const std::string &filename) {
  Logger::getInstance()->logf("Initializing map from >%s<", Logger::INFORMATION, &filename);
  Logger::getInstance()->addDecalage();
  this->_map = static_cast<Server::Map *>(Parser::Map::getInstance()->parse(filename));
  Logger::getInstance()->removeDecalage();
  if (!this->_map) {
    Logger::getInstance()->log("Cannot load map", Logger::FATAL);
    return (false);
  }
  Logger::getInstance()->log("Initializing Ressources Spots", Logger::INFORMATION);
  Logger::getInstance()->addDecalage();
  const std::vector<Server::Map::Spot> &spots = this->_map->getSpots();
  const std::map<unsigned int, std::map<unsigned int, Vector3d> > &hexas = this->_map->getHexas();
  for (auto spot : spots) {
    Kernel::ID::id_t id = Kernel::ID::getInstance()->get(Kernel::ID::ITEM);
    try {
      double x = hexas.at(spot.y).at(spot.x).x + this->_map->_hexaX;
      double y = hexas.at(spot.y).at(spot.x).y + this->_map->_hexaY;
      Logger::getInstance()->logf("Spot located #%d located in %f %f containing %d units of %s", Logger::INFORMATION,
				  id, x, y, spot.amount, &(::Game::Ressources::translate(spot.type)));
      Server::RessourcesSpot *item = Factory::Server::RessourcesSpot::create(id, x, y, this->_map->getHeight(spot.x, spot.y),
									     spot.type, spot.amount);
      this->addItem(item);
    }
    catch (std::out_of_range &) {
      Logger::getInstance()->log("Bad spot position", Logger::FATAL);
    }
  }
  Logger::getInstance()->removeDecalage();
  Logger::getInstance()->log("Map initialized.", Logger::INFORMATION);
  return (true);
}

void	Server::Game::initTeams(const std::vector< ::Game::Team *> &teams,
				const std::vector<Server::GamePlayer *> &players) {
  for (auto it : teams) {
    this->_teams[it] = 0;
  }
  for (auto it : players) {
    this->_teams[const_cast< ::Game::Team *>(it->getTeam())]++;
  }
}

void	Server::Game::initPlayer(Server::GamePlayer *player) {
  const ::Game::Race *race = player->getRace();
  const std::map<Kernel::Serial, unsigned int> &start = static_cast<const Server::Race *>(race)->getStartInfos();
  double x, y, z;
  if (!this->_map->getStartingPosition(x, y, z)) {
    Logger::getInstance()->logf("Cannot find a start location for player #%d", Logger::FATAL, player->getID());
    this->_end = true;
    return ;
  }
  unsigned int food = 0;
  for (auto it : start) {
    for (unsigned int j = 0 ; j < it.second ; ++j) {
      Server::Event *event = Factory::Server::Event::create(Server::Event::CREATE_ITEM,
							    Server::Event::SERVER_GAME, this);
      Kernel::assign_serial(event->create.serial, it.first);
      event->create.player = player;
      event->create.x = x;
      event->create.y = y;
      event->create.z = z;
      event->create.o = 0;
      this->addEvent(event);
      const Kernel::Cost *cost = Kernel::Manager::getInstance()->getCost(it.first);
      if (cost) {
	food += cost->ressources.get(::Game::Ressources::FOOD);
      }
      Logger::getInstance()->logf("Creating unit %s in %f ; %f ; %f", Logger::INFORMATION, &it.first, x, y, z);
    }
  }
  player->removeRessources(::Game::Ressources::FOOD, food);
}

Server::AItem	*Server::Game::_getItem(Kernel::ID::id_t id, int objType) const {
  if (objType & ::Game::UNIT && this->_units.find(id) != this->_units.end() && this->_units.at(id)) {
    return (this->_units.at(id));
  }
  if (objType & ::Game::BUILDING && this->_buildings.find(id) != this->_buildings.end() && this->_buildings.at(id)) {
    return (this->_buildings.at(id));
  }
  if (objType & ::Game::HERO && this->_heroes.find(id) != this->_heroes.end() && this->_heroes.at(id)) {
    return (this->_heroes.at(id));
  }
  if (objType & ::Game::PROJECTILE && this->_projectiles.find(id) != this->_projectiles.end() && this->_projectiles.at(id)) {
    return (this->_projectiles.at(id));
  }
  if (objType & ::Game::OBJECT && this->_objects.find(id) != this->_objects.end() && this->_objects.at(id)) {
    return (this->_objects.at(id));
  }
  if (objType & ::Game::RESSSPOT && this->_ressourcesSpots.find(id) != this->_ressourcesSpots.end() && this->_ressourcesSpots.at(id)) {
    return (this->_ressourcesSpots.at(id));
  }
  return (NULL);
}

const Server::AItem	*Server::Game::getItem(Kernel::ID::id_t id, int objType) const {
  return (this->_getItem(id, objType));
}

const Server::GameLoader *Server::Game::getLoader() const { return (this->_loader); }
Server::GameLoader	*Server::Game::getLoader() { return (this->_loader); }

void	Server::Game::createUpdateEvent(Kernel::ID::id_t id, ::Game::Type type, double factor) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::UPDATE, Server::Event::SERVER_ITEM, this);
  event->update.factor = factor;
  event->update.id = id;
  event->update.type = type;
  this->addEvent(event);
}

bool	Server::Game::updatePlayer(Kernel::ID::id_t playerID, double factor) {
  if (this->_players.find(playerID) != this->_players.end()) {
    this->_players[playerID]->update(this->_clock, factor);
    this->handleItemEvents(this->_players[playerID]);
  }
  return (true);
}

void	Server::Game::createUpdatePlayerEvent(Kernel::ID::id_t playerID, double factor) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::UPDATE, Server::Event::SERVER_GAMEPLAYER, this);
  event->update.factor = factor;
  event->update.id = playerID;
  this->addEvent(event);
}

void	Server::Game::checkEndGame(void) {
  ScopeLock s(this->_teamMutex);
  unsigned int amount = 0;
  for (auto it : this->_teams) {
    if (it.second != 0) {
      amount++;
    }
  }
  if (amount <= 1) {
    this->_end = true;
  }
}

bool	Server::Game::_update(const Clock *, double factor) {
  if (this->_clock->getElapsedTimeSince(this->_lastDayNightSwitch) >= Kernel::Config::getInstance()->getDouble(Kernel::Config::DAY_NIGHT_SWITCH_TIMER)) {
    this->_lastDayNightSwitch->clone(this->_clock);
    this->_day = !this->_day;
    this->onDayNightSwitch();
  }
  {
    LOCK_UNIT;
    for (auto it : this->_units) {
      if (it.second) {
	this->createUpdateEvent(it.first, ::Game::UNIT, factor);
      }
    }
  }
  {
    LOCK_BUILDING;
    for (auto it : this->_buildings) {
      if (it.second) {
	this->createUpdateEvent(it.first, ::Game::BUILDING, factor);
      }
    }
  }
  {
    LOCK_PROJECTILE;
    for (auto it : this->_projectiles) {
      if (it.second) {
	this->createUpdateEvent(it.first, ::Game::PROJECTILE, factor);
      }
    }
  }
  {
    LOCK_HERO;
    for (auto it : this->_heroes) {
      if (it.second) {
	this->createUpdateEvent(it.first, ::Game::HERO, factor);
      }
    }
  }
  {
    LOCK_PLAYER;
    for (auto it : this->_players) {
      if (it.second) {
	this->createUpdatePlayerEvent(it.first, factor);
      }
    }
  }
  {
    LOCK_RESSSPOT;
    for (auto it : this->_ressourcesSpots) {
      if (it.second) {
	this->createUpdateEvent(it.first, ::Game::RESSSPOT, factor);
      }
    }
  }
  this->handleItemEvents(this->_map);
  return (true);
}

bool	Server::Game::update(const Clock *clock, double factor) {
  ScopeLock s(this->_mutex);
  this->_clock->clone(clock);
  this->checkEndGame();
  if (this->_end == false) {
    this->_update(clock, factor);
  }
  else {
    Logger::getInstance()->logf("Game #%d has been set to end. Waiting ...", Logger::INFORMATION, this->getID());
  }
  bool v = this->_spectator->update(clock, factor);
  this->handleSpecJobs();
  return (!(!v && this->_end));
}

void	Server::Game::handleSpecJobs(void) {
  Protocol::Job *job;
  do {
    job = this->_spectator->popJob();
    if (job) {
      if (this->_firstSpecJob) {
	ScopeLock s(this->_uMutex);
	Protocol::Job *j;
	for (auto it : this->_users) {
	  j = Factory::Protocol::Job::create();
	  j->youAre(0);
	  this->addSpecJob(it->getSocket(), j);
	}
	this->_firstSpecJob = false;
      }
      ScopeLock s(this->_uMutex);
      for (auto it : this->_users) {
	this->addSpecJob(it->getSocket(), job->clone());
      }
      Factory::Protocol::Job::remove(job);
    }
  } while (job);
}

void	Server::Game::addSpecJob(Network::SSocket *socket, Protocol::Job *job) {
  ScopeLock s(this->_specJobsMutex);
  this->_specJobs.push(std::make_pair(socket, job));
}

bool	Server::Game::popSpecJob(Network::SSocket **socket, Protocol::Job **job) {
  ScopeLock s(this->_specJobsMutex);
  *socket = NULL;
  *job = NULL;
  if (this->_specJobs.empty() == false) {
    *socket = this->_specJobs.front().first;
    *job = this->_specJobs.front().second;
    this->_specJobs.pop();
    return (true);
  }
  return (false);
}

void	Server::Game::handleItemJobs(Server::HasJobs *item, bool spec, Kernel::ID::id_t id) {
  Protocol::Job *job;
  do {
    job = item->popJob();
    if (job) {
      if (spec) {
	this->addSpectatorJob(Kernel::ID::ITEM, id, job);
      }
      this->addJob(job);
    }
  } while (job);
}

void	Server::Game::handleItemEvents(Server::HasEvent *item) {
  Server::Event *event;
  do {
    event = item->popEvent();
    if (event) {
      if (event->objType == Server::Event::SERVER_GAME && event->ptr == NULL) {
	event->ptr = this;
      }
      this->addEvent(event);
    }
  } while (event);
}

void	Server::Game::createDeleteEvent(Server::AItem *item) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::REMOVE, Server::Event::SERVER_ITEM, this);
  event->remove.id = item->getID();
  event->remove.type = item->getType();
  this->addEvent(event);
}

void	Server::Game::onDayNightSwitch(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->dayNightSwitch(this->_day);
  this->addJob(job);
  this->addSpectatorJob(Kernel::ID::ITEM, 0, job);
  this->_map->setDayNight(this->_day);
}

bool	Server::Game::update(Kernel::ID::id_t id, ::Game::Type type, double factor) {
  Server::AItem *item;
  {
    ScopeLock sl(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (!item || item->lock() == false) {
      return (true);
    }
  }
  bool v = item->update(this->_clock, factor);
  this->handleItemEvents(item);
  this->handleItemJobs(item, true, item->getID());
  if (!v) {
    this->createDeleteEvent(item);
  }
  return (true);
}

bool	Server::Game::addItem(Server::AItem *item) {
  {
    ScopeLock s(this->_mutex);
    if (!this->_map->tryPosition(item, item->getX(), item->getY())) {
      return (false);
    }
    this->_map->push(item);
  }
  switch (item->getType()) {
  case ::Game::UNIT:
    this->addUnit(static_cast<Server::Unit *>(item));
    this->onAddItem(item, true);
    break;
  case ::Game::PROJECTILE:
    this->addProjectile(static_cast<Server::Projectile *>(item));
    this->onAddItem(item, true);
    break;
  case ::Game::HERO:
    this->addHero(static_cast<Server::Hero *>(item));
    this->onAddItem(item, true);
    break;
  case ::Game::BUILDING:
    this->addBuilding(static_cast<Server::Building *>(item));
    this->onAddItem(item, true);
    this->changePlayerFood(item, false);
    this->changePlayerFood(item, false);
    break;
  case ::Game::RESSSPOT:
    this->addRessourcesSpot(static_cast<Server::RessourcesSpot *>(item));
    this->onAddItem(item, false);
    break;
  default:
    this->_map->remove(item);
    return (false);
  }
  return (true);
}

void	Server::Game::changePlayerFood(const Server::AItem *item, bool minus) {
  Server::GamePlayer *player;
  LOCK_PLAYER;
  player = this->_players[item->getPlayer()->getID()];
  if (!player) {
    return ;
  }
  unsigned int food = item->getInfos().cost->ressources.get(::Game::Ressources::FOOD);
  if (minus) {
    player->removeRessources(::Game::Ressources::FOOD, food);
  }
  else {
    player->addRessources(::Game::Ressources::FOOD, food);
  }
}

void	Server::Game::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type type) {
  {
    LOCK_UNIT;
    for (auto it : this->_units) {
      it.second->onItemDestroyed(id, type);
    }
  }
  {
    LOCK_BUILDING;
    for (auto it : this->_buildings) {
      it.second->onItemDestroyed(id, type);
    }
  }
  {
    LOCK_PROJECTILE;
    for (auto it : this->_projectiles) {
      it.second->onItemDestroyed(id, type);
    }
  }
  {
    LOCK_OBJECT;
    for (auto it : this->_objects) {
      it.second->onItemDestroyed(id, type);
    }
  }
  {
    LOCK_HERO;
    for (auto it : this->_heroes) {
      it.second->onItemDestroyed(id, type);
    }
  }
  {
    LOCK_RESSSPOT;
    for (auto it : this->_ressourcesSpots) {
      it.second->onItemDestroyed(id, type);
    }
  }
  Server::HasEvent::onItemDestroyed(id, type);
}

void	Server::Game::removeItem(::Game::Type type, Kernel::ID::id_t id) {
  ScopeLock s1(this->_removeMutex);
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (!item) {
      return ;
    }
    item->lockDestroy();
  }
  if (dynamic_cast<Server::CanBeDamaged *>(item)) {
    Server::CanBeDamaged *base = dynamic_cast<Server::CanBeDamaged *>(item);
    const std::map<Kernel::ID::id_t, unsigned int> &damagedBy = base->getAttackers();
    unsigned int xp = Kernel::Config::getInstance()->getUInt(item->getType() == ::Game::BUILDING ?
							     Kernel::Config::XP_BUILDING : Kernel::Config::XP_UNIT);
    for (auto it : damagedBy) {
      Server::Event *event = Factory::Server::Event::create(Server::Event::ADD_XP, Server::Event::SERVER_GAME, this);
      event->xp.id = it.first;
      event->xp.amount = xp / damagedBy.size();
      this->addEvent(event);
    }
  }
  switch (type) {
  case ::Game::UNIT:
    this->removeUnit(static_cast<Server::Unit *>(item));
    this->onRemoveItem(item, true);
    this->changePlayerFood(item, false);
    Factory::Server::Unit::remove(static_cast<Server::Unit *>(item));
    break;
  case ::Game::PROJECTILE:
    this->removeProjectile(static_cast<Server::Projectile *>(item));
    this->onRemoveItem(item, true);
    this->_map->remove(item);
    Factory::Server::Projectile::remove(static_cast<Server::Projectile *>(item));
    break;
  case ::Game::HERO:
    this->removeHero(static_cast<Server::Hero *>(item));
    this->onRemoveItem(item, true);
    this->changePlayerFood(item, false);
    Factory::Server::Hero::remove(static_cast<Server::Hero *>(item));
    break;
  case ::Game::BUILDING:
    this->removeBuilding(static_cast<Server::Building *>(item));
    this->onRemoveItem(item, true);
    this->changePlayerFood(item, true);
    Factory::Server::Building::remove(static_cast<Server::Building *>(item));
    break;
  default:
    item->unlock();
    return ;
  }
  this->onItemDestroyed(id, type);
  Kernel::ID::getInstance()->push(id, Kernel::ID::ITEM);
  item->unlock();
}

void	Server::Game::onAddItem(Server::AItem *item, bool p) {
  ScopeLock s(this->_mutex);
  if (p) {
    LOCK_PLAYER;
    Server::GamePlayer *player = this->getPlayerOwningItem(item);
    if (player) {
      player->hasBuiltItem(item->serial());
      switch (item->getType()) {
      case ::Game::UNIT:
	player->addUnit(1);
	break;
      case ::Game::HERO:
	player->addHero(1);
	break;
      case ::Game::BUILDING:
	player->addBuilding(1);
	break;
      default:
	break;
      }
    }
  }
}

void	Server::Game::onRemoveItem(Server::AItem *item, bool p) {
  ScopeLock s(this->_mutex);
  if (p) {
    LOCK_PLAYER;
    Server::GamePlayer *player = this->getPlayerOwningItem(item);
    if (player) {
      player->hasLostItem(item->serial());
      switch (item->getType()) {
      case ::Game::UNIT:
	player->addUnit(1);
	break;
      case ::Game::HERO:
	player->addHero(1);
	break;
      case ::Game::BUILDING:
	player->addBuilding(1);
	break;
      default:
	break;
      }
    }
  }
  this->_map->remove(item);
}

bool	Server::Game::isDay(void) const {
  return (this->_day);
}

void	Server::Game::onPing(Kernel::ID::id_t playerID, double x, double y) {
  LOCK_PLAYER;
  for (auto it : this->_players) {
    if (it.second->isAlliedWith(this->_players[playerID])) {
      it.second->onPing(playerID, x, y);
    }
  }
}

void	Server::Game::changeUnitStance(Kernel::ID::id_t unitID, ::Game::Type objectType, ::Game::Stance::Type stance) {
  Server::Unit *unit;
  {
    ScopeLock s(this->_mutexes[objectType]);
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, objectType));
    if (!unit || !unit->lock()) {
      return ;
    }
  }
  unit->setStance(stance);
  unit->unlock();
}

void	Server::Game::stopItemAction(Kernel::ID::id_t id, ::Game::Type type) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (!item || !item->lock()) {
      return ;
    }
  }
  item->stopAction();
  item->unlock();
}

void	Server::Game::setItemAction(Kernel::ID::id_t id, ::Game::Type type, ::Game::Action *action) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (!item || !item->lock()) {
      return ;
    }
  }
  item->setAction(action);
  item->unlock();
}

void	Server::Game::addItemToProductionList(Kernel::ID::id_t id, const Kernel::Serial &serial,
					      Server::GamePlayer *player) {
  Server::Building *building;
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(id, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      return ;
    }
  }
  const Kernel::Cost *cost = Kernel::Manager::getInstance()->getCost(serial);
  if (cost) {
    player->spendRessources(cost->ressources);
    building->addToQueue(serial);
  }
  building->unlock();
}

void	Server::Game::removeItemFromProductionList(Kernel::ID::id_t id, const Kernel::Serial &serial,
						   Server::GamePlayer *player) {
  Server::Building *building;
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(id, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      return ;
    }
  }
  const Kernel::Cost *cost = Kernel::Manager::getInstance()->getCost(serial);
  if (cost && building->removeFromQueue(serial)) {
    player->addRessources(cost->ressources);
  }
  building->unlock();
}

void	Server::Game::heroPickUpObject(Kernel::ID::id_t heroID, Kernel::ID::id_t objectID) {
  Server::Hero *hero;
  Server::Object *object;
  {
    LOCK_HERO;
    hero = static_cast<Server::Hero *>(this->_getItem(heroID, ::Game::HERO));
    if (hero == NULL || hero->lock() == false) {
      return ;
    }
  }
  {
    LOCK_OBJECT;
    object = static_cast<Server::Object *>(this->_getItem(objectID, ::Game::OBJECT));
    if (object == NULL || object->lock() == false) {
      hero->unlock();
      return ;
    }
  }
  if (hero->canPickUpObject() == false) {
    this->sendErrorToPlayer(this->getPlayerOwningItem(hero), ::Game::Controller::Error::HERO_CANNOT_CARRY_MORE_OBJECTS);
  }
  else if (object->pickUp(heroID) == false) {
    this->sendErrorToPlayer(this->getPlayerOwningItem(hero), ::Game::Controller::Error::OBJECT_IS_ALREADY_BEING_CARRIED);
  }
  else if (::Game::Controller::getInstance()->itemIsInRangeOfItem(hero, object)) {
    hero->addObject(object);
  }
  else {
    this->_askHeroPickUpObject(hero, object);
  }
  hero->unlock();
  object->unlock();
}

void	Server::Game::heroReleaseObject(Kernel::ID::id_t heroID, Kernel::ID::id_t objectID, double x, double y, double z) {
  Server::Hero *hero;
  {
    LOCK_HERO;
    hero = static_cast<Server::Hero *>(this->_getItem(heroID, ::Game::HERO));
    if (hero == NULL || !hero->lock()) {
      return ;
    }
  }
  Server::Object *object = NULL;
  for (auto it : hero->getObjects()) {
    if (it->getID() == objectID) {
      object = static_cast<Server::Object *>(it);
      break;
    }
  }
  if (::Game::Controller::getInstance()->itemIsInRangeOfPoint(hero, x, y, z)) {
    hero->removeObject(object);
    object->release();
    object->setPosition(x, y, z);
  }
  else {
    this->_askHeroReleaseObject(hero, object, x, y, z);
  }
  hero->unlock();
}

Server::GamePlayer	*Server::Game::getPlayerOwningItem(const Server::AItem *item) const {
  Kernel::ID::id_t id = item->getPlayer()->getID();
  if (this->_players.find(id) != this->_players.end()) {
    return (this->_players.at(id));
  }
  return (NULL);
}

void	Server::Game::sendErrorToPlayer(Server::GamePlayer *player, ::Game::Controller::Error::Type error) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->fail(::Command::ERR, ::Game::Controller::getInstance()->translate(error));
  player->addJob(job);
}

void	Server::Game::unitBuildBuilding(Kernel::ID::id_t unitID, Kernel::ID::id_t buildingID) {
  Server::Unit *unit;
  Server::Building *building;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(buildingID, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      unit->unlock();
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->unitCanBuildBuilding(unit, building, error)) {
    if (::Game::Controller::getInstance()->itemIsInRangeOfItem(unit, building)) {
      Server::Action *action = Factory::Server::Action::create(::Game::ACTION_BUILDING);
      action->building = building->getID();
      unit->setAction(action);
      building->addBuilder(unitID);
    }
    else {
      this->_askUnitToBuild(unit, building);
    }
  }
  unit->unlock();
  building->unlock();
}

void	Server::Game::unitStopBuilding(Kernel::ID::id_t unitID, Kernel::ID::id_t buildingID) {
  Server::Unit *unit;
  Server::Building *building;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(buildingID, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      unit->unlock();
      return ;
    }
  }
  unit->stopAction();
  building->removeBuilder(unitID);
  building->unlock();
  unit->unlock();
}

void	Server::Game::unitHarvestSpot(Kernel::ID::id_t unitID, Kernel::ID::id_t spotID) {
  Server::Unit *unit;
  Server::RessourcesSpot *spot;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_RESSSPOT;
    spot = static_cast<Server::RessourcesSpot *>(this->_getItem(spotID, ::Game::RESSSPOT));
    if (spot == NULL || spot->lock() == false) {
      unit->unlock();
      return ;
    }
  }
  unit->stopAction();
  if (::Game::Controller::getInstance()->itemIsInRangeOfItem(unit, spot)) {
    ::Game::Ressources::Type type = spot->getSpotType();
    if (unit->getRessources(type) == 0) {
      unit->setRessources(::Game::Ressources::WOOD, 0);
      unit->setRessources(::Game::Ressources::GOLD, 0);
      unit->setRessources(::Game::Ressources::FOOD, 0);
    }
    Server::Action *action = Factory::Server::Action::create(::Game::ACTION_HARVEST);
    action->spot = spotID;
    unit->setAction(action);
    unit->setSpot(spotID);
    spot->addHarvester(unitID, unit->getRessources(type));
  }
  else {
    this->createUnitHarvestEvent(unit, spot);
  }
  unit->unlock();
  spot->unlock();
}

void	Server::Game::unitHarvestRessources(Kernel::ID::id_t id, ::Game::Ressources::Type type, unsigned int amount) {
  Server::Unit *unit;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(id, ::Game::UNIT));
    if (!unit || unit->lock() == false) {
      return ;
    }
  }
  unit->addRessources(type, static_cast<int>(amount));
  unit->unlock();
}

Server::Building	*Server::Game::getClosestBuilding(const Server::AItem *item,
							  ::Game::Ressources::Type type) const {
  std::vector<Server::Building *> b;
  for (auto it : this->_buildings) {
    if (it.second && ::Game::Controller::getInstance()->unitCanDepositInBuilding(item, it.second, type)) {
      b.push_back(it.second);
    }
  }
  Server::Building *choice = NULL;
  unsigned int dist = std::numeric_limits<unsigned int>::max();
  std::vector<Vector3d> waypoints;
  Vector3d v;
  for (auto it : b) {
    /*    waypoints = this->_pathfinding.resolve(item, Vector3d(item->getX(), item->getY(), item->getZ()),
					   Vector3d(it->getX(), it->getY(), it->getZ()));
					   if (waypoints.size() < dist) {*/
    v = item->getPosition() - it->getPosition();
    unsigned int size = v.sqLength();
    if (size < dist) {
      dist = size; //waypoints.size();
      choice = it;
    }
  }
  return (choice);
}

void	Server::Game::_unitStopsHarvestAndDeposit(Server::Unit *unit, const Server::Building *building, ::Game::Ressources::Type type) {
  Server::Event *event;
  if (::Game::Controller::getInstance()->itemIsInRangeOfItem(unit, building)) {
    event = Factory::Server::Event::create(Server::Event::DEPOSIT_IN_BUILDING, Server::Event::SERVER_GAME, this);
  }
  else {
    event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_BUILDING_AND_DEPOSIT, Server::Event::SERVER_GAME, this);
  }
  event->deposit.unit = unit->getID();
  event->deposit.building = building->getID();
  event->deposit.type = type;
  unit->addEvent(event);
}

void	Server::Game::unitStopsHarvestAndDeposit(Kernel::ID::id_t unitID, Kernel::ID::id_t spotID) {
  Server::Unit *unit;
  Server::RessourcesSpot *spot;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_RESSSPOT;
    spot = static_cast<Server::RessourcesSpot *>(this->_getItem(spotID, ::Game::RESSSPOT));
    if (spot == NULL || spot->lock() == false) {
      unit->unlock();
      return ;
    }
  }
  spot->removeHarvester(unitID);
  Server::Building *building;
  {
    LOCK_BUILDING;
    building = this->getClosestBuilding(unit, spot->getSpotType());
    if (!building || building->lock() == false) {
      unit->unlock();
      spot->unlock();
      return ;
    }
  }
  this->_unitStopsHarvestAndDeposit(unit, building, spot->getSpotType());
  building->unlock();
  unit->unlock();
  spot->unlock();
}

void	Server::Game::unitStopsHarvest(Kernel::ID::id_t unitID, Kernel::ID::id_t spotID) {
  Server::Unit *unit;
  Server::RessourcesSpot *spot;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_RESSSPOT;
    spot = static_cast<Server::RessourcesSpot *>(this->_getItem(spotID, ::Game::RESSSPOT));
    if (spot == NULL || spot->lock() == false) {
      unit->unlock();
      return ;
    }
  }
  spot->removeHarvester(unitID);
  unit->unlock();
  spot->unlock();
}

void	Server::Game::unitStopsBuilding(Kernel::ID::id_t unitID, Kernel::ID::id_t buildingID) {
  Server::Unit *unit;
  Server::Building *building;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(buildingID, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      unit->unlock();
      return ;
    }
  }
  building->removeBuilder(unitID);
  unit->unlock();
  building->unlock();
}

void	Server::Game::unitDepositInBuilding(Kernel::ID::id_t unitID, Kernel::ID::id_t buildingID, ::Game::Ressources::Type type) {
  Server::Unit *unit;
  Server::Building *building;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(buildingID, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      building = this->getClosestBuilding(unit, type);
      if (building == NULL || building->lock() == false) {
	unit->unlock();
	return ;
      }
    }
  }
  if (::Game::Controller::getInstance()->itemIsInRangeOfItem(unit, building)) {
    unsigned int amount = unit->depositRessources(type);
    if (amount > 0) {
      LOCK_PLAYER;
      Server::GamePlayer *player = this->_players[unit->getPlayer()->getID()];
      player->addRessources(type, amount);
    }
  }
  else {
    this->_unitStopsHarvestAndDeposit(unit, building, type);
  }
  unit->unlock();
  building->unlock();
}

void	Server::Game::effectTimeout(Kernel::ID::id_t id, ::Game::Type type, const Kernel::Serial &effect) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  item->onEffectTimeout(effect);
  item->unlock();
}

void	Server::Game::askItemInformations(Server::GamePlayer *player, Kernel::ID::id_t id) {
  Server::AItem *item = this->_getItem(id);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::OASKINFO, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(id, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::OASKINFO, ::Game::Controller::Error::UNKNOWN_ITEM);
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskItemInformations(player, item, error)) {
    player->addJob(item->onAskInfos());
  }
  else {
    this->sendErrorJob(player, ::Command::OASKINFO, error);
  }
  item->unlock();
}

void	Server::Game::askPlayerRessources(Server::GamePlayer *player, Kernel::ID::id_t id) {
  Server::GamePlayer *oth = NULL;
  {
    LOCK_PLAYER;
    for (auto it : this->_players) {
      if (it.second->getID() == id) {
	oth = it.second;
	break ;
      }
    }
    if (oth) {
      oth->lock();
    }
  }
  if (oth == NULL) {
    this->sendErrorJob(player, ::Command::ASKPRESS, ::Game::Controller::Error::UNKNOWN_PLAYER);
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskPlayerRessources(player, oth, error)) {
    player->addJob(oth->onAskRessources());
  }
  else {
    this->sendErrorJob(player, ::Command::ASKPRESS, error);
  }
  oth->unlock();
}

void	Server::Game::itemIsInRangeOfAnotherItem(::Game::Type sourceType, Kernel::ID::id_t sourceID,
						 ::Game::Type targetType, Kernel::ID::id_t targetID) {
  Server::AItem *source;
  Server::AItem *target;
  {
    ScopeLock s(this->_mutexes[sourceType]);
    source = this->_getItem(sourceID, sourceType);
    if (source == NULL || source->lock() == false) {
      return ;
    }
  }
  {
    ScopeLock s(this->_mutexes[targetType]);
    target = this->_getItem(targetID, targetType);
    if (target == NULL || target->lock() == false) {
      source->unlock();
      return ;
    }
  }
  source->itemInRange(target);
  source->unlock();
  target->unlock();
}

void	Server::Game::askUnitsToHarvest(Server::GamePlayer *player, Kernel::ID::id_t id,
					const std::vector<Kernel::ID::id_t> &units) {
  for (auto it : units) {
    this->askUnitToHarvest(player, it, id);
  }
}

void	Server::Game::createUnitHarvestEvent(Server::Unit *unit, const Server::RessourcesSpot *spot) {
  Server::Event *event;
  if (::Game::Controller::getInstance()->itemIsInRangeOfItem(unit, spot)) {
    event = Factory::Server::Event::create(Server::Event::HARVEST_SPOT, Server::Event::SERVER_GAME, this);
  }
  else {
    event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_SPOT_AND_HARVEST, Server::Event::SERVER_GAME, this);
  }
  event->harvest.unit = unit->getID();
  event->harvest.spot = spot->getID();
  unit->addEvent(event);
}

void	Server::Game::sendErrorJob(Server::GamePlayer *player, ::Command::Command command,
				   ::Game::Controller::Error::Type type) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->fail(command, ::Game::Controller::getInstance()->translate(type));
  player->addJob(job);
}

void	Server::Game::askUnitToHarvest(Server::GamePlayer *player, Kernel::ID::id_t unitID, Kernel::ID::id_t spotID) {
  Server::AItem *item;
  Server::AItem *spot;
  {
    LOCK_RESSSPOT;
    spot = this->_getItem(spotID, ::Game::RESSSPOT);
    if (spot == NULL || spot->lock() == false) {
      this->sendErrorJob(player, ::Command::UASKHARVEST, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  {
    LOCK_UNIT;
    item = this->_getItem(unitID, ::Game::UNIT);
    if (item == NULL || item->lock() == false) {
      spot->unlock();
      this->sendErrorJob(player, ::Command::UASKHARVEST, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskUnitToHarvestRessourcesSpot(player, item, spot, error)) {
    this->createUnitHarvestEvent(static_cast<Server::Unit *>(item),
				 static_cast<const Server::RessourcesSpot *>(spot));
  }
  else {
    this->sendErrorJob(player, ::Command::UASKHARVEST, error);
  }
  item->unlock();
  spot->unlock();
}

void	Server::Game::askRessourcesSpotID(Server::GamePlayer *player, double x, double y, double z) {
  LOCK_RESSSPOT;
  for (auto it : this->_ressourcesSpots) {
    if (it.second->getX() == x && it.second->getY() == y && z == it.second->getZ()) {
      player->addJob(it.second->getRessJob());
      return ;
    }
  }
  this->sendErrorJob(player, ::Command::ASKRESSSPOTID, ::Game::Controller::Error::UNKNOWN_ITEM);
}

void	Server::Game::_askUnitToBuild(Server::Unit *unit, const Server::Building *building) {
  Server::Event *event;
  if (::Game::Controller::getInstance()->itemIsInRangeOfItem(unit, building)) {
    event = Factory::Server::Event::create(Server::Event::UNIT_BUILD,
					   Server::Event::SERVER_GAME, this);
  }
  else {
    event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_ITEM_AND_BUILD,
					   Server::Event::SERVER_GAME, this);
  }
  event->build.unit = unit->getID();
  event->build.building = building->getID();
  unit->addEvent(event);
}

void	Server::Game::askUnitToBuild(Server::GamePlayer *player, Kernel::ID::id_t unitID,
				     Kernel::ID::id_t buildingID) {
  Server::Unit *unit;
  Server::Building *building;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      this->sendErrorJob(player, ::Command::UASKBUILD, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(buildingID, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      unit->unlock();
      this->sendErrorJob(player, ::Command::UASKBUILD, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskUnitToBuild(player,
								 static_cast<const Server::Unit *>(unit),
								 static_cast<const Server::Building *>(building),
								 error)) {
    this->_askUnitToBuild(unit, building);
  }
  else {
    this->sendErrorJob(player, ::Command::UASKBUILD, error);
  }
  unit->unlock();
  building->unlock();
}

void	Server::Game::askUnitsToBuild(Server::GamePlayer *player, Kernel::ID::id_t buildingID,
				      const std::vector<Kernel::ID::id_t> &units) {
  for (auto it : units) {
    this->askUnitToBuild(player, it, buildingID);
  }
}

void	Server::Game::askPlayerInformations(Server::GamePlayer *player, Kernel::ID::id_t playerID) {
  Server::GamePlayer *oth = NULL;
  {
    LOCK_PLAYER;
    for (auto it : this->_players) {
      if (it.second->getID() == playerID) {
	oth = it.second;
	break;
      }
    }
    if (oth) {
      oth->lock();
    }
  }
  ::Game::Controller::Error::Type error;
  if (oth == NULL) {
    this->sendErrorJob(player, ::Command::ASKPAMELIO, ::Game::Controller::Error::PLAYER_DOES_NOT_EXIST);
    return ;
  }
  else if (::Game::Controller::getInstance()->playerCanAskAmeliorations(player, oth, error)) {
    Protocol::Job *job = NULL;
    for (auto it : oth->getAmeliorations()) {
      job = Factory::Protocol::Job::create();
      job->playerHasAmelioration(oth->getID(), it->appliesTo(), it->serial());
      player->addJob(job);
    }
  }
  else {
    this->sendErrorJob(player, ::Command::ASKPAMELIO, error);
  }
  oth->unlock();
}

void	Server::Game::askProduceAmelioration(Server::GamePlayer *player, const Kernel::Serial &item,
					     const Kernel::Serial &amelioration) {
  ::Game::Controller::Error::Type error = ::Game::Controller::Error::NOT_ENOUGH_RESSOURCES;
  const Kernel::Cost *cost = Kernel::Manager::getInstance()->getCost(amelioration);
  if (::Game::Controller::getInstance()->playerCanProduceAmelioration(player, item, amelioration, error) && cost &&
      player->lockRessources(cost->ressources)) {
    Server::Event *event = Factory::Server::Event::create(Server::Event::PRODUCE_AMELIORATION,
							  Server::Event::SERVER_GAMEPLAYER, player);
    event->ameliorationProductionList.player = player;
    Kernel::assign_serial(event->ameliorationProductionList.type, item);
    Kernel::assign_serial(event->ameliorationProductionList.amelioration, amelioration);
    this->addEvent(event);
  }
  else {
    this->sendErrorJob(player, ::Command::ASKPRODAMELIO, error);
  }
}

void	Server::Game::askSpecificElementAmeliorations(Server::GamePlayer *player, Kernel::ID::id_t playerID,
						      const Kernel::Serial &item) {
  Server::GamePlayer *oth = NULL;
  {
    LOCK_PLAYER;
    for (auto it : this->_players) {
      if (it.second->getID() == playerID) {
	oth = it.second;
	oth->lock();
	if (oth->getID() == playerID) {
	  break;
	}
	oth->unlock();
	oth = NULL;
      }
    }
  }
  ::Game::Controller::Error::Type error;
  if (oth == NULL) {
    this->sendErrorJob(player, ::Command::ASKPSAMELIO, ::Game::Controller::Error::PLAYER_DOES_NOT_EXIST);
  }
  else if (::Game::Controller::getInstance()->playerCanAskAmeliorations(player, oth, item, error)) {
    Protocol::Job *job = NULL;
    for (auto it : oth->getAmeliorations(item)) {
      job = Factory::Protocol::Job::create();
      job->playerHasAmelioration(oth->getID(), it->appliesTo(), it->serial());
      player->addJob(job);
    }
  }
  else {
    this->sendErrorJob(player, ::Command::ASKPSAMELIO, error);
  }
  if (oth) {
    oth->unlock();
  }
}

void	Server::Game::askHeroObjects(Server::GamePlayer *player, Kernel::ID::id_t heroID) {
  Server::Hero *hero;
  {
    LOCK_HERO;
    hero = static_cast<Server::Hero *>(this->_getItem(heroID, ::Game::HERO));
    if (hero == NULL || hero->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKHOBJ, ::Game::Controller::Error::UNKNOWN_ITEM);
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskHeroObjects(player, hero, error)) {
    std::vector<Kernel::ID::id_t> objects;
    for (auto it : hero->getObjects()) {
      objects.push_back(it->getID());
    }
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->heroObjectsList(hero->getID(), objects);
    player->addJob(job);
  }
  else {
    this->sendErrorJob(player, ::Command::ASKHOBJ, error);
  }
  hero->unlock();
}

void	Server::Game::_askHeroReleaseObject(Server::Hero *hero, const Server::Object *object,
					    double x, double y, double z) {
  Server::Event *event;
  if (::Game::Controller::getInstance()->itemIsInRangeOfPoint(hero, x, y, z)) {
    event = Factory::Server::Event::create(Server::Event::RELEASE_OBJECT, Server::Event::SERVER_GAME, this);
  }
  else {
    event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_POINT_AND_RELEASE_OBJECT, Server::Event::SERVER_GAME, this);
  }
  event->releaseObject.hero = hero->getID();
  event->releaseObject.object = object->getID();
  event->releaseObject.x = x;
  event->releaseObject.y = y;
  event->releaseObject.z = z;
  hero->addEvent(event);
}

void	Server::Game::askHeroReleaseObject(Server::GamePlayer *player, Kernel::ID::id_t heroID,
					   Kernel::ID::id_t objectID, double x, double y, double z) {
  Server::Hero *hero;
  {
    LOCK_HERO;
    hero = static_cast<Server::Hero *>(this->_getItem(heroID, ::Game::HERO));
    if (hero == NULL || hero->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKHROBJ, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  Server::Object *object = NULL;
  for (auto it : hero->getObjects()) {
    if (it->getID() == objectID) {
      object = static_cast<Server::Object *>(it);
      break;
    }
  }
  ::Game::Controller::Error::Type error;
  if (object == NULL) {
    this->sendErrorJob(player, ::Command::ASKHROBJ, ::Game::Controller::Error::UNKNOWN_ITEM);
  }
  else if (::Game::Controller::getInstance()->playerCanAskHeroToReleaseObject(player, hero, object, error)) {
    this->_askHeroReleaseObject(hero, object, x, y, z);
  }
  else {
    this->sendErrorJob(player, ::Command::ASKHROBJ, error);
  }
  hero->unlock();
}

void	Server::Game::askStopItemAction(Server::GamePlayer *player, Kernel::ID::id_t itemID) {
  Server::AItem *item = this->_getItem(itemID);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::OSTOP, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(itemID, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::OSTOP, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanStopItemAction(player, item, error)) {
    Server::Event *event = Factory::Server::Event::create(Server::Event::STOP_ACTION,
							  Server::Event::SERVER_GAME, this);
    event->itemInfos.id = item->getID();
    event->itemInfos.type = item->getType();
    item->addEvent(event);
  }
  else {
    this->sendErrorJob(player, ::Command::OSTOP, error);
  }
  item->unlock();
}

void	Server::Game::_askHeroPickUpObject(Server::Hero *hero, const Server::Object *object) {
  Server::Event *event;
  if (::Game::Controller::getInstance()->itemIsInRangeOfItem(hero, object)) {
    event = Factory::Server::Event::create(Server::Event::PICK_UP_OBJECT,
					   Server::Event::SERVER_GAME, this);
  }
  else {
    event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_OBJECT_AND_PICK_UP_OBJECT,
					   Server::Event::SERVER_GAME, this);
  }
  event->pickUpObject.hero = hero->getID();
  event->pickUpObject.object = object->getID();
  hero->addEvent(event);
}

void	Server::Game::askHeroPickUpObject(Server::GamePlayer *player, Kernel::ID::id_t heroID,
					  Kernel::ID::id_t objectID) {
  Server::Object *object;
  Server::Hero *hero;
  {
    LOCK_HERO;
    hero = static_cast<Server::Hero *>(this->_getItem(heroID, ::Game::HERO));
    if (hero == NULL || hero->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKHPUOBJ, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  {
    LOCK_OBJECT;
    object = static_cast<Server::Object *>(this->_getItem(objectID, ::Game::OBJECT));
    if (object == NULL || object->lock() == false) {
      hero->unlock();
      this->sendErrorJob(player, ::Command::ASKHPUOBJ, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskHeroToPickUpObject(player, hero, object, error)) {
    this->_askHeroPickUpObject(hero, object);
  }
  else {
    this->sendErrorJob(player, ::Command::ASKHPUOBJ, error);
  }
  object->unlock();
  hero->unlock();
}

void	Server::Game::askSetItemsAction(Server::GamePlayer *player, ::Game::eAction action,
					const std::vector<Kernel::ID::id_t> &items) {
  for (auto it : items) {
    this->askSetItemAction(player, it, action);
  }
}

void	Server::Game::askSetItemAction(Server::GamePlayer *player, Kernel::ID::id_t itemID,
				       ::Game::eAction action) {
  ::Game::Controller::Error::Type error;
  Server::AItem *item = this->_getItem(itemID);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::OSETACTION, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(itemID, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::OSETACTION, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  if (::Game::Controller::getInstance()->playerCanSetItemAction(player, item, error)) {
    Server::Event *event = Factory::Server::Event::create(Server::Event::SET_ACTION,
							  Server::Event::SERVER_GAME, this);
    event->itemAction.id = item->getID();
    event->itemAction.type = item->getType();
    event->itemAction.action = action;
    item->addEvent(event);
  }
  else {
    this->sendErrorJob(player, ::Command::OSETACTION, error);
  }
  item->unlock();
}

void	Server::Game::askUnitProduction(Server::GamePlayer *player, Kernel::ID::id_t buildingID,
					const Kernel::Serial &unit) {
  Server::Building *building;
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(buildingID, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      this->sendErrorJob(player, ::Command::UASKPROD, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error = ::Game::Controller::Error::NOT_ENOUGH_RESSOURCES;
  const Kernel::Cost *cost = Kernel::Manager::getInstance()->getCost(unit);
  if (::Game::Controller::getInstance()->playerCanCreateUnit(player, building, unit, error) && cost &&
      player->lockRessources(cost->ressources)) {
    Server::Event *event = Factory::Server::Event::create(Server::Event::PUT_IN_PRODUCTION_LIST,
							  Server::Event::SERVER_GAME, this);
    event->production.id = building->getID();
    Kernel::assign_serial(event->production.serial, unit);
    event->production.player = player;
    building->addEvent(event);
  }
  else {
    this->sendErrorJob(player, ::Command::UASKPROD, error);
  }
  building->unlock();
}

void	Server::Game::askRemoveUnitFromProduction(Server::GamePlayer *player, Kernel::ID::id_t buildingID,
						  const Kernel::Serial &unit) {
  Server::Building *building;
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(buildingID, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      this->sendErrorJob(player, ::Command::UASKDPROD, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanRemoveUnitFromProductionList(player, building, unit, error)) {
    Server::Event *event = Factory::Server::Event::create(Server::Event::REMOVE_PRODUCTION_LIST,
							  Server::Event::SERVER_GAME, this);
    event->production.id = building->getID();
    Kernel::assign_serial(event->production.serial, unit);
    event->production.player = player;
    building->addEvent(event);
  }
  else {
    this->sendErrorJob(player, ::Command::UASKDPROD, error);
  }
  building->unlock();
}

void	Server::Game::askItemEffects(Server::GamePlayer *player, Kernel::ID::id_t itemID) {
  ::Game::Controller::Error::Type error = ::Game::Controller::Error::UNKNOWN_ITEM;
  Server::AItem *item = this->_getItem(itemID);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::ASKOEFFECT, error);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(itemID, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKOEFFECT, error);
      return ;
    }
  }
  if (::Game::Controller::getInstance()->playerCanAskItemEffects(player, item, error)) {
    const ::Game::AElement::EffectContainer &effects = item->getEffects();
    Protocol::Job *job;
    for (auto it : effects) {
      for (auto effect : it.second) {
	job = Factory::Protocol::Job::create();
	job->itemEffect(item->getID(), effect->serial(), effect->getPercentage());
	player->addJob(job);
      }
    }
  }
  else {
    this->sendErrorJob(player, ::Command::ASKOEFFECT, error);
  }
  item->unlock();
}

void	Server::Game::askUnitPickedUpRessources(Server::GamePlayer *player, Kernel::ID::id_t unitID) {
  Server::Unit *item;
  {
    LOCK_UNIT;
    item = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKUPURESS, ::Game::Controller::Error::UNKNOWN_ITEM);
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskUnitPickedUpRessources(player, item, error)) {
    Protocol::Job *job = item->getRessJob();
    player->addJob(job);
  }
  else {
    this->sendErrorJob(player, ::Command::ASKUPURESS, error);
  }
  item->unlock();
}

void	Server::Game::askItemAction(Server::GamePlayer *player, Kernel::ID::id_t itemID) {
  Server::AItem *item = this->_getItem(itemID);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::OASKACT, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(itemID, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::OASKACT, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskItemAction(player, item, error)) {
    Protocol::Job *job = item->onAskItemAction();
    player->addJob(job);
  }
  else {
    this->sendErrorJob(player, ::Command::OASKACT, error);
  }
  item->unlock();
}

void	Server::Game::unitGoBackToSpot(Kernel::ID::id_t unitID, Kernel::ID::id_t spotID) {
  Server::Unit *unit;
  Server::RessourcesSpot *spot;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_RESSSPOT;
    spot = static_cast<Server::RessourcesSpot *>(this->_getItem(spotID, ::Game::RESSSPOT));
    if (spot == NULL || spot->lock() == false) {
      unit->unlock();
      return ;
    }
  }
  this->createUnitHarvestEvent(unit, spot);
  unit->unlock();
  spot->unlock();
}

void	Server::Game::createMoveEvent(Server::AItem *item, double x, double y, double z, Server::GroupMove *grpMove) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::START_MOVE,
							Server::Event::SERVER_GAME, this);
  event->move.id = item->getID();
  event->move.type = item->getType();
  event->move.x = x;
  event->move.y = y;
  event->move.z = z;
  item->addEvent(event);
  item->setGroupMove(grpMove);
}

void	Server::Game::_askMoveItem(Server::GamePlayer *player, Server::AItem *item,
				   double x, double y, double z, Server::GroupMove *grpMove) {
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanMoveItem(player, item, error)) {
    this->createMoveEvent(item, x, y, z, grpMove);
  }
  else {
    this->sendErrorJob(player, ::Command::OASKMOVE, error);
  }
}

void	Server::Game::askMoveItem(Server::GamePlayer *player, Kernel::ID::id_t itemID,
				  double x, double y, double z) {
  Server::AItem *item = this->_getItem(itemID, MOVABLE_OBJECTS);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::OASKMOVE, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(itemID, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::OASKMOVE, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  this->_askMoveItem(player, item, x, y, z, NULL);
  item->unlock();
}

Server::Movable *Server::Game::getMovableBase(Server::AItem *item, ::Game::Type type) const {
  Server::Movable *movableBase = NULL;
  switch (type) {
  case ::Game::UNIT:
  case ::Game::HERO:
    movableBase = static_cast<Server::Movable *>(static_cast<Server::Unit *>(item));
    break;
  case ::Game::PROJECTILE:
    movableBase = static_cast<Server::Movable *>(static_cast<Server::Projectile *>(item));
    break;
  default:
    break;
  }
  return (movableBase);
}

void	Server::Game::_startMoveItem(Server::AItem *item, Server::Movable *movableBase,
				     double x, double y, double z, Algo::Pathfinding::WhatToDo,
				     Server::Action *next, bool dAction) {
  // const std::vector<Vector3d> waypoints = this->_pathfinding.resolve(item,
  // 								     Vector3d(item->getX(), item->getY(),
  // 									      item->getZ()),
  // 								     Vector3d(x, y, z),
  // 								     w);
   std::vector<Vector3d> waypoints;
  waypoints.push_back(Vector3d(x, y, z));
  if (waypoints.size() == 0) {
    return ;
  }
  if (dAction) {
    Server::Action *action = Factory::Server::Action::create(::Game::ACTION_MOVE);
    action->move.x = x;
    action->move.y = y;
    action->move.z = z;
    action->setNextAction(next);
    item->stopAction();
    item->setAction(action);
    movableBase->onStartMove(x, y, z);
  }
  movableBase->setWaypoints(waypoints);
}

void	Server::Game::startMoveItem(Kernel::ID::id_t itemID, ::Game::Type type,
				    double x, double y, double z) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(itemID, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  Server::Movable *movableBase = this->getMovableBase(item, type);
  if (movableBase == NULL) {
    return ;
  }
  //  movableBase->setWaypoints(std::vector<Vector3d>(1, Vector3d(x, y, z)));
  this->_startMoveItem(item, movableBase, x, y, z, Algo::Pathfinding::CLOSEST, NULL);
  item->unlock();
}

// void	Server::Game::_moveItem(Server::AItem *item, double factor) {
//   Server::Movable *movableBase = this->getMovableBase(item, item->getType());
//   if (movableBase == NULL) {
//     return ;
//   }
//   Vector3d vd;
//   if (!this->getDestinationData(movableBase, vd, factor)) {
//     Logger::getInstance()->log("Cannot get destination data");
//     return ;
//   }
//   Vector3d dest = movableBase->getFrontWaypoint();
//   double nx, ny, nz;
//   nx = dest.x;
//   ny = dest.y;
//   nz = dest.z;
//   if (this->moveItem(item, movableBase, nx, ny, nz) == false) {
//   }
//   else {
//     movableBase->waypointReached();
//     if (!this->getDestinationData(movableBase, vd, factor)) {
//       movableBase->onDestinationReached();
//     }
//   }
// }

void	Server::Game::_moveItem(Server::AItem *item, double factor) {
  Server::Movable *movableBase = this->getMovableBase(item, item->getType());
  if (movableBase == NULL) {
    return ;
  }
  if (item->getSpeed() == 0) {
    movableBase->onDestinationReached();
    return ;
  }
  Vector3d vd;
  if (!this->getDestinationData(movableBase, vd, factor)) {
    return ;
  }
  Vector3d dest = movableBase->getFrontWaypoint();
  double nx, ny, nz;
  bool onIt = false;
  unsigned int nbr = movableBase->getSpeed();
  for (unsigned int j = 0 ; j < nbr ; ++j) {
    onIt = false;
    nx = item->getX() + vd.x;
    ny = item->getY() + vd.y;
    nz = item->getZ() + vd.z;
    if (IS_BETWEEN_B(item->getX(), dest.x, nx) && IS_BETWEEN_B(item->getY(), dest.y, ny)) {
      nx = dest.x;
      ny = dest.y;
      nz = dest.z;
      onIt = true;
    }
    if (this->moveItem(item, movableBase, nx, ny, nz) == false) {
      movableBase->onBlocked();
      break;
    }
    else {
      if (onIt) {
	movableBase->waypointReached();
	if (!this->getDestinationData(movableBase, vd, factor)) {
	  movableBase->onDestinationReached();
	  break;
	}
	else if (this->checkStopCondition(item)) {
	  movableBase->onDestinationReached();
	  break;
	}
      }
    }
  }
}

bool	Server::Game::checkStopCondition(Server::AItem *source) {
  Server::Action *next = static_cast<const Server::Action *>(source->getAction())->getNextAction();
  if (next == NULL) {
    return (false);
  }
  bool item = false;
  Vector3d zone;
  Kernel::ID::id_t targetID = 0;
  ::Game::Type targetType = ::Game::NONE;
  switch (next->getType()) {
  case ::Game::ACTION_START_CAPACITY:
    if (next->useCapacity.type == ::Game::Capacity::ITEM) {
      item = true;
      targetID = next->useCapacity.target.id;
      targetType = next->useCapacity.target.type;
    }
    else {
      item = false;
      zone.x = next->useCapacity.point.x;
      zone.y = next->useCapacity.point.y;
      zone.z = next->useCapacity.point.z;
    }
    break;
  default:
    return (false);
  }
  if (item) {
    Server::AItem *target;
    {
      ScopeLock s(this->_mutexes[targetType]);
      target = this->_getItem(targetID, targetType);
      if (target == NULL || target->lock() == false) {
	return (false);
      }
    }
    bool r = this->checkStopCondition(source, target);
    target->unlock();
    return (r);
  }
  else {
    this->checkStopCondition(source, zone);
  }
  return (false);
}

bool	Server::Game::checkStopCondition(Server::AItem *source, const Server::AItem *target) {
  return (this->checkStopCondition(source, target->getPosition()));
}

bool	Server::Game::checkStopCondition(Server::AItem *source, const Vector3d &zone) {
  int stopCond = this->getMovableBase(source, source->getType())->getStopCondition();
  if (stopCond & Server::Movable::ATTACK_RANGE) {
    if (!::Game::Controller::getInstance()->itemHasAttackRangeOnPoint(source, zone.x, zone.y, zone.z)) {
      return (false);
    }
  }
  if (stopCond & Server::Movable::VISION) {
    if (!::Game::Controller::getInstance()->itemHasVisionOnPoint(this->_map, source, zone.x, zone.y, zone.z)) {
      return (false);
    }
  }
  return (true);
}

void	Server::Game::moveItem(Kernel::ID::id_t itemID, ::Game::Type type, double factor) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(itemID, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  DEBUG_LOL
  item->lockMove();
  DEBUG_LOL
  this->_moveItem(item, factor);
  item->unlockMove();
  item->unlock();
}

bool	Server::Game::getDestinationData(Server::Movable *movableBase, Vector3d &vd, double factor) const {
  if (movableBase->hasWaypoints() == false) {
    return (false);
  }
  const Vector3d destination = movableBase->getFrontWaypoint();
  const Vector3d position = movableBase->getPosition();
  vd = destination - position;
  //  Logger::getInstance()->logf("Vector3d(%f ; %f ; %f)", Logger::DEBUG, vd.x, vd.y, vd.z);
  if (vd.x == 0.0 && vd.y == 0.0) {
    movableBase->waypointReached();
    return (this->getDestinationData(movableBase, vd, factor));
  }
  vd /= sqrt(vd.x * vd.x + vd.y * vd.y);
  //  Logger::getInstance()->logf("Vector3d(%f ; %f ; %f)", Logger::DEBUG, vd.x, vd.y, vd.z);
  vd *= factor;
  //  Logger::getInstance()->logf("Vector3d(%f ; %f ; %f)", Logger::DEBUG, vd.x, vd.y, vd.z);
  vd *= (static_cast<double>(movableBase->getSpeed()) / 70.0);
  vd /= 100.0;
  //  Logger::getInstance()->logf("Vector3d(%f ; %f ; %f)", Logger::DEBUG, vd.x, vd.y, vd.z);
  //vd /= (Kernel::Config::getInstance()->getDouble(Kernel::Config::UPDATES_PER_SECOND));
  vd.z = 0.0;
  return (true);
}

bool	Server::Game::moveItem(Server::AItem *item, Server::Movable *movableBase, double nx, double ny, double nz) {
  if (!this->_map->move(item, movableBase, nx, ny, nz)) {
    return (false);
  }
  return (true);
}

void	Server::Game::moveTowardsPointAndReleaseObject(Kernel::ID::id_t heroID, Kernel::ID::id_t objectID,
						       double x, double y, double z) {
  Server::Hero *hero;
  {
    LOCK_HERO;
    hero = static_cast<Server::Hero *>(this->_getItem(heroID, ::Game::HERO));
    if (hero == NULL || hero->lock() == false) {
      return ;
    }
  }
  Server::Action *next = Factory::Server::Action::create(::Game::ACTION_RELEASE);
  next->release.object = objectID;
  next->release.x = x;
  next->release.y = y;
  next->release.z = z;
  this->_startMoveItem(hero, static_cast<Server::Movable *>(hero), x, y, z,
		       Algo::Pathfinding::CLOSEST_IN_RANGE, next);
  hero->unlock();
}

void	Server::Game::moveTowardsObjectAndPickUp(Kernel::ID::id_t heroID, Kernel::ID::id_t objectID) {
  Server::Hero *hero;
  Server::Object *object;
  {
    LOCK_HERO;
    hero = static_cast<Server::Hero *>(this->_getItem(heroID, ::Game::HERO));
    if (hero == NULL || hero->lock() == false) {
      return ;
    }
  }
  {
    LOCK_OBJECT;
    object = static_cast<Server::Object *>(this->_getItem(objectID, ::Game::OBJECT));
    if (object == NULL || object->lock() == false) {
      hero->unlock();
      return ;
    }
  }
  Server::Action *next = Factory::Server::Action::create(::Game::ACTION_PICKUP);
  next->object = objectID;
  this->_startMoveItem(hero, static_cast<Server::Movable *>(hero), object->getX(), object->getY(), object->getZ(),
		       Algo::Pathfinding::CLOSEST_IN_RANGE, next);
  object->unlock();
  hero->unlock();
}

void	Server::Game::moveTowardsItemAndBuild(Kernel::ID::id_t unitID, Kernel::ID::id_t buildingID) {
  Server::Unit *unit;
  Server::Building *building;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(buildingID, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      unit->unlock();
      return ;
    }
  }
  Server::Action *next = Factory::Server::Action::create(::Game::ACTION_START_BUILDING);
  next->building = building->getID();
  this->_startMoveItem(unit, static_cast<Server::Movable *>(unit), building->getX(), building->getY(),
		       building->getZ(), Algo::Pathfinding::CLOSEST_IN_RANGE, next);
  unit->unlock();
  building->unlock();
}

void	Server::Game::moveTowardsBuildingAndDeposit(Kernel::ID::id_t unitID, Kernel::ID::id_t buildingID,
						    ::Game::Ressources::Type type) {
  Server::Unit *unit;
  Server::Building *building;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_BUILDING;
    building = static_cast<Server::Building *>(this->_getItem(buildingID, ::Game::BUILDING));
    if (building == NULL || building->lock() == false) {
      building = this->getClosestBuilding(unit, type);
      if (building == NULL || building->lock() == false) {
	unit->unlock();
	return ;
      }
    }
  }
  Server::Action *next = Factory::Server::Action::create(::Game::ACTION_DEPOSIT);
  next->deposit.building = building->getID();
  next->deposit.type = type;
  this->_startMoveItem(unit, static_cast<Server::Movable *>(unit), building->getX(), building->getY(),
		       building->getZ(), Algo::Pathfinding::CLOSEST_IN_RANGE, next);
  unit->unlock();
  building->unlock();
}

void	Server::Game::moveTowardsSpotAndHarvest(Kernel::ID::id_t unitID, Kernel::ID::id_t spotID) {
  Server::Unit *unit;
  Server::RessourcesSpot *spot;
  {
    LOCK_UNIT;
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  {
    LOCK_RESSSPOT;
    spot = static_cast<Server::RessourcesSpot *>(this->_getItem(spotID, ::Game::RESSSPOT));
    if (spot == NULL || spot->lock() == false) {
      unit->unlock();
      return ;
    }
  }
  Server::Action *next = Factory::Server::Action::create(::Game::ACTION_START_HARVEST);
  next->spot = spot->getID();
  this->_startMoveItem(unit, static_cast<Server::Movable *>(unit), spot->getX(), spot->getY(), spot->getZ(),
		       Algo::Pathfinding::CLOSEST_IN_RANGE, next);
  unit->unlock();
  spot->unlock();
}

void	Server::Game::askItemToAttack(Server::GamePlayer *player, Kernel::ID::id_t item1ID, Kernel::ID::id_t item2ID) {
  Server::AItem *item1 = this->_getItem(item1ID);
  if (item1 == NULL) {
    this->sendErrorJob(player, ::Command::ASKOATTACK, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s1(this->_mutexes[item1->getType()]);
    item1 = this->_getItem(item1ID, item1->getType());
    if (item1 == NULL || item1->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKOATTACK, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  Server::AItem *item2 = this->_getItem(item2ID);
  if (item2 == NULL) {
    this->sendErrorJob(player, ::Command::ASKOATTACK, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s2(this->_mutexes[item2->getType()]);
    item2 = this->_getItem(item2ID, item2->getType());
    if (item2 == NULL || item2->lock() == false) {
      item1->unlock();
      this->sendErrorJob(player, ::Command::ASKOATTACK, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskItemToAttackItem(player, item1, item2, error)) {
    Server::CanUseCapacity *base = this->getCanUseCapacityBase(item1);
    this->processAskItemToUseCapacity(player, item1, item2, base->getAttackCapacity());
  }
  else {
    this->sendErrorJob(player, ::Command::ASKOATTACK, error);
  }
  item1->unlock();
  item2->unlock();
}

Server::CanUseCapacity	*Server::Game::getCanUseCapacityBase(Server::AItem *item) const {
  Server::CanUseCapacity *base = NULL;
  switch (item->getType()) {
  case ::Game::UNIT:
    base = static_cast<Server::CanUseCapacity *>(static_cast<Server::Unit *>(item));
    break;
  case ::Game::HERO:
    base = static_cast<Server::CanUseCapacity *>(static_cast<Server::Hero *>(item));
    break;
  case ::Game::BUILDING:
    base = static_cast<Server::CanUseCapacity *>(static_cast<Server::Building *>(item));
    break;
  default:
    break;
  }
  return (base);
}

void	Server::Game::createProjectile(Server::AItem *projectile, Kernel::ID::id_t item1ID, ::Game::Type item1Type,
				       Kernel::ID::id_t item2ID, ::Game::Type item2Type) {
  {
    Server::AItem *item1;
    Server::AItem *item2;
    {
      ScopeLock s(this->_mutexes[item1Type]);
      item1 = this->_getItem(item1ID, item1Type);
      if (item1 == NULL || item1->lock() == false) {
	Factory::Server::Projectile::remove(static_cast<Server::Projectile *>(projectile));
	return ;
      }
    }
    {
      ScopeLock s(this->_mutexes[item2Type]);
      item2 = this->_getItem(item2ID, item2Type);
      if (item2 == NULL || item2->lock() == false) {
	item1->unlock();
	Factory::Server::Projectile::remove(static_cast<Server::Projectile *>(projectile));
	return ;
      }
    }
    static_cast<Server::Projectile *>(projectile)->setSource(item1);
    static_cast<Server::Projectile *>(projectile)->setTarget(item2);
    Server::Action *action = Factory::Server::Action::create(::Game::ACTION_PROJECTILE_FOLLOW);
    projectile->setAction(action);
    item1->unlock();
    item2->unlock();
  }
  this->addItem(projectile);
}

void	Server::Game::createProjectile(Server::AItem *projectile, Kernel::ID::id_t itemID, ::Game::Type type,
				       double x, double y, double z) {
  ScopeLock s(this->_mutexes[type]);
  Server::AItem *item = this->_getItem(itemID, type);
  if (item == NULL) {
    Factory::Server::Projectile::remove(static_cast<Server::Projectile *>(projectile));
    return ;
  }
  static_cast<Server::Projectile *>(projectile)->setSource(item);
  static_cast<Server::Projectile *>(projectile)->setTarget(x, y, z);
  Server::Action *action = Factory::Server::Action::create(::Game::ACTION_PROJECTILE_MOVE);
  action->move.x = x;
  action->move.y = y;
  action->move.z = z;
  projectile->setAction(action);
}

void	Server::Game::followItem(Kernel::ID::id_t item1ID, ::Game::Type item1Type,
				 Kernel::ID::id_t item2ID, ::Game::Type item2Type, double factor) {
  Server::AItem *item1;
  Server::AItem *item2;
  {
    ScopeLock s(this->_mutexes[item1Type]);
    item1 = this->_getItem(item1ID, item1Type);
    if (item1 == NULL || item1->lock() == false) {
      return ;
    }
  }
  {
    ScopeLock s(this->_mutexes[item2Type]);
    item2 = this->_getItem(item2ID, item2Type);
    if (item2 == NULL || item2->lock() == false) {
      item1->unlock();
      return ;
    }
  }
  Server::Movable *movableBase = this->getMovableBase(item1, item1->getType());
  if (movableBase->mustGeneratePath(item2)) {
    this->_startMoveItem(item1, movableBase, item2->getX(), item2->getY(), item2->getZ(),
			 Algo::Pathfinding::CLOSEST, NULL, false);
  }
  this->_moveItem(item1, factor);
  item1->unlock();
  item2->unlock();
}

void	Server::Game::askUnitStance(Server::GamePlayer *player, Kernel::ID::id_t unitID) {
  Server::AItem *item;
  item = this->_getItem(unitID, ::Game::UNIT | ::Game::HERO);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::UASKSTANCE, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(unitID, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::UASKSTANCE, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskUnitStance(player, item, error)) {
    ::Game::Stance::Type stance = static_cast<const Server::Unit *>(item)->getStance();
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->unitStance(item->getID(), stance);
    player->addJob(job);
  }
  else {
    this->sendErrorJob(player, ::Command::UASKSTANCE, error);
  }
  item->unlock();
}

void	Server::Game::askChangeUnitStance(Server::GamePlayer *player, Kernel::ID::id_t unitID,
					  ::Game::Stance::Type stance) {
  Server::AItem *item = this->_getItem(unitID, ::Game::UNIT | ::Game::HERO);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::UCHGSTANCE, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(unitID, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::UCHGSTANCE, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanChangeUnitStance(player, item, error)) {
    this->createChangeUnitStanceEvent(static_cast<Server::Unit *>(item), stance);
  }
  else {
    this->sendErrorJob(player, ::Command::UCHGSTANCE, error);
  }
  item->unlock();
}

void	Server::Game::createChangeUnitStanceEvent(Server::Unit *unit, ::Game::Stance::Type stance) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::UNIT_STANCE, Server::Event::SERVER_GAME, this);
  event->stance.id = unit->getID();
  event->stance.type = unit->getType();
  event->stance.stance = stance;
  unit->addEvent(event);
}

void	Server::Game::askChangeUnitsStance(Server::GamePlayer *player, ::Game::Stance::Type stance,
					   const std::vector<Kernel::ID::id_t> &units) {
  for (auto it : units) {
    this->askChangeUnitStance(player, it, stance);
  }
}

void	Server::Game::askUnitToPatrol(Server::GamePlayer *player, Kernel::ID::id_t unitID, double x, double y) {
  Server::Unit *unit = static_cast<Server::Unit *>(this->_getItem(unitID, ::Game::UNIT | ::Game::HERO));
  if (unit == NULL) {
    this->sendErrorJob(player, ::Command::ASKUPATROL, ::Game::Controller::Error::UNKNOWN_ITEM);
  }
  {
    ScopeLock s(this->_mutexes[unit->getType()]);
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, unit->getType()));
    if (unit == NULL || unit->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKUPATROL, ::Game::Controller::Error::UNKNOWN_ITEM);
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskUnitToPatrol(player, unit, error)) {
    this->_askUnitToPatrol(unit, x, y, NULL);
  }
  else {
    this->sendErrorJob(player, ::Command::ASKUPATROL, error);
  }
  unit->unlock();
}

void	Server::Game::_askUnitToPatrol(Server::Unit *unit, double x, double y, Server::GroupMove *groupMove) {
  unit->setGroupMove(groupMove);
  Server::Event *event = Factory::Server::Event::create(Server::Event::PATROL, Server::Event::SERVER_GAME, this);
  event->patrol.item.id = unit->getID();
  event->patrol.item.type = unit->getType();
  event->patrol.x = x;
  event->patrol.y = y;
  unit->addEvent(event);
}

void	Server::Game::askUnitsToPatrol(Server::GamePlayer *player, const std::vector<Kernel::ID::id_t> &units,
				       double x, double y, bool adapt) {
  std::list<Server::AItem *> items;
  std::list<const Server::AItem *> grp;
  Server::GroupMove *groupMove = NULL;
  {
    LOCK_UNIT;
    LOCK_HERO;
    Server::Unit *unit;
    for (auto it : units) {
      unit = static_cast<Server::Unit *>(this->_getItem(it, ::Game::UNIT | ::Game::HERO));
      if (unit && unit->lock() == true) {
	items.push_back(unit);
	if (adapt) {
	  grp.push_back(unit);
	}
      }
    }
    if (adapt) {
      groupMove = Factory::Server::GroupMove::create(grp);
    }
  }
  ::Game::Controller::Error::Type error;
  for (auto it : items) {
    if (::Game::Controller::getInstance()->playerCanAskUnitToPatrol(player, it, error)) {
      this->_askUnitToPatrol(static_cast<Server::Unit *>(it), x, y, groupMove);
    }
    it->unlock();
  }
}

void	Server::Game::unitPatrol(Kernel::ID::id_t unitID, ::Game::Type type, double x, double y) {
  Server::Unit *unit;
  {
    ScopeLock s(this->_mutexes[type]);
    unit = static_cast<Server::Unit *>(this->_getItem(unitID, type));
    if (unit == NULL || unit->lock() == false) {
      return ;
    }
  }
  Server::Action *action = Factory::Server::Action::create(::Game::ACTION_PATROL);
  action->patrol.from.x = unit->getX();
  action->patrol.from.y = unit->getY();
  action->patrol.to.x = x;
  action->patrol.to.y = y;
  action->patrol.way = true;
  unit->setAction(action);
  unit->unlock();
}

void	Server::Game::moveToPoint(Kernel::ID::id_t itemID, ::Game::Type type, double x, double y, double factor) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(itemID, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  this->_moveToPoint(item, x, y, factor);
  item->unlock();
}

void	Server::Game::_moveToPoint(Server::AItem *item, double x, double y, double factor) {
  Server::Movable *movableBase = this->getMovableBase(item, item->getType());
  if (movableBase->mustGeneratePath()) {
    this->_startMoveItem(item, movableBase, x, y, 1.0, /* WARNING !!!! */
			 Algo::Pathfinding::CLOSEST, NULL, false);
  }
  this->_moveItem(item, factor);
}

void	Server::Game::askMoveItemGroup(Server::GamePlayer *player, const std::vector<Kernel::ID::id_t> &units,
				       bool adapt, double x, double y, double z) {
  Server::GroupMove *groupMove = NULL;
  std::list<Server::AItem *> items;
  std::list<const Server::AItem *> grp;
  Server::AItem *item;
  for (auto it : units) {
    item = this->_getItem(it);
    if (item == NULL) {
      this->sendErrorJob(player, ::Command::OGRPMOVE, ::Game::Controller::Error::UNKNOWN_ITEM);
    }
    else {
      ScopeLock s(this->_mutexes[item->getType()]);
      item = this->_getItem(it, item->getType());
      if (item == NULL || item->lock() == false) {
	this->sendErrorJob(player, ::Command::OGRPMOVE, ::Game::Controller::Error::UNKNOWN_ITEM);
      }
      else {
	items.push_back(item);
	if (adapt) {
	  grp.push_back(item);
	}
      }
    }
  }
  if (adapt) {
    groupMove = Factory::Server::GroupMove::create(grp);
  }
  ::Game::Controller::Error::Type error;
  for (auto it : items) {
    if (::Game::Controller::getInstance()->playerCanMoveItem(player, it, error)) {
      this->createMoveEvent(it, x, y, z, groupMove);
    }
    else {
      this->sendErrorJob(player, ::Command::OGRPMOVE, error);
    }
    it->unlock();
  }
}

void	Server::Game::askPing(Kernel::ID::id_t playerID, double x, double y) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::PING, Server::Event::SERVER_GAME, this);
  event->ping.playerID = playerID;
  event->ping.x = x;
  event->ping.y = y;
  this->addEvent(event);
}

void	Server::Game::askRemoveAmeliorationFromProductionQueue(Server::GamePlayer *player, const Kernel::Serial &item,
							       const Kernel::Serial &amelioration) {
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanRemoveAmeliorationFromProductionQueue(player, item, amelioration, error)) {
    Server::Event *event = Factory::Server::Event::create(Server::Event::REMOVE_AMELIORATION_FROM_PRODUCTION_LIST,
							  Server::Event::SERVER_GAMEPLAYER,
							  player);
    event->ameliorationProductionList.player = player;
    Kernel::assign_serial(event->ameliorationProductionList.type, item);
    Kernel::assign_serial(event->ameliorationProductionList.amelioration, amelioration);
    this->addEvent(event);
  }
  else {
    this->sendErrorJob(player, ::Command::ASKDAMELIO, error);
  }
}

void	Server::Game::askItemsToAttack(Server::GamePlayer *player, const std::vector<Kernel::ID::id_t> &sources,
				       Kernel::ID::id_t target) {
  for (auto it : sources) {
    this->askItemToAttack(player, it, target);
  }
}

void	Server::Game::askItemToAttackZone(Server::GamePlayer *player, Kernel::ID::id_t id,
					  double x, double y, double z) {
  Server::AItem *item = this->_getItem(id);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::ASKOZONEATTACK, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(id, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKOZONEATTACK, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskItemToAttackZone(player, item, error)) {
    Server::CanUseCapacity *base = this->getCanUseCapacityBase(item);
    this->processAskItemUseCapacityZone(player, item, base->getAttackZoneCapacity(), x, y, z);
  }
  else {
    this->sendErrorJob(player, ::Command::ASKOZONEATTACK, error);
  }
  item->unlock();
}

void	Server::Game::askItemsToAttackZone(Server::GamePlayer *player, const std::vector<Kernel::ID::id_t> &items,
					   double x, double y, double z) {
  for (auto it : items) {
    this->askItemToAttackZone(player, it, x, y, z);
  }
}

void	Server::Game::askItemToUseCapacity(Server::GamePlayer *player, Kernel::ID::id_t source,
					   Kernel::ID::id_t target, const Kernel::Serial &serial) {
  Server::AItem *item1 = this->_getItem(source);
  if (item1 == NULL) {
    this->sendErrorJob(player, ::Command::ASKOUSECAP, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item1->getType()]);
    item1 = this->_getItem(source, item1->getType());
    if (item1 == NULL || item1->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKOUSECAP, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  Server::AItem *item2 = this->_getItem(target);
  if (item2 == NULL) {
    this->sendErrorJob(player, ::Command::ASKOUSECAP, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s1(this->_mutexes[item2->getType()]);
    item2 = this->_getItem(target, item2->getType());
    if (item2 == NULL || item2->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKOUSECAP, ::Game::Controller::Error::UNKNOWN_ITEM);
      item2->unlock();
      return ;
    }
  }
  this->processAskItemToUseCapacity(player, item1, item2, serial);
  item1->unlock();
  item2->unlock();
}

void	Server::Game::processAskItemToUseCapacity(Server::GamePlayer *player, Server::AItem *source,
						  const Server::AItem *target, const Kernel::Serial &capacity) {
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskItemToUseCapacity(player, source, capacity, error)) {
    this->_askItemToUseCapacity(source, target, capacity);
  }
  else {
    this->sendErrorJob(player, ::Command::ASKOUSECAP, error);
  }
}

void	Server::Game::_askItemToUseCapacity(Server::AItem *source, const Server::AItem *target,
					    const Kernel::Serial &capacity) {
  Server::Event *event;
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->itemHasAttackRangeOnItem(source, target) &&
      ::Game::Controller::getInstance()->itemHasVisionOnItem(this->_map, source, target)) {
    event = Factory::Server::Event::create(Server::Event::START_USE_CAPACITY,
					   Server::Event::SERVER_GAME, this);
  }
  else if (::Game::Controller::getInstance()->itemCanMove(source, error)) {
    event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_ITEM_AND_START_USE_CAPACITY,
					   Server::Event::SERVER_GAME, this);
  }
  else {
    return ;
  }
  event->useCapacity.item.id = source->getID();
  event->useCapacity.item.type = source->getType();
  event->useCapacity.type = ::Game::Capacity::ITEM;
  Kernel::assign_serial(event->useCapacity.capacity, capacity);
  event->useCapacity.target.id = target->getID();
  event->useCapacity.target.type = target->getType();
  this->addEvent(event);
}

void	Server::Game::itemStartUseCapacityTarget(Kernel::ID::id_t sourceID, ::Game::Type sourceType,
						 const Kernel::Serial &capacity,
						 Kernel::ID::id_t targetID, ::Game::Type targetType) {
  Server::AItem *source;
  Server::AItem *target;
  {
    ScopeLock s(this->_mutexes[sourceType]);
    source = this->_getItem(sourceID, sourceType);
    if (source == NULL || source->lock() == false) {
      return ;
    }
  }
  {
    ScopeLock s(this->_mutexes[targetType]);
    target = this->_getItem(targetID, targetType);
    if (target == NULL || target->lock() == false) {
      source->unlock();
      return ;
    }
  }
  this->processItemStartUseCapacityTarget(source, target, capacity);
  source->unlock();
  target->unlock();
}

Server::Action	*Server::Game::createActionStartUseCapacityTarget(const Server::AItem *source,
								  const Server::AItem *target,
								  const Kernel::Serial &capacity) const {
  Server::Action *action = Factory::Server::Action::create(::Game::ACTION_START_CAPACITY);
  action->useCapacity.item.id = source->getID();
  action->useCapacity.item.type = source->getType();
  action->useCapacity.type = ::Game::Capacity::ITEM;
  action->useCapacity.target.id = target->getID();
  action->useCapacity.target.type = target->getType();
  Kernel::assign_serial(action->useCapacity.capacity, capacity);
  return (action);
}

void	Server::Game::processItemStartUseCapacityTarget(Server::AItem *source, const Server::AItem *target,
							const Kernel::Serial &capacity) {
  if (::Game::Controller::getInstance()->itemHasAttackRangeOnItem(source, target) &&
      ::Game::Controller::getInstance()->itemHasVisionOnItem(this->_map, source, target)) {
    Server::Action *action = this->createActionStartUseCapacityTarget(source, target, capacity);
    source->setAction(action);
    Server::CanUseCapacity *base = this->getCanUseCapacityBase(source);
    base->setTarget(target->getID(), target->getType());
  }
  else {
    this->_askItemToUseCapacity(source, target, capacity);
  }
}


void	Server::Game::itemUseCapacityTarget(Kernel::ID::id_t sourceID, ::Game::Type sourceType,
					    const Kernel::Serial &capacity,
					    Kernel::ID::id_t targetID, ::Game::Type targetType) {
  Server::AItem *source;
  Server::AItem *target;
  {
    ScopeLock s(this->_mutexes[sourceType]);
    source = this->_getItem(sourceID, sourceType);
    if (source == NULL || source->lock() == false) {
      return ;
    }
  }
  {
    ScopeLock s(this->_mutexes[targetType]);
    target = this->_getItem(targetID, targetType);
    if (target == NULL || target->lock() == false) {
      source->unlock();
      return ;
    }
  }
  this->processItemUseCapacityTarget(source, target, capacity);
}

void	Server::Game::processItemUseCapacityTarget(Server::AItem *source, const Server::AItem *target,
						   const Kernel::Serial &capacity) {
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->itemCanUseCapacity(source, capacity, error) &&
      ::Game::Controller::getInstance()->itemHasAttackRangeOnItem(source, target) &&
      ::Game::Controller::getInstance()->itemHasVisionOnItem(this->_map, source, target)) {
    Server::CanUseCapacity *base = this->getCanUseCapacityBase(source);
    base->useCapacityTarget(target, capacity);
  }
}

void	Server::Game::askItemToUseZoneCapacity(Server::GamePlayer *player, Kernel::ID::id_t id,
					       const Kernel::Serial &capacity, double x, double y, double z) {
  Server::AItem *item = this->_getItem(id);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::ASKOUSEZONECAP, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(id, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::ASKOUSEZONECAP, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  this->processAskItemUseCapacityZone(player, item, capacity, x, y, z);
  item->unlock();
}

void	Server::Game::processAskItemUseCapacityZone(Server::GamePlayer *player, Server::AItem *item,
						    const Kernel::Serial &capacity, double x,
						    double y, double z) {
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->playerCanAskItemToUseCapacity(player, item, capacity, error)) {
    this->_askItemToUseZoneCapacity(item, capacity, x, y, z);
  }
  else {
    this->sendErrorJob(player, ::Command::ASKOUSEZONECAP, error);
  }
}

void	Server::Game::_askItemToUseZoneCapacity(Server::AItem *item, const Kernel::Serial &capacity,
						double x, double y, double z) {
  Server::Event *event;
  if (::Game::Controller::getInstance()->itemHasAttackRangeOnPoint(item, x, y, z) &&
      ::Game::Controller::getInstance()->itemHasVisionOnPoint(this->_map, item, x, y, z)) {
    event = Factory::Server::Event::create(Server::Event::START_USE_CAPACITY,
					   Server::Event::SERVER_GAME, this);
  }
  else {
    event = Factory::Server::Event::create(Server::Event::MOVE_TOWARDS_POINT_AND_USE_CAPACITY,
					   Server::Event::SERVER_GAME, this);
  }
  event->useCapacity.item.id = item->getID();
  event->useCapacity.item.type = item->getType();
  event->useCapacity.type = ::Game::Capacity::ZONE;
  Kernel::assign_serial(event->useCapacity.capacity, capacity);
  event->useCapacity.point.x = x;
  event->useCapacity.point.y = y;
  event->useCapacity.point.z = z;
  item->addEvent(event);
}

Server::Action	*Server::Game::createActionStartUseCapacityZone(const Server::AItem *item,
								const Kernel::Serial &capacity,
								double x, double y, double z) const {
  Server::Action *action = Factory::Server::Action::create(::Game::ACTION_START_CAPACITY);
  action->useCapacity.item.id = item->getID();
  action->useCapacity.item.type = item->getType();
  action->useCapacity.type = ::Game::Capacity::ZONE;
  action->useCapacity.point.x = x;
  action->useCapacity.point.y = y;
  action->useCapacity.point.z = z;
  Kernel::assign_serial(action->useCapacity.capacity, capacity);
  return (action);
}

void	Server::Game::itemStartUseCapacityZone(Kernel::ID::id_t id, ::Game::Type type, const Kernel::Serial &capacity,
					       double x, double y, double z) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  if (::Game::Controller::getInstance()->itemHasAttackRangeOnPoint(item, x, y, z) &&
      ::Game::Controller::getInstance()->itemHasVisionOnPoint(this->_map, item, x, y, z)) {
    Server::Action *action = this->createActionStartUseCapacityZone(item, capacity, x, y, z);
    item->setAction(action);
    Server::CanUseCapacity *base = this->getCanUseCapacityBase(item);
    base->setTarget(x, y, z);
  }
  else {
    this->_askItemToUseZoneCapacity(item, capacity, x, y, z);
  }
  item->unlock();
}

void	Server::Game::itemUseCapacityZone(Kernel::ID::id_t id, ::Game::Type type, const Kernel::Serial &capacity,
					  double x, double y, double z) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->itemCanUseCapacity(item, capacity, error) &&
      ::Game::Controller::getInstance()->itemHasAttackRangeOnPoint(item, x, y, z) &&
      ::Game::Controller::getInstance()->itemHasVisionOnPoint(this->_map, item, x, y, z)) {
    Server::CanUseCapacity *base = this->getCanUseCapacityBase(item);
    base->useCapacityZone(capacity, x, y, z);
  }
  item->unlock();
}

void	Server::Game::askItemToUseCapacity(Server::GamePlayer *player, Kernel::ID::id_t id,
					   const Kernel::Serial &capacity) {
  Server::AItem *item;
  item = this->_getItem(id);
  if (item == NULL) {
    this->sendErrorJob(player, ::Command::OASKACTCAPA, ::Game::Controller::Error::UNKNOWN_ITEM);
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(id, item->getType());
    if (item == NULL || item->lock() == false) {
      this->sendErrorJob(player, ::Command::OASKACTCAPA, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
  }
  ::Game::Controller::Error::Type error;
  if (::Game::Controller::getInstance()->itemCanUseCapacity(item, capacity, error)) {
    this->_askItemToUseCapacity(item, capacity);
  }
  else {
    this->sendErrorJob(player, ::Command::OASKACTCAPA, error);
  }
  item->unlock();
}

void	Server::Game::_askItemToUseCapacity(Server::AItem *item, const Kernel::Serial &capacity) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::START_USE_CAPACITY,
							Server::Event::SERVER_GAME, this);
  event->useCapacity.item.id = item->getID();
  event->useCapacity.item.type = item->getType();
  Kernel::assign_serial(event->useCapacity.capacity, capacity);
  event->useCapacity.type = ::Game::Capacity::NONE;
  item->addEvent(event);
}

void	Server::Game::itemStartUseCapacity(Kernel::ID::id_t id, ::Game::Type type, const Kernel::Serial &capacity) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  Server::Action *action = Factory::Server::Action::create(::Game::ACTION_START_CAPACITY);
  action->useCapacity.item.id = item->getID();
  action->useCapacity.item.type = item->getType();
  action->useCapacity.type = ::Game::Capacity::NONE;
  Kernel::assign_serial(action->useCapacity.capacity, capacity);
  item->setAction(action);
  Server::CanUseCapacity *base = this->getCanUseCapacityBase(item);
  base->setTarget(item->getID(), item->getType());
  item->unlock();
}

void	Server::Game::itemUseCapacity(Kernel::ID::id_t id, ::Game::Type type, const Kernel::Serial &capacity) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  Server::CanUseCapacity *base = this->getCanUseCapacityBase(item);
  base->useCapacity(capacity);
  item->unlock();
}

void	Server::Game::moveTowardsItemAndStartUseCapacity(Kernel::ID::id_t sourceID, ::Game::Type sourceType,
							 const Kernel::Serial &capacity,
							 Kernel::ID::id_t targetID, ::Game::Type targetType) {
  Server::AItem *source;
  Server::AItem *target;
  {
    ScopeLock s(this->_mutexes[sourceType]);
    source = this->_getItem(sourceID, sourceType);
    if (source == NULL || source->lock() == false) {
      return ;
    }
  }
  {
    ScopeLock s(this->_mutexes[targetType]);
    target = this->_getItem(targetID, targetType);
    if (target == NULL || target->lock() == false) {
      source->unlock();
      return ;
    }
  }
  this->processMoveTowardsItemAndStartUseCapacity(source, target, capacity);
  source->unlock();
  target->unlock();
}

void	Server::Game::processMoveTowardsItemAndStartUseCapacity(Server::AItem *source, const Server::AItem *target,
								const Kernel::Serial &capacity) {
  Server::Action *next = this->createActionStartUseCapacityTarget(source, target, capacity);
  Vector3d vd = target->getPosition() - source->getPosition();
  double length = sqrt(vd.sqLength());
  vd /= (source->getAttackRange() / length);
  Server::Movable *base = this->getMovableBase(source, source->getType());
  this->_startMoveItem(source, base, source->getX() + vd.x, source->getY() + vd.y,
		       source->getZ() + vd.z, Algo::Pathfinding::CLOSEST, next);
  base->setStopCondition(Server::Movable::ATTACK_RANGE | Server::Movable::VISION);
}

void	Server::Game::moveTowardsPointAndUseCapacity(Kernel::ID::id_t id, ::Game::Type type,
						     const Kernel::Serial &capacity, double x, double y, double z) {
  Server::AItem *source;
  {
    ScopeLock s(this->_mutexes[type]);
    source = this->_getItem(id, type);
    if (source == NULL || source->lock() == false) {
      return ;
    }
  }
  Server::Action *next = this->createActionStartUseCapacityZone(source, capacity, x, y, z);
  Vector3d vd = Vector3d(x, y, z) - source->getPosition();
  Server::Movable *base = this->getMovableBase(source, source->getType());
  this->_startMoveItem(source, base, vd.x, vd.y, vd.z, Algo::Pathfinding::CLOSEST, next);
  base->setStopCondition(Server::Movable::ATTACK_RANGE);
  source->unlock();
}

  void	Server::Game::askAutomaticCapacity(Server::GamePlayer *player, Kernel::ID::id_t id,
					   const Kernel::Serial &capacity, bool automatic) {
    Server::AItem *item = this->_getItem(id);
    if (item == NULL) {
      this->sendErrorJob(player, ::Command::OASKAUTOCAPA, ::Game::Controller::Error::UNKNOWN_ITEM);
      return ;
    }
    {
      ScopeLock s(this->_mutexes[item->getType()]);
      item = this->_getItem(id, item->getType());
      if (item == NULL || item->lock() == false) {
	this->sendErrorJob(player, ::Command::OASKAUTOCAPA, ::Game::Controller::Error::UNKNOWN_ITEM);
	return ;
      }
    }
    ::Game::Controller::Error::Type error;
    if (::Game::Controller::getInstance()->playerCanAskItemToSetCapacityAutomatic(player, item, capacity, error)) {
      Server::Event *event = Factory::Server::Event::create(Server::Event::SET_CAPACITY_AUTOMATIC,
							    Server::Event::SERVER_GAME, this);
      event->setAutomatic.item.id = item->getID();
      event->setAutomatic.item.type = item->getType();
      Kernel::assign_serial(event->setAutomatic.capacity, capacity);
      event->setAutomatic.automatic = automatic;
      item->addEvent(event);
    }
    else {
      this->sendErrorJob(player, ::Command::OASKAUTOCAPA, error);
    }
    item->unlock();
  }

void	Server::Game::setCapacityAutomatic(Kernel::ID::id_t id, ::Game::Type type, const Kernel::Serial &capacity,
					   bool automatic) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  item->setCapacityAutomatic(capacity, automatic);
  item->unlock();
}

void	Server::Game::addXP(Kernel::ID::id_t id, unsigned int amount) {
  Server::AItem *item = this->_getItem(id);
  if (item == NULL) {
    return ;
  }
  {
    ScopeLock s(this->_mutexes[item->getType()]);
    item = this->_getItem(id, item->getType());
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  if (item->getType() == ::Game::UNIT || item->getType() == ::Game::HERO) {
    static_cast<Server::Unit *>(item)->addXP(amount);
  }
  item->unlock();
}

void	Server::Game::statsChanged(Kernel::ID::id_t id, ::Game::Type type) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  this->_map->refreshUnitVisionAndRange(item);
  item->unlock();
}

void	Server::Game::addSpectatorJob(Kernel::ID::Type type, Kernel::ID::id_t id, Protocol::Job *job) {
  this->_spectator->addJob(type, id, job);
}

void	Server::Game::playerHasLost(Server::GamePlayer *player) {
  ScopeLock s(this->_teamMutex);
  this->_teams[const_cast< ::Game::Team *>(player->getTeam())]--;
}

void	Server::Game::userReconnected(Server::User *user) {
  LOCK_PLAYER;
  Server::GamePlayer *player = NULL;
  for (auto it : this->_players) {
    if (it.second->getUser() == user) {
      player = it.second;
      break;
    }
  }
  if (player == NULL) {
    std::string s = __PRETTY_FUNCTION__;
    Logger::getInstance()->logf("Cannot find user in %s", Logger::FATAL, &s);
    return ;
  }
  const std::vector<Protocol::Job *> job = this->_spectator->dump(player->getID());
  player->addReconnectJobs(job);
  for (auto it : job) {
    player->addJob(it);
  }
  player->inGame();
  Protocol::Job *j = Factory::Protocol::Job::create();
  j->gameLaunch();
  player->addJob(j);
}

void	Server::Game::addSpectator(Server::User *user) {
  const std::vector<Protocol::Job *> job = this->_spectator->dump();
  for (auto it : job) {
    Network::Manager::Server::getInstance()->push(user->getSocket(), it);
  }
  Protocol::Job *j = Factory::Protocol::Job::create();
  j->gameLaunch();
  Network::Manager::Server::getInstance()->push(user->getSocket(), j);
  this->addUser(user);
}

void	Server::Game::askCreateBuilding(Server::GamePlayer *player, const Kernel::Serial &building,
					double x, double y, double z, int orientation) {
  ::Game::Controller::Error::Type error;
  const Kernel::Cost *cost = Kernel::Manager::getInstance()->getCost(building);
  if (::Game::Controller::getInstance()->playerCanCreateBuilding(player, building, error) &&
      player->lockRessources(cost->ressources)) {
    player->spendRessources(cost->ressources);
    Server::Event *event = Factory::Server::Event::create(Server::Event::CREATE_ITEM,
							  Server::Event::SERVER_GAME, this);
    Kernel::assign_serial(event->create.serial, building);
    event->create.player = player;
    event->create.x = x;
    event->create.y = y;
    event->create.z = z;
    event->create.o = orientation;
    this->addEvent(event);
  }
  else {
    this->sendErrorJob(player, Command::ASKCBUILDING, error);
  }
}

void	Server::Game::endItemAction(Kernel::ID::id_t id, ::Game::Type type) {
  Server::AItem *item;
  {
    ScopeLock s(this->_mutexes[type]);
    item = this->_getItem(id, type);
    if (item == NULL || item->lock() == false) {
      return ;
    }
  }
  item->endAction();
  item->unlock();
}

void	Server::Game::playerHasProducedAmelioration(Kernel::ID::id_t id, const Kernel::Serial &type,
						    const Kernel::Serial &amelioration) {
  Server::GamePlayer *player = NULL;
  {
    LOCK_PLAYER;
    player = this->_players.find(id) != this->_players.end() ? this->_players[id] : NULL;
    if (player == NULL || player->lock() == false) {
      return ;
    }
  }
  player->addAmelioration(type, amelioration);
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->playerHasJustProducedAmelioration(id,
					 type,
					 amelioration);
  this->addJob(job);
  this->addSpectatorJob(Kernel::ID::PLAYER, id, job);
  job = Factory::Protocol::Job::create();
  job->playerHasAmelioration(id, type, amelioration);
  this->addJob(job);
  this->addSpectatorJob(Kernel::ID::PLAYER, id, job);
  player->unlock();
}
