//
// Unit.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 17:19:53 2014 geoffrey bauduin
// Last update Wed Mar 26 11:16:18 2014 Gautier Lefebvre
//

#ifndef SERVER_UNIT_HPP_
# define SERVER_UNIT_HPP_

#include	"Game/Unit.hpp"
#include	"Server/HasEvent.hpp"
#include	"Server/AItem.hpp"
#include	"IFromFactory.hpp"
#include	"Server/Movable.hpp"
#include	"Server/CanUseCapacity.hpp"
#include	"Server/CanBeDamaged.hpp"
#include	"Server/IHasReferenceToOtherObject.hpp"

#define	U_UPDATE_ACTION_PTR_FUNC	4

namespace	Server {

  class	Unit: virtual public ::Game::Unit, public Server::AItem,
	      public Server::Movable, virtual public IFromFactory, public Server::CanUseCapacity,
	      public Server::CanBeDamaged, virtual public Server::IHasReferenceToOtherObject {

  private:
    Mutex		*_mutex;
    bool		_initialized;
    bool		_targeting;
    Mutex		*_inRangeMutex;

    void		onStanceChange(void);
    void		onRessourcesChange(void);

    void		onStatsChanged(void);

    struct uUpdateActionPtrFunc {
      ::Game::eAction action;
      void (Server::Unit::*func)(const Clock *, double);
    };

    void	updateByAction(const Clock *, double);

    void	updateActionStartBuilding(const Clock *, double);
    void	updateActionDeposit(const Clock *, double);
    void	updateActionStartHarvest(const Clock *, double);
    void	updatePatrolAction(const Clock *, double);

    static const uUpdateActionPtrFunc _uUpdateActionPtrFunc[U_UPDATE_ACTION_PTR_FUNC];

    virtual Protocol::Job *createInfosJob(void) const;

  protected:
    virtual void	setXP(unsigned int);
    virtual void	setLevel(unsigned int);

    virtual void	_stopAction(void);

  public:
    Unit(void);
    virtual ~Unit(void);

    void		init(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
			     int orientation, double x, double y, double z, ::Game::Type type = ::Game::UNIT);
    virtual void	destroy(void);
    virtual bool	update(const Clock *, double);

    virtual void	itemInRange(const Server::AItem *);
    virtual void	addRessources(::Game::Ressources::Type, int);
    virtual void	setRessources(::Game::Ressources::Type, unsigned int);
    virtual void	removeRessources(::Game::Ressources::Type, int);

    unsigned int	depositRessources(::Game::Ressources::Type);

    virtual void	onDestinationReached(void);
    virtual void	onBlocked(void);

    virtual void	onAttacked(const Server::AItem *);

    virtual void	setStance(::Game::Stance::Type);

    virtual void	stopAction(void);

    Protocol::Job *getRessJob(void) const;

    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);

    virtual void	addXP(unsigned int);

  };

}

#endif
