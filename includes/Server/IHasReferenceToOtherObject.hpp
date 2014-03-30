//
// IHasReferenceToOtherObject.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Sun Feb 23 00:58:07 2014 geoffrey bauduin
// Last update Sun Feb 23 01:01:52 2014 geoffrey bauduin
//

#ifndef SERVER_HASREFERENCETOOTHEROBJECT_HPP_
# define SERVER_HASREFERENCETOOTHEROBJECT_HPP_

namespace	Server {

  class	IHasReferenceToOtherObject {

  public:
    virtual ~IHasReferenceToOtherObject(void) {}

    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type) = 0;

  };

}

#endif
