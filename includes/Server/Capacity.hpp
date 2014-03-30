//
// Capacity.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Wed Feb 12 15:32:34 2014 geoffrey bauduin
// Last update Sat Mar 15 15:02:37 2014 geoffrey bauduin
//

#ifndef SERVER_CAPACITY_HPP_
# define SERVER_CAPACITY_HPP_

#include	"Game/Capacity.hpp"
#include	"Server/HasJobs.hpp"
#include	"Server/IUpdatable.hpp"
#include	"IFromFactory.hpp"

namespace	Server {

  class	Capacity: public ::Game::Capacity, public Server::HasJobs,
		  public Server::IUpdatable, virtual public IFromFactory {

  private:
    mutable Mutex	*_mutex;
    void	onSetAutomatic(void);

  public:
    Capacity(void);
    virtual ~Capacity(void);

    void	init(const Kernel::Serial &, bool, const ::Game::Player *, Kernel::ID::id_t);
    virtual void	destroy(void);

    virtual bool	update(const Clock *, double);
    virtual void	setAutomatic(bool);

  };

}

#endif
