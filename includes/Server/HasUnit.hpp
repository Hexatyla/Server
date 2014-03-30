//
// HasUnit.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 17:58:51 2014 geoffrey bauduin
// Last update Sat Mar 15 15:26:08 2014 geoffrey bauduin
//

#ifndef SERVER_HASUNIT_HPP_
# define SERVER_HASUNIT_HPP_

#include	<map>
#include	"Threading/Mutex.hpp"
#include	"Server/Unit.hpp"
#include	"IFromFactory.hpp"

namespace	Server {

  class	HasUnit: virtual public IFromFactory {

  protected:
    Mutex					*_unitMutex;
    std::map<Kernel::ID::id_t, Server::Unit *>	_units;

    HasUnit(void);
    virtual ~HasUnit(void);

    void	init(void);
    virtual void destroy(void);
    void	addUnit(Server::Unit *);
    void	removeUnit(Server::Unit *);
    void	removeUnit(Kernel::ID::id_t);

  };

}


#endif
