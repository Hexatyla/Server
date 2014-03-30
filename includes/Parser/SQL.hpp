//
// SQL.hpp for ex in /home/deleme/pfa
// 
// Made by anthony delemer
// Login   <deleme_a@epitech.net>
// 
// Started on  Wed Jan 29 14:18:42 2014 anthony delemer
// Last update Fri Jan 31 12:17:19 2014 geoffrey bauduin
//

#ifndef		PARSER_SQL_HPP_
# define	PARSER_SQL_HPP_

#include	<iostream>
#include	<string>
#include	"Parser/JSON.hpp"

namespace	Parser
{
  class		SQL
  {
  private:
    std::string _username;
    std::string _password;
  public:
    SQL();
    ~SQL();

    bool	parse(const std::string&);
    const std::string& getUsername() const;
    const std::string& getPassword() const;
  };
}

#endif
