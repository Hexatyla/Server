//
// HasObject.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 17:58:51 2014 geoffrey bauduin
// Last update Sat Mar 15 15:19:33 2014 geoffrey bauduin
//

#ifndef SERVER_HASOBJECT_HPP_
# define SERVER_HASOBJECT_HPP_

#include	<map>
#include	"Threading/Mutex.hpp"
#include	"Server/Object.hpp"
#include	"IFromFactory.hpp"

namespace	Server {

  class	HasObject: virtual public IFromFactory {

  protected:
    Mutex					*_objectMutex;
    std::map<Kernel::ID::id_t, Server::Object *>	_objects;

    HasObject(void);
    virtual ~HasObject(void);

    void	init(void);
    virtual void destroy(void);

    void	addObject(Server::Object *);
    void	removeObject(Server::Object *);
    void	removeObject(Kernel::ID::id_t);

  };

}


#endif
