//
// MapInfos.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Feb 12 21:23:41 2014 gautier lefebvre
// Last update Thu Feb 13 13:13:15 2014 gautier lefebvre
//

#include	"Room/MapInfos.hpp"

Room::MapInfos::MapInfos(const std::string &fp, const std::string &fn, const std::string &tt):
  fullpath(fp),
  filename(fn),
  title(tt)
{}

Room::MapInfos::MapInfos(const Room::MapInfos &oth):
  fullpath(oth.fullpath),
  filename(oth.filename),
  title(oth.title)
{}

Room::MapInfos&	Room::MapInfos::operator=(const Room::MapInfos &oth) {
  if (this != &oth) {
    this->fullpath = oth.fullpath;
    this->filename = oth.filename;
    this->title = oth.title;
  }
  return (*this);
}

Room::MapInfos::~MapInfos() {}
