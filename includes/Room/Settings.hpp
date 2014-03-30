//
// Settings.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Mon Jan 20 14:31:38 2014 gautier lefebvre
// Last update Fri Feb  7 21:56:25 2014 gautier lefebvre
//

#ifndef		__ROOM_SETTINGS_HPP__
#define		__ROOM_SETTINGS_HPP__

namespace	Room {
  struct	Settings {
    bool	lockedTeams;
    bool	cheat;
    bool	chat;

    Settings(bool lt = true, bool cheat = false, bool chat = true);
    ~Settings();
  };
}

#endif
