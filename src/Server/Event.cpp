//
// Event.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 16:14:04 2014 geoffrey bauduin
// Last update Tue Mar 25 10:25:27 2014 geoffrey bauduin
//

#include	<cstddef>
#include	"Server/Event.hpp"

Server::Event::Event(void):
  type(NONE), objType(Server::Event::NO_TYPE), ptr(NULL) {}

Server::Event::~Event(void) {}

void	Server::Event::init(Server::Event::Type type, Server::Event::objectType objType, void *ptr) {
  this->type = type;
  this->objType = objType;
  this->ptr = ptr;
}

void	Server::Event::destroy(void) {

}

static bool	comp_item_(const item_ &item, Kernel::ID::id_t id, ::Game::Type type) {
  return (item.id == id && item.type == type);
}

bool	Server::Event::hasReferenceToItem(Kernel::ID::id_t id, ::Game::Type type) const {
  switch (this->type) {
  case ON_RANGE:
    return (comp_item_(this->onRange.source, id, type) || comp_item_(this->onRange.target, id, type));
  case CREATE_PROJ:
    return (comp_item_(this->projectile.source, id, type) || comp_item_(this->projectile.target, id, type));
  case EFFECT_TIMEOUT:
    return (comp_item_(this->effectTimeout.item, id, type));
  case UNIT_STANCE:
    return (this->stance.id == id && this->stance.type == type);
  case MOVE:
    return (this->moveItem.id == id && this->moveItem.type == type);
  case FOLLOW:
    return (comp_item_(this->follow.source, id, type) || comp_item_(this->follow.target, id, type));
  case HARVEST_SPOT:
  case MOVE_TOWARDS_SPOT_AND_HARVEST:
  case UNIT_GO_BACK_TO_SPOT:
  case WAITLIST_TO_HARVEST:
  case STOP_ACTION_HARVEST:
    return ((this->harvest.unit == id && type == ::Game::UNIT) ||
	    (this->harvest.spot == id && type == ::Game::RESSSPOT));
  case STOP_HARVEST_AND_DEPOSIT:
    return (this->harvest.unit == id && type == ::Game::UNIT);
  case UNIT_ADD_RESS:
    return (this->harvestAmount.unit == id && type == ::Game::UNIT);
  case RELEASE_OBJECT:
  case MOVE_TOWARDS_POINT_AND_RELEASE_OBJECT:
    return ((this->releaseObject.hero == id && type == ::Game::HERO) ||
	    (this->releaseObject.object == id && type == ::Game::OBJECT));
  case PICK_UP_OBJECT:
  case MOVE_TOWARDS_OBJECT_AND_PICK_UP_OBJECT:
    return ((this->pickUpObject.hero == id && ::Game::HERO == type) ||
	    (this->pickUpObject.object == id && type == ::Game::OBJECT));
  case UNIT_BUILD:
  case STOP_BUILDING:
  case MOVE_TOWARDS_ITEM_AND_BUILD:
  case STOP_ACTION_BUILD:
    return ((this->build.unit == id && type == ::Game::UNIT) ||
	    (this->build.building == id && type == ::Game::BUILDING));
  case DEPOSIT_IN_BUILDING:
  case MOVE_TOWARDS_BUILDING_AND_DEPOSIT:
    return ((this->deposit.unit == id && type == ::Game::UNIT) ||
	    (this->build.building == id && type == ::Game::BUILDING));
  case START_MOVE:
    return (this->move.id == id && this->move.type == type);
  case UPDATE:
    return (this->update.id == id && this->update.type == type);
  case STOP_ACTION:
    return (comp_item_(this->itemInfos, id, type));
  case SET_ACTION:
    return (this->itemAction.id == id && this->itemAction.type == type);
  case PUT_IN_PRODUCTION_LIST:
  case REMOVE_PRODUCTION_LIST:
    return (this->production.id == id && type == ::Game::BUILDING);
  default:
    return (false);
  }
  return (true);
}
