//
// SQL.cpp for ex in /home/deleme/pfa
// 
// Made by anthony delemer
// Login   <deleme_a@epitech.net>
// 
// Started on  Wed Jan 29 14:16:56 2014 anthony delemer
// Last update Fri Feb 14 23:02:27 2014 geoffrey bauduin
//

#include	"Parser/SQL.hpp"

Parser::SQL::SQL() {}

Parser::SQL::~SQL() {}

bool		Parser::SQL::parse(const std::string& filename)
{
  Json::Value root;
  if (!Parser::JSON::getInstance()->parse(filename, root)) {
    return (false);
  }
  const Json::Value &username = root["Username"];
  const Json::Value &password = root["Password"];

  this->_username = username.asString();
  this->_password = password.asString();
  return (true);
}

const std::string&	Parser::SQL::getPassword() const
{
  return (this->_password);
}

const std::string&	Parser::SQL::getUsername() const
{
  return (this->_username);
}
