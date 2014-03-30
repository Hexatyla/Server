//
// Projectile.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 17:19:53 2014 geoffrey bauduin
// Last update Tue Mar 25 10:19:08 2014 geoffrey bauduin
//

#ifndef SERVER_PROJECTILE_HPP_
# define SERVER_PROJECTILE_HPP_

#include	"Game/Projectile.hpp"
#include	"Server/AItem.hpp"
#include	"IFromFactory.hpp"
#include	"Server/Movable.hpp"
#include	"Threading/Mutex.hpp"
#include	"Server/IHasReferenceToOtherObject.hpp"

namespace	Server {

  class	Projectile: public ::Game::Projectile, public Server::AItem,
		    public Server::Movable, virtual public IFromFactory,
		    virtual public Server::IHasReferenceToOtherObject {

  private:
    Mutex	*__mutex;

  protected:
    virtual void	_stopAction(void);

  public:
    Projectile(void);
    virtual ~Projectile(void);

    void		init(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
			     int orientation, double x, double y, double z);
    virtual void	destroy(void);
    virtual bool	update(const Clock *, double);

    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);

  };

}

#endif
