//
// Exception.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Thu Jan 23 13:52:44 2014 gautier lefebvre
// Last update Thu Jan 23 13:53:08 2014 gautier lefebvre
//

#ifndef		__MYSQL_EXCEPTION_HPP__
#define		__MYSQL_EXCEPTION_HPP__

#include	<string>
#include	<exception>

namespace	MySQL {
  class		Exception :public std::exception {
  private:
    std::string	_what;

  public:
    Exception(const std::string &) throw();
    virtual ~Exception() throw();

    virtual const char *what() const throw();
  };
}

#endif
