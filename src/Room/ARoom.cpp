//
// ARoom.cpp for ex in /home/deleme/pfa/src/Room
// 
// Made by anthony delemer
// Login   <deleme_a@epitech.net>
// 
// Started on  Tue Jan 21 11:16:33 2014 anthony delemer
// Last update Thu Feb 13 13:15:50 2014 gautier lefebvre
//

#include	"Room/ARoom.hpp"

Room::ARoom::ARoom(Room::Type t):
  _type(t),
  _players(),
  _settings(),
  _map("", "", "")
{}

Room::ARoom::~ARoom() {}

Room::Type		Room::ARoom::getType() const { return (this->_type); }
const Room::Settings	&Room::ARoom::getSettings() const { return (this->_settings); }
const Room::MapInfos	&Room::ARoom::getMap() const { return (this->_map); }
const std::list<Server::User *> &Room::ARoom::getPlayers() const { return (this->_players); }
bool			Room::ARoom::isEmpty() const { return (this->_players.empty()); }

bool			Room::ARoom::isUserInRoom(const Server::User *user) const {
  for (auto it : this->_players) {
    if (it == user) {
      return (true);
    }
  }
  return (false);
}

void	Room::ARoom::addPlayer(Server::User *user) {
  this->_players.push_back(user);
}

void	Room::ARoom::addGroup(Server::Group *group) {
  for (auto it : group->getUsers()) {
    this->_players.push_back(it);
  }
}

void	Room::ARoom::kickPlayer(Server::User *user) {
  this->_players.remove(user);
}

void	Room::ARoom::setMap(const Room::MapInfos &map) {
  this->_map = map;
}

void	Room::ARoom::addPlayers(const std::list<Server::User*>& users)
{
  for (auto it : users)
    this->_players.push_back(it);
}
