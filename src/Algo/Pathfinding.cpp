//
// Pathfinding.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Fri Jan 24 16:29:46 2014 geoffrey bauduin
// Last update Wed Mar 26 14:01:53 2014 geoffrey bauduin
//

#define	ABS(x)		((x) < 0 ? -(x) : (x))
#define DIST_PP(a, b)	((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y))
#define DIST_NP(a, b)	((a.x - b->x) * (a.x - b->x) + (a.y - b->y) * (a.y - b->y))
#define DIST_NN(a, b)	((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y))
#define OFFSET(i, j)	((j) * this->_map->x * this->_map->precision + (i) * this->_map->precision)
#define BETWEEN(a, b, c)	((a) <= (b) && (b) <= (c))
#define INFINITE	(static_cast<unsigned int>(-1))
#include	<cstddef>
#include	<algorithm>
#include	"Algo/Pathfinding.hpp"
#include	"Factory/Game.hpp"
#include	"Logger.hpp"
#include	<iostream>
#include	"Game/Controller.hpp"

Algo::Pathfinding::Pathfinding(void):
  _map(NULL) {
}

Algo::Pathfinding::~Pathfinding(void) {

}

void	Algo::Pathfinding::setMap(const ::Game::Map *map) {
  this->_map = map;
}

void	Algo::Pathfinding::initAlgo(thisData &data, const std::vector< ::Game::Map::Node *> &nodes, const Vector3d &from, const Vector3d &to, bool hardReinit) const {
  if (hardReinit) {
    data._data.clear();
    data._begin = NULL;
    data._target = NULL;
  }
  double v;
  double step = 1.0 / static_cast<double>(this->_map->precision);
  for (auto it : nodes) {
    if (it) {
      // Logger::getInstance()->logf("Pathfinding : %f <= %f <= %f", Logger::DEBUG, it->x, from.x, it->x + step);
      // Logger::getInstance()->logf("Pathfinding : %f <= %f <= %f", Logger::DEBUG, it->x, to.x, it->x + step);
      // Logger::getInstance()->logf("Pathfinding : %f <= %f <= %f", Logger::DEBUG, it->y, from.y, it->y + step);
      // Logger::getInstance()->logf("Pathfinding : %f <= %f <= %f", Logger::DEBUG, it->y, to.y, it->y + step);
      // Logger::getInstance()->log("", Logger::DEBUG);
      if (hardReinit && BETWEEN(it->x, from.x, it->x + step) && BETWEEN(it->y, from.y, it->y + step)) {
	data._begin = it;
      }
      if (hardReinit && BETWEEN(it->x, to.x, it->x + step) && BETWEEN(it->y, to.y, it->y + step)) {
	data._target = it;
      }
      v = INFINITE + DIST_NP(from, it);
      data._data[it].g = INFINITE;
      data._data[it].f = v;
      data._data[it].status = false;
      data._data[it].from = NULL;
    }
  }
  data._data[data._begin].g = 0;
  data._data[data._begin].f = data._data[data._begin].g + DIST_NN(from, to);
  data._data[data._begin].status = true;
}


const ::Game::Map::Node	*Algo::Pathfinding::jump(const thisData &data, const ::Game::AElement *element, const ::Game::Map::Node *current, Direction direction) const {
  const ::Game::Map::Node *node = current;
  while (1) {
    if (node->neighbors.at(direction) == NULL) {
      return (NULL);
    }
    node = node->neighbors.at(direction);
    if (!this->testPath(element, current, node)) {
      return (NULL);
    }
    if (node == data._target) {
      return (node);
    }
    if (direction == NORTH_WEST || direction == NORTH_EAST || direction == SOUTH_EAST || direction == SOUTH_WEST) {
      const ::Game::Map::Node *j = this->jump(data, element, node, direction == NORTH_WEST || direction == SOUTH_WEST ? WEST : EAST);
      if (j) {
	return (node);
      }
      j = this->jump(data, element, node, direction == NORTH_EAST || direction == NORTH_WEST ? NORTH : SOUTH);
      if (j) {
	return (node);
      }
    }
  }
  return (node);
}

bool	Algo::Pathfinding::testPath(const ::Game::AElement *element, const ::Game::Map::Node *from,
				    const ::Game::Map::Node *to) const {
  double stepx = (to->x - from->x) / 30.0;
  double stepy = (to->x - from->x) / 30.0;
  double x = from->x + 0.5 / static_cast<double>(this->_map->precision);
  double y = from->y + 0.5 / static_cast<double>(this->_map->precision);
  if (this->_map->tryPosition(element, x, y) == false) {
    return (false);
  }
  for (unsigned int j = 0 ; j < 30 ; ++j) {
    x += stepx;
    y += stepy;
    if (this->_map->tryPosition(element, x, y) == false) {
      return (false);
    }
  }
  return (true);
}

void	Algo::Pathfinding::getSuccessors(const thisData &data, const ::Game::AElement *element, const ::Game::Map::Node *current, std::vector<const ::Game::Map::Node *> &nodes) const {
  for (auto it : current->neighbors) {
    if (it.second) {
      const ::Game::Map::Node *j = this->jump(data, element, current, it.first);
      if (j) {
	nodes.push_back(j);
      }
    }
  }
}

struct where {
  double x;
  double y;
};

static const where _where[8] = {
  {-1.0, -1.0},
  {0.0, -1.0},
  {1.0, -1.0},
  {1.0, 0.0},
  {1.0, 1.0},
  {0.0, 1.0},
  {-1.0, 1.0},
  {-1.0, 0.0}
};

const std::vector<Vector3d>	Algo::Pathfinding::rescuePathfinding(thisData &data,
								     const Game::AElement *element,
								     const Vector3d &from, const Vector3d &to,
								     const std::vector<Game::Map::Node *> &nodes,
								     WhatToDo ifFail) const {
  double precision = 0.1;//1.0 / static_cast<double>(this->_map->precision);
  int factor = 1;
  std::vector<Vector3d> waypoints;
  double x, y;
  bool stop = false;
  Vector3d nTo(0.0, 0.0, 0.0);
  while (waypoints.empty() && !stop) {
    x = precision * factor;
    y = precision * factor;
    for (unsigned int i = 0 ; i < 8 && waypoints.empty() ; ++i) {
      nTo.x = to.x + _where[i].x * x;
      nTo.y = to.y + _where[i].y * y;
      nTo.z = to.z;
      if (ifFail == CLOSEST_IN_RANGE && ::Game::Controller::getInstance()->itemIsInRangeOfPoint(element, to.x, to.y, to.z, nTo.x, nTo.y, nTo.z)) {
	this->doAlgo(data, element, from, nTo, nodes, false);
	this->getWaypoints(data, waypoints);
      }
      else {
	stop = true;
      }
    }
    factor++;
  }
  return (waypoints);
}

void	Algo::Pathfinding::getWaypoints(const thisData &data, std::vector<Vector3d> &waypoints) const {
  const Game::Map::Node *current = data._target;
  while (current) {
    waypoints.push_back(Vector3d(current->x, current->y, 0.0));
    current = data._data.at(current).from;
  }
}

void	Algo::Pathfinding::doAlgo(thisData &data, const Game::AElement *element, const Vector3d &from,
				  const Vector3d &to, const std::vector<Game::Map::Node *> &nodes, bool hardReinit) const {
  this->initAlgo(data, nodes, from, to, hardReinit);
  if (data._begin == NULL || data._target == NULL) {
    Logger::getInstance()->logf("Cannot resolve path : no such start/end", Logger::FATAL);
    return ;
  }
  std::list<const Game::Map::Node *> openset;
  openset.push_back(data._begin);
  double tentative_g;
  std::vector<const Game::Map::Node *> successors;
  const ::Game::Map::Node *current;
  while (openset.empty() == false) {
    current = openset.front();
    if (current == data._target) {
      break;
    }
    openset.pop_front();
    data._data[current].status = false;
    successors.assign(0, NULL);
    this->getSuccessors(data, element, current, successors);
    for (auto it : successors) {
      if (it) {
	tentative_g = data._data[current].g + DIST_PP(current, it);
	if (tentative_g < data._data[it].g) {
	  data._data[it].from = current;
	  data._data[it].g = tentative_g;
	  data._data[it].f = data._data[it].g + DIST_NP(to, it);
	  if (data._data[it].status == false) {
	    data._data[it].status = true;
	    bool insert = false;
	    for (auto it2 = openset.begin() ; it2 != openset.end() ; ++it2) {
	      if (data._data[it] < data._data[*it2]) {
		openset.insert(it2, it);
		insert = true;
		break;
	      }
	    }
	    if (!insert) {
	      openset.push_back(it);
	    }
	  }
	}
      }
    }
  }
}

const std::vector<Vector3d>	Algo::Pathfinding::resolve(const Game::AElement *element, const Vector3d &from,
							   const Vector3d &to, WhatToDo ifFail) const {
  Logger::getInstance()->logf("Start A* from (%f ; %f) to (%f ; %f)", Logger::NONE, from.x, from.y, to.x, to.y);
  thisData data;
  const std::vector<Game::Map::Node *> &nodes = this->_map->getPoints();
  this->doAlgo(data, element, from, to, nodes);
  std::vector<Vector3d> waypoints;
  this->getWaypoints(data, waypoints);
  if (waypoints.empty() && ifFail != NOTHING) {
    Logger::getInstance()->logf("TRY RESCUE", Logger::FATAL);
    waypoints = this->rescuePathfinding(data, element, from, to, nodes, ifFail);
  }
  std::reverse(waypoints.begin(), waypoints.end());
  Logger::getInstance()->log("End A*");
  return (waypoints);
}
