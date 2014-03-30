//
// Exception.cpp<2> for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Thu Jan 23 13:53:30 2014 gautier lefebvre
// Last update Thu Jan 23 13:53:45 2014 gautier lefebvre
//

#include	"MySQL/Exception.hpp"

MySQL::Exception::Exception(const std::string &what) throw():
  _what(what)
{}

MySQL::Exception::~Exception() throw() {}

const char *MySQL::Exception::what() const throw() { return (this->_what.c_str()); }
