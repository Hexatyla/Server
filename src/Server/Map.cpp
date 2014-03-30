//
// Map.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sat Feb 15 23:22:48 2014 geoffrey bauduin
// Last update Sat Mar 22 16:50:31 2014 geoffrey bauduin
//

#include	"Server/Map.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Game/Controller.hpp"
#include	"Factory/Server.hpp"
#include	"RandomIterator.hpp"

Server::Map::Map(void):
  ::Game::Map(), Server::HasEvent(),
  _mutex(new Mutex), _spots(), _starters() {

}

Server::Map::~Map(void) {
  delete this->_mutex;
}

void	Server::Map::init(unsigned int x, unsigned int y, const std::vector<unsigned int> &z,
			  const std::vector<Spot> &spots, const std::vector<std::pair<double, double> > &starters) {
  Server::HasEvent::init();
  ::Game::Map::init(x, y, z);
  this->_spots = spots;
  for (auto it : starters) {
    this->_starters.push_back(it);
  }
}

void	Server::Map::destroy(void) {
  ::Game::Map::destroy();
  Server::HasEvent::destroy();
  this->_starters.clear();
}

bool	Server::Map::getStartingPosition(double &x, double &y, double &z) {
  if (this->_starters.size() == 0) {
    return (false);
  }
  std::list<std::pair<double, double> >::iterator it = random_iterator(this->_starters);
  double x_ = (*it).first;
  double y_ = (*it).second;
  if (this->_hexas.find(y_) == this->_hexas.end()) {
    return (false);
  }
  if (this->_hexas[y_].find(x_) == this->_hexas[y_].end()) {
    return (false);
  }
  if (this->_hexas[y_][x_].z == 0.0) {
    this->_starters.erase(it);
    return (this->getStartingPosition(x, y, z));
  }
  z = this->_hexas[y_][x_].z;
  x = this->_hexas[y_][x_].x + this->_hexaX;
  y = this->_hexas[y_][x_].y + this->_hexaY;
  this->_starters.erase(it);
  return (true);
}

bool	Server::Map::push(const ::Game::AElement *element) {
  ScopeLock s(this->_mutex);
  return (::Game::Map::push(element));
}

bool	Server::Map::_push(const ::Game::AElement *element, bool rv) {
  if (::Game::Map::_push(element, rv)) {
    ::Game::Map::Node *node = this->getNode(element->getX(), element->getY());
    if (node) {
      for (auto it : node->range) {
	if (it != element &&
	    ::Game::Controller::getInstance()->itemHasAttackRangeOnItem(it, element) &&
	    ::Game::Controller::getInstance()->itemHasVisionOnItem(this, it, element)) {
	  this->createOnRangeEvent(it, element);
	}
      }
    }
    const std::vector<const ::Game::AElement *> *inRange = this->_objectsInRange(element->getX(), element->getY(), element->getAttackRange());
    if (inRange) {
      for (auto it : *inRange) {
	if (it != element &&
	    ::Game::Controller::getInstance()->itemHasVisionOnItem(this, element, it)) {
	  this->createOnRangeEvent(element, it);
	}
      }
      delete inRange;
    }
    return (true);
  }
  return (false);
}

bool	Server::Map::_remove(const ::Game::AElement *element, bool rv) {
  return (::Game::Map::_remove(element, rv));
}

bool	Server::Map::remove(const ::Game::AElement *element) {
  ScopeLock s(this->_mutex);
  return (::Game::Map::remove(element));
}

void	Server::Map::createOnRangeEvent(const ::Game::AElement *source, const ::Game::AElement *target) {
  Server::Event *event = Factory::Server::Event::create(Server::Event::ON_RANGE, Server::Event::SERVER_GAME, NULL);
  event->onRange.source.id = source->getID();
  event->onRange.target.id = target->getID();
  event->onRange.source.type = source->getType();
  event->onRange.target.type = target->getType();
  this->addEvent(event);
}

const std::vector<Server::Map::Spot>	&Server::Map::getSpots(void) const {
  return (this->_spots);
}
