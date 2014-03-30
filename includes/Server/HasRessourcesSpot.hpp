//
// HasRessourcesSpot.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 17:58:51 2014 geoffrey bauduin
// Last update Sat Mar 15 15:25:04 2014 geoffrey bauduin
//

#ifndef SERVER_HASRESSOURCESSPOT_HPP_
# define SERVER_HASRESSOURCESSPOT_HPP_

#include	<map>
#include	"Threading/Mutex.hpp"
#include	"Server/RessourcesSpot.hpp"
#include	"IFromFactory.hpp"

namespace	Server {

  class	HasRessourcesSpot: virtual public IFromFactory {

  protected:
    Mutex					*_ressourcesSpotMutex;
    std::map<Kernel::ID::id_t, Server::RessourcesSpot *>	_ressourcesSpots;

    HasRessourcesSpot(void);
    virtual ~HasRessourcesSpot(void);

    void	init(void);
    virtual void destroy(void);
    void	addRessourcesSpot(Server::RessourcesSpot *);
    void	removeRessourcesSpot(Server::RessourcesSpot *);
    void	removeRessourcesSpot(Kernel::ID::id_t);

  };

}


#endif
