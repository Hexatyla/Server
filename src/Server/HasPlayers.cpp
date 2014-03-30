//
// HasPlayers.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sun Feb 16 01:00:37 2014 geoffrey bauduin
// Last update Sat Mar 15 15:21:07 2014 geoffrey bauduin
//

#include	"Server/HasPlayers.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Server.hpp"

Server::HasPlayers::HasPlayers(void):
  _playerMutex(new Mutex), _players() {

}

Server::HasPlayers::~HasPlayers(void) {
  delete this->_playerMutex;
}

void	Server::HasPlayers::init(const std::vector<Server::GamePlayer *> &players) {
  for (auto it : players) {
    this->_players[it->getID()] = it;
  }
}

void	Server::HasPlayers::destroy(void) {
  for (auto it : this->_players) {
    if (it.second) {
      Factory::Server::GamePlayer::remove(it.second);
    }
  }
  this->_players.clear();
}

void	Server::HasPlayers::removePlayer(Kernel::ID::id_t id) {
  ScopeLock s(this->_playerMutex);
  this->_players.erase(id);
}

void	Server::HasPlayers::removePlayer(Server::GamePlayer *player) {
  this->removePlayer(player->getID());
}
