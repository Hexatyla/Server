//
// IItem.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Wed Feb 12 17:02:53 2014 geoffrey bauduin
// Last update Wed Feb 12 17:04:06 2014 geoffrey bauduin
//

#ifndef SERVER_IUPDATABLE_HPP_
# define SERVER_IUPDATABLE_HPP_

#include	"Clock.hpp"

namespace	Server {

  class	IUpdatable {

  public:
    virtual ~IUpdatable(void) {}

    virtual bool	update(const Clock *, double) = 0;

  };

}

#endif
