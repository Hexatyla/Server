//
// GamePlayer.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sat Feb  1 20:05:51 2014 geoffrey bauduin
// Last update Wed Mar 26 11:14:37 2014 Gautier Lefebvre
//

#ifndef SERVER_GAMEPLAYER_HPP_
# define SERVER_GAMEPLAYER_HPP_

#include	<list>
#include	"Game/Player.hpp"
#include	"Threading/Mutex.hpp"
#include	"Server/HasJobs.hpp"
#include	"Server/User.hpp"
#include	"Server/HasProductionQueue.hpp"
#include	"Server/HasEvent.hpp"
#include	"Server/HasStatistics.hpp"
#include	"IFromFactory.hpp"
#include	"NoDeleteWhileUsedByAThread.hpp"

namespace	Server {

  class	GamePlayer: public ::Game::Player, public Server::HasJobs, public Server::HasProductionQueue,
		    public Server::HasEvent, virtual public IFromFactory, public Server::HasStatistics,
		    public NoDeleteWhileUsedByAThread {

  private:
    mutable Mutex	*_mutex;
    mutable Mutex	*_ressourcesMutex;
    Server::User	*_user;
    ::Game::Ressources	_lockedRessources;
    ::Game::Ressources	_realRessources;
    std::list<Protocol::Job *>	_reconnect;
    mutable Mutex	*_reconnectMutex;

    void		onRessourcesChange(void);
    Protocol::Job	*createRessourcesJob(void) const;
    void		onAmeliorationProduced(const Kernel::Serial &, const Kernel::Serial &);

    void		onRemoveFromProductionList(const Kernel::Serial &, const Kernel::Serial &);
    void		onAddToProductionList(const Kernel::Serial &, const Kernel::Serial &, unsigned int);

    void		onLost(void);

  public:
    GamePlayer(void);
    virtual ~GamePlayer(void);

    virtual void	init(Server::User *, Kernel::ID::id_t, const ::Game::Race *, const ::Game::Team *);
    virtual void	destroy(void);

    void		update(const Clock *, double);

    virtual void	addRessources(const ::Game::Ressources &);
    virtual void	addRessources(::Game::Ressources::Type, unsigned int);

    virtual void	removeRessources(const ::Game::Ressources &);
    virtual void	removeRessources(::Game::Ressources::Type, unsigned int);

    virtual void	spendRessources(const ::Game::Ressources &);

    virtual void	addToQueue(const Kernel::Serial &, const Kernel::Serial &);
    virtual void	insertInQueue(const Kernel::Serial &, const Kernel::Serial &, unsigned int);
    virtual bool	removeFromQueue(const Kernel::Serial &, const Kernel::Serial &);

    void		removeItemFromProductionList(const Kernel::Serial &, const Kernel::Serial &);
    void		produceAmelioration(const Kernel::Serial &, const Kernel::Serial &);

    void		onPing(Kernel::ID::id_t, double, double);

    Protocol::Job	*onAskRessources(void) const;

    Server::User	*getUser(void);

    void		inGame(void);

    virtual void	removeUnit(unsigned int amount = 1);
    virtual void	removeHero(unsigned int amount = 1);
    virtual void	removeBuilding(unsigned int amount = 1);

    bool		lockRessources(const ::Game::Ressources &);
    bool		lockRessources(::Game::Ressources::Type, unsigned int);

    void		unlockRessources(const ::Game::Ressources &);
    void		unlockRessources(::Game::Ressources::Type, unsigned int);

    const ::Game::Ressources &getRealRessources(void) const;
    unsigned int	getRealRessources(::Game::Ressources::Type) const;

    virtual bool	hasEnoughRessources(const ::Game::Ressources &ressources) const;

    void	addReconnectJobs(const std::vector<Protocol::Job *> &);
    bool	isReconnectJob(Protocol::Job *);

  };

}

#endif
