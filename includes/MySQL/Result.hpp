//
// Result.hpp for MySQL in /home/gautier/Projets/Tek3/MySQL
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Oct  2 11:51:07 2013 gautier lefebvre
// Last update Mon Mar 24 00:17:09 2014 Gautier Lefebvre
//

#ifndef		__MYSQL_RESULT_HPP__
#define		__MYSQL_RESULT_HPP__

#include	"MySQL/Row.hpp"

namespace	MySQL {
  class		Result {
  private:
    MYSQL_RES		*_res;
    unsigned long long	_rows;
    unsigned int	_numFields;
    std::vector<MYSQL_FIELD*>* _fields;
    bool		_free;

  public:
    Result();
    ~Result();

    void		store(MYSQL*);
    void		free();
    Row*		fetchRow();
    bool		fetchRow(Row &);
    unsigned long long	numRows() const;
    unsigned int	numFields() const;

    bool		isFreed() const;
  };
}

#endif
