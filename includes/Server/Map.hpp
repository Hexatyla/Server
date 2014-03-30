//
// Map.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sat Feb 15 23:22:55 2014 geoffrey bauduin
// Last update Wed Mar 26 11:13:24 2014 Gautier Lefebvre
//

#ifndef SERVER_MAP_HPP_
# define SERVER_MAP_HPP_

#include	<vector>
#include	<list>
#include	"Game/Map.hpp"
#include	"Server/HasEvent.hpp"
#include	"Threading/Mutex.hpp"
#include	"Game/Ressources.hpp"

namespace	Server {

  class	Map: public ::Game::Map, public Server::HasEvent, virtual public IFromFactory {

  public:
    struct Spot {
      ::Game::Ressources::Type type;
      double x;
      double y;
      unsigned int amount;
    };

  private:
    Mutex	*_mutex;
    std::vector<Spot> _spots;
    std::list<std::pair<double, double> > _starters;

    void	createOnRangeEvent(const ::Game::AElement *, const ::Game::AElement *);

    virtual bool	_push(const ::Game::AElement *, bool rv = true);
    virtual bool	_remove(const ::Game::AElement *, bool rv = true);

  public:
    Map(void);
    virtual ~Map(void);

    void	init(unsigned int, unsigned int, const std::vector<unsigned int> &, const std::vector<Spot> &,
		     const std::vector<std::pair<double, double> > &);
    virtual void destroy(void);

    virtual bool	push(const ::Game::AElement *);
    virtual bool	remove(const ::Game::AElement *);

    const std::vector<Spot> &getSpots(void) const;
    bool	getStartingPosition(double &, double &, double &);

  };

}

#endif
