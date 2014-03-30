//
// HasProductionQueue.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Thu Feb  6 14:26:31 2014 geoffrey bauduin
// Last update Wed Mar 26 11:15:50 2014 Gautier Lefebvre
//

#ifndef SERVER_HASPRODUCTIONQUEUE_HPP_
# define SERVER_HASPRODUCTIONQUEUE_HPP_

#include	<cstddef>
#include	<map>
#include	"Game/ProductionQueue.hpp"
#include	"Clock.hpp"
#include	"Threading/Mutex.hpp"
#include	"Kernel/Serial.hpp"
#include	"IFromFactory.hpp"

namespace	Server {

  class	HasProductionQueue: virtual public IFromFactory {

  private:
    struct data {
      Clock	*start;
      double	end;
    };

    std::map< ::Game::ProductionQueue *, data>	_data;
    Mutex					*_mutex;

    void	startProduction(const Clock*, ::Game::ProductionQueue *, double decal = 0.0);
    void	finishProduction(::Game::ProductionQueue *);

  public:
    enum	Status {
      START,
      END_RESTART,
      END,
      NONE
    };

    HasProductionQueue(void);
    virtual ~HasProductionQueue(void);

    void	init(void);
    virtual void destroy(void);

    Status	update(const Clock *, ::Game::ProductionQueue *, Kernel::Serial &);


  };

}

#endif
