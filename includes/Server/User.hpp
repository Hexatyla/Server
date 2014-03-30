//
// User.hpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Jan 17 16:38:53 2014 gautier lefebvre
// Last update Wed Mar 26 11:10:44 2014 Gautier Lefebvre
//

#ifndef		__SERVER_USER_HPP__
#define		__SERVER_USER_HPP__

#include	<list>
#include	<string>

#include	"Protocol/Race.hpp"
#include	"Protocol/Color.hpp"
#include	"Protocol/User.hpp"
#include	"MySQL/Database.hpp"
#include	"Protocol/Error.hpp"
#include	"Threading/ScopeLock.hpp"

namespace	Network {
  class		SSocket;
}

namespace	Server {
  class		Group;
  class		Game;

  class		User {
  private:
    unsigned int	_id;
    ::User::Status	_status;
    std::string		_nick;
    std::string		_password;
    std::string		_mail;
    std::string		_avatar;
    std::list<User*>	_friends;
    std::list<User*>	_requestFriend;
    User*		_requestGroup;

    unsigned int	_winRaceX;
    unsigned int	_loseRaceX;
    unsigned int	_winRaceY;
    unsigned int	_loseRaceY;
    unsigned int	_winRaceZ;
    unsigned int	_loseRaceZ;

    Race::Race		_race;
    Color::Color	_color;
    int			_team;
    bool		_ready;
    Network::SSocket*	_socket;

    bool		_spectating;
    ::Server::Game	*_gamespectating;

  public:
    User();
    ~User();

    static Error::Code registration(const std::string&, const std::string&, const std::string&, std::list<Server::User*>&);
    static Error::Code	connection(const std::string&, const std::string&, std::list<Server::User*>&, Network::SSocket *);
    static void			load(std::list<Server::User*>&);
    Error::Code        addFriend(User *);
    Error::Code        answerFriendRequest(User *, bool);
    Error::Code        removeFriend(User *);

    unsigned int		getId() const;
    void			setId(unsigned int id);
    ::User::Status		getStatus() const;
    void			setStatus(::User::Status status);
    const std::string&		getNick() const;
    void			setNick(const std::string& nick);
    const std::string&		getPassword() const;
    void			setPassword(const std::string& pass);
    const std::string&		getMail() const;
    void			setMail(const std::string& mail);
    const std::string&		getAvatar() const;
    void			setAvatar(const std::string& avatar);

    unsigned int	getWinRaceX() const;
    unsigned int	getWinRaceY() const;
    unsigned int	getWinRaceZ() const;
    void		setWinRaceX(unsigned int);
    void		setWinRaceY(unsigned int);
    void		setWinRaceZ(unsigned int);
    unsigned int	getLoseRaceX() const;
    unsigned int	getLoseRaceY() const;
    unsigned int	getLoseRaceZ() const;
    void		setLoseRaceX(unsigned int);
    void		setLoseRaceY(unsigned int);
    void		setLoseRaceZ(unsigned int);

    Network::SSocket	*getSocket();
    void		setSocket(Network::SSocket *);
    void		changeStatus(::User::Status);
    void		setRace(Race::Race);
    Race::Race		getRace() const;
    void		setColor(Color::Color);
    Color::Color	getColor() const;
    void		setTeam(int);
    int			getTeam() const;
    void		ready(bool);
    bool		isReady() const;
    const User*		getRequestGroup() const;

    void		reinitRoomInfos(bool b = false);

    Error::Code        answerGroupRequest(User *);
    Error::Code        groupRequest(User *);

    void		isSpectating(bool);
    bool		isSpectating() const;

    void		spectating(::Server::Game *);
    Server::Game*	getGameSpectating();

    bool		operator==(const Network::SSocket *) const;
    bool		operator==(const std::string &) const;
  };
}

#endif
