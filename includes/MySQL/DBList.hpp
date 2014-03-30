//
// DBList.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Thu Jan 23 14:16:23 2014 gautier lefebvre
// Last update Thu Jan 23 15:07:34 2014 gautier lefebvre
//

#ifndef		__MYSQL_DBLIST_HPP__
#define		__MYSQL_DBLIST_HPP__

#include	<map>
#include	"MySQL/Database.hpp"
#include	"Singleton.hpp"

namespace	MySQL {
  class		DBList :public Singleton<MySQL::DBList> {
    friend class	Singleton;

  public:
    enum	ID {
      MAIN
    };

  private:
    std::map<ID, MySQL::Database*>	_dbs;

    DBList();
    virtual ~DBList();

  public:
    MySQL::Database*	get(ID);
  };
}

#endif
