//
// TwoVsTwo.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Feb  7 21:47:08 2014 gautier lefebvre
// Last update Fri Feb  7 21:47:45 2014 gautier lefebvre
//

#ifndef		__ROOM_TWOVSTWO_HPP__
#define		__ROOM_TWOVSTWO_HPP__

#include	"Room/AQuickLaunch.hpp"

namespace	Room {
  class		TwoVsTwo :public Room::AQuickLaunch {
  public:
    TwoVsTwo();
    virtual ~TwoVsTwo();

    virtual bool	isReady() const;
  };
}

#endif
