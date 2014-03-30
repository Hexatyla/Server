//
// RoomList.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Feb 12 20:40:12 2014 gautier lefebvre
// Last update Fri Mar  7 14:33:36 2014 gautier lefebvre
//

#ifndef		__ROOM_MAPLIST_HPP__
#define		__ROOM_MAPLIST_HPP__

#include	<vector>
#include	"Singleton.hpp"
#include	"Room/MapInfos.hpp"

namespace	Room {
  static const std::string MAPS_FOLDER = "races/";

  class		MapList:
    public Singleton<Room::MapList> {
    friend class Singleton<Room::MapList>;

  private:
    std::vector<Room::MapInfos>	_maps;

    std::string	getMapTitleFromFileName(const std::string&) const;
    bool	isFileNameCorrect(const std::string &) const;

    MapList();
    virtual ~MapList();

  public:
    void	init();

    const std::vector<Room::MapInfos>	&getMaps() const;
  };
}

#endif
