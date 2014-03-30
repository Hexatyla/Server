//
// Row.cpp for MySQL in /home/gautier/Projets/Tek3/MySQL
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Oct  2 13:17:03 2013 gautier lefebvre
// Last update Mon Oct 14 23:11:13 2013 geoffrey bauduin
//

#include	"MySQL/Row.hpp"

MySQL::Row::Row(): _numFields(0), _fields(NULL)
{}

MySQL::Row::~Row()
{}

bool		MySQL::Row::load(MYSQL_RES *res, std::vector<MYSQL_FIELD*> *fields)
{
  if (res == NULL || (_row = mysql_fetch_row(res)) == NULL)
    return (false);
  _numFields = mysql_num_fields(res);
  _fields = fields;
  return (true);
}

unsigned int	MySQL::Row::numFields() const
{
  return (_numFields);
}

std::string	MySQL::Row::operator[](unsigned int idx) const
{
  return (idx >= _numFields ? "" : (_row[idx] != NULL ? _row[idx] : ""));
}

std::string	MySQL::Row::operator[](const std::string &field) const
{
  unsigned int	i = 0;

  for (std::vector<MYSQL_FIELD*>::const_iterator it = _fields->begin() ; it != _fields->end() ; ++it)
    {
      if (std::string((*it)->name) == field && i < _numFields)
	{
	  return (_row[i] != NULL ? _row[i] : "");
	}
      ++i;
    }
  return ("");
}
