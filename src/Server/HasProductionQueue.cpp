//
// HasProductionQueue.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Thu Feb  6 14:26:46 2014 geoffrey bauduin
// Last update Tue Mar 25 11:37:26 2014 geoffrey bauduin
//

#include	"Server/HasProductionQueue.hpp"
#include	"Factory/Factory.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Kernel/Manager.hpp"

Server::HasProductionQueue::HasProductionQueue(void):
  _data(), _mutex(new Mutex) {

}

Server::HasProductionQueue::~HasProductionQueue(void) {
  delete this->_mutex;
}

void	Server::HasProductionQueue::init(void) {

}

void	Server::HasProductionQueue::destroy(void) {
  for (auto it : this->_data) {
    if (it.second.start) {
      Factory::Clock::remove(it.second.start);
    }
  }
  this->_data.clear();
}

void	Server::HasProductionQueue::startProduction(const Clock *clock, Game::ProductionQueue *queue, double decal) {
  const Kernel::Time *timers = Kernel::Manager::getInstance()->getTimer(queue->front());
  this->_data[queue].start->clone(clock);
  this->_data[queue].end = clock->getElapsedTime() + timers->creation - decal;
}

void	Server::HasProductionQueue::finishProduction(Game::ProductionQueue *queue) {
  queue->pop();
}

Server::HasProductionQueue::Status	Server::HasProductionQueue::update(const Clock *clock,
									   Game::ProductionQueue *queue,
									   Kernel::Serial &produced) {
  ScopeLock s(this->_mutex);
  if (queue->empty() == false) {
    if (this->_data.find(queue) == this->_data.end()) {
      this->_data[queue] = {Factory::Clock::create(), 0.0};
      this->startProduction(clock, queue);
      return (Server::HasProductionQueue::START);
    }
    else if (clock->getElapsedTime() >= this->_data[queue].end) {
      produced.assign(queue->front());
      this->finishProduction(queue);
      if (!queue->empty()) {
	this->startProduction(clock, queue, clock->getElapsedTime() - this->_data[queue].end);
	return (Server::HasProductionQueue::END_RESTART);
      }
      return (Server::HasProductionQueue::END);
    }
  }
  return (Server::HasProductionQueue::NONE);
}
