//
// HasJobs.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 28 16:44:39 2014 geoffrey bauduin
// Last update Fri Feb 14 18:22:27 2014 geoffrey bauduin
//

#ifndef SERVER_HASJOBS_HPP_
# define SERVER_HASJOBS_HPP_

#include	<queue>
#include	"Protocol/Job.hpp"
#include	"Threading/Mutex.hpp"
#include	"IFromFactory.hpp"

namespace	Server {

  class	HasJobs: virtual public IFromFactory {

  private:
    std::queue<Protocol::Job *>	_jobs;
    Mutex			*_jobsMutex;

  public:
    HasJobs(void);
    virtual ~HasJobs(void);

    void	init(void);
    virtual void destroy(void);

    void	addJob(Protocol::Job *);
    Protocol::Job	*popJob(void);

  };

}


#endif
