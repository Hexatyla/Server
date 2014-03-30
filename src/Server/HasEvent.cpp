//
// HasEvent.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Tue Jan 21 17:10:20 2014 geoffrey bauduin
// Last update Sat Mar 15 15:17:27 2014 geoffrey bauduin
//

#include	<algorithm>
#include	<cstddef>
#include	"Server/HasEvent.hpp"
#include	"Factory/Server.hpp"
#include	"Threading/ScopeLock.hpp"

Server::HasEvent::HasEvent(void):
  _events(), _eventMutex(new Mutex) {

}

Server::HasEvent::~HasEvent(void) {
  delete this->_eventMutex;
}

void	Server::HasEvent::init(void) {
}

static void	deleteEvent(Server::Event *event) {
  Factory::Server::Event::remove(event);
}

void	Server::HasEvent::destroy(void) {
  for_each(this->_events.begin(), this->_events.end(), &deleteEvent);
  this->_events.clear();
}

void	Server::HasEvent::addEvent(Server::Event *event) {
  ScopeLock s(this->_eventMutex);
  this->_events.push_back(event);
}

void	Server::HasEvent::onItemDestroyed(Kernel::ID::id_t id, ::Game::Type type) {
  ScopeLock s(this->_eventMutex);
  for (auto it = this->_events.begin() ; it != this->_events.end() ; ) {
    if ((*it)->hasReferenceToItem(id, type)) {
      it = this->_events.erase(it);
    }
    else {
      ++it;
    }
  }
}

Server::Event	*Server::HasEvent::popEvent(void) {
  ScopeLock s(this->_eventMutex);
  Server::Event *event = NULL;
  if (this->_events.empty() == false) {
    event = this->_events.front();
    this->_events.pop_front();
  }
  return (event);
}

unsigned int	Server::HasEvent::size(void) const {
  ScopeLock s(this->_eventMutex);
  return (this->_events.size());
}

void	Server::HasEvent::onReferenceDeleted(void *ptr) {
  ScopeLock s(this->_eventMutex);
  for (auto it = this->_events.begin() ; it != this->_events.end() ; ) {
    if ((*it)->ptr == ptr) {
      it = this->_events.erase(it);
    }
    else {
      ++it;
    }
  }
}
