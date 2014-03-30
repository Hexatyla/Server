//
// UserJob.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Jan 22 17:09:07 2014 gautier lefebvre
// Last update Fri Jan 24 15:38:07 2014 gautier lefebvre
//

#include	"Network/UserJob.hpp"

Network::UserJob::UserJob(Network::SSocket *s, Protocol::Job *j):
  socket(s),
  job(j)
{}

Network::UserJob::~UserJob() {}
