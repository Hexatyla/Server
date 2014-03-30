//
// Controller.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Sun Feb  9 20:28:58 2014 gautier lefebvre
// Last update Sun Feb  9 20:36:56 2014 gautier lefebvre
//

#include	"Threading/ScopeLock.hpp"
#include	"Server/Controller.hpp"

Server::Controller::Controller():
  _m(new Mutex()),
  _groups(NULL),
  _rooms(NULL)
{}

Server::Controller::~Controller() {}

void		Server::Controller::setGroupList(const std::list<Server::Group*>* g) { _groups = g; }
void		Server::Controller::setRoomList(const std::list<Room::ARoom*>* r) { _rooms = r; }
