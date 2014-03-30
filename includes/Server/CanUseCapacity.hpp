//
// Canusecapacity.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Fri Feb 21 11:08:13 2014 geoffrey bauduin
// Last update Wed Mar 26 11:18:27 2014 Gautier Lefebvre
//

#ifndef SERVER_CANUSECAPACITY_HPP_
# define SERVER_CANUSECAPACITY_HPP_

#include	"Game/CanUseCapacity.hpp"
#include	"Server/HasJobs.hpp"
#include	"IFromFactory.hpp"
#include	"Threading/Mutex.hpp"
#include	"Game/AElement.hpp"
#include	"Server/IUpdatable.hpp"
#include	"Server/HasEvent.hpp"

#define	C_UPDATE_PTR_FUNC	5

namespace	Server {

  class AItem;

  class	CanUseCapacity: virtual public ::Game::CanUseCapacity, virtual public IFromFactory,
			virtual public Server::IUpdatable, virtual public Server::HasJobs,
			virtual public Server::HasEvent {

  private:
    mutable Mutex	*_mutex;
    Clock	*_last;

    double	_castTime;

    void	onSetTarget(void);
    void	onSetTargetZone(void);

    struct cUpdatePtrFunc {
      ::Game::eAction	action;
      void	(Server::CanUseCapacity::*func)(const Clock *, double);
    };

    void	updateAttack(const Clock *, double);
    void	updateStartCapacity(const Clock *, double);
    void	updateCastCapacity(const Clock *, double);

    static const cUpdatePtrFunc _cUpdatePtrFunc[C_UPDATE_PTR_FUNC];

    bool	_useCapacity(const Kernel::Serial &);

    void	createProjectileTarget(const Kernel::Serial &, const Server::AItem *);
    void	createProjectileZone(const Kernel::Serial &, double, double, double);

    void	onUseCapacity(const Kernel::Serial &);
    void	onUseCapacity(const Kernel::Serial &, Kernel::ID::id_t);
    void	onUseCapacity(const Kernel::Serial &, double, double, double);

  public:
    CanUseCapacity(void);
    virtual ~CanUseCapacity(void);

    void	init(void);
    virtual void	destroy(void);

    virtual bool	update(const Clock *, double);

    virtual void	setTarget(Kernel::ID::id_t, ::Game::Type);
    virtual void	setTarget(double, double, double);

    void		useCapacityTarget(const Server::AItem *, const Kernel::Serial &);
    void		useCapacityZone(const Kernel::Serial &, double, double, double);
    void		useCapacity(const Kernel::Serial &);

  };

}

#endif
