//
// Server.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 15:18:46 2014 geoffrey bauduin
// Last update Tue Mar 11 17:21:51 2014 geoffrey bauduin
//

#ifndef FACTORY_SERVER_HPP_
# define FACTORY_SERVER_HPP_

#include	"Kernel/ID.hpp"
#include	"Server/Game.hpp"
#include	"Server/Event.hpp"
#include	"Server/Building.hpp"
#include	"Server/Unit.hpp"
#include	"Server/Hero.hpp"
#include	"Server/Projectile.hpp"
#include	"Server/GamePlayer.hpp"
#include	"Server/Object.hpp"
#include	"Server/Effect.hpp"
#include	"Server/Capacity.hpp"
#include	"Server/Action.hpp"
#include	"Server/GameLoader.hpp"
#include	"Server/GroupMove.hpp"
#include	"Server/Map.hpp"
#include	"Server/Spectator.hpp"
#include	"Server/Race.hpp"

namespace	Factory {

  namespace	Server {

    void	init(void);
    void	end(void);

    class	Game {

    public:
      static ::Server::Game	*create(::Kernel::ID::id_t, const std::vector< ::Server::GamePlayer *> &,
					const ::Server::HasUsers::Container &,
					const std::vector< ::Game::Team *> &, const std::string &,
					::Server::GameLoader *);
      static void		remove(::Server::Game *);

    };

    class	Event {

    public:
      static ::Server::Event	*create(::Server::Event::Type, ::Server::Event::objectType, void *);
      static void		remove(::Server::Event *);

    };

    class	Building {

    public:
      static ::Server::Building	*create(::Kernel::ID::id_t id, const ::Kernel::Serial &serial, const ::Game::Player *player,
					int orientation, double x, double y, double z);
      static void		remove(::Server::Building *);

    };

    class	Unit {

    public:
      static ::Server::Unit	*create(::Kernel::ID::id_t id, const ::Kernel::Serial &serial, const ::Game::Player *player,
					int orientation, double x, double y, double z);
      static void		remove(::Server::Unit *);

    };

    class	Hero {

    public:
      static ::Server::Hero	*create(::Kernel::ID::id_t id, const ::Kernel::Serial &serial, const ::Game::Player *player,
					int orientation, double x, double y, double z);
      static void		remove(::Server::Hero *);

    };

    class	Projectile {

    public:
      static ::Server::Projectile	*create(::Kernel::ID::id_t id, const ::Kernel::Serial &serial, const ::Game::Player *player,
					int orientation, double x, double y, double z);
      static void		remove(::Server::Projectile *);

    };

    class	GamePlayer {

    public:
      static ::Server::GamePlayer	*create(::Server::User *, ::Kernel::ID::id_t, const ::Game::Race *, const ::Game::Team *);
      static void			remove(::Server::GamePlayer *);

    };

    class	Object {

    public:
      static ::Server::Object	*create(::Kernel::ID::id_t id, const ::Kernel::Serial &serial,
					int orientation, double x, double y, double z);
      static void		remove(::Server::Object *);

    };

    class	RessourcesSpot {

    public:
      static ::Server::RessourcesSpot	*create(::Kernel::ID::id_t id, double x, double y, double z, ::Game::Ressources::Type, unsigned int);
      static void		remove(::Server::RessourcesSpot *);

    };

    class	Effect {

    public:
      static ::Server::Effect *create(const ::Kernel::Serial &, const ::Game::Player *, ::Kernel::ID::id_t);
      static void	remove(::Server::Effect *);

    };

    class	Capacity {

    public:
      static ::Server::Capacity *create(const ::Kernel::Serial &, bool, const ::Game::Player *, ::Kernel::ID::id_t);
      static void	remove(::Server::Capacity *);

    };

    class	Map {

    public:
      static ::Server::Map	*create(unsigned int, unsigned int, const std::vector<unsigned int> &,
					const std::vector< ::Server::Map::Spot> &,
					const std::vector<std::pair<double, double> > &);
      static void		remove(::Server::Map *);

    };

    class	Action {

    public:
      static ::Server::Action	*create(::Game::eAction);
      static void		remove(::Server::Action *);

    };

    class	GameLoader {
    public:
      static ::Server::GameLoader *create(const std::list< ::Server::User*> &);
      static void		remove(::Server::GameLoader*);
    };

    class	GroupMove {

    public:
      static ::Server::GroupMove	*create(const std::list<const ::Server::AItem *> &);
      static void	remove(::Server::GroupMove *);

    };

    class	Spectator {

    public:
      static ::Server::Spectator	*create(const Clock *);
      static void			remove(::Server::Spectator *);

      class	ItemData {

      public:
	static ::Server::Spectator::ItemData	*create(void);
	static void				remove(::Server::Spectator::ItemData *);

      };

      class	PlayerData {

      public:
	static ::Server::Spectator::PlayerData	*create(void);
	static void				remove(::Server::Spectator::PlayerData *);

      };

    };

    class	Race {

    public:
      static ::Server::Race	*create(const std::string &, const ::Kernel::Serial &);
      static void		remove(::Server::Race *);

    };

  }


}

#endif
