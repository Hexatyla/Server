//
// UserJob.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Jan 22 13:02:42 2014 gautier lefebvre
// Last update Fri Jan 24 15:23:27 2014 gautier lefebvre
//

#ifndef		__NETWORK_USERJOB_HPP__
#define		__NETWORK_USERJOB_HPP__

#include	"Network/Socket.hpp"
#include	"Protocol/Job.hpp"

namespace	Network {
  struct	UserJob {
    Network::SSocket	*socket;
    Protocol::Job	*job;

    UserJob(Network::SSocket *s = NULL, Protocol::Job *j = NULL);
    ~UserJob();
  };
}

#endif
