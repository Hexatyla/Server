//
// Row.hpp for MySQL in /home/gautier/Projets/Tek3/MySQL
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Oct  2 11:52:57 2013 gautier lefebvre
// Last update Thu Jan 23 13:52:31 2014 gautier lefebvre
//

#ifndef			__MYSQL_ROW_HPP__
#define			__MYSQL_ROW_HPP__

#include		<string>
#include		<vector>
#include		<mysql/mysql.h>

namespace		MySQL {
  class			Row {
  private:
    MYSQL_ROW		_row;
    unsigned int	_numFields;
    std::vector<MYSQL_FIELD *> *_fields;

  public:
    Row();
    ~Row();

    bool		load(MYSQL_RES *, std::vector<MYSQL_FIELD*>*);
    unsigned int	numFields() const;
    std::string		operator[](unsigned int) const;
    std::string		operator[](const std::string &) const;
  };
}

#endif
