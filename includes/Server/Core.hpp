//
// Core.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sat Jan 18 15:56:19 2014 geoffrey bauduin
// Last update Wed Mar 26 14:15:02 2014 geoffrey bauduin
//

#ifndef SERVER_CORE_HPP_
# define SERVER_CORE_HPP_

#include	<queue>
#include	<map>
#include	<vector>
#include	<list>
#include	"Threading/Pool.hpp"
#include	"Server/Game.hpp"
#include	"Threading/Mutex.hpp"
#include	"Threading/CondVar.hpp"
#include	"Server/Event.hpp"
#include	"Clock.hpp"
#include	"Parser/Race.hpp"
#include	"Network/Manager/Server.hpp"
#include	"MySQL/Database.hpp"
#include	"DoubleAccessContainer.hpp"
#include	"Server/HasJobs.hpp"
#include	"Game/Controller.hpp"
#include	"Protocol/Error.hpp"
#include	"Server/Group.hpp"
#include	"Room/ARoom.hpp"
#include	"IFromFactory.hpp"
#include	"Server/GameLoader.hpp"
#include	"ThreadData.hpp"

#define NBR_FUNC	49
#define	NBR_FUNC_JOB	62

namespace	Server {

  class	Core: public Server::HasEvent, public Server::HasUsers, virtual public IFromFactory {

  public:
    struct game {
      Server::Game	*game;
      Clock		*clock;
    };

  private:
    /** THREADPOOL SHIT */
    ThreadPool					*_thPool;
    CondVar					*_cond;
    /* ***** */
    Clock					*_clock;
    std::map<Kernel::ID::id_t, game>		_games;
    Mutex					*_gMutex;
    bool					_end;
    Parser::Race				*_parserRace;
    Thread					*_threadNetwork;
    Mutex					*_jMutex;
    std::queue<std::pair<Network::SSocket *, Protocol::Job *> >		_jobs;
    DoubleAccessContainer<Server::Game *, Server::GamePlayer *>		_ingamePlayers;
    std::list<Group *>				_groups;
    std::list<Room::ARoom*>			_rooms;
    std::list<Server::GameLoader*>		_gameLoaders;
    Mutex					_mOG;
    std::vector<ThreadData>			_threadsStatus;
    Mutex					*_stTh;

    bool	initSQL(void);

    bool	loadRace(const std::string &);
    void	loadUsers();
    void	addJob(Network::SSocket *, Protocol::Job *);
    void	sendErrorJob(Network::UserJob*, Error::Code);
    void	confirmJob(Network::UserJob*);
    Group*	findGroup(const Server::User*);
    Room::ARoom* findRoom(const Server::User*);
    Server::GameLoader* findGameLoader(const Server::User*);
    Server::GameLoader* findGameLoaderSpectator(const Server::User*);
    Server::Game* findGameByGameLoader(const Server::GameLoader *);

    void	createGame(Server::GameLoader *);
    void	deleteGame(Kernel::ID::id_t);
    void	handleGameJobs(Server::Game *);
    void	handleGameEvents(Server::Game *);

    Server::AItem *_createItem(const createItem &);

    void	createErrorJob(Network::UserJob *, ::Game::Controller::Error::Type);
    void	createErrorJob(Network::UserJob *, Error::Code c);

    void	updateObject(Server::Event *);
    void	handleJobs(Server::Event *);
    void	removeObject(Server::Event *);
    void	handleGamePlayersJobs(Server::Event *);
    void	handlePingEvent(Server::Event *);
    void	handleChangeStanceEvent(Server::Event *);
    void	handleStopActionEvent(Server::Event *);
    void	handleSetActionEvent(Server::Event *);
    void	handleCreateItemEvent(Server::Event *);
    void	handlePutInProductionListEvent(Server::Event *);
    void	handleRemoveFromProductionListEvent(Server::Event *);
    void	handleReleaseObjectEvent(Server::Event *);
    void	handlePickUpObjectEvent(Server::Event *);
    void	handleMoveTowardsPointAndReleaseObjectEvent(Server::Event *);
    void	handleFinishedAmeliorationProductionEvent(Server::Event *);
    void	handleRemoveAmeliorationFromProductionListEvent(Server::Event *);
    void	handleProduceAmeliorationEvent(Server::Event *);
    void	handleUnitBuildEvent(Server::Event *);
    void	handleMoveTowardsItemAndBuildEvent(Server::Event *);
    void	handleStopBuildEvent(Server::Event *);
    void	handleHarvestSpotEvent(Server::Event *);
    void	handleMoveTowardsSpotAndHarvestEvent(Server::Event *);
    void	handleWaitListToHarvestEvent(Server::Event *);
    void	handleUnitAddRessEvent(Server::Event *);
    void	handleStopHarvestAndDepositEvent(Server::Event *);
    void	handleMoveTowardsBuildingAndDeposit(Server::Event *);
    void	handleDepositInBuildingEvent(Server::Event *);
    void	handleUnitGoBackToSpotEvent(Server::Event *);
    void	handleEffectTimeoutEvent(Server::Event *);
    void	handleOnRangeEvent(Server::Event *);
    void	handleStartMoveEvent(Server::Event *);
    void	handleMoveEvent(Server::Event *);
    void	handleMoveTowardsObjectAndPickUpObject(Server::Event *);
    void	handleCreateProjEvent(Server::Event *);
    void	handleFollowEvent(Server::Event *);
    void	dumpThread(Server::Event *);
    void	handlePatrolEvent(Server::Event *);
    void	handleMoveToPointEvent(Server::Event *);
    void	handleStartUseCapacityEvent(Server::Event *);
    void	handleUseCapacityEvent(Server::Event *);
    void	handleMoveTowardsItemAndStartUseCapacityEvent(Server::Event *);
    void	handleMoveTowardsPointAndUseCapacityEvent(Server::Event *);
    void	handleSetCapacityAutomaticEvent(Server::Event *);
    void	handleAddXPEvent(Server::Event *);
    void	handleStatsChangedEvent(Server::Event *);
    void	handlePlayerLostEvent(Server::Event *);
    void	handleEndActionEvent(Server::Event *);
    void	handleStopActionHarvestEvent(Server::Event *);
    void	handleStopActionBuildEvent(Server::Event *);

    struct ptrFunc {
      Server::Event::Type	type;
      void (Server::Core::*func)(Server::Event *);
    };

    const ptrFunc _ptrFunc[NBR_FUNC] = {
      {Server::Event::UPDATE, &Server::Core::updateObject},
      {Server::Event::JOBS, &Server::Core::handleJobs},
      {Server::Event::REMOVE, &Server::Core::removeObject},
      {Server::Event::GPJOBS, &Server::Core::handleGamePlayersJobs},
      {Server::Event::PING, &Server::Core::handlePingEvent},
      {Server::Event::UNIT_STANCE, &Server::Core::handleChangeStanceEvent},
      {Server::Event::STOP_ACTION, &Server::Core::handleStopActionEvent},
      {Server::Event::SET_ACTION, &Server::Core::handleSetActionEvent},
      {Server::Event::CREATE_ITEM, &Server::Core::handleCreateItemEvent},
      {Server::Event::PUT_IN_PRODUCTION_LIST, &Server::Core::handlePutInProductionListEvent},
      {Server::Event::REMOVE_PRODUCTION_LIST, &Server::Core::handleRemoveFromProductionListEvent},
      {Server::Event::PICK_UP_OBJECT, &Server::Core::handlePickUpObjectEvent},
      {Server::Event::RELEASE_OBJECT, &Server::Core::handleReleaseObjectEvent},
      {Server::Event::MOVE_TOWARDS_POINT_AND_RELEASE_OBJECT, &Server::Core::handleMoveTowardsPointAndReleaseObjectEvent},
      {Server::Event::FINISHED_AMELIORATION_PRODUCTION, &Server::Core::handleFinishedAmeliorationProductionEvent},
      {Server::Event::REMOVE_AMELIORATION_FROM_PRODUCTION_LIST, &Server::Core::handleRemoveAmeliorationFromProductionListEvent},
      {Server::Event::PRODUCE_AMELIORATION, &Server::Core::handleProduceAmeliorationEvent},
      {Server::Event::UNIT_BUILD, &Server::Core::handleUnitBuildEvent},
      {Server::Event::MOVE_TOWARDS_ITEM_AND_BUILD, &Server::Core::handleMoveTowardsItemAndBuildEvent},
      {Server::Event::STOP_BUILDING, &Server::Core::handleStopBuildEvent},
      {Server::Event::HARVEST_SPOT, &Server::Core::handleHarvestSpotEvent},
      {Server::Event::MOVE_TOWARDS_SPOT_AND_HARVEST, &Server::Core::handleMoveTowardsSpotAndHarvestEvent},
      {Server::Event::WAITLIST_TO_HARVEST, &Server::Core::handleWaitListToHarvestEvent},
      {Server::Event::UNIT_ADD_RESS, &Server::Core::handleUnitAddRessEvent},
      {Server::Event::STOP_HARVEST_AND_DEPOSIT, &Server::Core::handleStopHarvestAndDepositEvent},
      {Server::Event::DEPOSIT_IN_BUILDING, &Server::Core::handleDepositInBuildingEvent},
      {Server::Event::MOVE_TOWARDS_BUILDING_AND_DEPOSIT, &Server::Core::handleMoveTowardsBuildingAndDeposit},
      {Server::Event::UNIT_GO_BACK_TO_SPOT, &Server::Core::handleUnitGoBackToSpotEvent},
      {Server::Event::EFFECT_TIMEOUT, &Server::Core::handleEffectTimeoutEvent},
      {Server::Event::ON_RANGE, &Server::Core::handleOnRangeEvent},
      {Server::Event::START_MOVE, &Server::Core::handleStartMoveEvent},
      {Server::Event::MOVE, &Server::Core::handleMoveEvent},
      {Server::Event::MOVE_TOWARDS_OBJECT_AND_PICK_UP_OBJECT, &Server::Core::handleMoveTowardsObjectAndPickUpObject},
      {Server::Event::CREATE_PROJ, &Server::Core::handleCreateProjEvent},
      {Server::Event::FOLLOW, &Server::Core::handleFollowEvent},
      {Server::Event::DUMP_THREADS, &Server::Core::dumpThread},
      {Server::Event::PATROL, &Server::Core::handlePatrolEvent},
      {Server::Event::MOVE_TO_POINT, &Server::Core::handleMoveToPointEvent},
      {Server::Event::START_USE_CAPACITY, &Server::Core::handleStartUseCapacityEvent},
      {Server::Event::USE_CAPACITY, &Server::Core::handleUseCapacityEvent},
      {Server::Event::MOVE_TOWARDS_ITEM_AND_START_USE_CAPACITY, &Server::Core::handleMoveTowardsItemAndStartUseCapacityEvent},
      {Server::Event::MOVE_TOWARDS_POINT_AND_USE_CAPACITY, &Server::Core::handleMoveTowardsPointAndUseCapacityEvent},
      {Server::Event::SET_CAPACITY_AUTOMATIC, &Server::Core::handleSetCapacityAutomaticEvent},
      {Server::Event::ADD_XP, &Server::Core::handleAddXPEvent},
      {Server::Event::STATS_CHANGED, &Server::Core::handleStatsChangedEvent},
      {Server::Event::PLAYER_LOST, &Server::Core::handlePlayerLostEvent},
      {Server::Event::END_ACTION, &Server::Core::handleEndActionEvent},
      {Server::Event::STOP_ACTION_HARVEST, &Server::Core::handleStopActionHarvestEvent},
      {Server::Event::STOP_ACTION_BUILD, &Server::Core::handleStopActionBuildEvent}
    };

    struct jobFunc {
      Command::Command command;
      void (Server::Core::*func)(Network::UserJob*);
    };

    Server::GamePlayer	*getGamePlayer(Network::SSocket *);

    void	registration(Network::UserJob*);		//REG
    void	connection(Network::UserJob*);			//CO
    void	playerSetStatus(Network::UserJob*);		//SETSTATUS
    void	playerSetRace(Network::UserJob*);		//SETRACE
    void	disconnect(Network::UserJob*);			//EXIT
    void	friendRequest(Network::UserJob*);		//AFD
    void	removeFriend(Network::UserJob*);		//DFD
    void	answerFriendRequest(Network::UserJob*);		//AIFD
    void	addPlayerInGroup(Network::UserJob*);		//APIG
    void	answerGroupRequest(Network::UserJob*);		//AGR
    void	kickPlayerGroup(Network::UserJob*);		//KPG
    void	leaveGroup(Network::UserJob*);			//LVGP
    void	deleteGroup(Network::UserJob*);			//DELGRP
    void	sendWhisp(Network::UserJob*);			//SWH
    void	sendMsgGroup(Network::UserJob*);		//SMSGGRP
    void	quicklaunch(Network::UserJob*);			//QUICKLAUNCH
    void	leaveGameSearch(Network::UserJob*);		//LEAVEGAMESEARCH
    void	answerFileExists(Network::UserJob*);		//FILEEXISTS
    void	iAmReady(Network::UserJob*);			//IAMREADY
    void	sendMsgGlob(Network::UserJob*);			//SMSGGLB
    void	joinSpectator(Network::UserJob*);		//JOINSPECTATOR
    void	leaveSpectator(Network::UserJob*);		//LEAVESPECTATOR

    void	ping(Network::UserJob *);
    void	changeUnitStance(Network::UserJob *);
    void	changeUnitGrpStance(Network::UserJob *);
    void	askUnitStance(Network::UserJob *);
    void	askItemInformations(Network::UserJob *);
    void	askMoveItem(Network::UserJob *);
    void	askDayOrNight(Network::UserJob *);
    void	askPlayerRessources(Network::UserJob *);
    void	askItemAction(Network::UserJob *);
    void	askMoveGrp(Network::UserJob *);
    void	askActivateCapacity(Network::UserJob *);
    void	askUnitPickedUpRessources(Network::UserJob *);
    void	askItemEffects(Network::UserJob *);
    void	askUnitProduction(Network::UserJob *);
    void	stopItemAction(Network::UserJob *);
    void	askSetItemAction(Network::UserJob *);
    void	askSetItemsAction(Network::UserJob *);
    void	askRemoveUnitFromProduction(Network::UserJob *);
    void	askHeroObjects(Network::UserJob *);
    void	askHeroPickUpObject(Network::UserJob *);
    void	askHeroReleaseObject(Network::UserJob *);
    void	askSpecificElementAmeliorations(Network::UserJob *);
    void	askRemoveAmeliorationFromProductionQueue(Network::UserJob *);
    void	askProduceAmelioration(Network::UserJob *);
    void	askPlayerAmeliorations(Network::UserJob *);
    void	askUnitToBuild(Network::UserJob *);
    void	askRessourcesSpotID(Network::UserJob *);
    void	askUnitToHarvest(Network::UserJob *);
    void	askUnitsToHarvest(Network::UserJob *);
    void	askUnitsToBuild(Network::UserJob *);
    void	askItemToUseCapacity(Network::UserJob *);
    void	askItemToAttack(Network::UserJob *);
    void	askUnitPatrol(Network::UserJob *);
    void	askUnitsPatrol(Network::UserJob *);
    void	askItemsToAttack(Network::UserJob *);
    void	askItemToAttackZone(Network::UserJob *);
    void	askItemsToAttackZone(Network::UserJob *);
    void	askItemToUseZoneCapacity(Network::UserJob *);
    void	askItemToSetCapacityAutomatic(Network::UserJob *);
    void	askCreateBuilding(Network::UserJob *);

    const jobFunc jobFuncTab[NBR_FUNC_JOB]={
      {Command::REG, &Server::Core::registration},
      {Command::CO, &Server::Core::connection},
      {Command::SETSTATUS, &Server::Core::playerSetStatus},
      {Command::SETRACE, &Server::Core::playerSetRace},
      {Command::EXIT, &Server::Core::disconnect},
      {Command::AFD, &Server::Core::friendRequest},
      {Command::DFD, &Server::Core::removeFriend},
      {Command::AIFD, &Server::Core::answerFriendRequest},
      {Command::APIG, &Server::Core::addPlayerInGroup},
      {Command::AGR, &Server::Core::answerGroupRequest},
      {Command::KPG, &Server::Core::kickPlayerGroup},
      {Command::LVGP, &Server::Core::leaveGroup},
      {Command::DELGRP, &Server::Core::deleteGroup},
      {Command::SWH, &Server::Core::sendWhisp},
      {Command::SMSGGRP, &Server::Core::sendMsgGroup},
      {Command::QUICKLAUNCH, &Server::Core::quicklaunch},
      {Command::LEAVEGAMESEARCH, &Server::Core::leaveGameSearch},
      {Command::FILEEXISTS, &Server::Core::answerFileExists},
      {Command::IAMREADY, &Server::Core::iAmReady},
      {Command::JOINSPECTATOR, &Server::Core::joinSpectator},
      {Command::LEAVESPECTATOR, &Server::Core::leaveSpectator},
      {Command::SMSGGLB, &Server::Core::sendMsgGlob},
      {Command::PING, &Server::Core::ping},
      {Command::UCHGSTANCE, &Server::Core::changeUnitStance},
      {Command::UCHGGRPSTANCE, &Server::Core::changeUnitGrpStance},
      {Command::UASKSTANCE, &Server::Core::askUnitStance},
      {Command::OASKINFO, &Server::Core::askItemInformations},
      {Command::OASKMOVE, &Server::Core::askMoveItem},
      {Command::ASKDAYNIGHT, &Server::Core::askDayOrNight},
      {Command::ASKPRESS, &Server::Core::askPlayerRessources},
      {Command::OASKACT, &Server::Core::askItemAction},
      {Command::OGRPMOVE, &Server::Core::askMoveGrp},
      {Command::OASKACTCAPA, &Server::Core::askActivateCapacity},
      {Command::ASKUPURESS, &Server::Core::askUnitPickedUpRessources},
      {Command::ASKOEFFECT, &Server::Core::askItemEffects},
      {Command::UASKPROD, &Server::Core::askUnitProduction},
      {Command::OSTOP, &Server::Core::stopItemAction},
      {Command::OSETACTION, &Server::Core::askSetItemAction},
      {Command::OSETACTIONS, &Server::Core::askSetItemsAction},
      {Command::UASKDPROD, &Server::Core::askRemoveUnitFromProduction},
      {Command::ASKHOBJ, &Server::Core::askHeroObjects},
      {Command::ASKHPUOBJ, &Server::Core::askHeroPickUpObject},
      {Command::ASKHROBJ, &Server::Core::askHeroReleaseObject},
      {Command::ASKPSAMELIO, &Server::Core::askSpecificElementAmeliorations},
      {Command::ASKDAMELIO, &Server::Core::askRemoveAmeliorationFromProductionQueue},
      {Command::ASKPRODAMELIO, &Server::Core::askProduceAmelioration},
      {Command::PAMELIO, &Server::Core::askPlayerAmeliorations},
      {Command::UASKBUILD, &Server::Core::askUnitToBuild},
      {Command::ASKRESSSPOTID, &Server::Core::askRessourcesSpotID},
      {Command::UASKHARVEST, &Server::Core::askUnitToHarvest},
      {Command::USASKHARVEST, &Server::Core::askUnitsToHarvest},
      {Command::USASKBUILD, &Server::Core::askUnitsToBuild},
      {Command::ASKOATTACK, &Server::Core::askItemToAttack},
      {Command::ASKOUSECAP, &Server::Core::askItemToUseCapacity},
      {Command::ASKUPATROL, &Server::Core::askUnitPatrol},
      {Command::ASKUSPATROL, &Server::Core::askUnitsPatrol},
      {Command::ASKOSATTACK, &Server::Core::askItemsToAttack},
      {Command::ASKOZONEATTACK, &Server::Core::askItemToAttackZone},
      {Command::ASKOSZONEATTACK, &Server::Core::askItemsToAttackZone},
      {Command::ASKOUSEZONECAP, &Server::Core::askItemToUseZoneCapacity},
      {Command::OASKAUTOCAPA, &Server::Core::askItemToSetCapacityAutomatic},
      {Command::ASKCBUILDING, &Server::Core::askCreateBuilding}
    };

  public:
    Core(void);
    ~Core(void);

    int		run(void);
    bool	init(int, char **);
    virtual void destroy(void);
    void	runThread(void);
    void	end(void);

  };

}

#endif
