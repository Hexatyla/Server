//
// MatchMaking.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Feb  7 22:47:09 2014 gautier lefebvre
// Last update Thu Mar 13 11:47:06 2014 gautier lefebvre
//

#include	<stdlib.h>
#include	"Server/MatchMaking.hpp"
#include	"Logger.hpp"
#include	"Converter.hpp"
#include	"Network/Manager/Server.hpp"

/*
  rooms est la liste du core
  users est le groupe qui veut rejoindre (1 seul user si le mec est pas dans un groupe)
  j ai déjà vérifié avant d appeler ta fonction que le groupe n est pas trop grand pour rejoindre une partie de
  type t

  Fais le matchmaking au feeling, faut juste que la fonction respecte ça :
  - la liste Users représente un groupe, donc faudra qu ils puissent être dans la même équipe
  - si tu ne trouves pas de places pour les users, crée une nouvelle room de type t et add les users
  - après avoir mis les users dans la room, set leur couleur (faut pas que 2 users aient la même couleur)
  - utilise juste 1 et 2 pour les teams (je check que les team 1 et 2 pour vérifier que la room est prête)

  si t == Room::OneVsOne -> new ARoom::OneVsOne(Room::OneVsOne)
  etc...

  après ça manque plus qu à voir avec baudui pour lier nos parties et lancer la game.
  PAS OUBLIER DE DELETE LA ROOM QUAND LA GAME EST LANCE
  - après avoir ajouté les users dans une room, set leur color et leur team :
  - envoie un job->gameSearching() à tous les users dans la room
  - si room->isReady() == true, envoie un job endGameSearching() à tous les users
*/


  /*
    j ai add qlq commentaires (j ai rien changé à part le delete de la room que je fais apres avoir appelé le
    matchmaking, my bad)

    ta fonction _isFirstCanFightWithSecond a l air ultra hardcore pour matcher, j aurai plutot fais une moyenne
    des 2 teams au lieu de verifier pour chaque joueur (mais ca a l air de marcher donc laisse comme ca)

    par contre, quand tu trouves la room tu mets directement les joueurs dans la team 2 sans verifier qu il n y
    a la place pour tout le groupe dans cette team. A part ça ca devrait etre bon
  */

void		MatchMaking::run(Room::Type t, std::list<Room::ARoom*>& rooms, const std::list<Server::User*>& users) {
  for (auto user : users) {
    if (user->getRace() == Race::RANDOM) {
      user->setRace(static_cast<Race::Race>(rand() % 3));
      Logger::getInstance()->log(user->getNick() + " : race : " + Race::RaceToString(user->getRace()), Logger::INFORMATION);
    }
  }
  if (rooms.size() == 0) {
    Logger::getInstance()->log("Creating room (no room created)", Logger::INFORMATION);
    _ifNoRoom(rooms, t, users);
    return ;
  }
  for (auto itRoom : rooms)
    {
      if (itRoom->getType() == t)
	{
	  unsigned int i = 0;
	  for (auto itUser : itRoom->getPlayers())
	    {
	      for (auto itNewPlayers : users)
		if (_isFirstCanFightWithSecond(itUser, itNewPlayers))
		  i++;
	    }
	  if (i > 0 && _isEnoughPlace(itRoom, users)) //(i <= (itRoom->getPlayers().size() - 1)) && (i > 0))
	    {
	      Logger::getInstance()->log("Found room", Logger::INFORMATION);
	      _setSettingsToUsers(users, itRoom->getPlayers(), 2);
	      itRoom->addPlayers(users);
	      _sendJobGameSearching(users);
	      if (itRoom->isReady()) {
		Logger::getInstance()->log("Room ready", Logger::INFORMATION);
		_sendJobEndGameSearching(itRoom->getPlayers());
	      }
	      return ;
	    }
	}
    }
  Logger::getInstance()->log("Creating room (no room found)", Logger::INFORMATION);
  _ifNoRoom(rooms, t, users);
}

bool		MatchMaking::_isEnoughPlace(Room::ARoom *tmp, const std::list<Server::User*>& user)
{
  // user = user;
  Logger::getInstance()->log("Checking nb of slots", Logger::INFORMATION);
  switch (tmp->getType())
    {
    case (Room::ONEVSONE):
      {
	if ((tmp->getPlayers().size() == 1) && user.size() == 1) {
	  Logger::getInstance()->log("Enough slots", Logger::INFORMATION);
	  return (true);
	}
	break ;
      }
    case (Room::TWOVSTWO):
      {
	if ((tmp->getPlayers().size() == 2) && user.size() == 2) {
	  Logger::getInstance()->log("Enough slots", Logger::INFORMATION);
	  return (true);
	}
	break ;
      }
    case (Room::THREEVSTHREE):
      {
	if ((tmp->getPlayers().size() == 3) && user.size() == 3) {
	  Logger::getInstance()->log("Enough slots", Logger::INFORMATION);
	  return (true);
	}
	break ;
      }
    default :
      break;
    }
  Logger::getInstance()->log("Not enough slots", Logger::FATAL);
  return (false);
}

void		MatchMaking::_ifNoRoom(std::list<Room::ARoom*>& rooms, Room::Type t, const std::list<Server::User*>& users)
{
  switch (t)
    {
    case (Room::ONEVSONE):
      {
	Room::OneVsOne *RoomOvO;
	RoomOvO = new Room::OneVsOne();

	Logger::getInstance()->log("Created room 1v1", Logger::INFORMATION);
	_setSettingsToUsers(users, RoomOvO->getPlayers(), 1);
       	RoomOvO->addPlayers(users);
	_sendJobGameSearching(users);
	if (RoomOvO->isReady())
	  _sendJobEndGameSearching(users);
	rooms.push_back(RoomOvO);
	break ;
      }
    case (Room::TWOVSTWO):
      {
	Room::TwoVsTwo *RoomTvT;
	RoomTvT = new Room::TwoVsTwo();

	Logger::getInstance()->log("Created room 3v3", Logger::INFORMATION);
	_setSettingsToUsers(users, RoomTvT->getPlayers(), 1);
	RoomTvT->addPlayers(users);
	_sendJobGameSearching(users);
	if (RoomTvT->isReady())
	  _sendJobEndGameSearching(users);
	rooms.push_back(RoomTvT);
	break ;
      }
    case (Room::THREEVSTHREE):
      {
	Room::ThreeVsThree *RoomTvT;
	RoomTvT = new Room::ThreeVsThree();
	Logger::getInstance()->log("Created room 3v3", Logger::INFORMATION);

	_setSettingsToUsers(users, RoomTvT->getPlayers(), 1);
	RoomTvT->addPlayers(users);
	_sendJobGameSearching(users);
	if (RoomTvT->isReady())
	  _sendJobEndGameSearching(users);
	rooms.push_back(RoomTvT);
	break ;
      }
    default :
      break;
    }
}

bool			MatchMaking::_isColorAlreadyChosed(const std::list<Server::User*>& users, const std::list<Server::User*>& opponents, Color::Color color)
{
  for (auto user : users) {
    if (user->getColor() == color)
      return (true);
  }
  for (auto user : opponents) {
    if (user->getColor() == color)
      return (true);
  }
  return (false);
}

const std::list<Server::User*>&	MatchMaking::_setSettingsToUsers(const std::list<Server::User*>& users, const std::list<Server::User *>& opponents, int team)
{
  std::list<Server::User*>::const_iterator	it;

  for (it = users.begin(); it != users.end(); ++it)
    {
      int	i = 1;
      bool	colorChosed = false;
      while (colorChosed == false)
	{
	  if (_isColorAlreadyChosed(users, opponents, static_cast<Color::Color>(i)) == false)
	    {
	      (*it)->setColor(static_cast<Color::Color>(i));
	      colorChosed = true;
	      Logger::getInstance()->log((*it)->getNick() + " : color : " + Color::ColorToString((*it)->getColor()), Logger::INFORMATION);
	    }
	  else
	    i++;
	}
      (*it)->setTeam(team);
      Logger::getInstance()->log((*it)->getNick() + " : team : " + Converter::StringOf<int>(team), Logger::INFORMATION);
    }
  return (users);
}

void				MatchMaking::_sendJobGameSearching(const std::list<Server::User*>& users)
{
  for (auto user : users)
    {
      Protocol::Job* job = Factory::Protocol::Job::create();
      job->gameSearching();
      Network::Manager::Server::getInstance()->push(user->getSocket(), job);
    }
}

void			       MatchMaking::_sendJobEndGameSearching(const std::list<Server::User*>& users)
{
  for (auto user : users)
    {
      Protocol::Job* job = Factory::Protocol::Job::create();
      job->endGameSearching();
      Network::Manager::Server::getInstance()->push(user->getSocket(), job);
    }
}

bool				MatchMaking::_isFirstCanFightWithSecond(Server::User *, Server::User *)
{
  // if (us1->getRace() == Race::RANDOM)
  //   us1->setRace(Race::GENICIA);
  // if (us2->getRace() == Race::RANDOM)
  //   us2->setRace(Race::GENICIA);

  // unsigned int resPourcentageVictoire1 = ((us1->getWinRaceX() + us1->getWinRaceY() + us1->getWinRaceZ()) * 100) /
  //   (us1->getWinRaceX() + us1->getLoseRaceX() + us1->getWinRaceY() + us1->getLoseRaceY() + us1->getWinRaceZ() + us1->getLoseRaceZ());
  // if ((us1->getWinRaceX() + us1->getLoseRaceX() + us1->getWinRaceY() + us1->getLoseRaceY() + us1->getWinRaceZ() + us1->getLoseRaceZ()) < 10)
  //   resPourcentageVictoire1 *= (us1->getWinRaceX() + us1->getLoseRaceX() + us1->getWinRaceY() + us1->getLoseRaceY() + us1->getWinRaceZ() + us1->getLoseRaceZ());

  // unsigned int resPourcentageVictoire2 = ((us2->getWinRaceX() + us2->getWinRaceY() + us2->getWinRaceZ()) * 100) /
  //   (us2->getWinRaceX() + us2->getLoseRaceX() + us2->getWinRaceY() + us2->getLoseRaceY() + us2->getWinRaceZ() + us2->getLoseRaceZ());

  // if ((us1->getWinRaceX() + us1->getLoseRaceX() + us1->getWinRaceY() + us1->getLoseRaceY() + us1->getWinRaceZ() + us1->getLoseRaceZ()) < 10)
  //   resPourcentageVictoire2 *= (us2->getWinRaceX() + us2->getLoseRaceX() + us2->getWinRaceY() + us2->getLoseRaceY() + us2->getWinRaceZ() + us2->getLoseRaceZ());

  // if ((resPourcentageVictoire1 <= (resPourcentageVictoire2 - 20)) &&
  //     (resPourcentageVictoire1 >= (resPourcentageVictoire2 + 20)))
  //   return (true);
  // return (false);

  return (true); // pour les test
}
