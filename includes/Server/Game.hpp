//
// Game.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 14:19:56 2014 geoffrey bauduin
// Last update Wed Mar 26 11:25:10 2014 Gautier Lefebvre
//

#ifndef SERVER_GAME_HPP_
# define SERVER_GAME_HPP_

#include	<map>
#include	<vector>
#include	"Kernel/ID.hpp"
#include	"HasID.hpp"
#include	"HasEvent.hpp"
#include	"Threading/Mutex.hpp"
#include	"Server/HasUnit.hpp"
#include	"Server/HasBuilding.hpp"
#include	"Server/HasProjectile.hpp"
#include	"Server/HasHero.hpp"
#include	"Server/HasJobs.hpp"
#include	"Server/HasUsers.hpp"
#include	"Server/HasObject.hpp"
#include	"Server/HasRessourcesSpot.hpp"
#include	"Server/Map.hpp"
#include	"Server/HasPlayers.hpp"
#include	"Algo/Pathfinding.hpp"
#include	"Game/Controller.hpp"
#include	"IFromFactory.hpp"
#include	"Server/GameLoader.hpp"
#include	"Protocol/Job.hpp"
#include	"Server/Action.hpp"
#include	"Server/IHasReferenceToOtherObject.hpp"
#include	"Server/Spectator.hpp"

namespace	Server {

  class	Game: public HasID, public HasEvent, public HasJobs, public HasUsers,
	      public HasHero, public HasProjectile, public HasBuilding, public HasUnit,
	      public HasObject, public HasRessourcesSpot,
	      virtual public IFromFactory, public HasPlayers, virtual public IHasReferenceToOtherObject {

  private:
    Mutex						*_mutex;
    Clock						*_clock;
    Server::Map						*_map;
    Algo::Pathfinding					_pathfinding;
    Clock						*_lastDayNightSwitch;
    bool						_day;
    Mutex						*_removeMutex;
    GameLoader						*_loader;
    std::map< ::Game::Type, Mutex *>			_mutexes;
    Mutex						*_teamMutex;
    std::map< ::Game::Team *, unsigned int>		_teams;
    Server::Spectator					*_spectator;
    bool						_end;
    Mutex						*_specJobsMutex;
    bool						_firstSpecJob;
    std::queue<std::pair<Network::SSocket *, Protocol::Job *> >	_specJobs;

    void	createUpdateEvent(Kernel::ID::id_t, ::Game::Type, double);
    void	handleItemEvents(Server::HasEvent *);
    void	handleItemJobs(Server::HasJobs *, bool spec = false, Kernel::ID::id_t id = 0);
    void	handleSpecJobs(void);
    void	createDeleteEvent(Server::AItem *);
    void	createUpdatePlayerEvent(Kernel::ID::id_t, double);

    void	initPlayer(Server::GamePlayer *);

    void	onRemoveItem(Server::AItem *, bool);
    void	onAddItem(Server::AItem *, bool);

    void	onDayNightSwitch(void);
    void	sendErrorToPlayer(Server::GamePlayer *, ::Game::Controller::Error::Type);

    Server::AItem *_getItem(Kernel::ID::id_t, int objType = ::Game::BUILDING | ::Game::UNIT | ::Game::HERO | ::Game::PROJECTILE | ::Game::OBJECT | ::Game::RESSSPOT) const;
    Server::GamePlayer *getPlayerOwningItem(const Server::AItem *) const;

    void	sendErrorJob(Server::GamePlayer *, ::Command::Command, ::Game::Controller::Error::Type);

    Server::Movable	*getMovableBase(Server::AItem *, ::Game::Type) const;
    Server::CanUseCapacity	*getCanUseCapacityBase(Server::AItem *) const;

    void	createMoveEvent(Server::AItem *, double, double, double, Server::GroupMove *);
    void	_askMoveItem(Server::GamePlayer *, Server::AItem *, double, double, double, Server::GroupMove *);

    void	createUnitHarvestEvent(Server::Unit *, const Server::RessourcesSpot *);

    bool	moveItem(Server::AItem *, Server::Movable *, double, double, double);

    Server::Building *getClosestBuilding(const Server::AItem *item, ::Game::Ressources::Type) const;

    bool	getDestinationData(Server::Movable *, Vector3d &, double) const;
    void	_startMoveItem(Server::AItem *, Server::Movable *, double, double, double,
			       Algo::Pathfinding::WhatToDo, Server::Action *, bool dAction = true);
    void	_askHeroReleaseObject(Server::Hero *, const Server::Object *, double, double, double);
    void	_askHeroPickUpObject(Server::Hero *, const Server::Object *);
    void	_askUnitToBuild(Server::Unit *, const Server::Building *);
    void	_unitStopsHarvestAndDeposit(Server::Unit *, const Server::Building *, ::Game::Ressources::Type);
    void	_moveItem(Server::AItem *, double);
    void	_askUnitToPatrol(Server::Unit *, double, double, Server::GroupMove *);

    void	_moveToPoint(Server::AItem *, double, double, double);

    void	_askItemToAttackZone(Server::AItem *, double, double, double);

    void	createChangeUnitStanceEvent(Server::Unit *, ::Game::Stance::Type);

    void	processAskItemToUseCapacity(Server::GamePlayer *, Server::AItem *, const Server::AItem *,
					    const Kernel::Serial &);
    void	_askItemToUseCapacity(Server::AItem *, const Server::AItem *, const Kernel::Serial &);

    void	processAskItemUseCapacityZone(Server::GamePlayer *, Server::AItem *, const Kernel::Serial &,
					   double, double, double);

    void	_askItemToUseZoneCapacity(Server::AItem *, const Kernel::Serial &, double, double, double);
    void	_askItemToUseCapacity(Server::AItem *, const Kernel::Serial &);

    void	processItemStartUseCapacityTarget(Server::AItem *, const Server::AItem *, const Kernel::Serial &);
    void	processItemUseCapacityTarget(Server::AItem *, const Server::AItem *, const Kernel::Serial &);

    bool	checkStopCondition(Server::AItem *);
    bool	checkStopCondition(Server::AItem *, const Server::AItem *);
    bool	checkStopCondition(Server::AItem *, const Vector3d &);

    Server::Action	*createActionStartUseCapacityTarget(const Server::AItem *, const Server::AItem *,
							    const Kernel::Serial &) const;
    Server::Action	*createActionStartUseCapacityZone(const Server::AItem *, const Kernel::Serial &,
							  double, double, double) const;
    void	processMoveTowardsItemAndStartUseCapacity(Server::AItem *, const Server::AItem *,
							  const Kernel::Serial &);


    bool	initMap(const std::string &);
    void	initTeams(const std::vector< ::Game::Team *> &, const std::vector<Server::GamePlayer *> &);

    void	checkEndGame(void);
    bool	_update(const Clock *, double);

    void	addSpecJob(Network::SSocket *, Protocol::Job *);

    void	changePlayerFood(const Server::AItem *, bool);

  public:
    Game(void);
    ~Game(void);

    void	init(Kernel::ID::id_t, const std::vector<Server::GamePlayer *> &,
		     const Server::HasUsers::Container &, const std::vector< ::Game::Team *> &, const std::string &,
		     GameLoader *);
    virtual void	destroy(void);

    const GameLoader*	getLoader() const;
    GameLoader *	getLoader();

    bool	popSpecJob(Network::SSocket **, Protocol::Job **);

    bool	update(const Clock *, double);
    bool	update(Kernel::ID::id_t, ::Game::Type, double);
    bool	updatePlayer(Kernel::ID::id_t, double);
    bool	addItem(Server::AItem *);
    void	removeItem(::Game::Type, Kernel::ID::id_t);
    bool	isDay(void) const;

    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);

    const Server::AItem *getItem(Kernel::ID::id_t, int objType = ::Game::BUILDING | ::Game::UNIT | ::Game::HERO | ::Game::PROJECTILE | ::Game::OBJECT | ::Game::RESSSPOT) const;

    void	onPing(Kernel::ID::id_t, double, double);

    void	changeUnitStance(Kernel::ID::id_t, ::Game::Type, ::Game::Stance::Type);
    void	stopItemAction(Kernel::ID::id_t, ::Game::Type);
    void	setItemAction(Kernel::ID::id_t, ::Game::Type, ::Game::Action *);
    void	addItemToProductionList(Kernel::ID::id_t, const Kernel::Serial &, Server::GamePlayer *);
    void	removeItemFromProductionList(Kernel::ID::id_t, const Kernel::Serial &, Server::GamePlayer *);
    void	heroPickUpObject(Kernel::ID::id_t, Kernel::ID::id_t);
    void	heroReleaseObject(Kernel::ID::id_t, Kernel::ID::id_t, double, double, double);
    void	unitBuildBuilding(Kernel::ID::id_t, Kernel::ID::id_t);
    void	unitStopBuilding(Kernel::ID::id_t, Kernel::ID::id_t);
    void	unitHarvestSpot(Kernel::ID::id_t, Kernel::ID::id_t);
    void	unitHarvestRessources(Kernel::ID::id_t, ::Game::Ressources::Type, unsigned int);
    void	unitStopsHarvestAndDeposit(Kernel::ID::id_t, Kernel::ID::id_t);
    void	unitDepositInBuilding(Kernel::ID::id_t, Kernel::ID::id_t, ::Game::Ressources::Type);
    void	effectTimeout(Kernel::ID::id_t, ::Game::Type, const Kernel::Serial &);
    void	itemIsInRangeOfAnotherItem(::Game::Type, Kernel::ID::id_t, ::Game::Type, Kernel::ID::id_t);
    void	askItemInformations(Server::GamePlayer *, Kernel::ID::id_t);
    void	askPlayerRessources(Server::GamePlayer *, Kernel::ID::id_t);

    void	askUnitsToHarvest(Server::GamePlayer *, Kernel::ID::id_t, const std::vector<Kernel::ID::id_t> &);
    void	askUnitToHarvest(Server::GamePlayer *, Kernel::ID::id_t, Kernel::ID::id_t);
    void	askRessourcesSpotID(Server::GamePlayer *, double, double, double);
    void	askUnitToBuild(Server::GamePlayer *, Kernel::ID::id_t, Kernel::ID::id_t);
    void	askPlayerInformations(Server::GamePlayer *, Kernel::ID::id_t);
    void	askProduceAmelioration(Server::GamePlayer *, const Kernel::Serial &, const Kernel::Serial &);
    void	askSpecificElementAmeliorations(Server::GamePlayer *, Kernel::ID::id_t, const Kernel::Serial &);
    void	askHeroObjects(Server::GamePlayer *, Kernel::ID::id_t);
    void	askHeroReleaseObject(Server::GamePlayer *, Kernel::ID::id_t, Kernel::ID::id_t,
				     double, double, double);
    void	askStopItemAction(Server::GamePlayer *, Kernel::ID::id_t);
    void	askHeroPickUpObject(Server::GamePlayer *, Kernel::ID::id_t, Kernel::ID::id_t);
    void	askSetItemsAction(Server::GamePlayer *, ::Game::eAction, const std::vector<Kernel::ID::id_t> &);
    void	askSetItemAction(Server::GamePlayer *, Kernel::ID::id_t, ::Game::eAction);
    void	askUnitProduction(Server::GamePlayer *, Kernel::ID::id_t, const Kernel::Serial &);
    void	askRemoveUnitFromProduction(Server::GamePlayer *, Kernel::ID::id_t, const Kernel::Serial &);
    void	askItemEffects(Server::GamePlayer *, Kernel::ID::id_t);
    void	askUnitPickedUpRessources(Server::GamePlayer *, Kernel::ID::id_t);
    void	askItemAction(Server::GamePlayer *, Kernel::ID::id_t);
    void	unitGoBackToSpot(Kernel::ID::id_t, Kernel::ID::id_t);
    void	askUnitsToBuild(Server::GamePlayer *, Kernel::ID::id_t, const std::vector<Kernel::ID::id_t> &);
    void	askMoveItem(Server::GamePlayer *, Kernel::ID::id_t, double, double, double);
    void	askItemToAttack(Server::GamePlayer *, Kernel::ID::id_t, Kernel::ID::id_t);
    void	askUnitStance(Server::GamePlayer *, Kernel::ID::id_t);
    void	askChangeUnitStance(Server::GamePlayer *, Kernel::ID::id_t, ::Game::Stance::Type);
    void	askChangeUnitsStance(Server::GamePlayer *, ::Game::Stance::Type,
				     const std::vector<Kernel::ID::id_t> &);

    void	startMoveItem(Kernel::ID::id_t, ::Game::Type, double, double, double);
    void	moveItem(Kernel::ID::id_t, ::Game::Type, double);

    void	moveTowardsPointAndReleaseObject(Kernel::ID::id_t, Kernel::ID::id_t, double, double, double);
    void	moveTowardsObjectAndPickUp(Kernel::ID::id_t, Kernel::ID::id_t);
    void	moveTowardsItemAndBuild(Kernel::ID::id_t, Kernel::ID::id_t);
    void	moveTowardsBuildingAndDeposit(Kernel::ID::id_t, Kernel::ID::id_t, ::Game::Ressources::Type);
    void	moveTowardsSpotAndHarvest(Kernel::ID::id_t, Kernel::ID::id_t);


    void	itemStartAttack(Kernel::ID::id_t, ::Game::Type, Kernel::ID::id_t, ::Game::Type);
    void	itemAttack(Kernel::ID::id_t, ::Game::Type, Kernel::ID::id_t, ::Game::Type);

    void	createProjectile(Server::AItem *, Kernel::ID::id_t, ::Game::Type, Kernel::ID::id_t, ::Game::Type);
    void	createProjectile(Server::AItem *, Kernel::ID::id_t, ::Game::Type, double, double, double);
    void	followItem(Kernel::ID::id_t, ::Game::Type, Kernel::ID::id_t, ::Game::Type, double);

    void	askUnitToPatrol(Server::GamePlayer *, Kernel::ID::id_t, double, double);
    void	askUnitsToPatrol(Server::GamePlayer *, const std::vector<Kernel::ID::id_t> &,
				 double, double, bool);
    void	unitPatrol(Kernel::ID::id_t, ::Game::Type, double, double);

    void	moveToPoint(Kernel::ID::id_t, ::Game::Type, double, double, double);

    void	askMoveItemGroup(Server::GamePlayer *, const std::vector<Kernel::ID::id_t> &, bool,
				 double, double, double);
    void	askPing(Kernel::ID::id_t, double, double);
    void	askRemoveAmeliorationFromProductionQueue(Server::GamePlayer *, const Kernel::Serial &,
							 const Kernel::Serial &);
    void	askItemsToAttack(Server::GamePlayer *, const std::vector<Kernel::ID::id_t> &, Kernel::ID::id_t);
    void	askItemToAttackZone(Server::GamePlayer *, Kernel::ID::id_t, double, double, double);
    void	askItemsToAttackZone(Server::GamePlayer *, const std::vector<Kernel::ID::id_t> &,
				     double, double, double);

    void	askItemToUseCapacity(Server::GamePlayer *, Kernel::ID::id_t,
				     Kernel::ID::id_t, const Kernel::Serial &);

    void	itemStartUseCapacityTarget(Kernel::ID::id_t, ::Game::Type, const Kernel::Serial &,
					   Kernel::ID::id_t, ::Game::Type);

    void	itemUseCapacityTarget(Kernel::ID::id_t, ::Game::Type, const Kernel::Serial &,
				      Kernel::ID::id_t, ::Game::Type);
    void	askItemToUseZoneCapacity(Server::GamePlayer *, Kernel::ID::id_t, const Kernel::Serial &,
					 double, double, double);
    void	itemStartUseCapacityZone(Kernel::ID::id_t, ::Game::Type, const Kernel::Serial &,
					 double, double, double);
    void	itemUseCapacityZone(Kernel::ID::id_t, ::Game::Type, const Kernel::Serial &,
				    double, double, double);
    void	askItemToUseCapacity(Server::GamePlayer *, Kernel::ID::id_t, const Kernel::Serial &);
    void	itemStartUseCapacity(Kernel::ID::id_t, ::Game::Type, const Kernel::Serial &);
    void	itemUseCapacity(Kernel::ID::id_t, ::Game::Type, const Kernel::Serial &);
    void	moveTowardsItemAndStartUseCapacity(Kernel::ID::id_t, ::Game::Type, const Kernel::Serial &,
						   Kernel::ID::id_t, ::Game::Type);
    void	moveTowardsPointAndUseCapacity(Kernel::ID::id_t, ::Game::Type, const Kernel::Serial &,
					       double, double, double);
    void	askAutomaticCapacity(Server::GamePlayer *, Kernel::ID::id_t, const Kernel::Serial &, bool);
    void	setCapacityAutomatic(Kernel::ID::id_t, ::Game::Type, const Kernel::Serial &, bool);
    void	addXP(Kernel::ID::id_t, unsigned int);
    void	statsChanged(Kernel::ID::id_t, ::Game::Type);

    void	addSpectatorJob(Kernel::ID::Type, Kernel::ID::id_t, Protocol::Job *);

    void	playerHasLost(Server::GamePlayer *);

    void	userReconnected(Server::User *);
    void	addSpectator(Server::User *);

    void	askCreateBuilding(Server::GamePlayer *, const Kernel::Serial &, double, double, double, int);

    void	endItemAction(Kernel::ID::id_t, ::Game::Type);

    void	unitStopsHarvest(Kernel::ID::id_t, Kernel::ID::id_t);
    void	unitStopsBuilding(Kernel::ID::id_t, Kernel::ID::id_t);

    void	playerHasProducedAmelioration(Kernel::ID::id_t, const Kernel::Serial &, const Kernel::Serial &);

  };

}

#endif
