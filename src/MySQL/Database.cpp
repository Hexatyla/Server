//
// Database.cpp for MySQL in /home/gautier/Projets/Tek3/MySQL
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Oct  2 13:05:04 2013 gautier lefebvre
// Last update Mon Mar 10 17:10:50 2014 geoffrey bauduin
//

#include	<cstring>
#include	"MySQL/Database.hpp"

MySQL::Database::Database(): _mysql(NULL), _closed(true)
{}

MySQL::Database::~Database()
{
  this->close();
}

Mutex*		MySQL::Database::getMutex() { return (&this->_mutex); }

bool		MySQL::Database::connect(const char *host, const char *user, const char *pwd, const char *db, unsigned int port, const char *socket, unsigned long flag)
{
  if (_closed == false)
    this->close();

  if ((_mysql = mysql_init(NULL)) != NULL)
    {
      _mysql = mysql_real_connect(_mysql, host, user, pwd, db, port, socket, flag);
      _closed = false;
    }
  return (_mysql != NULL);
}

bool		MySQL::Database::connect(const std::string &host, const std::string &user, const std::string& pwd, const std::string &db, unsigned int port, const char *socket, unsigned long flag)
{
  if (_closed == false)
    this->close();

  if ((_mysql = mysql_init(NULL)) != NULL)
    {
      _mysql = mysql_real_connect(_mysql, host.c_str(), user.c_str(), pwd.c_str(), db.c_str(), port, socket, flag);
      _closed = false;
    }
  return (_mysql != NULL);
}

bool		MySQL::Database::query(const char *str)
{
  return (mysql_query(_mysql, str) == 0);
}

bool		MySQL::Database::query(const std::string &str) {
  return (this->query(str.c_str()));
}

const char *	MySQL::Database::error()
{
  return (mysql_error(_mysql));
}

MySQL::Result *	MySQL::Database::storeResult()
{
  MySQL::Result	*res = new MySQL::Result();

  res->store(_mysql);
  return (res);
}

void		MySQL::Database::storeResult(MySQL::Result &res)
{
  res.store(_mysql);
}

const std::string MySQL::Database::realEscapeString(const char *from)
{
  unsigned long	length = strlen(from);
  char *	to = new char[length * 2 + 1];
  std::string n;

  mysql_real_escape_string(_mysql, to, from, length);
  n = to;
  delete[] to;

  return (n);
}

const std::string	MySQL::Database::realEscapeString(const std::string &str) {
  return (this->realEscapeString(str.c_str()));
}

void		MySQL::Database::close()
{
  if (_closed == false)
    mysql_close(_mysql);
  _mysql = NULL;
  _closed = true;
}
