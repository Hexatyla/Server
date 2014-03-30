//
// HasUsers.hpp for $ in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 29 16:02:28 2014 geoffrey bauduin
// Last update Sat Mar 15 21:40:19 2014 geoffrey bauduin
//

#ifndef SERVER_HASUSERS_HPP_
# define SERVER_HASUSERS_HPP_

#include	<list>
#include	"Server/User.hpp"
#include	"Threading/Mutex.hpp"
#include	"IFromFactory.hpp"

namespace	Network {
  class		SSocket;
}

namespace	Server {

  class	HasUsers: virtual public IFromFactory {

  public:
    typedef std::list<Server::User *> Container;

  protected:
    Mutex	*_uMutex;

  protected:
    Container	_users;

  public:
    HasUsers(void);
    virtual ~HasUsers(void);

    void	init(void);
    virtual void destroy(void);
    void	init(const Container &);
    const Container &getUsers(void) const;
    Container	&getUsers(void);
    void	addUser(Server::User *);
    void	removeUser(Server::User *);
    void	addUsers(const Container &);
    Server::User*	findUser(const std::string&);
    Server::User*	findUser(const Network::SSocket *);
  };

}

#endif
