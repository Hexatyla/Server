//
// DBList.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Thu Jan 23 14:48:43 2014 gautier lefebvre
// Last update Mon Mar 10 16:46:32 2014 geoffrey bauduin
//

#include	<stdexcept>
#include	"Logger.hpp"
#include	"MySQL/DBList.hpp"

MySQL::DBList::DBList():
  _dbs(){
  if (mysql_library_init(0, NULL, NULL)) {
    Logger::getInstance()->log("Cannot initialize SQL library", Logger::FATAL);
  }
  this->_dbs[MySQL::DBList::MAIN] = new MySQL::Database();
}

MySQL::DBList::~DBList() {
  Logger::getInstance()->log("Destroying SQL Handlers", Logger::INFORMATION);
  for (auto it : this->_dbs) {
    delete (it.second);
  }
  mysql_library_end();
}

MySQL::Database*	MySQL::DBList::get(MySQL::DBList::ID id) {
  try {
    return (this->_dbs.at(id));
  }
  catch (const std::out_of_range&) {
    return (NULL);
  }
}
