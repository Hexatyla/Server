//
// ThreeVsThree.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Feb  7 21:49:23 2014 gautier lefebvre
// Last update Fri Feb  7 21:49:47 2014 gautier lefebvre
//

#include	"Room/ThreeVsThree.hpp"

Room::ThreeVsThree::ThreeVsThree():
  Room::AQuickLaunch::AQuickLaunch(Room::THREEVSTHREE)
{}

Room::ThreeVsThree::~ThreeVsThree() {}

bool		Room::ThreeVsThree::isReady() const {
  int		t1 = 0, t2 = 0;

  if (this->_players.empty() == true || this->_players.size() != 6) { return (false); }
  for (auto it : this->_players) {
    if (it->getTeam() == 1) {
      t1++;
    }
    else if (it->getTeam() == 2) {
      t2++;
    }
  }
  return (t1 == 3 && t2 == 3);
}
