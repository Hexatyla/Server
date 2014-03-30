//
// GameLoader.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Thu Feb 13 12:51:05 2014 gautier lefebvre
// Last update Wed Mar 26 11:57:29 2014 Gautier Lefebvre
//

#ifndef		__SERVER_GAMELOADER_HPP__
#define		__SERVER_GAMELOADER_HPP__

#include	<map>
#include	<list>
#include	"IFromFactory.hpp"
#include	"Threading/Mutex.hpp"
#include	"Server/User.hpp"

namespace	Server {
  class		GameLoader :virtual public IFromFactory {
  private:
    struct	File {
      std::string			fullpath;
      std::string			filename;
      std::map<Server::User*, bool>	users;

      File(const std::string&, const std::string &, const std::list<Server::User*>&);
      File(const File&);
      File& operator=(const File &);
      ~File();

      bool	isReady() const;
      void	sendFileToUser(Server::User *);
    };

    Mutex				*_mutex;
    std::list<File>			_files;
    std::list<Server::User*>		_userList;
    std::list<Server::User*>		_spectators;
    std::map<Server::User*, bool>	_users;

  public:
    GameLoader();
    ~GameLoader();

    virtual void	destroy(void);
    void	init(const std::list<Server::User*>&);

    const std::string getMapPath() const;
    void	signalLaunch();
    void	addFile(const std::string &);
    void	askFileExists();
    bool	isGameReady() const;
    void	playerAnswered(const Server::User *, const std::string &, bool);
    void	playerReconnected(Server::User*);
    void	playerIsReady(Server::User *);
    bool	isUserInGameLoader(const Server::User*) const;

    const std::list<Server::User*> &getUserList() const;

    void	askFileExistsSpectator(Server::User *);
    bool	isSpectatorInGameLoader(const Server::User*) const;
    void	addSpectator(Server::User *);
    void	removeSpectator(Server::User *);
  };
}

#endif
