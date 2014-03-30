//
// GroupMove.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Thu Feb 27 00:04:15 2014 geoffrey bauduin
// Last update Sat Mar 15 15:06:27 2014 geoffrey bauduin
//

#ifndef SERVER_GROUPMOVE_HPP_
# define SERVER_GROUPMOVE_HPP_

#include	<list>
#include	"Threading/Mutex.hpp"
#include	"IFromFactory.hpp"

namespace	Server {

  class AItem;

  class	GroupMove: public IFromFactory {

  private:
    mutable Mutex	*_mutex;
    std::list<const Server::AItem *> _items;
    unsigned int	_currentSpeed;

  public:
    GroupMove(void);
    virtual ~GroupMove(void);

    void	init(const std::list<const Server::AItem *> &);
    virtual void	destroy(void);

    void	addItem(const Server::AItem *);
    bool	removeItem(const Server::AItem *);

    void	update(void);

    unsigned int	getSpeed(void) const;

  };

}

#endif
