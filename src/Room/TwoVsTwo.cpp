//
// TwoVsTwo.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Feb  7 21:48:29 2014 gautier lefebvre
// Last update Fri Feb  7 21:49:03 2014 gautier lefebvre
//

#include	"Room/TwoVsTwo.hpp"

Room::TwoVsTwo::TwoVsTwo():
  Room::AQuickLaunch::AQuickLaunch(Room::TWOVSTWO)
{}

Room::TwoVsTwo::~TwoVsTwo() {}

bool		Room::TwoVsTwo::isReady() const {
  int		t1 = 0, t2 = 0;

  if (this->_players.empty() == true || this->_players.size() != 4) { return (false); }
  for (auto it : this->_players) {
    if (it->getTeam() == 1) {
      t1++;
    }
    else if (it->getTeam() == 2) {
      t2++;
    }
  }
  return (t1 == 2 && t2 == 2);
}
