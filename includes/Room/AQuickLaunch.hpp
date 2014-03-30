//
// AQuickLaunch.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Feb  7 21:23:04 2014 gautier lefebvre
// Last update Fri Feb  7 21:31:41 2014 gautier lefebvre
//

#ifndef		__ROOM_AQUICKLAUNCH_HPP__
#define		__ROOM_AQUICKLAUNCH_HPP__

#include	"Room/ARoom.hpp"

namespace	Room {
  class		AQuickLaunch :public Room::ARoom {
  public:
    AQuickLaunch(Room::Type t);
    virtual ~AQuickLaunch();

    virtual void	init();
    virtual void	setLockTeams(bool);
    virtual void	setCheats(bool);
    virtual void	setChat(bool);
    virtual bool	isReady() const = 0;
  };
}

#endif
