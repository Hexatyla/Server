//
// Core.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sat Jan 18 15:57:15 2014 geoffrey bauduin
// Last update Wed Mar 26 15:35:56 2014 geoffrey bauduin
//

#include	<unistd.h>
#include	<iostream>
#include	<cstddef>
#include	<algorithm>
#include	<stdlib.h>
#include	<time.h>
#include	"Factory/Factory.hpp"
#include	"Factory/Server.hpp"
#include	"Factory/Network.hpp"
#include	"Factory/Protocol.hpp"
#include	"Server/Core.hpp"
#include	"Kernel/ID.hpp"
#include	"Logger.hpp"
#include	"Parser/JSON.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Algo/MD5.hpp"
#include	"Kernel/Manager.hpp"
#include	"MySQL/DBList.hpp"
#include	"Parser/SQL.hpp"
#include	"Factory/Protocol.hpp"
#include	"Server/User.hpp"
#include	"Server/MatchMaking.hpp"
#include	"Room/MapList.hpp"
#include	"FileLoader.hpp"
#include	"Factory/Game.hpp"
#include	"Kernel/Config.hpp"
#include	"Protocol/JobResolver.hpp"

Server::Core::Core(void):
  Server::HasEvent(), Server::HasUsers(),
  _thPool(NULL), _cond(new CondVar),
  _clock(NULL), _games(), _gMutex(new Mutex), _end(false), _parserRace(new Parser::Race),
  _threadNetwork(new Thread),
  _jMutex(new Mutex), _jobs(), _ingamePlayers(), _mOG(),
  _threadsStatus(), _stTh(new Mutex) {
  (void) Kernel::ID::getInstance();
  (void) Parser::JSON::getInstance();
  (void) Kernel::Manager::getInstance();
  (void) Algo::MD5::getInstance();
  (void) MySQL::DBList::getInstance();
  (void) ::Game::Controller::getInstance();
  (void) Room::MapList::getInstance();
  (void) FileLoader::getInstance();
  (void) Kernel::Config::getInstance();
  (void) Protocol::JobResolver::getInstance();
  (void) Network::Manager::Server::getInstance();
}

Server::Core::~Core(void) {
  Logger::getInstance()->log("Shutting down server ...", Logger::INFORMATION);
  for (unsigned int i = 0 ; i < this->_thPool->getNumberOfThreads() ; ++i) {
    this->_cond->signal();
  }
  if (this->_clock) {
    Factory::Clock::remove(this->_clock);
  }
  for (auto it : this->_games) {
    Factory::Server::Game::remove(it.second.game);
    Factory::Clock::remove(it.second.clock);
  }
  Logger::getInstance()->log("Deleting Singleton objects ...", Logger::INFORMATION);
  Kernel::ID::deleteInstance();
  Parser::JSON::deleteInstance();
  Algo::MD5::deleteInstance();
  Kernel::Manager::deleteInstance();
  MySQL::DBList::deleteInstance();
  {
    const ::Game::Controller::Container races = ::Game::Controller::getInstance()->flushRaces();
    for (auto it : races) {
      Factory::Server::Race::remove(static_cast<Server::Race *>(it.second));
    }
    ::Game::Controller::deleteInstance();
  }
  Room::MapList::deleteInstance();
  Kernel::Config::deleteInstance();
  Protocol::JobResolver::deleteInstance();
  FileLoader::deleteInstance();
  Logger::getInstance()->log("Destroying Factory ...", Logger::INFORMATION);
  delete this->_thPool;
  delete this->_threadNetwork;
  Logger::getInstance()->log("Threads destroyed", Logger::INFORMATION);
  delete this->_cond;
  delete this->_jMutex;
  delete this->_stTh;
  delete this->_parserRace;
  delete this->_gMutex;
  Network::Manager::Server::deleteInstance();
  for (auto user : this->_users) {
    delete (user);
  }
  Logger::getInstance()->log("Users destroyed", Logger::INFORMATION);
  Factory::Server::end();
  Factory::end();
  Logger::getInstance()->log("Factory destroyed", Logger::INFORMATION);
}

static void	*start_thread(void *param) {
  Server::Core *core = reinterpret_cast<Server::Core *>(param);
  core->runThread();
  return (NULL);
}

bool	Server::Core::initSQL(void) {
  Parser::SQL		infos;
  if (!infos.parse("config/mysql.json")) {
    Logger::getInstance()->log("Database file does not exist", Logger::FATAL);
    return (false);
  }
  if (MySQL::DBList::getInstance()->get(MySQL::DBList::MAIN)->connect("127.0.0.1", infos.getUsername(), infos.getPassword(), "hexatyla_db") == false) {
    Logger::getInstance()->log("Cannot connect to MySQL database", Logger::FATAL);
    return (false);
  }
  Logger::getInstance()->log("Connected to MySQL database", Logger::INFORMATION);
  return (true);
}

void	Server::Core::destroy(void) {
  Server::HasEvent::destroy();
  Server::HasUsers::destroy();
}

bool	Server::Core::init(int, char **av) {
  int port = atoi(av[1]);
  bool	success = true;
  srand(time(NULL));

  Factory::init();
  Factory::Server::init();

  Logger::getInstance()->log("Initializing Server::Core ...", Logger::INFORMATION);
  Logger::getInstance()->addDecalage();

  Kernel::Manager::getInstance()->init();

  if (!(this->initSQL())) {
    success = false;
    this->_end = true;
  }
  else {
    Server::User::load(this->_users);
  }
  Room::MapList::getInstance()->init();

  this->_clock = Factory::Clock::create();
  this->_clock->init();
  Server::HasEvent::init();
  Server::HasUsers::init();

  if (success != false && Network::Manager::Server::getInstance()->init(port)) {
    _threadNetwork->start(&start_network_manager, Network::Manager::Server::getInstance());
    Logger::getInstance()->log("Network initialized ...", Logger::INFORMATION);
  }
  else {
    this->_end = true;
    success = false;
  }

  this->_thPool = new ThreadPool(DEFINE_FOR_ME);
  //this->_thPool = new ThreadPool(1);
  this->_thPool->start(&start_thread, this);
  Logger::getInstance()->log("Threads started", Logger::INFORMATION);

  Logger::getInstance()->log("Loading default races ...", Logger::INFORMATION);
  std::vector<std::string> races;
  FileLoader::getInstance()->loadFileByExtension("./races/", ".hxtl", races);
  Logger::getInstance()->addDecalage();
  for (auto it : races) {
    if (!this->loadRace(it)) {
      Logger::getInstance()->logf("Cannot load %s", Logger::FATAL, &it);
    }
  }
  Logger::getInstance()->removeDecalage(2);
  Logger::getInstance()->log("Server::Core initialized.", Logger::INFORMATION);
  return (success);
}

bool	Server::Core::loadRace(const std::string &file) {
  Parser::Race::Container c = this->_parserRace->parse(file);
  bool ret = false;
  for (auto it : c) {
    if (it.second) {
      it.second->dump();
      ::Game::Controller::getInstance()->addRace(file, it.second);
      ret = true;
    }
  }
  return (ret);
}

void	Server::Core::createErrorJob(Network::UserJob *uj, ::Game::Controller::Error::Type error) {
  this->createErrorJob(uj, ::Game::Controller::getInstance()->translate(error));
}

void	Server::Core::createErrorJob(Network::UserJob *uj, Error::Code code) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->fail(uj->job->getCommand(), code);
  this->addJob(uj->socket, job);
}

void	Server::Core::sendErrorJob(Network::UserJob *uj, Error::Code code) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->fail(uj->job->getCommand(), code);
  Network::Manager::Server::getInstance()->push(uj->socket, job);
}

void	Server::Core::confirmJob(Network::UserJob *uj) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->success(uj->job->getCommand());
  Network::Manager::Server::getInstance()->push(uj->socket, job);
}

Server::Group*	Server::Core::findGroup(const Server::User* u) {
  for (auto it : _groups) {
    if (it->isInGroup(u)) {
      return (it);
    }
  }
  return (NULL);
}

Room::ARoom*	Server::Core::findRoom(const Server::User* u) {
  for (auto it : _rooms) {
    if (it->isUserInRoom(u)) {
      return (it);
    }
  }
  return (NULL);
}

Server::GameLoader*	Server::Core::findGameLoader(const Server::User *u) {
  for (auto it : _gameLoaders) {
    if (it->isUserInGameLoader(u)) {
      return (it);
    }
  }
  for (auto it : _games) {
    if (it.second.game->getLoader() && it.second.game->getLoader()->isUserInGameLoader(u)) {
      return (it.second.game->getLoader());
    }
  }
  return (NULL);
}

Server::Game*		Server::Core::findGameByGameLoader(const Server::GameLoader* u) {
  for (auto it : _games) {
    if (it.second.game->getLoader() == u) {
      return (it.second.game);
    }
  }
  return (NULL);
}

Server::GameLoader*	Server::Core::findGameLoaderSpectator(const Server::User *u) {
  for (auto it : _gameLoaders) {
    if (it->isSpectatorInGameLoader(u)) {
      return (it);
    }
  }
  for (auto it : _games) {
    if (it.second.game->getLoader() && it.second.game->getLoader()->isSpectatorInGameLoader(u)) {
      return (it.second.game->getLoader());
    }
  }
  return (NULL);
}

void	Server::Core::registration(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  const Protocol::Argument *args = uj->job->getArguments();
  Error::Code error = Server::User::registration(args[0].pseudo, args[1].mail, args[2].password, this->_users);
  if (error != Error::NONE) {
    this->sendErrorJob(uj, error);
  }
  else {
    this->confirmJob(uj);
  }
}

void	Server::Core::connection(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  const Protocol::Argument *args = uj->job->getArguments();
  Server::User *user = this->findUser(uj->socket);

  if (user) {
    this->sendErrorJob(uj, Error::ALREADYCONNECTED);
    return ;
  }

  Error::Code error = Server::User::connection(args[0].pseudo, args[1].password, this->_users, uj->socket);
  if (error != Error::NONE) {
    this->sendErrorJob(uj, error);
    return ;
  }
  user = this->findUser(uj->socket);
  if (!user) {
    this->sendErrorJob(uj, Error::NOSUCHNICK);
    return ;
  }
  Server::GameLoader *gl = this->findGameLoader(user);
  if (gl) {
    gl->playerReconnected(user);
    user->changeStatus(::User::BUSY);
  }
}

void	Server::Core::playerSetStatus(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }

  const Protocol::Argument *args = uj->job->getArguments();
  user->changeStatus(args[0].userStatus);
}

void	Server::Core::playerSetRace(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }

  if (this->findRoom(user)) {
    this->sendErrorJob(uj, Error::CURRENTLYWAITING);
    return ;
  }

  const Protocol::Argument *args = uj->job->getArguments();
  user->setRace(args[0].race);

  Server::Group *group = this->findGroup(user);
  if (group) {
    for (auto it : group->getUsers()) {
      Protocol::Job *job = Factory::Protocol::Job::create();
      job->groupMemberChangedRace(user->getNick(), user->getRace());
      Network::Manager::Server::getInstance()->push(it->getSocket(), job);
    }
  }
}

void	Server::Core::disconnect(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);

  if (!user) { return ; }

  /* alerte les amis de la deconnexion */
  user->changeStatus(::User::DISCONNECTED);

  /* delete la socket */
  Factory::Network::SSocket::remove(uj->socket);
  user->setSocket(NULL);

  if (user->isSpectating()) {
    Server::Game *game = user->getGameSpectating();
    game->removeUser(user);
    user->spectating(NULL);
    user->isSpectating(false);
  }

  Server::Group *group = this->findGroup(user);
  Room::ARoom *room = this->findRoom(user);
  Server::GameLoader *gl = this->findGameLoader(user);

  /* arrete la recherche de partie */
  if (room) {
    room->kickPlayer(user);
    if (group) { // retire le groupe du matchmaking / leur envoie une alerte
      for (auto it : group->getUsers()) {
	if (it != user) {
	  Protocol::Job *job = Factory::Protocol::Job::create();
	  job->endGameSearching();
	  Network::Manager::Server::getInstance()->push(it->getSocket(), job);
	  room->kickPlayer(it);
	}
      }
    }
    if (room->isEmpty()) {
      this->_rooms.remove(room);
      delete (room);
      room = NULL;
    }
  }

  /* si le joueur était en game, ne retire pas du groupe */
  /* remove from group */
  if (group && !gl) {
    if (group->getLeader() == user) { // si l'user est chef de group -> delete le groupe
      group->removeUser(user);
      if (group->getUsers().empty() == false) {
	for (auto it : group->getUsers()) {
	  Protocol::Job * job = Factory::Protocol::Job::create();
	  job->groupDeleted();
	  Network::Manager::Server::getInstance()->push(it->getSocket(), job);
	}
      }
      this->_groups.remove(group);
      delete (group);
      group = NULL;
    }
    else { // si pas chef de groupe -> remove du group et delete le groupe si empty ou 1 joueur
      group->removeUser(user);
      if (group->getUsers().empty() == false) {
	for (auto it : group->getUsers()) {
	  Protocol::Job *job = Factory::Protocol::Job::create();
	  job->playerLeftGroup(user->getNick());
	  Network::Manager::Server::getInstance()->push(it->getSocket(), job);
	}
      }
      if (group->getUsers().empty() == true || group->getUsers().size() <= 1) {
	if (!group->getUsers().empty()) {
	  for (auto it : group->getUsers()) {
	    Protocol::Job *job = Factory::Protocol::Job::create();
	    job->groupDeleted();
	    Network::Manager::Server::getInstance()->push(it->getSocket(), job);
	  }
	}
	this->_groups.remove(group);
	delete (group);
	group = NULL;
      }
    }
  }
}

void	Server::Core::friendRequest(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);
  const Protocol::Argument *args = uj->job->getArguments();
  Server::User *target = this->findUser(args[0].pseudo);

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  } else if (!target) {
    this->sendErrorJob(uj, Error::NOSUCHNICK);
    return ;
  }

  if (user == target) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  Error::Code error = user->addFriend(target);
  if (error != Error::NONE) {
    this->sendErrorJob(uj, error);
  }
}

void	Server::Core::removeFriend(Network::UserJob *uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);
  const Protocol::Argument *args = uj->job->getArguments();
  Server::User *target = this->findUser(args[0].pseudo);

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  } else if (!target) {
    this->sendErrorJob(uj, Error::NOSUCHNICK);
    return ;
  }

  if (user == target) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  Error::Code error = user->removeFriend(target);
  if (error != Error::NONE) {
    this->sendErrorJob(uj, error);
  }
}

void	Server::Core::answerFriendRequest(Network::UserJob *uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);
  const Protocol::Argument *args = uj->job->getArguments();
  Server::User *target = this->findUser(args[0].pseudo);

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  } else if (!target) {
    this->sendErrorJob(uj, Error::NOSUCHNICK);
    return ;
  }

  if (user == target) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  Error::Code error = user->answerFriendRequest(target, args[1].yes);
  if (error != Error::NONE) {
    this->sendErrorJob(uj, error);
  }
}

void	Server::Core::addPlayerInGroup(Network::UserJob *uj)
{
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);
  const Protocol::Argument *args = uj->job->getArguments();
  Server::User *target = this->findUser(args[0].pseudo);

  if (!user) { // si le joueur est pas connecté
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }
  else if (this->findGameLoader(user)) { // si le joueur est en game
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }
  else if (!target) { // si la target n'existe pas
    this->sendErrorJob(uj, Error::NOSUCHNICK);
    return ;
  }
  else if (user == target || this->findRoom(target) != NULL) { // si l'user s'invite lui-même || si la target est en file d'attente
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }
  else if (target->getSocket() == NULL) { // si la target n'est pas connectée
    this->sendErrorJob(uj, Error::PLAYERNOTCONNECTED);
    return ;
  }
  else if (user->getRequestGroup() != NULL) { // si le joueur est invité dans un groupe
    this->sendErrorJob(uj, Error::YOUAREINVITEDINAGROUP);
    return ;
  }
  else if (this->findRoom(user) != NULL) { // si le groupe est en file d'attente
    this->sendErrorJob(uj, Error::CURRENTLYWAITING);
    return ;
  }

  Server::Group *group = this->findGroup(user);
  size_t	nbPlayers = 0;
  if (group) {
    if (group->getLeader() != user) { // si l'user est dans un groupe dont il n'est pas chef
    this->sendErrorJob(uj, Error::NOTGROUPLEADER);
      return ;
    }
    else if (group->getUsers().empty() == false && group->getUsers().size() >= 3) { // si le groupe est plein
      this->sendErrorJob(uj, Error::GROUPFULL);
      return ;
    }
    else {
      nbPlayers = group->getUsers().size();
    }
  }
  for (auto it : this->_users) { // vérifier au niveau des invits si le groupe serait plein
    if (it != user && it->getRequestGroup() == user) {
      nbPlayers++;
    }
  }
  if (nbPlayers >= 3) {
    this->sendErrorJob(uj, Error::GROUPFULL);
    return ;
  }

  Error::Code error = ((this->findGroup(target) != NULL) ? (Error::USERINGROUP) : (user->groupRequest(target)));
  if (error != Error::NONE) {
    this->sendErrorJob(uj, error);
  }
}

void	Server::Core::answerGroupRequest(Network::UserJob *uj)
{
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);
  const Protocol::Argument *args = uj->job->getArguments();
  Server::User *target = this->findUser(args[0].pseudo);

  if (!user) { // user pas connecté
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }
  else if (this->findGameLoader(user)) { // si le joueur est en game ou en chargement de game
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }
  else if (!target) { // target existe pas
    this->sendErrorJob(uj, Error::NOSUCHNICK);
    return ;
  }

  if (user == target) { // user se répond a lui meme
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  if (target->getSocket() == NULL) { // target pas connectée
    this->sendErrorJob(uj, Error::PLAYERNOTCONNECTED);
    return ;
  }

  if (this->findRoom(user) != NULL) {
    this->sendErrorJob(uj, Error::YOUAREINVITEDINAGROUP);
    return ;
  }

  Error::Code error = user->answerGroupRequest(target);
  if (error == Error::NONE)
    {
      if (args[1].yes) // si requete acceptée
	{
	  Server::Group *group = this->findGroup(target);
	  if (group == NULL) { // si le groupe n'existe pas encore, le crée et met target en leader
	    group = new Server::Group(target);
	    group->addUser(target);
	    this->_groups.push_back(group);
	    Protocol::Job *job = Factory::Protocol::Job::create();
	    job->groupLeader(target->getNick());
	    Network::Manager::Server::getInstance()->push(target->getSocket(), job);
	    job = Factory::Protocol::Job::create();
	    job->newPlayerInGroup(target->getNick(), target->getRace());
	    Network::Manager::Server::getInstance()->push(target->getSocket(), job);
	  }
	  Protocol::Job *job = Factory::Protocol::Job::create(); // previens le nouvel arrivant que target est leader
	  job->groupLeader(target->getNick());
	  Network::Manager::Server::getInstance()->push(user->getSocket(), job); 
	  job = Factory::Protocol::Job::create();
	  job->newPlayerInGroup(user->getNick(), user->getRace());
	  Network::Manager::Server::getInstance()->push(user->getSocket(), job);
	  for (auto it : group->getUsers()) // donne la liste des users dans le groupe
	    {
	      Protocol::Job *job1 = Factory::Protocol::Job::create();
	      job1->newPlayerInGroup(it->getNick(), it->getRace());
	      Network::Manager::Server::getInstance()->push(user->getSocket(), job1);
	      job1 = Factory::Protocol::Job::create();
	      job1->newPlayerInGroup(user->getNick(), user->getRace());
	      Network::Manager::Server::getInstance()->push(it->getSocket(), job1);
	    }
	  group->addUser(user); // add user au groupe
	}
    }
  else
    {
      this->sendErrorJob(uj, error);
    }
}

void	Server::Core::kickPlayerGroup(Network::UserJob *uj)
{
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);
  const Protocol::Argument *args = uj->job->getArguments();
  Server::User *target = this->findUser(args[0].pseudo);

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }
  else if (this->findGameLoader(user)) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }
  else if (!target) {
    this->sendErrorJob(uj, Error::NOSUCHNICK);
    return ;
  }

  if (user == target) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  if (target->getSocket() == NULL) {
    this->sendErrorJob(uj, Error::PLAYERNOTCONNECTED);
    return ;
  }

  if (this->findRoom(user)) {
    this->sendErrorJob(uj, Error::YOUAREINVITEDINAGROUP);
  }

  Server::Group *groupUser = this->findGroup(user);
  Server::Group *groupTarget = this->findGroup(target);
  Error::Code error = Error::NONE;

  if (!groupUser) {
    error = Error::NOTINGROUP;
  }
  else if (!groupTarget || groupUser != groupTarget) {
    error = Error::NOTSAMEGROUP;
  }
  else if (groupUser->getLeader() != user) {
    error = Error::NOTGROUPLEADER;
  }
  if (error != Error::NONE) {
    this->sendErrorJob(uj, error);
    return ;
  }

  groupTarget->removeUser(target);
  {
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->groupKicked();
    Network::Manager::Server::getInstance()->push(target->getSocket(), job);
  }

  for (auto it : groupTarget->getUsers()) {
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->playerLeftGroup(target->getNick());
    Network::Manager::Server::getInstance()->push(it->getSocket(), job);
  }

  if (groupTarget->getUsers().empty() == true || groupTarget->getUsers().size() <= 1) {
    if (!groupTarget->getUsers().empty()) {
      for (auto it : groupTarget->getUsers()) {
	Protocol::Job *job = Factory::Protocol::Job::create();
	job->groupDeleted();
	Network::Manager::Server::getInstance()->push(it->getSocket(), job);
      }
    }
    this->_groups.remove(groupTarget);
    delete (groupTarget);
    groupTarget = NULL;
    return ;
  }
}

void		Server::Core::leaveGroup(Network::UserJob *uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);
  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }

  Server::Group *group = this->findGroup(user);
  if (!group) {
    this->sendErrorJob(uj, Error::NOTINGROUP);
    return ;
  }
  if (this->findGameLoader(user)) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  Room::ARoom *room = this->findRoom(user);
  if (room != NULL) {
    if (!group->getUsers().empty()) {
      for (auto it : group->getUsers()) {
	Protocol::Job *job = Factory::Protocol::Job::create();
	job->endGameSearching();
	Network::Manager::Server::getInstance()->push(it->getSocket(), job);
	room->kickPlayer(it);
      }
    }
  }

  group->removeUser(user);

  if (!group->getUsers().empty()) {
    for (auto it : group->getUsers()) {
      Protocol::Job *job = Factory::Protocol::Job::create();
      job->playerLeftGroup(user->getNick());
      Network::Manager::Server::getInstance()->push(it->getSocket(), job);
    }
  }

  if (group->getUsers().empty() == true || group->getUsers().size() <= 1) {
    if (!group->getUsers().empty()) {
      for (auto it : group->getUsers()) {
	Protocol::Job *job = Factory::Protocol::Job::create();
	job->groupDeleted();
	Network::Manager::Server::getInstance()->push(it->getSocket(), job);
      }
    }
    this->_groups.remove(group);
    delete (group);
    group = NULL;
    return ;
  }
}

void		Server::Core::deleteGroup(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }
  if (this->findGameLoader(user)) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  Server::Group *group = this->findGroup(user);
  if (!group) {
    this->sendErrorJob(uj, Error::NOTINGROUP);
    return ;
  }
  else if (user != group->getLeader()) {
    this->sendErrorJob(uj, Error::NOTGROUPLEADER);
    return ;
  }

  for (auto it : group->getUsers()) {
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->groupDeleted();
    Network::Manager::Server::getInstance()->push(it->getSocket(), job);
  }
  this->_groups.remove(group);
  delete (group);
}

void		Server::Core::sendWhisp(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  Server::User* user = this->findUser(uj->socket);
  const Protocol::Argument* args = uj->job->getArguments();
  Server::User*	target = this->findUser(args[0].pseudo);

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }
  else if (!target) {
    this->sendErrorJob(uj, Error::NOSUCHNICK);
    return ;
  }

  if (user == target) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  if (target->getSocket() == NULL) {
    this->sendErrorJob(uj, Error::PLAYERNOTCONNECTED);
    return ;
  }

  {
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->recvWhisp(user->getNick(), target->getNick(), args[1].msg);
    Network::Manager::Server::getInstance()->push(target->getSocket(), job);
  }
  {
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->recvWhisp(user->getNick(), target->getNick(), args[1].msg);
    Network::Manager::Server::getInstance()->push(user->getSocket(), job);
  }
}

void		Server::Core::sendMsgGroup(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  Server::User* user = this->findUser(uj->socket);
  const Protocol::Argument* args = uj->job->getArguments();

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }

  Server::Group *group = this->findGroup(user);
  if (!group) {
    this->sendErrorJob(uj, Error::NOTINGROUP);
    return ;
  }

  for (auto it : group->getUsers()) {
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->recvMsgGroup(user->getNick(), args[0].msg);
    Network::Manager::Server::getInstance()->push(it->getSocket(), job);
  }

}

void		Server::Core::sendMsgGlob(Network::UserJob* uj) {
  ScopeLock	sl(&(this->_mOG));
  Server::User *user = this->findUser(uj->socket);
  const Protocol::Argument *args = uj->job->getArguments();

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }

  Server::GameLoader *gl = this->findGameLoader(user);
  if (!gl) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  if (this->findGameByGameLoader(gl) == NULL) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }
  for (auto it : gl->getUserList()) {
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->recvMsgGlob(user->getNick(), args[0].msg);
    Network::Manager::Server::getInstance()->push(it->getSocket(), job);
  }
}

void		Server::Core::quicklaunch(Network::UserJob *uj) {
  ScopeLock sl(&this->_mOG);
  Server::User* user = this->findUser(uj->socket);
  const Protocol::Argument* args = uj->job->getArguments();

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }
  if (this->findGameLoader(user)) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  Room::ARoom *room = this->findRoom(user);
  if (room) {
    this->sendErrorJob(uj, Error::ALREADYWAITING);
    return ;
  }

  /* DELETE CE BLOC SI ON FAIT LES PARTIES CUSTOM */
  if (args[0].roomType == Room::CUSTOM) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  Server::Group *group = this->findGroup(user);
  size_t nbPlayer = 1;
  if (group) {
    if (group->getLeader() != user) { // si dans un groupe dont il n'est pas le chef
      this->sendErrorJob(uj, Error::NOTGROUPLEADER);
      return ;
    }
    else {
      nbPlayer = group->getUsers().size();
    }
  }

  if ((args[0].roomType == Room::ONEVSONE && nbPlayer > 1) ||
      (args[0].roomType == Room::TWOVSTWO && nbPlayer > 2)) { // si trop de membre dans le groupe pour lancer la partie correspondante
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  Logger::getInstance()->log("Entering match making", Logger::INFORMATION);
  if (group) {
    MatchMaking::run(args[0].roomType, this->_rooms, group->getUsers());
  } else {
    std::list<Server::User*> players;
    players.push_back(user);
    MatchMaking::run(args[0].roomType, this->_rooms, players);
  }
  Logger::getInstance()->log("Leaving match making", Logger::INFORMATION);

  /* lance le gameloader pour les rooms pretes et les supprime de la liste */
  for (std::list<Room::ARoom*>::iterator it = this->_rooms.begin() ; it != this->_rooms.end() ; ++it) {
    if ((*it)->isReady()) {
      Room::ARoom *room = *it;
      it = this->_rooms.erase(it);
      for (auto it : room->getPlayers()) {
	it->changeStatus(::User::BUSY);
	Protocol::Job *job = Factory::Protocol::Job::create();
	job->gameLoading();
	Network::Manager::Server::getInstance()->push(it->getSocket(), job);
      }
      Server::GameLoader *gl = Factory::Server::GameLoader::create(room->getPlayers());
      gl->addFile(room->getMap().fullpath);
      delete (room);
      gl->askFileExists();
      this->_gameLoaders.push_back(gl);
    }
  }

}

void		Server::Core::leaveGameSearch(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  Server::User* user = this->findUser(uj->socket);

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }
  if (this->findGameLoader(user)) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  Room::ARoom *room = this->findRoom(user);
  if (!room) {
    this->sendErrorJob(uj, Error::NOTWAITING);
    return ;
  }

  Server::Group *group = this->findGroup(user);
  if (group && group->getLeader() != user) {
    this->sendErrorJob(uj, Error::NOTGROUPLEADER);
    return ;
  }

  if (group) {
    for (auto it : group->getUsers()) {
      Protocol::Job *job = Factory::Protocol::Job::create();
      job->endGameSearching();
      Network::Manager::Server::getInstance()->push(it->getSocket(), job);
      room->kickPlayer(it);
    }
  } else {
      Protocol::Job *job = Factory::Protocol::Job::create();
      job->endGameSearching();
      Network::Manager::Server::getInstance()->push(user->getSocket(), job);
      room->kickPlayer(user);
  }

}

void	Server::Core::answerFileExists(Network::UserJob* uj) {
  ScopeLock sl(&this->_mOG);
  Server::User* user = this->findUser(uj->socket);
  const Protocol::Argument *args = uj->job->getArguments();

  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }

  Server::GameLoader *spec = this->findGameLoaderSpectator(user);
  if (spec) {
    if (args[1].yes == false) {
      this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
      spec->removeSpectator(user);
      this->_mOG.unlock();
      leaveSpectator(uj);
    }
    return ;
  }
  Server::GameLoader *gl = this->findGameLoader(user);
  if (!gl) {
    this->sendErrorJob(uj, Error::NOTLOADINGGAME);
    return ;
  }
  gl->playerAnswered(user, args[0].filename, args[1].yes);
}

void	Server::Core::iAmReady(Network::UserJob *uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);
  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }
  Server::GameLoader *gl = this->findGameLoader(user);
  Server::GameLoader *spec = this->findGameLoaderSpectator(user);
  if (spec) {
    Server::Game *game = this->findGameByGameLoader(spec);
    if (game) {
      game->addSpectator(user);
    }
    return ;
  }
  if (!gl) {
    this->sendErrorJob(uj, Error::NOTLOADINGGAME);
    return ;
  }
  gl->playerIsReady(user);

  Server::Game *game = this->findGameByGameLoader(gl);
  if (game) {
    game->userReconnected(user);
  }
  else {
    if (gl->isGameReady()) {
      this->createGame(gl);
    }
  }
}

void	Server::Core::joinSpectator(Network::UserJob *uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);
  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;    
  }
  if (this->findGroup(user) != NULL || this->findRoom(user) != NULL || this->findGameLoader(user) || user->isSpectating()) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }

  Server::User *target = this->findUser(uj->job->getArguments()[0].pseudo);
  if (!target) {
    this->sendErrorJob(uj, Error::NOSUCHNICK);
    return ;
  }
  Server::GameLoader *gl = this->findGameLoader(target);
  if (!gl) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;  
  }
  Server::Game *game = this->findGameByGameLoader(gl);
  if (!game) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->gameLoading();
  Network::Manager::Server::getInstance()->push(user->getSocket(), job);

  gl->addSpectator(user);
  gl->askFileExistsSpectator(user);
  user->isSpectating(true);
  user->spectating(game);
}

void		Server::Core::leaveSpectator(Network::UserJob *uj) {
  ScopeLock sl(&this->_mOG);
  Server::User *user = this->findUser(uj->socket);
  if (!user) {
    this->sendErrorJob(uj, Error::NOTCONNECTED);
    return ;
  }
  if (!(user->isSpectating())) {
    this->sendErrorJob(uj, Error::FORBIDDENOPERATION);
    return ;
  }
  Server::GameLoader *loader = this->findGameLoaderSpectator(user);
  if (loader) {
    loader->removeSpectator(user);
  }
  user->isSpectating(false);
  Server::Game *game = user->getGameSpectating();
  user->spectating(NULL);
  game->removeUser(user);
}

Server::GamePlayer	*Server::Core::getGamePlayer(Network::SSocket *socket) {
  const std::map<Server::GamePlayer *, Server::Game *> &map = this->_ingamePlayers.getSecondMap();
  for (auto it : map) {
    if (it.first->getUser()->getSocket() == socket) {
      if (it.first->lock() == false) {
	return (NULL);
      }
      if (it.first->getUser()->getSocket() != socket) {
	it.first->unlock();
	return (NULL);
      }
      return (it.first);
    }
  }
  return (NULL);
}

Server::AItem	*Server::Core::_createItem(const createItem &create) {
  Server::AItem *item = NULL;
  Kernel::ID::id_t id = Kernel::ID::getInstance()->get(Kernel::ID::ITEM);
  switch (Kernel::Manager::getInstance()->getData(create.serial)->type) {
  case ::Game::UNIT:
    item = Factory::Server::Unit::create(id, create.serial, create.player, create.o, create.x, create.y, create.z);
    break;
  case ::Game::HERO:
    item = Factory::Server::Hero::create(id, create.serial, create.player, create.o, create.x, create.y, create.z);
    break;
  case ::Game::BUILDING:
    item = Factory::Server::Building::create(id, create.serial, create.player, create.o, create.x, create.y, create.z);
    break;
  case ::Game::PROJECTILE:
    item = Factory::Server::Projectile::create(id, create.serial, create.player, create.o, create.x, create.y, create.z);
    break;
  default:
    break;
  }
  return (item);
}

void	Server::Core::handleCreateItemEvent(Server::Event *event) {
  Server::AItem *item = this->_createItem(event->create);
  if (item == NULL) {
    return ;
  }
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->addItem(item);
}

void	Server::Core::handlePingEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->onPing(event->ping.playerID, event->ping.x, event->ping.y);
}

void	Server::Core::ping(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  double x, y;
  Protocol::JobResolver::getInstance()->ping(uj->job, id, x, y);
  game->askPing(id, x, y);
  player->unlock();
}

void	Server::Core::handleChangeStanceEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->changeUnitStance(event->stance.id, event->stance.type, event->stance.stance);
}

void	Server::Core::changeUnitStance(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Kernel::ID::id_t unitID;
  ::Game::Stance::Type stance;
  Protocol::JobResolver::getInstance()->askChangeUnitStance(uj->job, unitID, stance);
  Server::Game *game = this->_ingamePlayers[player];
  game->askChangeUnitStance(player, unitID, stance);
  player->unlock();
}

void	Server::Core::changeUnitGrpStance(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  ::Game::Stance::Type stance;
  std::vector<Kernel::ID::id_t> units;
  Protocol::JobResolver::getInstance()->unitGroupStance(uj->job, units, stance);
  game->askChangeUnitsStance(player, stance, units);
  player->unlock();
}

void	Server::Core::askUnitStance(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Kernel::ID::id_t id;
  Server::Game *game = this->_ingamePlayers[player];
  Protocol::JobResolver::getInstance()->unitAskStance(uj->job, id);
  game->askUnitStance(player, id);
  player->unlock();
}

void	Server::Core::askItemInformations(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askItemInfo(uj->job, id);
  game->askItemInformations(player, id);
  player->unlock();
}

void	Server::Core::askMoveItem(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  double x, y, z;
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askMoveItem(uj->job, id, x, y, z);
  game->askMoveItem(player, id, x, y, z);
  player->unlock();
}

void	Server::Core::askDayOrNight(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Protocol::Job *j = Factory::Protocol::Job::create();
  j->dayNightSwitch(game->isDay());
  player->addJob(j);
  player->unlock();
}

void	Server::Core::askPlayerRessources(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askPlayerRessources(uj->job, id);
  game->askPlayerRessources(player, id);
  player->unlock();
}

void	Server::Core::askItemAction(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askItemAction(uj->job, id);
  game->askItemAction(player, id);
  player->unlock();
}

void	Server::Core::askMoveGrp(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  std::vector<Kernel::ID::id_t> units;
  bool adapt;
  double x, y, z;
  Protocol::JobResolver::getInstance()->askMoveItemGroup(uj->job, units, x, y, z, adapt);
  game->askMoveItemGroup(player, units, adapt,
			 x, y, z);
  player->unlock();
}

void	Server::Core::askActivateCapacity(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  Kernel::Serial capacity;
  Protocol::JobResolver::getInstance()->askActivateCapacity(uj->job, id, capacity);
  game->askItemToUseCapacity(player, id, capacity);
  player->unlock();
}

void	Server::Core::askUnitPickedUpRessources(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (!player) {
    return;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askUnitPickedUpRessources(uj->job, id);
  game->askUnitPickedUpRessources(player, id);
  player->unlock();
}

void	Server::Core::handleEffectTimeoutEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->effectTimeout(event->effectTimeout.item.id, event->effectTimeout.item.type, event->effectTimeout.effect);
}

void	Server::Core::askItemEffects(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (!player) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askItemEffects(uj->job, id);
  game->askItemEffects(player, id);
  player->unlock();
}

void	Server::Core::handleRemoveFromProductionListEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->removeItemFromProductionList(event->production.id, event->production.serial, event->production.player);
}

void	Server::Core::askRemoveUnitFromProduction(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::Serial serial;
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askRemoveUnitFromProduction(uj->job, id, serial);
  game->askRemoveUnitFromProduction(player, id, serial);
  player->unlock();
}

void	Server::Core::handlePutInProductionListEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->addItemToProductionList(event->production.id, event->production.serial, event->production.player);
}

void	Server::Core::askUnitProduction(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::Serial serial;
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askUnitProd(uj->job, id, serial);
  game->askUnitProduction(player, id, serial);
  player->unlock();
}

void	Server::Core::handleSetActionEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  Server::Action *action = Factory::Server::Action::create(event->itemAction.action);
  game->setItemAction(event->itemAction.id, event->itemAction.type, action);
}

void	Server::Core::askSetItemAction(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  ::Game::eAction action;
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askSetItemAction(uj->job, id, action);
  game->askSetItemAction(player, id, action);
  player->unlock();
}

void	Server::Core::askSetItemsAction(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  std::vector<Kernel::ID::id_t> units;
  ::Game::eAction action;
  Protocol::JobResolver::getInstance()->askSetItemsAction(uj->job, units, action);
  game->askSetItemsAction(player, action, units);
  player->unlock();
}

void	Server::Core::stopItemAction(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->itemStopAction(uj->job, id);
  game->askStopItemAction(player, id);
  player->unlock();
}

void	Server::Core::handleStopActionEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->stopItemAction(event->itemInfos.id, event->itemInfos.type);
}

void	Server::Core::handleReleaseObjectEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->heroReleaseObject(event->releaseObject.hero, event->releaseObject.object,
			  event->releaseObject.x, event->releaseObject.y, event->releaseObject.z);
}

void	Server::Core::handleMoveTowardsObjectAndPickUpObject(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->moveTowardsObjectAndPickUp(event->pickUpObject.hero, event->pickUpObject.object);
}

void	Server::Core::handlePickUpObjectEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->heroPickUpObject(event->pickUpObject.hero, event->pickUpObject.object);
}

void	Server::Core::askHeroPickUpObject(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t hero, object;
  Protocol::JobResolver::getInstance()->askHeroToTakeObject(uj->job, hero, object);
  game->askHeroPickUpObject(player, hero, object);
  player->unlock();
}

void	Server::Core::handleMoveTowardsPointAndReleaseObjectEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->moveTowardsPointAndReleaseObject(event->releaseObject.hero, event->releaseObject.object,
					 event->releaseObject.x, event->releaseObject.y, event->releaseObject.z);
}

void	Server::Core::askHeroReleaseObject(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t hero, object;
  double x, y, z;
  Protocol::JobResolver::getInstance()->askHeroToReleaseObject(uj->job, hero, object, x, y, z);
  game->askHeroReleaseObject(player, hero, object, x, y, z);
  player->unlock();
}

void	Server::Core::askHeroObjects(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askHeroObjects(uj->job, id);
  game->askHeroObjects(player, id);
  player->unlock();
}

void	Server::Core::handleFinishedAmeliorationProductionEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  Kernel::Serial type, amelioration;
  type = Kernel::get_serial(event->ameliorationFinished.type);
  amelioration = Kernel::get_serial(event->ameliorationFinished.serial);
  game->playerHasProducedAmelioration(event->ameliorationFinished.playerID, type, amelioration);
}

void	Server::Core::askSpecificElementAmeliorations(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::Serial serial;
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askElementAmeliorations(uj->job, id, serial);
  game->askSpecificElementAmeliorations(player, id, serial);
  player->unlock();
}

void	Server::Core::handleRemoveAmeliorationFromProductionListEvent(Server::Event *event) {
  Server::GamePlayer *player = reinterpret_cast<Server::GamePlayer *>(event->ptr);
  player->removeItemFromProductionList(event->ameliorationProductionList.type, event->ameliorationProductionList.amelioration);
}

void	Server::Core::askRemoveAmeliorationFromProductionQueue(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::Serial item, amelioration;
  Protocol::JobResolver::getInstance()->askRemoveAmeliorationFromProductionQueue(uj->job, item, amelioration);
  game->askRemoveAmeliorationFromProductionQueue(player, item, amelioration);
  player->unlock();
}

void	Server::Core::handleProduceAmeliorationEvent(Server::Event *event) {
  Server::GamePlayer *player = reinterpret_cast<Server::GamePlayer *>(event->ptr);
  Kernel::Serial type, amelioration;
  type = Kernel::get_serial(event->ameliorationProductionList.type);
  amelioration = Kernel::get_serial(event->ameliorationProductionList.amelioration);
  player->produceAmelioration(type, amelioration);
}

void	Server::Core::askProduceAmelioration(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::Serial item, amelioration;
  Protocol::JobResolver::getInstance()->askProduceAmelioration(uj->job, item, amelioration);
  game->askProduceAmelioration(player, item, amelioration);
  player->unlock();
}

void	Server::Core::askPlayerAmeliorations(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askPlayerAmeliorations(uj->job, id);
  game->askPlayerInformations(player, id);
  player->unlock();
}

void	Server::Core::handleStopBuildEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitStopBuilding(event->build.unit, event->build.building);
}

void	Server::Core::handleUnitBuildEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitBuildBuilding(event->build.unit, event->build.building);
}

void	Server::Core::handleMoveTowardsItemAndBuildEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->moveTowardsItemAndBuild(event->build.unit, event->build.building);
}


void	Server::Core::askUnitToBuild(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Kernel::ID::id_t unit, building;
  Protocol::JobResolver::getInstance()->askUnitToBuild(uj->job, unit, building);
  Server::Game *game = this->_ingamePlayers[player];
  game->askUnitToBuild(player, unit, building);
  player->unlock();
}

void	Server::Core::askRessourcesSpotID(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  double x, y, z;
  Protocol::JobResolver::getInstance()->askSpotID(uj->job, x, y, z);
  game->askRessourcesSpotID(player, x, y, z);
  player->unlock();
}

void	Server::Core::handleUnitGoBackToSpotEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitGoBackToSpot(event->harvest.unit, event->harvest.spot);
}

void	Server::Core::handleMoveTowardsBuildingAndDeposit(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->moveTowardsBuildingAndDeposit(event->deposit.unit, event->deposit.building, event->deposit.type);
}

void	Server::Core::handleDepositInBuildingEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitDepositInBuilding(event->deposit.unit, event->deposit.building, event->deposit.type);
}


void	Server::Core::handleStopHarvestAndDepositEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitStopsHarvestAndDeposit(event->harvest.unit, event->harvest.spot);
}

void	Server::Core::handleUnitAddRessEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitHarvestRessources(event->harvestAmount.unit, event->harvestAmount.type, event->harvestAmount.amount);
}

void	Server::Core::handleWaitListToHarvestEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitGoBackToSpot(event->harvest.unit, event->harvest.spot);
}

void	Server::Core::handleHarvestSpotEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitHarvestSpot(event->harvest.unit, event->harvest.spot);
}

void	Server::Core::handleMoveTowardsSpotAndHarvestEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->moveTowardsSpotAndHarvest(event->harvest.unit, event->harvest.spot);
}

void	Server::Core::askUnitToHarvest(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t unit, spot;
  Protocol::JobResolver::getInstance()->askUnitHarvest(uj->job, unit, spot);
  game->askUnitToHarvest(player, unit, spot);
  player->unlock();
}

void	Server::Core::askUnitsToHarvest(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  std::vector<Kernel::ID::id_t> units;
  Kernel::ID::id_t spot;
  Protocol::JobResolver::getInstance()->askUnitsHarvest(uj->job, units, spot);
  game->askUnitsToHarvest(player, spot, units);
  player->unlock();
}

void	Server::Core::askUnitsToBuild(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  std::vector<Kernel::ID::id_t> units;
  Kernel::ID::id_t id;
  Protocol::JobResolver::getInstance()->askUnitsToBuild(uj->job, units, id);
  game->askUnitsToBuild(player, id, units);
  player->unlock();
}

void	Server::Core::handleOnRangeEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->itemIsInRangeOfAnotherItem(event->onRange.source.type, event->onRange.source.id, event->onRange.target.type, event->onRange.target.id);
}

void	Server::Core::handleStartMoveEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->startMoveItem(event->move.id, event->move.type, event->move.x, event->move.y, event->move.z);
}

void	Server::Core::handleMoveEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->moveItem(event->moveItem.id, event->moveItem.type, event->moveItem.factor);
}

void	Server::Core::askItemToUseCapacity(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Kernel::Serial capacity;
  Kernel::ID::id_t target, source;
  Protocol::JobResolver::getInstance()->askItemUseCapacity(uj->job, source, target, capacity);
  Server::Game *game = this->_ingamePlayers[player];
  game->askItemToUseCapacity(player, source, target, capacity);
  player->unlock();
}

void	Server::Core::askItemToAttack(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t source, target;
  Protocol::JobResolver::getInstance()->askItemAttack(uj->job, source, target);
  game->askItemToAttack(player, source, target);
  player->unlock();
}

void	Server::Core::askItemsToAttack(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t target;
  std::vector<Kernel::ID::id_t> sources;
  Protocol::JobResolver::getInstance()->askItemsAttack(uj->job, sources, target);
  game->askItemsToAttack(player, sources, target);
  player->unlock();
}

void	Server::Core::handleCreateProjEvent(Server::Event *event) {
  Server::AItem *proj = this->_createItem(static_cast<const createItem>(event->projectile));
  if (proj == NULL) {
    return ;
  }
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  if (event->projectile.targetItem) {
    game->createProjectile(proj, event->projectile.source.id, event->projectile.source.type,
			   event->projectile.target.id, event->projectile.target.type);
  }
  else {
    game->createProjectile(proj, event->projectile.source.id, event->projectile.source.type,
			   event->projectile.destination.x, event->projectile.destination.y,
			   event->projectile.destination.z);
  }
}

void	Server::Core::handleFollowEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->followItem(event->follow.source.id, event->follow.source.type,
		   event->follow.target.id, event->follow.target.type, event->follow.factor);
}

void	Server::Core::askUnitPatrol(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  double x, y;
  Protocol::JobResolver::getInstance()->askUnitPatrol(uj->job, id, x, y);
  game->askUnitToPatrol(player, id, x, y);
  player->unlock();
}

void	Server::Core::askUnitsPatrol(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  std::vector<Kernel::ID::id_t> units;
  bool adapt;
  double x, y;
  Protocol::JobResolver::getInstance()->askUnitsPatrol(uj->job, units, x, y, adapt);
  game->askUnitsToPatrol(player, units, x, y, adapt);
  player->unlock();
}

void	Server::Core::handlePatrolEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitPatrol(event->patrol.item.id, event->patrol.item.type,
		   event->patrol.x, event->patrol.y);
}

void	Server::Core::handleMoveToPointEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->moveToPoint(event->moveToPoint.item.id, event->moveToPoint.item.type, event->moveToPoint.point.x, event->moveToPoint.point.y, event->moveToPoint.factor);
}

void	Server::Core::askItemToAttackZone(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  double x, y, z;
  Protocol::JobResolver::getInstance()->askItemAttackZone(uj->job, id, x, y, z);
  game->askItemToAttackZone(player, id, x, y, z);
  player->unlock();
}

void	Server::Core::askItemsToAttackZone(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  std::vector<Kernel::ID::id_t> items;
  double x, y, z;
  Protocol::JobResolver::getInstance()->askItemsAttackZone(uj->job, items, x, y, z);
  game->askItemsToAttackZone(player, items, x, y, z);
  player->unlock();
}

void	Server::Core::handleStartUseCapacityEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  switch (event->useCapacity.type) {
  case ::Game::Capacity::ITEM:
    game->itemStartUseCapacityTarget(event->useCapacity.item.id, event->useCapacity.item.type,
				     event->useCapacity.capacity,
				     event->useCapacity.target.id, event->useCapacity.target.type);
    break;
  case ::Game::Capacity::ZONE:
    game->itemStartUseCapacityZone(event->useCapacity.item.id, event->useCapacity.item.type,
				   event->useCapacity.capacity,
				   event->useCapacity.point.x, event->useCapacity.point.y, event->useCapacity.point.y);
    break;
  case ::Game::Capacity::NONE:
    game->itemStartUseCapacity(event->useCapacity.item.id, event->useCapacity.item.type, event->useCapacity.capacity);
    break;
  default:
    break;
  }
}

void	Server::Core::handleStopActionHarvestEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitStopsHarvest(event->harvest.unit, event->harvest.spot);
}

void	Server::Core::handleStopActionBuildEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->unitStopsBuilding(event->build.unit, event->build.building);
}

void	Server::Core::handleUseCapacityEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  switch (event->useCapacity.type) {
  case ::Game::Capacity::ITEM:
    game->itemUseCapacityTarget(event->useCapacity.item.id, event->useCapacity.item.type,
				event->useCapacity.capacity,
				event->useCapacity.target.id, event->useCapacity.target.type);
    break;
  case ::Game::Capacity::ZONE:
    game->itemUseCapacityZone(event->useCapacity.item.id, event->useCapacity.item.type,
			      event->useCapacity.capacity,
			      event->useCapacity.point.x, event->useCapacity.point.y, event->useCapacity.point.z);
    break;
  case ::Game::Capacity::NONE:
    game->itemUseCapacity(event->useCapacity.item.id, event->useCapacity.item.type,
			  event->useCapacity.capacity);
    break;
  default:
    break;
  }
}

void	Server::Core::handleEndActionEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->endItemAction(event->itemInfos.id, event->itemInfos.type);
}

void	Server::Core::askItemToUseZoneCapacity(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (!player) {
    return ;
  }
  Kernel::ID::id_t id;
  Kernel::Serial capacity;
  double x, y, z;
  Server::Game *game = this->_ingamePlayers[player];
  Protocol::JobResolver::getInstance()->askItemUseZoneCapacity(uj->job, id, capacity, x, y, z);
  game->askItemToUseZoneCapacity(player, id, capacity, x, y, z);
  player->unlock();
}

void	Server::Core::handleMoveTowardsItemAndStartUseCapacityEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->moveTowardsItemAndStartUseCapacity(event->useCapacity.item.id, event->useCapacity.item.type,
					   event->useCapacity.capacity,
					   event->useCapacity.target.id, event->useCapacity.target.type);
}

void	Server::Core::handleMoveTowardsPointAndUseCapacityEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->moveTowardsPointAndUseCapacity(event->useCapacity.item.id, event->useCapacity.item.type,
				       event->useCapacity.capacity, event->useCapacity.point.x,
				       event->useCapacity.point.y, event->useCapacity.point.z);
}

void	Server::Core::askItemToSetCapacityAutomatic(Network::UserJob *uj) {
  std::string s = __PRETTY_FUNCTION__;
  Logger::getInstance()->logf("Desactivated %s", Logger::FATAL, &s);
  return ;
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::ID::id_t id;
  bool automatic;
  Kernel::Serial capacity;
  Protocol::JobResolver::getInstance()->askAutomaticCapacity(uj->job, id, capacity, automatic);
  game->askAutomaticCapacity(player, id, capacity, automatic);
  player->unlock();
}

void	Server::Core::handleSetCapacityAutomaticEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->setCapacityAutomatic(event->setAutomatic.item.id, event->setAutomatic.item.type,
			     event->setAutomatic.capacity, event->setAutomatic.automatic);
}

void	Server::Core::handleAddXPEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->addXP(event->xp.id, event->xp.amount);
}

void	Server::Core::handleStatsChangedEvent(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  game->statsChanged(event->item.id, event->item.type);
}

void	Server::Core::handlePlayerLostEvent(Server::Event *event) {
  Server::GamePlayer *player = reinterpret_cast<Server::GamePlayer *>(event->ptr);
  Server::Game *game = this->_ingamePlayers[player];
  game->playerHasLost(player);
}

void	Server::Core::askCreateBuilding(Network::UserJob *uj) {
  Server::GamePlayer *player = this->getGamePlayer(uj->socket);
  if (player == NULL) {
    return ;
  }
  Server::Game *game = this->_ingamePlayers[player];
  Kernel::Serial serial;
  double x, y, z;
  int orientation;
  Protocol::JobResolver::getInstance()->askCreateBuilding(uj->job, serial, x, y, z, orientation);
  game->askCreateBuilding(player, serial, x, y, z, orientation);
}

void	Server::Core::addJob(Network::SSocket *user, Protocol::Job *job) {
  ScopeLock s(this->_jMutex);
  this->_jobs.push(std::make_pair(user, job));
}

void	Server::Core::handleGamePlayersJobs(Server::Event *) {
  Protocol::Job *job;
  for (auto itGame : this->_games) {
    for (auto player : this->_ingamePlayers[itGame.second.game]) {
      do {
	job = player->popJob();
	if (job) {
	  //	  Logger::getInstance()->logf("Out: %j", Logger::INFORMATION, job);
	  if (!player->isReconnectJob(job)) {
	    itGame.second.game->addSpectatorJob(Kernel::ID::PLAYER, player->getID(), job);
	  }
	  if (player->getUser() && player->getUser()->getSocket()) {
	    this->addJob(player->getUser()->getSocket(), job);
	  }
	  else {
	    Factory::Protocol::Job::remove(job);
	  }
	}
      } while (job);
    }
  }
}

void	Server::Core::handleJobs(Server::Event *) {
  Network::UserJob *uj;
  do {
    uj = Network::Manager::Server::getInstance()->getJob();
    if (uj) {
      Logger::getInstance()->logf("In: %j", Logger::INFORMATION, uj->job);
      for (int i = 0 ; i < NBR_FUNC_JOB ; ++i) {
	if (uj->job->getCommand() == this->jobFuncTab[i].command) {
	  (this->*jobFuncTab[i].func)(uj);
	  break ;
	}
      }
      Factory::Protocol::Job::remove(uj->job);
      Factory::Network::UserJob::remove(uj);
    }
  } while (uj);

  Protocol::Job *job;
  Network::SSocket *socket;
  do {
    job = NULL;
    socket = NULL;
    {
      ScopeLock s(this->_jMutex);
      if (this->_jobs.empty() == false) {
	job = this->_jobs.front().second;
	socket = this->_jobs.front().first;
	this->_jobs.pop();
      }
    } 
    if (job && socket) {
      Logger::getInstance()->logf("Out: %j on socket (%d)", Logger::INFORMATION, job, socket->getFD());
      Network::Manager::Server::getInstance()->push(socket, job);
    }
    else if (job) {
      Logger::getInstance()->logf("Out: %j", Logger::INFORMATION, job);
    }
  } while (job);
}

void	Server::Core::handleGameEvents(Server::Game *game) {
  Server::Event *event;
  do {
    event = game->popEvent();
    if (event) {
      this->addEvent(event);
    }
  } while (event);
}

void	Server::Core::handleGameJobs(Server::Game *game) {
  Protocol::Job *job = NULL;
  do {
    job = game->popJob();
    if (job) {
      for (auto u : this->_ingamePlayers[game]) {
	if (u->getUser()) {
	  this->addJob(u->getUser()->getSocket(), job->clone());
	}
      }
    }
    Factory::Protocol::Job::remove(job);
  } while (job);
  Network::SSocket *socket = NULL;
  while (game->popSpecJob(&socket, &job)) {
    this->addJob(socket, job);
  }
  for (auto player : this->_ingamePlayers[game]) {
    do {
      job = player->popJob();
      if (job) {
	//	  Logger::getInstance()->logf("Out: %j", Logger::INFORMATION, job);
	if (!player->isReconnectJob(job)) {
	  game->addSpectatorJob(Kernel::ID::PLAYER, player->getID(), job);
	}
	if (player->getUser() && player->getUser()->getSocket()) {
	  this->addJob(player->getUser()->getSocket(), job);
	}
	else {
	  Factory::Protocol::Job::remove(job);
	}
      }
    } while (job);
  }
}

void	Server::Core::removeObject(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  if (event->objType == Server::Event::SERVER_ITEM) {
    game->removeItem(event->remove.type, event->remove.id);
  }
}

void	Server::Core::updateObject(Server::Event *event) {
  Server::Game *game = reinterpret_cast<Server::Game *>(event->ptr);
  if (event->objType == Server::Event::SERVER_GAME) {
    bool v = game->update(this->_clock, event->update.factor);
    this->handleGameEvents(game);
    this->handleGameJobs(game);
    if (!v) {
      this->deleteGame(game->getID());
    }
  }
  else if (event->objType == Server::Event::SERVER_ITEM) {
    game->update(event->update.id, event->update.type, event->update.factor);
  }
  else if (event->objType == Server::Event::SERVER_GAMEPLAYER) {
    game->updatePlayer(event->update.id, event->update.factor);
  }
}

void	Server::Core::runThread(void) {
  unsigned int myID;
  Clock clock;
  clock.clone(this->_clock);
  {
    ScopeLock s(this->_stTh);
    static unsigned int id = 0;
    myID = id;
    this->_threadsStatus.push_back({id, SLEEPING, &clock, Server::Event::NONE});
    ++id;
  }
  if (myID == 0) {
    while (!this->_end) {
      usleep(150000);
      this->dumpThread(NULL);
    }
    return ;
  }
  Server::Event *event = NULL;
  while (!this->_end) {
    this->_threadsStatus[myID].status = SLEEPING;
    this->_threadsStatus[myID].clock->clone(this->_clock);
    this->_threadsStatus[myID].event = Server::Event::NONE;
    this->_cond->wait();
    this->_threadsStatus[myID].status = WORKING;
    do {
      this->_threadsStatus[myID].clock->clone(this->_clock);
      event = this->popEvent();
      if (event) {
	this->_threadsStatus[myID].event = event->type;
	unsigned int j = 0;
	for ( ; j < NBR_FUNC ; ++j) {
	  if (this->_ptrFunc[j].type == event->type) {
	    (this->*_ptrFunc[j].func)(event);
	    break;
	  }
	}
	if (j == NBR_FUNC) {
	  Logger::getInstance()->logf("Cannot handle event of type >%d<", Logger::FATAL, event->type);
	}
	Factory::Server::Event::remove(event);
      }
    } while (event);
  }
  //  this->_cond->wait();
}

void	Server::Core::dumpThread(Server::Event *) {
  Logger::getInstance()->dumpThread(this->_clock, this->_threadsStatus, Server::HasEvent::size());
}

int	Server::Core::run(void) {
  Clock lastUpdate;
  lastUpdate.clone(this->_clock);
  while (!this->_end) {
    this->addEvent(Factory::Server::Event::create(Server::Event::JOBS, Server::Event::NO_TYPE, NULL));
    //    this->addEvent(Factory::Server::Event::create(Server::Event::GPJOBS, Server::Event::NO_TYPE, NULL));
    this->_clock->update();
    // if (this->_clock->getElapsedTimeSince(&lastUpdate) >= 0.3) {
    //   this->addEvent(Factory::Server::Event::create(Server::Event::DUMP_THREADS, Server::Event::NO_TYPE, NULL));
    //   lastUpdate.clone(this->_clock);
    // }
    this->_cond->signal();
    this->_cond->signal();
    this->_cond->signal();
    double waitTime = 0.3;
    double updates = Kernel::Config::getInstance()->getDouble(Kernel::Config::UPDATES_PER_SECOND);
    for (auto it : this->_games) {
      double eTime = this->_clock->getElapsedTimeSince(it.second.clock);
      double wTime = 1.0 / updates - eTime;
      if (wTime <= 0.0) {
	int sTime = static_cast<int>(wTime * -updates);
	Server::Event *event = Factory::Server::Event::create(Server::Event::UPDATE, Server::Event::SERVER_GAME, it.second.game);
	event->update.factor = sTime > 0 ? 1.0 + static_cast<double>(sTime) : 1.0;
	it.second.clock->clone(this->_clock);
	this->addEvent(event);
	this->_cond->signal();
	wTime = 0.00000000001;
      }
      if (wTime > 0 && wTime < waitTime) {
	waitTime = wTime;
      }
    }
    int timer = static_cast<int>(waitTime * 1000000.0);
    usleep(timer);
  }
  return (0);
}

void	Server::Core::createGame(Server::GameLoader *gl) {
  Kernel::ID::id_t id = Kernel::ID::getInstance()->get(Kernel::ID::GAME);
  const std::list<Server::User *>& users = gl->getUserList();
  std::vector<Server::GamePlayer*> players;
  std::vector< ::Game::Team*> teams;
  for (auto it : users) {
    const std::string fname = gl->getMapPath();
    const std::string rname = ::Race::RaceToString(it->getRace());
    const ::Game::Race *race = ::Game::Controller::getInstance()->getRaceNamed(fname, rname);
    if (!race) {
      Logger::getInstance()->logf("UNKNOWN RACE !!!! %s in %s", Logger::FATAL, &rname, &fname);
      return ;
    }
    Kernel::ID::id_t idplayer = Kernel::ID::getInstance()->get(Kernel::ID::PLAYER);
    int nbteam = it->getTeam();
    ::Game::Team *curTeam = NULL;
    for (auto team : teams) {
      if (team->getNb() == nbteam) {
	curTeam = team;
	break ;
      }
    }
    if (!curTeam) {
      curTeam = Factory::Game::Team::create();
      curTeam->setNb(nbteam);
      teams.push_back(curTeam);
    }
    Server::GamePlayer *gameplayer = Factory::Server::GamePlayer::create(it, idplayer, race, curTeam);
    players.push_back(gameplayer);
    curTeam->addPlayer(gameplayer);
  }
  Server::Game *game = Factory::Server::Game::create(id, players, Server::HasUsers::Container(), teams,
						     gl->getMapPath(), gl);
  for (auto it : players) {
    this->_ingamePlayers.push(game, it);
  }
  Clock *clock = this->_clock->clone();
  {
    ScopeLock s(this->_gMutex);
    this->_games[id] = {game, clock};
  }
  // gl->signalLaunch();
}

void	Server::Core::deleteGame(Kernel::ID::id_t id) {
  ScopeLock s(this->_gMutex);
  Server::Game *game = this->_games[id].game;
  this->onReferenceDeleted(game);
  Factory::Clock::remove(this->_games[id].clock);
  this->_games.erase(id);
  this->_ingamePlayers.remove(game);
  Factory::Server::Game::remove(game);
  Kernel::ID::getInstance()->push(id, Kernel::ID::GAME);
}

void	Server::Core::end(void) {
  Logger::getInstance()->log("Server has received SIGINT event", Logger::INFORMATION);
  this->_end = true;
  Network::Manager::Server::getInstance()->end();
}
