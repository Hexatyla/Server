//
// Result.cpp for MySQL in /home/gautier/Projets/Tek3/MySQL
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Oct  2 13:13:47 2013 gautier lefebvre
// Last update Fri Mar  7 17:40:56 2014 gautier lefebvre
//

#include	"MySQL/Result.hpp"

MySQL::Result::Result(): _res(NULL), _fields(NULL), _free(true)
{}

MySQL::Result::~Result()
{
  if (_free == false)
    mysql_free_result(_res);
  if (_fields) {
    delete (_fields);
  }
}

void		MySQL::Result::store(MYSQL *mysql)
{
  if (_free == false)
    this->free();

  _res = mysql_store_result(mysql);
  _rows = mysql_num_rows(_res);
  _numFields = mysql_num_fields(_res);

  if (_fields)
    delete (_fields);

  _fields = new std::vector<MYSQL_FIELD *>();

  MYSQL_FIELD	*field;

  while ((field = mysql_fetch_field(_res)) != NULL)
    _fields->push_back(field);
  _free = false;
}

MySQL::Row*	MySQL::Result::fetchRow()
{
  MySQL::Row*	row = new MySQL::Row();

  if (row->load(_res, _fields) == false)
    {
      delete (row);
      return (NULL);
    }
  return (row);
}

bool		MySQL::Result::fetchRow(Row &row)
{
  if (row.load(_res, _fields) == false)
    return (false);
  return (true);
}

unsigned long long	MySQL::Result::numRows() const
{
  return (_rows);
}

unsigned int	MySQL::Result::numFields() const
{
  return (_numFields);
}

bool		MySQL::Result::isFreed() const
{
  return (_free);
}

void		MySQL::Result::free()
{
  if (_free == false)
    mysql_free_result(_res);
  _free = true;
  _rows = 0;
  _numFields = 0;
  if (_fields) {
    delete (_fields);
  }
  _fields = NULL;
  _res = NULL;
}
