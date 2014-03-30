//
// HasObject.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 18:02:46 2014 geoffrey bauduin
// Last update Sat Mar 15 15:19:41 2014 geoffrey bauduin
//

#include	"Threading/ScopeLock.hpp"
#include	"Server/HasObject.hpp"
#include	"Factory/Server.hpp"

Server::HasObject::HasObject(void):
  _objectMutex(new Mutex), _objects() {}

Server::HasObject::~HasObject(void) {
  delete this->_objectMutex;

}

void	Server::HasObject::init(void) {}

void	Server::HasObject::destroy(void) {
  for (auto it : this->_objects) {
    Factory::Server::Object::remove(it.second);
  }
  this->_objects.clear();
}

void	Server::HasObject::addObject(Server::Object *object) {
  ScopeLock s(this->_objectMutex);
  this->_objects[object->getID()] = object;
}

void	Server::HasObject::removeObject(Server::Object *object) {
  this->removeObject(object->getID());
}

void	Server::HasObject::removeObject(Kernel::ID::id_t id) {
  ScopeLock s(this->_objectMutex);
  this->_objects.erase(id);
}
