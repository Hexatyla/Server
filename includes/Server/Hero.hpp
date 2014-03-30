//
// Hero.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 17:19:53 2014 geoffrey bauduin
// Last update Sat Mar 15 16:11:32 2014 geoffrey bauduin
//

#ifndef SERVER_HERO_HPP_
# define SERVER_HERO_HPP_

#include	"Game/Hero.hpp"
#include	"Server/AItem.hpp"
#include	"Server/Unit.hpp"
#include	"Server/Object.hpp"
#include	"IFromFactory.hpp"
#include	"Threading/Mutex.hpp"

#define	H_UPDATE_ACTION_PTR_FUNC	2

namespace	Server {

  class	Hero: public ::Game::Hero, public Server::Unit, virtual public IFromFactory {

  private:
    Mutex	*_mutex;

    void	onPickedUpObject(Kernel::ID::id_t);
    void	onReleasedObject(Kernel::ID::id_t);

    struct hUpdateActionPtrFunc {
      ::Game::eAction	action;
      void	(Server::Hero::*func)(const Clock *, double);
    };

    static const hUpdateActionPtrFunc _hUpdateActionPtrFunc[H_UPDATE_ACTION_PTR_FUNC];

    void	updateReleaseAction(const Clock *, double);
    void	updatePickUpAction(const Clock *, double);

    void	updateByAction(const Clock *, double);

  public:
    Hero(void);
    virtual ~Hero(void);

    void		init(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
			     int orientation, double x, double y, double z);
    virtual void	destroy(void);
    virtual bool	update(const Clock *, double);

    virtual void	addObject(::Game::Object *);
    virtual void	removeObject(::Game::Object *);

  };

}

#endif
