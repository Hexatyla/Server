//
// MapList.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Wed Feb 12 21:26:37 2014 gautier lefebvre
// Last update Thu Mar 13 11:42:18 2014 gautier lefebvre
//

#include	"Room/MapList.hpp"
#include	"FileLoader.hpp"
#include	"Logger.hpp"

Room::MapList::MapList():
  _maps()
{}

Room::MapList::~MapList() {}

const std::vector<Room::MapInfos>& Room::MapList::getMaps() const { return (this->_maps); }

bool		Room::MapList::isFileNameCorrect(const std::string &filename) const {
  return (filename.size() > 46 && filename[40] == '_');
}

std::string	Room::MapList::getMapTitleFromFileName(const std::string &filename) const {
  std::string	title = filename.substr(41, filename.size() - 46);
  size_t	pos;
  while ((pos = title.find("_")) != std::string::npos) {
    title[pos] = ' ';
  }
  return (title);
}

void		Room::MapList::init() {
  std::vector<std::string> files;

  Logger::getInstance()->log("Loading maps ...", Logger::INFORMATION);
  FileLoader::getInstance()->loadFileByExtension(Room::MAPS_FOLDER, ".hxtl", files);
  for (auto it : files) {
    std::string filename = FileLoader::get_filename(it, true);
    if (this->isFileNameCorrect(filename)) {
      this->_maps.push_back(Room::MapInfos(it, filename + ".hxtl", this->getMapTitleFromFileName(filename)));
    }
  }
  if (this->_maps.empty()) {
    Logger::getInstance()->log("There are no maps.", Logger::FATAL);
  }
  else {
    Logger::getInstance()->log("Maps loaded.", Logger::INFORMATION);
  }
}
