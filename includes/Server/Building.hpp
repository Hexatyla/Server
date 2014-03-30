//
// Building.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 17:19:53 2014 geoffrey bauduin
// Last update Mon Mar 24 18:18:15 2014 geoffrey bauduin
//

#ifndef SERVER_BUILDING_HPP_
# define SERVER_BUILDING_HPP_

#include	<map>
#include	"Game/Building.hpp"
#include	"Server/HasEvent.hpp"
#include	"Server/AItem.hpp"
#include	"Threading/Mutex.hpp"
#include	"Server/HasProductionQueue.hpp"
#include	"IFromFactory.hpp"
#include	"Server/CanUseCapacity.hpp"
#include	"Server/CanBeDamaged.hpp"
#include	"Server/IHasReferenceToOtherObject.hpp"

namespace	Server {

  class	Building: public ::Game::Building, public Server::AItem, public Server::HasProductionQueue,
		  virtual public IFromFactory, public Server::CanUseCapacity, public Server::CanBeDamaged,
		  virtual public Server::IHasReferenceToOtherObject {

  private:
    mutable Mutex	*_dataMutex;
    mutable Mutex	*_buildersMutex;

    std::map<Kernel::ID::id_t, double> _timers;

    void	onAddUnitToQueue(int pos = -1);
    void	onFinishedUnitProduction(const Kernel::Serial &);
    void	onRemoveUnitFromQueue(const Kernel::Serial &);

    void	updateProductionQueue(const Clock *, double);
    void	updateCurrentLife(const Clock *);

    void	createStopBuildingEvent(Kernel::ID::id_t);

    void	startProduction(const Kernel::Serial &, double decal = 0.0);

    void	onAddBuilder(Kernel::ID::id_t);
    void	onRemoveBuilder(Kernel::ID::id_t);
    void	onSetBuilt(void);

  public:
    Building(void);
    virtual ~Building(void);

    void		init(Kernel::ID::id_t id, const Kernel::Serial &serial, const ::Game::Player *player,
			     int orientation, double x, double y, double z);
    virtual void	destroy(void);
    virtual bool	update(const Clock *, double);
    virtual void	addToQueue(const Kernel::Serial &);
    virtual void	placeInQueue(const Kernel::Serial &, unsigned int);
    virtual bool	removeFromQueue(const Kernel::Serial &);

    virtual void	addBuilder(Kernel::ID::id_t);
    virtual void	removeBuilder(Kernel::ID::id_t);

    virtual void	setBuilt(void);

    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);

  };

}

#endif
