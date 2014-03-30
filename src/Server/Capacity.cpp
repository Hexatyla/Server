//
// Capacity.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Wed Feb 12 15:34:26 2014 geoffrey bauduin
// Last update Sat Mar 15 15:02:56 2014 geoffrey bauduin
//

#include	"Server/Capacity.hpp"
#include	"Factory/Protocol.hpp"

Server::Capacity::Capacity(void):
  Game::Capacity(),
  _mutex(new Mutex) {

}

Server::Capacity::~Capacity(void) {
  delete this->_mutex;
}

void	Server::Capacity::init(const Kernel::Serial &serial, bool isAuto, const Game::Player *player, Kernel::ID::id_t owner) {
  Game::Capacity::init(serial, isAuto, player, owner);
  HasJobs::init();
}

void	Server::Capacity::setAutomatic(bool automatic) {
  ScopeLock s(this->_mutex);
  ::Game::Capacity::setAutomatic(automatic);
  this->onSetAutomatic();
}

void	Server::Capacity::onSetAutomatic(void) {
  Protocol::Job *job = Factory::Protocol::Job::create();
  job->activateCapacity(this->getOwner(), this->serial(), this->isAutomatic());
  this->addJob(job);
}

bool	Server::Capacity::update(const Clock *, double) {
  return (true);
}

void	Server::Capacity::destroy(void) {
  Game::Capacity::destroy();
  HasJobs::destroy();
}
