//
// Pathfinding.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Fri Jan 24 15:58:48 2014 geoffrey bauduin
// Last update Sat Mar 22 17:41:04 2014 geoffrey bauduin
//

#ifndef ALGO_ASTAR_HPP_
# define ALGO_ASTAR_HPP_

#include	<vector>
#include	"Game/Map.hpp"
#include	"Game/AElement.hpp"
#include	"Vector.hpp"

namespace	Algo {

  class	Pathfinding {

  private:
    struct data {
      double		g;
      double		f;
      bool		status;
      const ::Game::Map::Node *from;

      bool operator<(const data &rhs) {
	return (this->g < rhs.g);
      }

      bool operator>(const data &rhs) {
	return (this->g > rhs.g);
      }

    };

  public:
    enum	WhatToDo {
      NOTHING,
      CLOSEST,
      CLOSEST_IN_RANGE
    };

  private:
    struct thisData {
      std::map<const ::Game::Map::Node *, data>	_data;
      const ::Game::Map::Node	*_begin;
      const ::Game::Map::Node	*_target;
    };

    const ::Game::Map *_map;

    void	initAlgo(thisData &, const std::vector< ::Game::Map::Node *> &, const Vector3d &,
			 const Vector3d &, bool) const;
    void	getSuccessors(const thisData &, const ::Game::AElement *, const ::Game::Map::Node *, std::vector<const ::Game::Map::Node *> &) const;
    const ::Game::Map::Node *jump(const thisData &, const ::Game::AElement *, const ::Game::Map::Node *, Direction) const;
    const std::vector<Vector3d> rescuePathfinding(thisData &, const Game::AElement *, const Vector3d &from,
						  const Vector3d &to, const std::vector<Game::Map::Node *> &nodes,
						  WhatToDo ifFail) const;
    void	doAlgo(thisData &, const ::Game::AElement *, const Vector3d &, const Vector3d &,
		       const std::vector<Game::Map::Node *> &, bool hardReinit = true) const;
    void	getWaypoints(const thisData &, std::vector<Vector3d> &) const;

    bool	testPath(const ::Game::AElement *, const ::Game::Map::Node *, const ::Game::Map::Node *) const;

  public:
    Pathfinding(void);
    ~Pathfinding(void);
    void	setMap(const Game::Map *);
    const std::vector<Vector3d>	resolve(const Game::AElement *, const Vector3d &from,
					const Vector3d &to, WhatToDo ifFail = NOTHING) const;

  };

}

#endif
