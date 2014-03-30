//
// AQuickLaunch.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Feb  7 21:30:26 2014 gautier lefebvre
// Last update Fri Mar  7 14:58:55 2014 gautier lefebvre
//

#include	"Room/AQuickLaunch.hpp"
#include	"Room/MapList.hpp"
#include	"Logger.hpp"

Room::AQuickLaunch::AQuickLaunch(Room::Type t):
  Room::ARoom::ARoom(t)
{
  this->init();
}

Room::AQuickLaunch::~AQuickLaunch() {}

void		Room::AQuickLaunch::init() {
  Room::MapList*	maplist = Room::MapList::getInstance();
  if (maplist && maplist->getMaps().empty() == false) {
    int nb = rand() % maplist->getMaps().size();
    this->_map = (maplist->getMaps())[nb];
  }
  else {
    Logger::getInstance()->log("No maps to init the room", Logger::FATAL);
  }
}

void		Room::AQuickLaunch::setLockTeams(bool) {}
void		Room::AQuickLaunch::setCheats(bool) {}
void		Room::AQuickLaunch::setChat(bool) {}
