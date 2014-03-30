//
// HasJobs.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 28 16:45:25 2014 geoffrey bauduin
// Last update Wed Mar 19 14:00:49 2014 geoffrey bauduin
//

#include	<algorithm>
#include	"Server/HasJobs.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Protocol.hpp"

Server::HasJobs::HasJobs(void):
  _jobs(), _jobsMutex(new Mutex) {

}

Server::HasJobs::~HasJobs(void) {
  delete this->_jobsMutex;
}

static void	deleteJob(Protocol::Job *job) {
  Factory::Protocol::Job::remove(job);
}

void	Server::HasJobs::init(void) {

}

void	Server::HasJobs::destroy(void) {
  while (this->_jobs.empty() == false) {
    deleteJob(this->_jobs.front());
    this->_jobs.pop();
  }
}

void	Server::HasJobs::addJob(Protocol::Job *job) {
  ScopeLock s(this->_jobsMutex);
  this->_jobs.push(job);
}

Protocol::Job	*Server::HasJobs::popJob(void) {
  ScopeLock s(this->_jobsMutex);
  Protocol::Job *job = NULL;
  if (this->_jobs.empty() == false) {
    job = this->_jobs.front();
    this->_jobs.pop();
  }
  return (job);
}
