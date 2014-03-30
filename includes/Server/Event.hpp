//
// Event.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 16:09:22 2014 geoffrey bauduin
// Last update Tue Mar 25 10:23:32 2014 geoffrey bauduin
//

#ifndef SERVER_EVENT_HPP_
# define SERVER_EVENT_HPP_

#include	<list>
#include	"IFromFactory.hpp"
#include	"data_struct.hpp"

namespace	Server {

  class GamePlayer;

  class	Event: virtual public IFromFactory {

  public:
    enum	Type {
      NONE,
      START_MOVE,
      MOVE,
      UPDATE,
      JOBS,
      GPJOBS,
      REMOVE,
      PING,
      UNIT_STANCE,
      STOP_ACTION,
      SET_ACTION,
      CREATE_ITEM,
      PUT_IN_PRODUCTION_LIST,
      REMOVE_PRODUCTION_LIST,
      PICK_UP_OBJECT,
      RELEASE_OBJECT,
      MOVE_TOWARDS_POINT_AND_RELEASE_OBJECT,
      MOVE_TOWARDS_OBJECT_AND_PICK_UP_OBJECT,
      FINISHED_AMELIORATION_PRODUCTION,
      REMOVE_AMELIORATION_FROM_PRODUCTION_LIST,
      PRODUCE_AMELIORATION,
      UNIT_BUILD,
      MOVE_TOWARDS_ITEM_AND_BUILD,
      STOP_BUILDING,
      HARVEST_SPOT,
      MOVE_TOWARDS_SPOT_AND_HARVEST,
      WAITLIST_TO_HARVEST,
      UNIT_ADD_RESS,
      STOP_HARVEST_AND_DEPOSIT,
      DEPOSIT_IN_BUILDING,
      MOVE_TOWARDS_BUILDING_AND_DEPOSIT,
      UNIT_GO_BACK_TO_SPOT,
      EFFECT_TIMEOUT,
      ON_RANGE,
      CREATE_PROJ,
      FOLLOW,
      DUMP_THREADS,
      PATROL,
      MOVE_TO_POINT,
      MOVE_TOWARDS_ITEM_AND_START_USE_CAPACITY,
      START_USE_CAPACITY,
      USE_CAPACITY,
      MOVE_TOWARDS_POINT_AND_USE_CAPACITY,
      SET_CAPACITY_AUTOMATIC,
      ADD_XP,
      CREATE_SPOT,
      STATS_CHANGED,
      PLAYER_LOST,
      END_ACTION,
      STOP_ACTION_HARVEST,
      STOP_ACTION_BUILD
    };

    enum	objectType {
      NO_TYPE,
      SERVER_GAME,
      SERVER_ITEM,
      SERVER_GAMEPLAYER
    };

    Type		type;
    objectType		objType;
    void		*ptr;

    struct productionList {
      Kernel::ID::id_t id;
      char	serial[32];
      Server::GamePlayer *player;
    };

    struct amelioproductionlist {
      Server::GamePlayer *player;
      char	type[32];
      char	amelioration[32];
    };

    union {
      createItem		create;
      updateItem		update;
      item_			remove;
      userPing			ping;
      unitStance		stance;
      item_			itemInfos;
      action			itemAction;
      productionList		production;
      pickupobject		pickUpObject;
      releaseobject		releaseObject;
      ameliorationfinished	ameliorationFinished;
      amelioproductionlist	ameliorationProductionList;
      build_			build;
      harvest_			harvest;
      harvestAmount_		harvestAmount;
      deposit_			deposit;
      effecttimeout		effectTimeout;
      onrange			onRange;
      move_			move;
      updateItem		moveItem;
      attack_			attack;
      projectile_		projectile;
      follow_			follow;
      patrol_			patrol;
      moveToPoint_		moveToPoint;
      attackPoint_		attackPoint;
      useCapacity_		useCapacity;
      setAutomatic_		setAutomatic;
      xp_			xp;
      item_			item;
    };

    Event(void);
    ~Event(void);

    void	init(Type, objectType, void *);
    virtual void destroy(void);

    bool	hasReferenceToItem(Kernel::ID::id_t, ::Game::Type) const;

  };

}

#endif
