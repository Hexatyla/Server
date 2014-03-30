//
// Race.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Tue Mar 11 17:08:36 2014 geoffrey bauduin
// Last update Tue Mar 11 17:10:08 2014 geoffrey bauduin
//

#ifndef SERVER_RACE_HPP_
# define SERVER_RACE_HPP_

#include	<map>
#include	"Kernel/Serial.hpp"
#include	"Game/Race.hpp"

namespace	Server {

  class	Race: public ::Game::Race, virtual public IFromFactory {

  protected:
    std::map<Kernel::Serial, unsigned int>	_start;

  public:
    Race(void);
    virtual ~Race(void);

    void	init(const std::string &, const Kernel::Serial &);
    virtual void	destroy(void);

    void	addStart(const Kernel::Serial &, unsigned int);

    const std::map<Kernel::Serial, unsigned int>	&getStartInfos(void) const;


  };

}

#endif
