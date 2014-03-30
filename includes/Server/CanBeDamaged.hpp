//
// CanBeDamaged.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Sun Feb 23 00:37:49 2014 geoffrey bauduin
// Last update Sat Mar 15 14:58:51 2014 geoffrey bauduin
//

#ifndef SERVER_CANBEDAMAGED_HPP_
# define SERVER_CANBEDAMAGED_HPP_

#include	<map>
#include	"Game/AElement.hpp"
#include	"Threading/Mutex.hpp"
#include	"IFromFactory.hpp"
#include	"Server/IHasReferenceToOtherObject.hpp"
#include	"Server/AItem.hpp"
#include	"Server/IUpdatable.hpp"

namespace	Server {

  class CanBeDamaged: virtual public IFromFactory, virtual public ::Game::AElement,
		      virtual public Server::IHasReferenceToOtherObject, virtual public Server::IUpdatable {

  private:
    mutable Mutex				*_mutex;
    std::map<Kernel::ID::id_t, unsigned int>	_damagedBy;
    Clock				*_lastDamage;
    bool				_damaged;

  public:
    CanBeDamaged(void);
    virtual ~CanBeDamaged(void);

    void	init(void);
    virtual void	destroy(void);

    void		takeDamage(const Server::AItem *);
    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);

    const std::map<Kernel::ID::id_t, unsigned int>	&getAttackers(void) const;

    virtual bool	update(const Clock *, double);

  };

}

#endif
