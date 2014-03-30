//
// Object.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Feb  5 13:29:50 2014 geoffrey bauduin
// Last update Sat Mar 15 15:28:50 2014 geoffrey bauduin
//

#ifndef SERVER_OBJECT_HPP_
# define SERVER_OBJECT_HPP_

#include	"Game/Object.hpp"
#include	"Server/AItem.hpp"
#include	"Server/IHasReferenceToOtherObject.hpp"
#include	"Threading/Mutex.hpp"

namespace	Server {

  class	Object: public ::Game::Object, public Server::AItem, virtual public IFromFactory,
		virtual public Server::IHasReferenceToOtherObject {

  private:
    mutable Mutex	*_mutex;

  public:
    Object(void);
    virtual ~Object(void);

    void	init(Kernel::ID::id_t id, const Kernel::Serial &serial, int orientation, double x, double y, double z);
    virtual void	destroy(void);

    virtual void	setPosition(double, double, double);

    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);

  };

}

#endif
