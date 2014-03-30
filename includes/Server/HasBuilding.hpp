//
// HasBuilding.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 17:58:51 2014 geoffrey bauduin
// Last update Sat Mar 15 15:15:16 2014 geoffrey bauduin
//

#ifndef SERVER_HASBUILDING_HPP_
# define SERVER_HASBUILDING_HPP_

#include	<map>
#include	"Threading/Mutex.hpp"
#include	"Server/Building.hpp"
#include	"IFromFactory.hpp"

namespace	Server {

  class	HasBuilding: virtual public IFromFactory {

  protected:
    Mutex					*_buildingMutex;
    std::map<Kernel::ID::id_t, Server::Building *>	_buildings;

    HasBuilding(void);
    virtual ~HasBuilding(void);

    void	init(void);
    virtual void destroy(void);

    void	addBuilding(Server::Building *);
    void	removeBuilding(Server::Building *);
    void	removeBuilding(Kernel::ID::id_t);

  };

}


#endif
