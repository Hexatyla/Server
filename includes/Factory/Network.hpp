//
// Network.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Jan 24 15:25:56 2014 gautier lefebvre
// Last update Thu Feb 20 17:46:02 2014 gautier lefebvre
//

#ifndef		__FACTORY_NETWORK_HPP__
#define		__FACTORY_NETWORK_HPP__

#include	"Network/UserJob.hpp"
#include	"Network/Socket.hpp"

namespace	Factory {
  namespace	Network {
    void	init();
    void	end();

    class	UserJob {
    public:
      static ::Network::UserJob	*create(::Network::SSocket *s = NULL, ::Protocol::Job *j = NULL);
      static void		remove(::Network::UserJob*);
    };

    class	SSocket {
    public:
      static ::Network::SSocket *create();
      static void		remove(::Network::SSocket *);
    };
  }
}

#endif
