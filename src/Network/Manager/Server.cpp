//
// Server.cpp for server in /home/deleme/pfa
// 
// Made by anthony delemer
// Login   <deleme_a@epitech.net>
// 
// Started on  Wed Jan 29 15:18:31 2014 anthony delemer
// Last update Tue Mar 18 16:40:03 2014 gautier lefebvre
//

//Debug
#include	<iostream>
#include	<string>
#include	<unistd.h>
//End

#include	<signal.h>

#include	"Factory/Protocol.hpp"
#include	"Network/Manager/Server.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Network/Exception.hpp"
#include	"Logger.hpp"
#include	"Factory/Network.hpp"

#define		MAX(x, y)	((x) > (y) ? (x) : (y))
#define		MAX_CLIENT	(100)

void		signal_sigpipe(int);

Network::Manager::Server::Server() : _socket(), _clock(), _clients(), _jobs(), _mutexExit(), _end(false) {}
Network::Manager::Server::~Server() {
  for (auto s : this->_clients) {
    Factory::Network::SSocket::remove(s);
  }
}

void	Network::Manager::Server::reinitClock()
{
  this->_clock.tv_sec = 0;
  this->_clock.tv_usec = 40000;
}

bool	Network::Manager::Server::init(int port)
{
  try {
    signal(SIGPIPE, &signal_sigpipe);
    this->_socket.create();
    this->_socket.bind(port);
    this->_socket.listen(MAX_CLIENT);
  }
  catch (Network::Exception *e) {
    Logger::getInstance()->log("Fail initializing Network Manager", Logger::FATAL);
    delete (e);
    return (false);
  }
  return (true);
}

void	Network::Manager::Server::run()
{
  fd_set	rset, wset;
  int	rmax, wmax;

  Logger::getInstance()->log("Network Manager launched", Logger::INFORMATION);

  while (true)
    {
      try {
	{
	  ScopeLock kikoolol(&this->_mutexExit);
	  if (this->_end) { break ; }
	}

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	rmax = 0;
	wmax = 0;
	FD_SET(this->_socket.getFD(), &rset);
	rmax = MAX(this->_socket.getFD(), rmax);
	for (auto it : this->_clients) {
	  FD_SET(it->getFD(), &rset);
	  rmax = MAX(it->getFD(), rmax);
	  if (it->hasSomethingToSend()) {
	    FD_SET(it->getFD(), &wset);
	    wmax = MAX(it->getFD(), wmax);
	  }
	}

	this->reinitClock();
	select(MAX(rmax, wmax) + 1, &rset, &wset, NULL, &(this->_clock));

	for (std::list<Network::SSocket*>::iterator it = this->_clients.begin() ; it != this->_clients.end() ; ++it) {
	  if (FD_ISSET((*it)->getFD(), &wset)) {
	    try {
	      (*it)->send();
	    }
	    catch (Network::Exception *e) {
	      delete (e);
	      Logger::getInstance()->log("Client disconnected", Logger::INFORMATION);
	      Network::SSocket *socket = *it;
	      it = this->_clients.erase(it);
	      Protocol::Job *job = Factory::Protocol::Job::create();
	      job->exit();
	      ScopeLock a(&(this->_jobs));
	      this->_jobs.push_back(Factory::Network::UserJob::create(socket, job));	      
	    }
	  }
	}

	if (FD_ISSET(this->_socket.getFD(), &rset)) {
	  Logger::getInstance()->log("New client connected", Logger::INFORMATION);
	  Network::SSocket *socket = Factory::Network::SSocket::create();
	  try {
	    this->_socket.accept(socket);
	    this->_clients.push_back(socket);
	  }
	  catch (Network::Exception *e) {
	    delete (e);
	  }
	}
	for (std::list<Network::SSocket*>::iterator it = this->_clients.begin() ; it != this->_clients.end() ; ++it) {
	  if (FD_ISSET((*it)->getFD(), &rset)) {
	    try {
	      (*it)->recv();
	    }
	    catch (Network::Exception *e) {
	      delete (e);
	      Logger::getInstance()->log("Client disconnected", Logger::INFORMATION);
	      Network::SSocket *socket = *it;
	      it = this->_clients.erase(it);
	      Protocol::Job *job = Factory::Protocol::Job::create();
	      job->exit();
	      ScopeLock a(&(this->_jobs));
	      this->_jobs.push_back(Factory::Network::UserJob::create(socket, job));
	    }
	  }
	}

	for (auto it : this->_clients) {
	  while (it->hasNewJob()) {
	    Protocol::Job *job = it->getNextJob();
	    if (job) {
	      ScopeLock s(&this->_jobs);
	      this->_jobs.push_back(Factory::Network::UserJob::create(it, job));
	    }
	  }
	}
      }
      catch (Network::Exception *e) {
	Logger::getInstance()->log(e->what(), Logger::FATAL);
	delete (e);
	usleep(50000);
      }
    }

  Logger::getInstance()->log("Network Manager ended successfully", Logger::INFORMATION);

}

void	Network::Manager::Server::push(Network::SSocket *sock, Protocol::Job* job)
{
  if (sock && job) {
    sock->push(job);
  }
  else if (!sock && job) {
    Factory::Protocol::Job::remove(job);
  }
}

Network::UserJob*	Network::Manager::Server::getJob() {
  ScopeLock s(&this->_jobs);
  if (this->_jobs.empty()) {
    return (NULL);
  }
  Network::UserJob *uj = this->_jobs.front();
  this->_jobs.pop_front();
  return (uj);
}

void		Network::Manager::Server::end() {
  ScopeLock kikoolol(&this->_mutexExit);
  this->_end = true;
}

void*		start_network_manager(void *ptr)
{
  Network::Manager::Server *nm = reinterpret_cast<Network::Manager::Server*>(ptr);
  nm->run();
  return (nm);
}
