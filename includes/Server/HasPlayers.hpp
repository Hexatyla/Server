//
// HasPlayers.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sun Feb 16 00:58:04 2014 geoffrey bauduin
// Last update Sat Mar 15 15:20:09 2014 geoffrey bauduin
//

#ifndef SERVER_HASPLAYERS_HPP_
# define SERVER_HASPLAYERS_HPP_

#include	<map>
#include	"Server/GamePlayer.hpp"
#include	"Threading/Mutex.hpp"

namespace	Server {

  class	HasPlayers: virtual public IFromFactory {

  protected:
    Mutex	*_playerMutex;
    std::map<Kernel::ID::id_t, Server::GamePlayer *>	_players;

  public:
    HasPlayers(void);
    virtual ~HasPlayers(void);

    void	init(const std::vector<Server::GamePlayer *> &);
    virtual void	destroy(void);

    void	removePlayer(Kernel::ID::id_t);
    void	removePlayer(Server::GamePlayer *);

  };

}

#endif
