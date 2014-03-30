//
// Action.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sun Feb 16 16:18:51 2014 geoffrey bauduin
// Last update Sat Mar 15 14:26:00 2014 geoffrey bauduin
//

#ifndef SERVER_ACTION_HPP_
# define SERVER_ACTION_HPP_

#include	"Game/Action.hpp"
#include	"IFromFactory.hpp"
#include	"Threading/Mutex.hpp"

namespace	Server {

  class	Action: public ::Game::Action, virtual public IFromFactory {

  private:
    mutable Mutex	*_mutex;
    Server::Action	*_next;

  public:
    Action(void);
    virtual ~Action(void);

    void	init(::Game::eAction);
    virtual void destroy(void);

    void		setNextAction(Server::Action *);
    Server::Action	*getNextAction(void) const;

    bool	hasReferenceToOtherObject(Kernel::ID::id_t, ::Game::Type) const;

  };

}

#endif
