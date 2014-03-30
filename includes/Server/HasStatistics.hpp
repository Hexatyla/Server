//
// HasStatistics.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Fri Feb 14 19:33:09 2014 geoffrey bauduin
// Last update Fri Feb 14 19:59:52 2014 geoffrey bauduin
//

#ifndef SERVER_HAS_STATISTICS_HPP_
# define SERVER_HAS_STATISTICS_HPP_

#include	<map>
#include	"Kernel/Serial.hpp"
#include	"IFromFactory.hpp"
#include	"Game/Race.hpp"
#include	"Threading/Mutex.hpp"
#include	"Game/Ressources.hpp"

namespace	Server {

  class	HasStatistics: virtual public IFromFactory {

  private:
    typedef std::map<Kernel::Serial, unsigned int> Quantifier;
    typedef std::map<Kernel::Serial, Quantifier> Container;

    Mutex	*_mutex;

    Container	_owKills;
    Container	_owDestructions;

    Quantifier	_built;
    Quantifier	_destroyed;

    ::Game::Ressources	_recolted;
    ::Game::Ressources	_spent;

    void	init(const std::vector<Kernel::Serial> &);

  public:
    HasStatistics(void);
    virtual ~HasStatistics(void);

    void	init(const ::Game::Race *);
    virtual void	destroy(void);

    void	hasKilledOtherUnit(const Kernel::Serial &, const Kernel::Serial &);
    void	hasDestroyedOtherBuilding(const Kernel::Serial &, const Kernel::Serial &);
    void	hasBuiltItem(const Kernel::Serial &);
    void	hasLostItem(const Kernel::Serial &);

    void	hasSpentRessources(const ::Game::Ressources &);
    void	hasSpentRessources(::Game::Ressources::Type, unsigned int);
    void	hasRecoltedRessources(const ::Game::Ressources &);
    void	hasRecoltedRessources(::Game::Ressources::Type, unsigned int);

    void	onUpdate(void);


  };

}

#endif
