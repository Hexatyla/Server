//
// Database.hpp for MySQL in /home/gautier/Projets/Tek3/MySQL
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Oct  2 11:41:59 2013 gautier lefebvre
// Last update Mon Mar 10 17:10:45 2014 geoffrey bauduin
//

#ifndef		__MYSQL_DATABASE_HPP__
#define		__MYSQL_DATABASE_HPP__

#include	<string>

#include	"Threading/Mutex.hpp"
#include	"MySQL/Result.hpp"

namespace	MySQL {
  class		Database {
  private:
    Mutex	_mutex;
    MYSQL	*_mysql;
    bool	_closed;

  public:
    Database();
    ~Database();

    Mutex*	getMutex();
    bool	connect(const char *, const char *, const char *, const char *, unsigned int p = 0, const char *s = NULL, unsigned long f = 0);
    bool	connect(const std::string&, const std::string&, const std::string&, const std::string&, unsigned int p = 0, const char *s = NULL, unsigned long f = 0);
    void	close();
    bool	query(const char *);
    bool	query(const std::string &);
    const char*	error();
    Result*	storeResult();
    void	storeResult(Result &);
    const std::string	realEscapeString(const char *);
    const std::string	realEscapeString(const std::string&);
  };
}

#endif
