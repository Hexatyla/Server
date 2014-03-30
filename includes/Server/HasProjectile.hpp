//
// HasProjectile.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 17:58:51 2014 geoffrey bauduin
// Last update Sat Mar 15 15:22:41 2014 geoffrey bauduin
//

#ifndef SERVER_HASPROJECTILE_HPP_
# define SERVER_HASPROJECTILE_HPP_

#include	<map>
#include	"Threading/Mutex.hpp"
#include	"Server/Projectile.hpp"
#include	"IFromFactory.hpp"

namespace	Server {

  class	HasProjectile: virtual public IFromFactory {

  protected:
    Mutex					*_projectileMutex;
    std::map<Kernel::ID::id_t, Server::Projectile *>	_projectiles;

    HasProjectile(void);
    virtual ~HasProjectile(void);

    void	init(void);
    virtual void	destroy(void);
    void	addProjectile(Server::Projectile *);
    void	removeProjectile(Server::Projectile *);
    void	removeProjectile(Kernel::ID::id_t);

  };

}


#endif
