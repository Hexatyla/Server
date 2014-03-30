//
// Movable.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Fri Feb 14 21:31:49 2014 geoffrey bauduin
// Last update Wed Mar 26 16:05:36 2014 geoffrey bauduin
//

#ifndef SERVER_MOVABLE_HPP_
# define SERVER_MOVABLE_HPP_

#include	"Game/Movable.hpp"
#include	"Threading/Mutex.hpp"
#include	"IFromFactory.hpp"
#include	"Server/HasJobs.hpp"
#include	"Server/HasEvent.hpp"
#include	"Server/IUpdatable.hpp"
#include	"Server/Action.hpp"
#include	"Vector.hpp"
#include	"Server/AItem.hpp"

#define	BLOCKED_PTR_FUNC		5
#define	M_UPDATE_PTR_FUNC		2
#define	DESTINATION_REACHED_PTR_FUNC	2

namespace	Server {

  class	Movable: virtual public ::Game::Movable, virtual public IFromFactory, virtual public Server::IUpdatable,
		 virtual public Server::HasEvent, virtual public Server::HasJobs, virtual public ::Game::AElement {

  private:
    Mutex	*_mutex;
    bool	_mustUpdate;
    Vector3d	_oldFollowingPos;
    int		_stopCondition;
    unsigned int	_recalc;
  protected:
    bool	mustBeUpdated(void) const;
    void	resetUpdateStatus(void);
    void	waypointJob(void);

    void	onChangeWaypoint(void);

  private:
    struct blockedPtrFunc {
      ::Game::eAction	action;
      void (Server::Movable::*func)(Server::Action *, Server::Event **);
    };

    struct mUpdatePtrFunc {
      ::Game::eAction	action;
      void (Server::Movable::*func)(const Clock *, double);
    };

    struct destinationReachedPtrFunc {
      ::Game::eAction	action;
      void (Server::Movable::*func)(void);
    };

    void	updateByAction(const Clock *, double);

    void	updateMoveAction(const Clock *, double);
    void	updateProjectileFollowAction(const Clock *, double);

    static const mUpdatePtrFunc _mUpdatePtrFunc[M_UPDATE_PTR_FUNC];

    void	blockedNextActionRelease(Server::Action *, Server::Event **);
    void	blockedPickUp(Server::Action *, Server::Event **);
    void	blockedBuilding(Server::Action *, Server::Event **);
    void	blockedDeposit(Server::Action *, Server::Event **);
    void	blockedStartHarvest(Server::Action *, Server::Event **);

    void	_setOrientation(int);

    static const blockedPtrFunc _blockedPtrFunc[BLOCKED_PTR_FUNC];

    void		endActualAction(void);

    static const destinationReachedPtrFunc _destinationReachedPtrFunc[DESTINATION_REACHED_PTR_FUNC];

  protected:
    virtual void	_stopAction(void);

  public:
    enum	StopCondition {
      ATTACK_RANGE = 0b001,
      VISION =       0b010,
      NONE =         0b000
    };

    Movable(void);
    virtual ~Movable(void);

    void		init(void);
    virtual void	destroy(void);
    virtual void	setX(double);
    virtual void	setY(double);
    virtual void	setZ(double);
    virtual void	setPosition(double, double, double);
    virtual void	setOrientation(int);
    virtual void	setWaypoints(const std::vector<Vector3d> &);

    virtual bool	update(const Clock *, double);

    virtual void	onDestinationReached(void);

    bool	mustGeneratePath(const Server::AItem *);
    bool	mustGeneratePath(void) const;

    void	onStartMove(double, double, double);
    void	onBlocked(void);
    void	waypointReached(void);

    void	setStopCondition(int);
    int		getStopCondition(void) const;

  };

}

#endif
