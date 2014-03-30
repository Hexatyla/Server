//
// MatchMaking.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Feb  7 22:42:50 2014 gautier lefebvre
// Last update Fri Mar  7 15:17:41 2014 gautier lefebvre
//

#ifndef		__MATCHMAKING_HPP__
#define		__MATCHMAKING_HPP__

#include	"Server/User.hpp"
#include	"Protocol/Job.hpp"
#include	"Room/ARoom.hpp"
#include	"Room/OneVsOne.hpp"
#include	"Room/TwoVsTwo.hpp"
#include	"Room/ThreeVsThree.hpp"
#include	"Factory/Factory.hpp"
#include	"Factory/Protocol.hpp"
#include	"Network/Socket.hpp"

class		MatchMaking {
private:
  static void		_ifNoRoom(std::list<Room::ARoom*>&, Room::Type, const std::list<Server::User*>&);
  static  const std::list<Server::User*>&	_setSettingsToUsers(const std::list<Server::User*>&, const std::list<Server::User*>&, int);
  static  void		_sendJobGameSearching(const std::list<Server::User*>&);
  static  void		_sendJobEndGameSearching(const std::list<Server::User*>&);
  static  bool		_isFirstCanFightWithSecond(Server::User*, Server::User*);
  static  bool		_isColorAlreadyChosed(const std::list<Server::User*>&, const std::list<Server::User*>&, Color::Color);

  static  bool		_isEnoughPlace(Room::ARoom *, const std::list<Server::User*>&);
public:
  static void	run(Room::Type t, std::list<Room::ARoom*>& rooms, const std::list<Server::User*>& players);
};

#endif
