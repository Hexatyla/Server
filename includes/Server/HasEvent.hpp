//
// HasEvent.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 17:07:06 2014 geoffrey bauduin
// Last update Sat Mar 15 15:17:31 2014 geoffrey bauduin
//

#ifndef SERVER_HASEVENT_HPP_
# define SERVER_HASEVENT_HPP_

#include	<list>
#include	"Server/Event.hpp"
#include	"Threading/Mutex.hpp"
#include	"IFromFactory.hpp"
#include	"Server/IHasReferenceToOtherObject.hpp"

namespace	Server {

  class	HasEvent: virtual public IFromFactory, virtual public Server::IHasReferenceToOtherObject {

  private:
    std::list<Server::Event *>	_events;
    Mutex			*_eventMutex;

  public:
    HasEvent(void);
    ~HasEvent(void);

    void	init(void);
    virtual void	destroy(void);

    void	clearEvents(void);
    void	addEvent(Server::Event *);

    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);

    void	onReferenceDeleted(void *);

    unsigned int size(void) const;

    Server::Event *popEvent(void);

  };

}

#endif
