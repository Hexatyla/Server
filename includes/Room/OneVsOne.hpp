//
// OneVsOne.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Feb  7 21:37:35 2014 gautier lefebvre
// Last update Fri Feb  7 21:49:49 2014 gautier lefebvre
//

#ifndef		__ROOM_ONEVSONE_HPP__
#define		__ROOM_ONEVSONE_HPP__

#include	"Room/AQuickLaunch.hpp"

namespace	Room {
  class		OneVsOne :public Room::AQuickLaunch {
  public:
    OneVsOne();
    virtual ~OneVsOne();

    virtual bool	isReady() const;
  };
}

#endif
