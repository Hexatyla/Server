//
// HasHero.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 17:58:51 2014 geoffrey bauduin
// Last update Sat Mar 15 15:18:15 2014 geoffrey bauduin
//

#ifndef SERVER_HASHERO_HPP_
# define SERVER_HASHERO_HPP_

#include	<map>
#include	"Threading/Mutex.hpp"
#include	"Server/Hero.hpp"
#include	"IFromFactory.hpp"

namespace	Server {

  class	HasHero: virtual public IFromFactory {

  protected:
    Mutex					*_heroMutex;
    std::map<Kernel::ID::id_t, Server::Hero *>	_heroes;

    HasHero(void);
    virtual ~HasHero(void);

    void	init(void);
    virtual void	destroy(void);
    void	addHero(Server::Hero *);
    void	removeHero(Server::Hero *);
    void	removeHero(Kernel::ID::id_t);

  };

}


#endif
