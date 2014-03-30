//
// Race.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 11:25:40 2014 geoffrey bauduin
// Last update Tue Mar 25 15:24:54 2014 geoffrey bauduin
//

#include	<sstream>
#include	<iostream>
#include	<cstddef>
#include	"Parser/Race.hpp"
#include	"Factory/Game.hpp"
#include	"Kernel/Manager.hpp"
#include	"Logger.hpp"
#include	"Threading/ScopeLock.hpp"
#include	"Factory/Server.hpp"

Parser::Race::Race(void):
  Parser::ARace(), _serverCategories(), _categoriesFunc(),
  _raceCreation(), _vRaceCreation(), _raceCreationData(),
  _unitOnMap(), _vUnitOnMap(), _unitOnMapData() {
  this->_serverCategories[UNIT_ON_MAP] = "UnitOnMap";
  this->_serverCategories[RACE_CREATION] = "RaceCreation";
  this->_reqCategories.push_back(this->_serverCategories[UNIT_ON_MAP]);
  this->_reqCategories.push_back(this->_serverCategories[RACE_CREATION]);

  this->_categoriesFunc[this->_categories[RACES]] = &Parser::Race::parseRace;
  this->_categoriesFunc[this->_categories[ELEMENTS]] = &Parser::Race::parseElements;
  this->_categoriesFunc[this->_serverCategories[UNIT_ON_MAP]] = &Parser::Race::parseUnitOnMap;
  this->_categoriesFunc[this->_serverCategories[RACE_CREATION]] = &Parser::Race::parseRaceCreation;

  this->_parseOrder.push_back(this->_categories[RACES]);
  this->_parseOrder.push_back(this->_categories[ELEMENTS]);
  this->_parseOrder.push_back(this->_serverCategories[UNIT_ON_MAP]);
  this->_parseOrder.push_back(this->_serverCategories[RACE_CREATION]);

  this->_raceCreation[RC_NAME] = "name";
  this->_raceCreation[RC_RACE] = "race";
  this->_raceCreation[RC_NB] = "nb";

  this->_vRaceCreation.push_back(this->_raceCreation[RC_NAME]);
  this->_vRaceCreation.push_back(this->_raceCreation[RC_RACE]);
  this->_vRaceCreation.push_back(this->_raceCreation[RC_NB]);

  this->_unitOnMap[UOM_NAME] = "name";
  this->_unitOnMap[UOM_NB] = "nb";
  this->_unitOnMap[UOM_X] = "x";
  this->_unitOnMap[UOM_Y] = "y";

  this->_vUnitOnMap.push_back(this->_unitOnMap[UOM_NAME]);
  this->_vUnitOnMap.push_back(this->_unitOnMap[UOM_NB]);
  this->_vUnitOnMap.push_back(this->_unitOnMap[UOM_X]);
  this->_vUnitOnMap.push_back(this->_unitOnMap[UOM_Y]);
}

Parser::Race::~Race(void) {}

bool	Parser::Race::_parse(const std::string &category, const Json::Value &data) {
  return (this->_categoriesFunc[category] && (this->*_categoriesFunc[category])(data));
}

bool	Parser::Race::parseUnitOnMap(const Json::Value &unitOnMap) {
  for (unsigned int j = 0 ; j < unitOnMap.size() ; ++j) {
    if (this->check(unitOnMap[j], this->_vUnitOnMap, "UnitOnMap") == false) {
      return (false);
    }
    else if (!this->createUnitOnMapData(unitOnMap[j])) {
      return (false);
    }
  }
  return (true);
}

bool	Parser::Race::createUnitOnMapData(const Json::Value &data) {
  std::string name = data[this->_unitOnMap[UOM_NAME]].asString();
  unsigned int nb = data[this->_unitOnMap[UOM_NB]].asInt();
  double x = data[this->_unitOnMap[UOM_X]].asDouble();
  double y = data[this->_unitOnMap[UOM_Y]].asDouble();
  if (this->_parsedUnits.find(name) == this->_parsedUnits.end() && this->_parsedObjects.find(name) == this->_parsedObjects.end() &&
      this->_parsedBuildings.find(name) == this->_parsedBuildings.end()) {
    Logger::getInstance()->logf("No Object/Unit/Building named %s", Logger::FATAL, &name);
    return (false);
  }
  this->_unitOnMapData[name] = {nb, x, y};
  return (true);
}

bool	Parser::Race::parseRaceCreation(const Json::Value &creation) {
  for (unsigned int j = 0 ; j < creation.size() ; ++j) {
    if (this->check(creation[j], this->_vRaceCreation, "RaceCreation") == false) {
      return (false);
    }
    else if (!this->createRaceCreationData(creation[j])) {
      return (false);
    }
  }
  for (auto it : this->_raceCreationData) {
    for (auto it2 : it.second) {
      static_cast<Server::Race *>(this->_container[it.first])->addStart(this->_parsedUnits[it2.first], it2.second);
    }
  }
  return (true);
}

bool	Parser::Race::createRaceCreationData(const Json::Value &data) {
  std::string race = data[this->_raceCreation[RC_RACE]].asString();
  std::string name = data[this->_raceCreation[RC_NAME]].asString();
  unsigned int nbr = data[this->_raceCreation[RC_NB]].asInt();
  Kernel::Serial serial = this->_parsedUnits[name];
  if (serial == "") {
    Logger::getInstance()->logf("Cannot find unit named >%s<", Logger::FATAL, &name);
    return (false);
  }
  if (this->_container[race] == NULL) {
    Logger::getInstance()->logf("Unknown Race >%s< in token >%s<", Logger::FATAL,
				&race, &this->_serverCategories[RACE_CREATION]);
    return (false);
  }
  else if (this->_container[race]->hasUnit(serial) == false) {
    Logger::getInstance()->logf("Unit >%s< isn't in Race >%s< in token >%s<", Logger::FATAL,
				&name, &race, &this->_serverCategories[RACE_CREATION]);
    return (false);
  }
  this->_raceCreationData[race].push_back(std::make_pair(name, nbr));
  return (true);
}

void	Parser::Race::emptyContainer(bool rm) {
  Parser::ARace::emptyContainer(rm);
  this->_raceCreationData.clear();
  this->_unitOnMapData.clear();
}

::Game::Race	*Parser::Race::createRace(const std::string &name, const Kernel::Serial &serial) const {
  return (Factory::Server::Race::create(name, serial));
}

void	Parser::Race::deleteRace(::Game::Race *race) const {
  Factory::Server::Race::remove(static_cast<Server::Race *>(race));
}
