//
// Waitlist.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Feb 18 20:03:52 2014 geoffrey bauduin
// Last update Tue Feb 18 22:01:28 2014 geoffrey bauduin
//

#ifndef SERVER_WAITLIST_HPP_
# define SERVER_WAITLIST_HPP_

#include	<deque>
#include	"IFromFactory.hpp"
#include	"Threading/Mutex.hpp"
#include	"Kernel/ID.hpp"

namespace	Server {

  class	Waitlist: virtual public IFromFactory {

  private:
    Mutex	*_mutex;
    std::deque<Kernel::ID::id_t>	_waitlist;

  public:
    Waitlist(void);
    virtual ~Waitlist(void);

    void	init(void);
    virtual void	destroy(void);

    void	push(Kernel::ID::id_t);
    void	remove(Kernel::ID::id_t);
    bool	isInWaitlist(Kernel::ID::id_t) const;
    const std::deque<Kernel::ID::id_t> &getWaitlist(void) const;
    Kernel::ID::id_t	pop(void);
    bool	empty(void) const;

  };

}

#endif
