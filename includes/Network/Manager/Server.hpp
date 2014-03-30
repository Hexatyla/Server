//
// Server.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Jan 22 12:58:38 2014 gautier lefebvre
// Last update Tue Mar 11 11:56:39 2014 gautier lefebvre
//

#ifndef		__NETWORK_MANAGER_SERVER_HPP__
#define		__NETWORK_MANAGER_SERVER_HPP__

#include	<vector>

#include	"Network/UserJob.hpp"
#include	"SList.hpp"
#include	"Singleton.hpp"

namespace	Network {
  namespace	Manager {
    class	Server :public Singleton<Network::Manager::Server> {
      friend class Singleton<Network::Manager::Server>;

    private:
      Network::SSocket	_socket;
      struct timeval	_clock;
      std::list<Network::SSocket*>	_clients;
      SList<Network::UserJob*>	_jobs;
      Mutex	_mutexExit;
      bool	_end;

      void		reinitClock();

    public:
      Server();
      ~Server();

      bool	init(int);
      void	run();

      void	push(Network::SSocket*, Protocol::Job*);
      Network::UserJob*	getJob();
      void	end();
    };
  }
}

void*		start_network_manager(void*);

#endif
