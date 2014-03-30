//
// Controller.hpp<2> for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Sun Feb  9 17:50:43 2014 gautier lefebvre
// Last update Mon Feb 10 13:56:46 2014 gautier lefebvre
//

#ifndef		__SERVER_CONTROLLER_HPP__
#define		__SERVER_CONTROLLER_HPP__

#include	"Singleton.hpp"
#include	"Threading/Mutex.hpp"
#include	"Server/User.hpp"
#include	"Server/Group.hpp"
#include	"Room/ARoom.hpp"

namespace	Server {
  class		Controller: public Singleton<Server::Controller> {
    friend class Singleton<Server::Controller>;

  private:
    Mutex*	_m;
    const std::list<Server::Group*>*	_groups;
    const std::list<Room::ARoom*>*	_rooms;

    Controller(void);
    virtual ~Controller(void);

  public:
    void	setGroupList(const std::list<Server::Group*>* g);
    void	setRoomList(const std::list<Room::ARoom*>* r);
  };
}

#endif
