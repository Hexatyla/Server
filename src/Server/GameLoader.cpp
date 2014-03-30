//
// GameLoader.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Thu Feb 13 13:19:03 2014 gautier lefebvre
// Last update Wed Mar 26 11:58:51 2014 Gautier Lefebvre
//

#include	<sys/types.h>
#include	<unistd.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<stdexcept>
#include	"FileLoader.hpp"
#include	"Server/GameLoader.hpp"
#include	"Logger.hpp"
#include	"Protocol/Job.hpp"
#include	"Factory/Protocol.hpp"
#include	"Network/Socket.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Network/Manager/Server.hpp"
#include	"Utils/Undefined.hpp"

#define	LOCK_LOADER	ScopeLock scopelockloader(this->_mutex)

Server::GameLoader::File::File(const std::string &fp, const std::string &fn, const std::list<Server::User*>& u):
  fullpath(fp),
  filename(fn),
  users()
{
  for (auto it : u) {
    users[it] = false;
  }
}

Server::GameLoader::File::File(const Server::GameLoader::File &oth):
  fullpath(oth.fullpath),
  filename(oth.filename),
  users(oth.users)
{}

Server::GameLoader::File &Server::GameLoader::File::operator=(const Server::GameLoader::File &oth) {
  if (this != &oth) {
    this->fullpath = oth.fullpath;
    this->filename = oth.filename;
    this->users = oth.users;
  }
  return (*this);
}

Server::GameLoader::File::~File() {}

bool	Server::GameLoader::File::isReady() const {
  for (auto it : this->users) {
    if (it.first->getSocket() != NULL && it.second == false) { return (false); }
  }
  return (true);
}

void	Server::GameLoader::File::sendFileToUser(Server::User *user) {
  char	buf[1025];
  int	ret, fd;
  unsigned long long size;
  Protocol::Job	*job;

  fd = open(this->fullpath.c_str(), O_RDONLY);
  if (fd == -1) {
    Logger::getInstance()->log("Could not open the file " + this->fullpath + " to send to player before starting the game", Logger::FATAL);
    return ;
  }
  lseek(fd, 0, SEEK_SET);
  size = static_cast<unsigned long long>(lseek(fd, 0, SEEK_END));
  lseek(fd, 0, SEEK_SET);

  job = Factory::Protocol::Job::create();
  job->fileInfos(this->filename, size, size / 1024 + (size % 1024 != 0 ? 1 : 0));
  Network::Manager::Server::getInstance()->push(user->getSocket(), job);
  unsigned int i = 0;
  while ((ret = read(fd, buf, 1024)) != 0 && ret != -1) {
    job = Factory::Protocol::Job::create();
    job->fileChunk(this->filename, i, buf, ret);
    Network::Manager::Server::getInstance()->push(user->getSocket(), job);
    i++;
  }
  close(fd);
  this->users[user] = true;
}

Server::GameLoader::GameLoader():
  _mutex(new Mutex),
  _files(),
  _users()
{}

Server::GameLoader::~GameLoader() {
  delete (this->_mutex);
}

void	Server::GameLoader::destroy() {
  LOCK_LOADER;
  this->_files.clear();
  this->_users.clear();
  this->_spectators.clear();
}

void	Server::GameLoader::init(const std::list<Server::User*> &u) {
  LOCK_LOADER;
  this->_userList = u;
  for (auto cur : u) {
    this->_users[cur] = false;
    for (auto tar : u) {
      Protocol::Job *job = Factory::Protocol::Job::create();
      job->playerInRoom(tar->getNick(), tar->getRace(), tar->getTeam(), false);
      Network::Manager::Server::getInstance()->push(cur->getSocket(), job);
    }
  }
}

const std::string Server::GameLoader::getMapPath() const {
  LOCK_LOADER;
  if (this->_files.empty()) {
    return ("");
  }
  else {
    return ("./races/" + this->_files.front().filename);
  }
}

void	Server::GameLoader::addFile(const std::string &fp) {
  LOCK_LOADER;
  for (auto it : _files) {
    if (it.fullpath == fp) { return ; }
  }
  std::list<Server::User *> users;
  for (auto it : _users) {
    users.push_back(it.first);
  }
  this->_files.push_back(Server::GameLoader::File(fp, FileLoader::get_filename(fp) + ".hxtl", users));
}

void	Server::GameLoader::askFileExists() {
  LOCK_LOADER;
  for (auto file : _files) {
    for (auto user : file.users) {
      Protocol::Job *job = Factory::Protocol::Job::create();
      job->askFileExists(file.filename);
      Network::Manager::Server::getInstance()->push(user.first->getSocket(), job);
    }
  }
}

bool	Server::GameLoader::isGameReady() const {
  LOCK_LOADER;
  for (auto it : _files) {
    if (it.isReady() == false) { return (false); }
  }
  for (auto it : _users) {
    if (it.first->getSocket() != NULL && it.second == false) { return (false); }
  }
  return (true);
}

void	Server::GameLoader::playerReconnected(Server::User *u) {
  LOCK_LOADER;
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->gameLoading();
  Network::Manager::Server::getInstance()->push(u->getSocket(), job);

  try {
    this->_users.at(u) = false;
  }
  catch (const std::out_of_range &) {
    return ;
  }

  for (auto file : _files) {
    for (auto user : file.users) {
      if (user.first == u) {
	user.second = false;
	job = Factory::Protocol::Job::create();
	job->askFileExists(file.filename);
	Network::Manager::Server::getInstance()->push(user.first->getSocket(), job);
      }
    }
  }
  for (auto user : this->_users) {
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->playerInRoom(user.first->getNick(), user.first->getRace(), user.first->getTeam(), user.second);
    Network::Manager::Server::getInstance()->push(u->getSocket(), job);
  }
}

void	Server::GameLoader::playerAnswered(const Server::User *u, const std::string &fn, bool answer) {
  LOCK_LOADER;
  for (auto &it : _files) {
    if (it.filename == fn) {
      for (auto &user : it.users) {
	if (user.first == u) {
	  user.second = true;
	  if (answer == false) {
	    it.sendFileToUser(user.first);
	  }
	}
      }
    }
  }
}

void	Server::GameLoader::playerIsReady(Server::User *u) {
  LOCK_LOADER;
  bool	rdy = true;
  for (auto file : _files) {
    for (auto user : file.users) {
      if (user.first == u && user.second == false) {
	rdy = false;
	Protocol::Job *job = Factory::Protocol::Job::create();
	job->askFileExists(file.filename);
	Network::Manager::Server::getInstance()->push(user.first->getSocket(), job);
      }
    }
  }
  if (rdy == true) {
    try {
      this->_users.at(u) = true;
      for (auto it : this->_users) {
	Protocol::Job *job = Factory::Protocol::Job::create();
	job->playerReady(it.first->getNick());
	Network::Manager::Server::getInstance()->push(u->getSocket(), job);
      }
    }
    catch (const std::out_of_range &) {
      return ;
    }
  }
}

void	Server::GameLoader::signalLaunch() {
  LOCK_LOADER;
  for (auto it : this->_users) {
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->gameLaunch();
    Network::Manager::Server::getInstance()->push(it.first->getSocket(), job);
  }
}

bool	Server::GameLoader::isUserInGameLoader(const Server::User *u) const {
  LOCK_LOADER;
  for (auto it : this->_users) {
    if (it.first == u) { return (true); }
  }
  return (false);
}

const std::list<Server::User*>&	Server::GameLoader::getUserList() const { return (this->_userList); }

void	Server::GameLoader::askFileExistsSpectator(Server::User *specto) {
  LOCK_LOADER;
  for (auto file : _files) {
    Protocol::Job *job = Factory::Protocol::Job::create();
    job->askFileExists(file.filename);
    Network::Manager::Server::getInstance()->push(specto->getSocket(), job);
  }
}

bool	Server::GameLoader::isSpectatorInGameLoader(const Server::User *specto) const {
  LOCK_LOADER;
  for (auto it : this->_spectators) {
    if (it == specto) {
      return (true);
    }
  }
  return (false);
}

void	Server::GameLoader::addSpectator(Server::User *specto) {
  LOCK_LOADER;
  for (auto it : this->_spectators) {
    if (it == specto) { return ; }
  }
  this->_spectators.push_back(specto);
}

void	Server::GameLoader::removeSpectator(Server::User *specto) {
  LOCK_LOADER;
  this->_spectators.remove(specto);
}
