//
// Effect.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Feb 11 21:48:33 2014 geoffrey bauduin
// Last update Wed Mar 26 11:17:22 2014 Gautier Lefebvre
//

#ifndef SERVER_EFFECT_HPP_
# define SERVER_EFFECT_HPP_

#include	"Game/Effect.hpp"
#include	"Clock.hpp"
#include	"Server/IUpdatable.hpp"
#include	"IFromFactory.hpp"
#include	"Threading/Mutex.hpp"

namespace	Server {

  class	Effect: public ::Game::Effect, public Server::IUpdatable, virtual public IFromFactory {

  private:
    mutable Mutex	*_mutex;
    Clock	*_start;

  public:
    Effect(void);
    virtual ~Effect(void);

    void	init(const Kernel::Serial &, const ::Game::Player *, Kernel::ID::id_t);
    virtual void	destroy(void);
    virtual bool	update(const Clock *, double);

  };

}

#endif
