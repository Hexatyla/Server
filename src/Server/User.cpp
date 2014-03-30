//
// User.cpp for ex in /home/deleme/pfa
// 
// Made by anthony delemer
// Login   <deleme_a@epitech.net>
// 
// Started on  Tue Jan 21 11:32:16 2014 anthony delemer
// Last update Wed Mar 26 11:19:45 2014 Gautier Lefebvre
//

#include	"Utils/Undefined.hpp"
#include	"Converter.hpp"
#include	"Server/User.hpp"
#include	"MySQL/DBList.hpp"
#include	"Factory/Protocol.hpp"
#include	"Network/Socket.hpp"
#include	"Logger.hpp"
#include	"Network/Manager/Server.hpp"
#include	"Server/Game.hpp"

Server::User::User() :
  _id(),
  _status(),
  _nick(),
  _password(),
  _mail(),
  _friends(),
  _requestFriend(),
  _requestGroup(NULL),
  _winRaceX(),
  _loseRaceX(),
  _winRaceY(),
  _loseRaceY(),
  _winRaceZ(),
  _loseRaceZ(),
  _race(Race::RANDOM),
  _color(Color::RANDOM),
  _team(0),
  _ready(false),
  _socket(NULL),
  _spectating(false),
  _gamespectating(NULL)
{}

Server::User::~User() {}

Error::Code Server::User::registration(const std::string& nick, const std::string& mail, const std::string& password, std::list<Server::User*>& users) {
  MySQL::Database	*db = MySQL::DBList::getInstance()->get(MySQL::DBList::MAIN);
  MySQL::Result		result;
  MySQL::Row		row;

  if (password.length() != 32) { return (Error::INVALIDPASSWORD); }
  if (nick == "me" || nick.length() < 1 || mail.length() < 1) { return (Error::FORBIDDENOPERATION); }

  if (!db) { return (Error::UNDEF); }

  std::string nickE = db->realEscapeString(nick);
  std::string mailE = db->realEscapeString(mail);
  std::string passwordE = db->realEscapeString(password);
  {
    ScopeLock sl(db->getMutex());
    if (db->query(std::string("SELECT idUser FROM User WHERE name='") + nickE + "'") == false) { return (Error::UNDEF); }
    db->storeResult(result);
  }
  if (result.numRows() != 0) {
    result.free();
    return (Error::NICKALREADYTAKEN);
  }
  result.free();
  if (db->query(std::string("INSERT INTO User (name, password, adresseMail, raceXLost, raceXWin, raceYLost, raceYWin, raceZLost, raceZWin, inscription, last_connect, avatar) VALUES ('") + nickE + "', '" + passwordE + "', '" + mailE + "', '0', '0', '0', '0', '0', '0', NOW(), NOW(), '')") == false) { return (Error::ERRORREGISTRATION); }

  {
    ScopeLock sl(db->getMutex());
    if (db->query("SELECT idUser, name, password, adresseMail, raceXLost, raceXWin, raceYLost, raceYWin, raceZLost, raceZWin, avatar FROM User WHERE name='" + nickE + "'") == false) { return (Error::UNDEF); }
    db->storeResult(result);
  }

  result.fetchRow(row);
  Server::User *user = new Server::User();
  user->_id = Converter::StringTo<unsigned int>(row["idUser"]);
  user->_status = ::User::DISCONNECTED;
  user->_nick = nick;
  user->_password = password;
  user->_mail = mail;
  user->_winRaceX = 0;
  user->_loseRaceX = 0;
  user->_winRaceY = 0;
  user->_loseRaceY = 0;
  user->_winRaceZ = 0;
  user->_loseRaceZ = 0;
  user->_avatar = "";
  user->_socket = NULL;
  users.push_back(user);
  result.free();
  return (Error::NONE);
}

Error::Code Server::User::connection(const std::string& nick_or_mail, const std::string& password, std::list<Server::User*>& users, Network::SSocket *socket) {
  Error::Code	error = Error::NOSUCHNICK;

  for (auto user : users) {
    if (user && (user->_nick == nick_or_mail || user->_mail == nick_or_mail)) {
      error = Error::WRONGPASSWORD;
      if (user->_password == password) {
	if (user->_status != ::User::DISCONNECTED) {
	  error = Error::ALREADYCONNECTED;
	} else {
	  error = Error::NONE;
	  user->setSocket(socket);
	  user->changeStatus(::User::CONNECTED); // previens les amis que user est connecté
	  user->_requestGroup = NULL;

	  {
	    Protocol::Job *job = Factory::Protocol::Job::create();
	    job->success(Command::CO);
	    Network::Manager::Server::getInstance()->push(user->_socket, job);
	  }

	  for (auto fdrq : user->_requestFriend) { // balance la liste de requetes d'amis en attente
	    Protocol::Job* job = Factory::Protocol::Job::create();
	    job->newFriendRequest(fdrq->_nick);
	    Network::Manager::Server::getInstance()->push(user->_socket, job);
	  }

	  for (auto frd : user->_friends) { // balance la liste d'amis
	    Protocol::Job* job = Factory::Protocol::Job::create();
	    job->newFriend(frd->_nick, frd->_status);
	    Network::Manager::Server::getInstance()->push(user->_socket, job);
	  }

	  Protocol::Job *job = Factory::Protocol::Job::create();
	  job->playerInfos(user->_nick, user->_mail, user->_avatar, user->_status);
	  Network::Manager::Server::getInstance()->push(user->_socket, job);

	  job = Factory::Protocol::Job::create();
	  job->playerStats(user->_winRaceX, user->_loseRaceX, user->_winRaceY, user->_loseRaceY, user->_winRaceZ, user->_loseRaceZ);
	  Network::Manager::Server::getInstance()->push(user->_socket, job);

	  MySQL::Database	*db = MySQL::DBList::getInstance()->get(MySQL::DBList::MAIN);
	  if (db) {
	    std::string idStr = db->realEscapeString(Converter::StringOf<unsigned int>(user->_id));
	    {
	      ScopeLock sl(db->getMutex());
	      db->query("UPDATE User SET last_connect=NOW() WHERE idUser='" + idStr +"'");
	    }
	  }
	}
      }
      return (error);
    }
  }
  return (error);
}

void		Server::User::load(std::list<Server::User*>& users) {
  MySQL::Database	*db = MySQL::DBList::getInstance()->get(MySQL::DBList::MAIN);
  MySQL::Result		result;
  MySQL::Row		row;

  if (!db) { return; }

  {
    ScopeLock sl(db->getMutex());
    if (db->query("SELECT idUser, name, password, adresseMail, raceXLost, raceXWin, raceYLost, raceYWin, raceZLost, raceZWin, avatar FROM User") == false) { return; }
    db->storeResult(result);
  }

  while (result.fetchRow(row)) {
    Server::User *user = new Server::User();
    user->_id = Converter::StringTo<unsigned int>(row["idUser"]);
    user->_status = ::User::DISCONNECTED;
    user->_nick = row["name"];
    user->_password = row["password"];
    user->_mail = row["adresseMail"];
    user->_winRaceX = Converter::StringTo<unsigned int>(row["raceXWin"]);
    user->_loseRaceX = Converter::StringTo<unsigned int>(row["raceXLost"]);
    user->_winRaceY = Converter::StringTo<unsigned int>(row["raceYWin"]);
    user->_loseRaceY = Converter::StringTo<unsigned int>(row["raceYLost"]);
    user->_winRaceZ = Converter::StringTo<unsigned int>(row["raceZWin"]);
    user->_loseRaceZ = Converter::StringTo<unsigned int>(row["raceZLost"]);
    user->_avatar = row["avatar"];
    user->_socket = NULL;
    user->_race = Race::RANDOM;
    users.push_back(user);
  }
  result.free();

  {
    ScopeLock sl(db->getMutex());
    if (db->query("SELECT idUser, idUserFriend1 AS idFriend FROM FriendList") == false) { return; }
    db->storeResult(result);
  }

  while (result.fetchRow(row)) {
    unsigned int id1 = Converter::StringTo<unsigned int>(row["idUser"]);
    unsigned int id2 = Converter::StringTo<unsigned int>(row["idFriend"]);

    if (id1 == id2) { continue; }

    Server::User *user1 = NULL, *user2 = NULL;
    for (auto it : users) {
      if (it->getId() == id1) {	user1 = it; }
      else if (it->getId() == id2) { user2 = it; }
    }

    if (user1 && user2 && user1 != user2) {
      user1->_friends.push_back(user2);
      user2->_friends.push_back(user1);
    }
  }

}

Error::Code	Server::User::addFriend(User *user) {
  MySQL::Database	*db = MySQL::DBList::getInstance()->get(MySQL::DBList::MAIN);
  for (auto it : _friends) // check si user deja dans les amis
    {
      if (user == it)
	{
	  return (Error::FRIENDALREADYEXIST);
	}
    }
  for (auto it : user->_requestFriend) // check si la friend request a déjà été envoyée
    {
      if (it == user)
	return (Error::FRIENDREQUESTALREADYSENT);
    }

  for (auto it : this->_requestFriend) { // si la target (user) avait envoyé une demande d'ami à this -> accepte la demande
    if (it == user) {
      Protocol::Job *job;
      if (user->_socket) {
	job = Factory::Protocol::Job::create();
	job->newFriend(this->_nick, this->_status);
	Network::Manager::Server::getInstance()->push(user->_socket, job);
      }
      job = Factory::Protocol::Job::create();
      job->newFriend(user->_nick, user->_status);
      Network::Manager::Server::getInstance()->push(this->_socket, job);
      this->_friends.push_back(user);
      user->_friends.push_back(this);
      user->_requestFriend.remove(this);
      this->_requestFriend.remove(user);
      if (db) {
	std::string id1 = db->realEscapeString(Converter::StringOf<unsigned int>(this->_id));
	std::string id2 = db->realEscapeString(Converter::StringOf<unsigned int>(user->_id));

	{
	  ScopeLock sl(db->getMutex());
	  if (!db->query(std::string("DELETE FROM FriendRequest WHERE (idAsk='") + id1 + "' AND idTarget='" + id2 + "') OR (idAsk='" + id2 + "' AND idTarget='" + id1 + "')")) {
	    Logger::getInstance()->log(db->error(), Logger::FATAL);
	  }
	}

	{
	  ScopeLock sl(db->getMutex());
	  if (!db->query(std::string("INSERT INTO FriendList (idUser, idUserFriend1) VALUES ('") + id1 + "', '" + id2 + "')")) {
	    Logger::getInstance()->log(db->error(), Logger::FATAL);
	  }
	}
      }
      return (Error::NONE);
    }
  }

  // sinon, send la friend request a la target (user) si il est connecté
  user->_requestFriend.push_back(this);
  if (user->_socket) {
    Protocol::Job*	job = Factory::Protocol::Job::create();
    if (!job) { return (Error::UNDEF); }
    job->newFriendRequest(this->_nick);
    Network::Manager::Server::getInstance()->push(user->_socket, job);
    if (db) {
      std::string id1 = db->realEscapeString(Converter::StringOf<unsigned int>(this->_id));
      std::string id2 = db->realEscapeString(Converter::StringOf<unsigned int>(user->_id));
      {
	ScopeLock sl(db->getMutex());
	db->query(std::string("INSERT INTO FriendRequest (idAsk, idTarget) VALUES ('") + id1 + "', '" + id2 + "')");
      }
    }
  }
  return (Error::NONE);
}

Error::Code	Server::User::answerFriendRequest(User *user, bool answer)
{
  MySQL::Database	*db = MySQL::DBList::getInstance()->get(MySQL::DBList::MAIN);
  for (auto it : _requestFriend)
    {
      if (it == user)
	{
	  if (answer)
	    {
	      Protocol::Job	*job1 = Factory::Protocol::Job::create();
	      job1->newFriend(user->getNick(), user->_status);
	      Network::Manager::Server::getInstance()->push(this->_socket, job1);
	      if (user->_socket) {
		Protocol::Job	*job2 = Factory::Protocol::Job::create();
		job2->newFriend(this->getNick(), user->_status);
		Network::Manager::Server::getInstance()->push(user->_socket, job2);
	      }
	      this->_friends.push_back(user);
	      user->_friends.push_back(this);
	    }
	  this->_requestFriend.remove(user);
	  user->_requestFriend.remove(this);
	  if (db) {
	    std::string id1 = db->realEscapeString(Converter::StringOf<unsigned int>(this->_id));
	    std::string id2 = db->realEscapeString(Converter::StringOf<unsigned int>(user->_id));

	    {
	      ScopeLock sl(db->getMutex());
	      db->query(std::string("DELETE FROM FriendRequest WHERE (idAsk='") + id1 + "' AND idTarget='" + id2 + "') OR (idAsk='" + id2 + "' AND idTarget='" + id1 + "')");
	    }

	    if (answer) {
	      ScopeLock sl(db->getMutex());
	      db->query(std::string("INSERT INTO FriendList (idUser, idUserFriend1) VALUES ('") + id1 + "', '" + id2 + "')");
	    }
	  }
	  return (Error::NONE);
	}
    }
  return (Error::FRIENDNOTINREQUEST);
}

Error::Code	Server::User::removeFriend(User *user) {
  for (auto it : _friends)
    {
      if (user == it)
	{
	  Protocol::Job*	jobLose1 = Factory::Protocol::Job::create();
	  jobLose1->loseFriend(user->_nick);
	  Network::Manager::Server::getInstance()->push(this->_socket, jobLose1);

	  if (user->_socket) {
	    Protocol::Job*	jobLose2 = Factory::Protocol::Job::create();
	    jobLose2->loseFriend(this->_nick);
	    Network::Manager::Server::getInstance()->push(user->_socket, jobLose2);
	  }
	  user->_friends.remove(this);
	  this->_friends.remove(user);
	  MySQL::Database	*db = MySQL::DBList::getInstance()->get(MySQL::DBList::MAIN);
	  if (db) {
	    std::string id1 = Converter::StringOf<unsigned int>(this->_id);
	    std::string id2 = Converter::StringOf<unsigned int>(user->_id);

	    {
	      ScopeLock sl(db->getMutex());
	      db->query(std::string("DELETE FROM FriendList WHERE (idUser='") + id1 + "' AND idUserFriend1='" + id2 + "') OR (idUser='" + id2 + "' AND idUserFriend1='" + id1 + "')");
	    }
	  }
	  return (Error::NONE);
	}
    }
  return (Error::FRIENDDONOTEXIST);
}

Error::Code		Server::User::groupRequest(Server::User *target) // this envoie une demande de groupe à target
{
  this->_requestGroup = NULL; // ocazou
  if (target->_requestGroup != NULL) {
    return (Error::USERINGROUP);
  }

  Protocol::Job*	job = Factory::Protocol::Job::create();

  if (!job) { return (Error::UNDEF); }
  job->groupRequest(this->getNick());
  Network::Manager::Server::getInstance()->push(target->_socket, job);
  target->_requestGroup = this;
  return (Error::NONE);
}

Error::Code		Server::User::answerGroupRequest(Server::User *user)
{
  if (user == this->_requestGroup) {
    this->_requestGroup = NULL;
    return (Error::NONE);
  } else {
    return (Error::USERDIDNOTREQUESTGROUP);
  }
}

unsigned int		Server::User::getId() const { return (this->_id); }
void			Server::User::setId(unsigned int id) { this->_id = id; }
::User::Status		Server::User::getStatus() const { return (this->_status); }
void			Server::User::setStatus(::User::Status status) { this->_status = status; }
const std::string&	Server::User::getNick() const { return (this->_nick); }
void			Server::User::setNick(const std::string& nick) { this->_nick = nick; }
const std::string&	Server::User::getPassword() const { return (this->_password); }
void			Server::User::setPassword(const std::string& pass) { this->_password = pass; }
const std::string&	Server::User::getMail() const { return (this->_mail); }
void			Server::User::setMail(const std::string& mail) { this->_mail = mail; }
const std::string&	Server::User::getAvatar() const { return (this->_avatar); }
void			Server::User::setAvatar(const std::string& avatar) { this->_avatar = avatar; }

unsigned int		Server::User::getWinRaceX() const { return (this->_winRaceX); }
unsigned int		Server::User::getWinRaceY() const { return (this->_winRaceY); }
unsigned int		Server::User::getWinRaceZ() const { return (this->_winRaceZ); }
void			Server::User::setWinRaceX(unsigned int raceX) { this->_winRaceX = raceX; }
void			Server::User::setWinRaceY(unsigned int raceY) { this->_winRaceY = raceY; }
void			Server::User::setWinRaceZ(unsigned int raceZ) { this->_winRaceZ = raceZ; }
unsigned int		Server::User::getLoseRaceX() const { return (this->_loseRaceX); }
unsigned int		Server::User::getLoseRaceY() const { return (this->_loseRaceY); }
unsigned int		Server::User::getLoseRaceZ() const { return (this->_loseRaceZ); }
void			Server::User::setLoseRaceX(unsigned int raceX) { this->_loseRaceX = raceX; }
void			Server::User::setLoseRaceY(unsigned int raceY) { this->_loseRaceY = raceY; }
void			Server::User::setLoseRaceZ(unsigned int raceZ) { this->_loseRaceZ = raceZ; }
Network::SSocket	*Server::User::getSocket() { return (this->_socket); }
void			Server::User::setSocket(Network::SSocket *socket) { this->_socket = socket; }
void			Server::User::setRace(Race::Race r) { this->_race = r; }
Race::Race		Server::User::getRace() const { return (this->_race); }
void			Server::User::setColor(Color::Color c) { this->_color = c; }
Color::Color		Server::User::getColor() const { return (this->_color); }
void			Server::User::setTeam(int t) { this->_team = t; }
int			Server::User::getTeam() const { return (this->_team); }
void			Server::User::ready(bool b) { this->_ready = b; }
bool			Server::User::isReady() const { return (this->_ready); }
const Server::User*	Server::User::getRequestGroup() const { return (this->_requestGroup); }

void			Server::User::reinitRoomInfos(bool reinitrace) {
  if (reinitrace) {
    this->_race = Race::RANDOM;
  }
  this->_color = Color::RANDOM;
  this->_team = 0;
  this->_ready = false;
}

void			Server::User::changeStatus(::User::Status status) {
  this->_status = status;
  for (auto it : this->_friends) {
    if (it->_status != ::User::DISCONNECTED && it->_socket != NULL) {
      Protocol::Job*	job = Factory::Protocol::Job::create();
      if (!job) { continue; }
      job->friendStatus(this->_nick, status);
      Network::Manager::Server::getInstance()->push(it->_socket, job);
    }
  }
}

bool		Server::User::isSpectating() const { return (this->_spectating); }
void		Server::User::isSpectating(bool s) { this->_spectating = s; }

Server::Game*	Server::User::getGameSpectating() { return (this->_gamespectating); }
void		Server::User::spectating(Server::Game *game) { this->_gamespectating = game; }

bool		Server::User::operator==(const Network::SSocket *s) const {
  return (this->_socket == s);
}

bool		Server::User::operator==(const std::string& s) const {
  return (this->_nick == s);
}
