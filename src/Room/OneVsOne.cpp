//
// OneVsOne.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Feb  7 21:41:58 2014 gautier lefebvre
// Last update Fri Feb  7 21:48:33 2014 gautier lefebvre
//

#include	"Room/OneVsOne.hpp"

Room::OneVsOne::OneVsOne():
  Room::AQuickLaunch::AQuickLaunch(Room::ONEVSONE)
{}

Room::OneVsOne::~OneVsOne() {}

bool		Room::OneVsOne::isReady() const {
  int		t1 = 0, t2 = 0;

  if (this->_players.empty() == true || this->_players.size() != 2) { return (false); }
  for (auto it : this->_players) {
    if (it->getTeam() == 1) {
      t1++;
    }
    else if (it->getTeam() == 2) {
      t2++;
    }
  }
  return (t1 == 1 && t2 == 1);
}
