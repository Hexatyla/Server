//
// ThreeVsThree.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Feb  7 21:47:50 2014 gautier lefebvre
// Last update Fri Feb  7 21:48:11 2014 gautier lefebvre
//

#ifndef		__ROOM_THREEVSTHREE_HPP__
#define		__ROOM_THREEVSTHREE_HPP__

#include	"Room/AQuickLaunch.hpp"

namespace	Room {
  class		ThreeVsThree :public Room::AQuickLaunch {
  public:
    ThreeVsThree();
    virtual ~ThreeVsThree();

    virtual bool	isReady() const;
  };
}

#endif
