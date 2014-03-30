//
// ARoom.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Mon Jan 20 14:22:20 2014 gautier lefebvre
// Last update Thu Feb 13 00:24:07 2014 gautier lefebvre
//

#ifndef		__ROOM_AROOM_HPP__
#define		__ROOM_AROOM_HPP__

#include	<list>

#include	"Server/Group.hpp"
#include	"Room/Settings.hpp"
#include	"Room/Type.hpp"
#include	"Room/MapInfos.hpp"

namespace	Room {
  class		ARoom {
  protected:
    Room::Type			_type;
    std::list<Server::User*>	_players;
    Settings			_settings;
    MapInfos			_map;

  public:
    ARoom(Room::Type);
    virtual ~ARoom();

    Room::Type		getType() const;
    const Settings	&getSettings() const;
    const MapInfos	&getMap() const;
    const std::list<Server::User*> &getPlayers() const;
    bool		isUserInRoom(const Server::User *) const;
    bool		isEmpty() const;

    virtual void	init() = 0;

    virtual void	setLockTeams(bool) = 0;
    virtual void	setCheats(bool) = 0;
    virtual void	setChat(bool) = 0;
    virtual bool	isReady() const = 0;

    virtual void	addPlayer(Server::User *);
    virtual void	addPlayers(const std::list<Server::User*>&);
    virtual void	addGroup(Server::Group *);
    virtual void	kickPlayer(Server::User *);
    virtual void	setMap(const MapInfos &);
  };
}

#endif
