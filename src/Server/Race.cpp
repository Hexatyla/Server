//
// Race.cpp<2> for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <geoffrey@epitech.net>
// 
// Started on  Tue Mar 11 17:10:19 2014 geoffrey bauduin
// Last update Tue Mar 11 17:23:33 2014 geoffrey bauduin
//

#include	"Server/Race.hpp"

Server::Race::Race(void):
  ::Game::Race(),
  _start() {

}

Server::Race::~Race(void) {

}

void	Server::Race::init(const std::string &name, const Kernel::Serial &serial) {
  ::Game::Race::init(name, serial);

}

void	Server::Race::destroy(void) {
  ::Game::Race::destroy();
  this->_start.clear();
}

void	Server::Race::addStart(const Kernel::Serial &serial, unsigned int nbr) {
  if (this->_units.find(serial) == this->_units.end()) {
    return ;
  }
  this->_start[serial] = nbr;
}

const std::map<Kernel::Serial, unsigned int>	&Server::Race::getStartInfos(void) const {
  return (this->_start);
}
