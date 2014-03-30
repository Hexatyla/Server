//
// Network.cpp<2> for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Jan 24 15:28:10 2014 gautier lefebvre
// Last update Thu Mar  6 11:30:30 2014 gautier lefebvre
//

#include	"Factory/Pool.hpp"
#include	"Factory/Network.hpp"

static Pool< ::Network::UserJob>	*poolUserJob = NULL;
static Pool< ::Network::SSocket>	*poolSocket = NULL;

void		Factory::Network::init() {
  poolUserJob = new Pool< ::Network::UserJob>(500, 100, "Network::UserJob");
  poolSocket = new Pool< ::Network::SSocket>(100, 20, "Network::SSocket");
}

void		Factory::Network::end() {
  delete poolUserJob;
  delete poolSocket;
}

::Network::UserJob*	Factory::Network::UserJob::create(::Network::SSocket *s, Protocol::Job *j) {
  ::Network::UserJob* uj = poolUserJob->get();
  uj->socket = s;
  uj->job = j;
  return (uj);
}

void		Factory::Network::UserJob::remove(::Network::UserJob *j) {
  poolUserJob->push(j);
}

::Network::SSocket *	Factory::Network::SSocket::create() {
  ::Network::SSocket *s = poolSocket->get();
  s->init();
  return (s);
}

void		Factory::Network::SSocket::remove(::Network::SSocket *s) {
  s->destroy();
  poolSocket->push(s);
}
