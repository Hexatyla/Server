//
// Server.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 15:18:58 2014 geoffrey bauduin
// Last update Tue Mar 11 17:14:09 2014 geoffrey bauduin
//

#include	"Factory/Server.hpp"
#include	"Factory/Pool.hpp"

static Pool< ::Server::Game>	*poolGame = NULL;
static Pool< ::Server::Event>	*poolEvent = NULL;
static Pool< ::Server::Building> *poolBuilding = NULL;
static Pool< ::Server::Unit> *poolUnit = NULL;
static Pool< ::Server::Projectile> *poolProjectile = NULL;
static Pool< ::Server::Hero> *poolHero = NULL;
static Pool< ::Server::GamePlayer> *poolGamePlayer = NULL;
static Pool< ::Server::Object> *poolObject = NULL;
static Pool< ::Server::RessourcesSpot> *poolRessourcesSpot = NULL;
static Pool< ::Server::Effect> *poolEffect = NULL;
static Pool< ::Server::Capacity> *poolCapacity = NULL;
static Pool< ::Server::Map> *poolMap = NULL;
static Pool< ::Server::Action> *poolAction = NULL;
static Pool< ::Server::GameLoader> *poolGameLoader = NULL;
static Pool< ::Server::GroupMove> *poolGroupMove = NULL;
static Pool< ::Server::Spectator> *poolSpectator = NULL;
static Pool< ::Server::Spectator::ItemData> *poolSpectatorItemData = NULL;
static Pool< ::Server::Spectator::PlayerData> *poolSpectatorPlayerData = NULL;
static Pool< ::Server::Race> *poolRace = NULL;

void	Factory::Server::init(void) {
  Logger::getInstance()->log("Loading server specific items pool ...", Logger::INFORMATION);
  poolGame = new Pool< ::Server::Game>(20, 10, "Server::Game");
  poolEvent = new Pool< ::Server::Event>(500, 200, "Server::Event");
  poolBuilding = new Pool< ::Server::Building>(1500, 300, "Server::Building");
  poolUnit = new Pool< ::Server::Unit>(3500, 500, "Server::Unit");
  poolHero = new Pool< ::Server::Hero>(200, 50, "Server::Hero");
  poolProjectile = new Pool< ::Server::Projectile>(3500, 500, "Server::Projectile");
  poolGamePlayer = new Pool< ::Server::GamePlayer>(300, 100, "Server::GamePlayer");
  poolObject = new Pool< ::Server::Object>(300, 100, "Server::Object");
  poolRessourcesSpot = new Pool< ::Server::RessourcesSpot>(750, 250, "Server::RessourcesSpot");
  poolEffect = new Pool< ::Server::Effect>(1000, 250, "Server::Effect");
  poolCapacity = new Pool< ::Server::Capacity>(500, 175, "Server::Capacity");
  poolMap = new Pool< ::Server::Map>(300, 150, "Server::Map");
  poolAction = new Pool< ::Server::Action>(500, 350, "Server::Action");
  poolGameLoader = new Pool< ::Server::GameLoader>(20, 10, "Server::GameLoader");
  poolGroupMove = new Pool< ::Server::GroupMove>(300, 100, "Server::GroupMove");
  poolSpectator = new Pool< ::Server::Spectator>(20, 10, "Server::Spectator");
  poolSpectatorItemData = new Pool< ::Server::Spectator::ItemData>(5000, 1000, "Server::Spectator::ItemData");
  poolSpectatorPlayerData = new Pool< ::Server::Spectator::PlayerData>(300, 200, "Server::Spectator::PlayerData");
  poolRace = new Pool< ::Server::Race>(100, 10, "Server::Race");
  Logger::getInstance()->log("Server specific items pool initialized.", Logger::INFORMATION);
}

void	Factory::Server::end(void) {
  delete poolGame;
  delete poolEvent;
  delete poolBuilding;
  delete poolUnit;
  delete poolHero;
  delete poolProjectile;
  delete poolGamePlayer;
  delete poolObject;
  delete poolRessourcesSpot;
  delete poolEffect;
  delete poolCapacity;
  delete poolMap;
  delete poolAction;
  delete poolGameLoader;
  delete poolGroupMove;
  delete poolSpectator;
  delete poolSpectatorItemData;
  delete poolSpectatorPlayerData;
  delete poolRace;
}

::Server::Game	*Factory::Server::Game::create(::Kernel::ID::id_t id,
					       const std::vector< ::Server::GamePlayer *> &players,
					       const ::Server::HasUsers::Container &users,
					       const std::vector< ::Game::Team *> &teams, const std::string &map,
					       ::Server::GameLoader *gl) {
  ::Server::Game *item = poolGame->get();
  item->init(id, players, users, teams, map, gl);
  return (item);
}

void	Factory::Server::Game::remove(::Server::Game *game) {
  game->destroy();
  poolGame->push(game);
}

::Server::Event	*Factory::Server::Event::create(::Server::Event::Type type, ::Server::Event::objectType objType, void *ptr) {
  ::Server::Event *item = poolEvent->get();
  item->init(type, objType, ptr);
  return (item);
}

void	Factory::Server::Event::remove(::Server::Event *event) {
  event->destroy();
  poolEvent->push(event);
}

::Server::Building	*Factory::Server::Building::create(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
							   int orientation, double x, double y, double z) {
  ::Server::Building *item = poolBuilding->get();
  item->init(id, serial, player, orientation, x, y, z);
  return (item);
}

void	Factory::Server::Building::remove(::Server::Building *item) {
  item->destroy();
  poolBuilding->push(item);
}


::Server::Unit	*Factory::Server::Unit::create(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
							   int orientation, double x, double y, double z) {
  ::Server::Unit *item = poolUnit->get();
  item->init(id, serial, player, orientation, x, y, z);
  return (item);
}

void	Factory::Server::Unit::remove(::Server::Unit *item) {
  item->destroy();
  poolUnit->push(item);
}


::Server::Hero	*Factory::Server::Hero::create(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
							   int orientation, double x, double y, double z) {
  ::Server::Hero *item = poolHero->get();
  item->init(id, serial, player, orientation, x, y, z);
  return (item);
}

void	Factory::Server::Hero::remove(::Server::Hero *item) {
  item->destroy();
  poolHero->push(item);
}

::Server::Projectile	*Factory::Server::Projectile::create(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
							   int orientation, double x, double y, double z) {
  ::Server::Projectile *item = poolProjectile->get();
  item->init(id, serial, player, orientation, x, y, z);
  return (item);
}

void	Factory::Server::Projectile::remove(::Server::Projectile *item) {
  item->destroy();
  poolProjectile->push(item);
}

::Server::GamePlayer	*Factory::Server::GamePlayer::create(::Server::User *user, Kernel::ID::id_t id, const ::Game::Race *race, const ::Game::Team *team) {
  ::Server::GamePlayer *item = poolGamePlayer->get();
  item->init(user, id, race, team);
  return (item);
}

void	Factory::Server::GamePlayer::remove(::Server::GamePlayer *item) {
  item->destroy();
  poolGamePlayer->push(item);
}

::Server::Object	*Factory::Server::Object::create(Kernel::ID::id_t id, const Kernel::Serial &serial,
							 int orientation, double x, double y, double z) {
  ::Server::Object *item = poolObject->get();
  item->init(id, serial, orientation, x, y, z);
  return (item);
}

void	Factory::Server::Object::remove(::Server::Object *item) {
  item->destroy();
  poolObject->push(item);
}

::Server::RessourcesSpot	*Factory::Server::RessourcesSpot::create(Kernel::ID::id_t id, double x, double y, double z, ::Game::Ressources::Type type, unsigned int amount) {
  ::Server::RessourcesSpot *item = poolRessourcesSpot->get();
  item->init(id, x, y, z, type, amount);
  return (item);
}

void	Factory::Server::RessourcesSpot::remove(::Server::RessourcesSpot *item) {
  item->destroy();
  poolRessourcesSpot->push(item);
}

::Server::Effect	*Factory::Server::Effect::create(const Kernel::Serial &serial, const ::Game::Player *player, Kernel::ID::id_t itemID) {
  ::Server::Effect *effect = poolEffect->get();
  effect->init(serial, player, itemID);
  return (effect);
}

void	Factory::Server::Effect::remove(::Server::Effect *effect) {
  effect->destroy();
  poolEffect->push(effect);
}

::Server::Capacity	*Factory::Server::Capacity::create(const Kernel::Serial &serial, bool isAuto, const ::Game::Player *player, Kernel::ID::id_t itemID) {
  ::Server::Capacity *capacity = poolCapacity->get();
  capacity->init(serial, isAuto, player, itemID);
  return (capacity);
}

void	Factory::Server::Capacity::remove(::Server::Capacity *capacity) {
  capacity->destroy();
  poolCapacity->push(capacity);
}

::Server::Map	*Factory::Server::Map::create(unsigned int x, unsigned int y, const std::vector<unsigned int> &z, const std::vector< ::Server::Map::Spot> &spots, const std::vector<std::pair<double, double> > &starters) {
  ::Server::Map *item = poolMap->get();
  item->init(x, y, z, spots, starters);
  return (item);
}

void	Factory::Server::Map::remove(::Server::Map *map) {
  map->destroy();
  poolMap->push(map);
}

::Server::Action	*Factory::Server::Action::create(::Game::eAction action) {
  ::Server::Action *item = poolAction->get();
  item->init(action);
  return (item);
}

void	Factory::Server::Action::remove(::Server::Action *action) {
  action->destroy();
  poolAction->push(action);
}

::Server::GameLoader	*Factory::Server::GameLoader::create(const std::list< ::Server::User *>& u) {
  ::Server::GameLoader *gl = poolGameLoader->get();
  gl->init(u);
  return (gl);
}

void	Factory::Server::GameLoader::remove(::Server::GameLoader *gl) {
  gl->destroy();
  poolGameLoader->push(gl);
}

::Server::GroupMove	*Factory::Server::GroupMove::create(const std::list<const ::Server::AItem *>& u) {
  ::Server::GroupMove *item = poolGroupMove->get();
  item->init(u);
  return (item);
}

void	Factory::Server::GroupMove::remove(::Server::GroupMove *item) {
  item->destroy();
  poolGroupMove->push(item);
}

::Server::Spectator	*Factory::Server::Spectator::create(const Clock *clock) {
  ::Server::Spectator *item = poolSpectator->get();
  item->init(clock);
  return (item);
}

void	Factory::Server::Spectator::remove(::Server::Spectator *item) {
  item->destroy();
  poolSpectator->push(item);
}

::Server::Spectator::ItemData	*Factory::Server::Spectator::ItemData::create(void) {
  ::Server::Spectator::ItemData *item = poolSpectatorItemData->get();
  item->init();
  return (item);
}

void	Factory::Server::Spectator::ItemData::remove(::Server::Spectator::ItemData *item) {
  item->destroy();
  poolSpectatorItemData->push(item);
}

::Server::Spectator::PlayerData	*Factory::Server::Spectator::PlayerData::create(void) {
  ::Server::Spectator::PlayerData *item = poolSpectatorPlayerData->get();
  item->init();
  return (item);
}

void	Factory::Server::Spectator::PlayerData::remove(::Server::Spectator::PlayerData *item) {
  item->destroy();
  poolSpectatorPlayerData->push(item);
}

::Server::Race	*Factory::Server::Race::create(const std::string &name, const Kernel::Serial &serial) {
  ::Server::Race *item = poolRace->get();
  item->init(name, serial);
  return (item);
}

void	Factory::Server::Race::remove(::Server::Race *item) {
  item->destroy();
  poolRace->push(item);
}
