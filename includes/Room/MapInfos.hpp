//
// RoomInfos.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Feb 12 21:18:53 2014 gautier lefebvre
// Last update Thu Feb 13 13:12:34 2014 gautier lefebvre
//

#ifndef		__ROOM_MAPINFOS_HPP__
#define		__ROOM_MAPINFOS_HPP__

#include	<string>

namespace	Room {
  struct        MapInfos {
    std::string	fullpath;
    std::string	filename;
    std::string	title;

    MapInfos(const std::string &, const std::string &, const std::string &);
    MapInfos(const MapInfos &);
    MapInfos& operator=(const MapInfos&);
    ~MapInfos();
  };
}

#endif
