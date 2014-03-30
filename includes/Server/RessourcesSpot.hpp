//
// RessourcesSpot.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Mon Feb 10 13:44:45 2014 geoffrey bauduin
// Last update Wed Mar 26 11:12:45 2014 Gautier Lefebvre
//

#ifndef SERVER_RESSOURCES_SPOT_HPP_
# define SERVER_RESSOURCES_SPOT_HPP_

#include	<map>
#include	<deque>
#include	"Game/RessourcesSpot.hpp"
#include	"Server/AItem.hpp"
#include	"Clock.hpp"
#include	"IFromFactory.hpp"
#include	"Server/Waitlist.hpp"
#include	"Server/IHasReferenceToOtherObject.hpp"

namespace	Server {

  class	RessourcesSpot: public ::Game::RessourcesSpot, public Server::AItem, virtual public IFromFactory,
			virtual public Server::IHasReferenceToOtherObject {

  private:
    Mutex				*_mutex;
    std::map<Kernel::ID::id_t, double>	_harvestersClock;
    std::map<Kernel::ID::id_t, unsigned int>	_hCapa;
    Waitlist				_waitlist;

    void	onUpdate(void);
    void	moveOneFromWaitlist(void);
    void	onUnitAddedToHarvesterList(Kernel::ID::id_t);
    void	onUnitRemovedFromHarvesterList(Kernel::ID::id_t);
    void	onAddToWaitlist(Kernel::ID::id_t);
    void	onRemoveFromWaitlist(Kernel::ID::id_t);
    void	createUnitHarvestEvent(Kernel::ID::id_t, unsigned int);

    bool	_removeHarvester(Kernel::ID::id_t);

  protected:
    virtual void	onCreate(void);

  public:
    RessourcesSpot(void);
    virtual ~RessourcesSpot(void);

    void	init(Kernel::ID::id_t, double, double, double, ::Game::Ressources::Type,
		     unsigned int amount = 0);
    virtual void	destroy(void);

    virtual void	addHarvester(Kernel::ID::id_t, unsigned int amount = 0);
    virtual void	removeHarvester(Kernel::ID::id_t);

    Protocol::Job *getRessJob(void) const;

    virtual bool	update(const Clock *, double);

    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);

  };

}

#endif
