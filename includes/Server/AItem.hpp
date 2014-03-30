//
// Item.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 16:06:04 2014 geoffrey bauduin
// Last update Wed Mar 26 11:16:41 2014 Gautier Lefebvre
//

#ifndef SERVER_ITEM_HPP_
# define SERVER_ITEM_HPP_

#include	"Server/HasEvent.hpp"
#include	"Clock.hpp"
#include	"Server/HasJobs.hpp"
#include	"Kernel/ID.hpp"
#include	"Kernel/Serial.hpp"
#include	"Game/AElement.hpp"
#include	"Protocol/Job.hpp"
#include	"Server/Effect.hpp"
#include	"Server/IUpdatable.hpp"
#include	"IFromFactory.hpp"
#include	"Server/IHasReferenceToOtherObject.hpp"
#include	"Server/GroupMove.hpp"
#include	"NoDeleteWhileUsedByAThread.hpp"

#define	UPDATE_ACTION_PTR_FUNC	0

namespace	Server {

  class	AItem: virtual public ::Game::AElement, virtual public Server::HasEvent, virtual public Server::HasJobs,
	       virtual public Server::IUpdatable, virtual public IFromFactory,
	       virtual public Server::IHasReferenceToOtherObject, public NoDeleteWhileUsedByAThread {

  private:
    mutable Mutex	*_dataMutex;
    mutable Mutex	*_effectMutex;
    mutable Mutex	*_moveMutex;

    Server::GroupMove *_grpMove;
    Clock *_lastUpdate;

    void	leaveGroupMove(void);

  protected:
    Clock	*_lastHpRegen;
    Clock	*_lastRessourceRegen;
    bool	_destroyed;
    bool	_mustUpdate;

    virtual void	onCreate(void);
    void		onUpdate(void);
    void		onDestroy(void);
    void		onSetAction(const ::Game::Action *);
    void		onStopAction(void);
    void		onAddEffect(const Kernel::Serial &);
    void		onRemoveEffect(const Kernel::Serial &);
    void		onAddTemporaryEffect(const ::Game::Effect *);
    void		onRemoveTemporaryEffect(const ::Game::Effect *);

    void		createEffectTimeoutEvent(Server::Effect *);

    virtual Protocol::Job	*createInfosJob(void) const;
    Protocol::Job	*createItemActionJob(const ::Game::Action *) const;

  private:
    void	updateLife(const Clock *, double);
    void	updatePersonnalRessources(const Clock *, double);
    void	updateEffects(const Clock *, double);
    void	updateCapacities(const Clock *, double);
    void	updateByAction(const Clock *, double);

  protected:
    virtual void	_setAction(::Game::Action *);
    virtual void	_stopAction(void);

  private:

    struct updateActionPtrFunc {
      ::Game::eAction	action;
      void	(Server::AItem::*func)(const Clock *, double);
    };

    static const updateActionPtrFunc	_updateActionPtrFunc[UPDATE_ACTION_PTR_FUNC];

  protected:
    virtual void	_endAction(void);

  public:
    AItem(void);
    virtual ~AItem(void);

    void		init(void);
    virtual void	destroy(void);

    void	lockMove(void);
    void	unlockMove(void);

    virtual bool	update(const Clock *, double);

    virtual void	setAction(::Game::Action *);
    virtual void	stopAction(void);
    void		endAction(void);

    Protocol::Job	*onAskInfos(void) const;
    Protocol::Job	*onAskItemAction(void) const;

    virtual void	addLife(unsigned int);
    virtual void	removeLife(unsigned int);

    virtual void	addEffect(const Kernel::Serial &);
    virtual void	addEffect(const ::Game::Effect *);
    virtual void	removeEffect(const ::Game::Effect *);
    virtual void	removeEffect(const Kernel::Serial &);

    virtual void	onAttacked(const Server::AItem *);

    virtual void	itemInRange(const Server::AItem *);

    void		onEffectTimeout(const Kernel::Serial &);

    virtual unsigned int	getSpeed(void) const;
    void		setGroupMove(Server::GroupMove *);

    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);

    virtual bool	removeRessources(unsigned int, Kernel::ElementRessources::Type);

  };

}

#endif
