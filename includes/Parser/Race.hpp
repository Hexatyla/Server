//
// Race.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Wed Jan 22 11:25:29 2014 geoffrey bauduin
// Last update Wed Mar 12 15:15:48 2014 geoffrey bauduin
//

#ifndef PARSER_RACE_HPP_
# define PARSER_RACE_HPP_

#include	"Parser/ARace.hpp"

namespace Parser {

  class Race: public Parser::ARace {

  private:
    enum TokenCategoryServer {
      UNIT_ON_MAP, RACE_CREATION
    };

    std::map<TokenCategoryServer, std::string>	_serverCategories;

  protected:
    std::map<std::string, bool (Parser::Race::*)(const Json::Value &)>	_categoriesFunc;

    bool	parseUnitOnMap(const Json::Value &);
    bool	parseRaceCreation(const Json::Value &);

    virtual bool	_parse(const std::string &, const Json::Value &);

    virtual ::Game::Race *createRace(const std::string &, const Kernel::Serial &) const;
    virtual void	deleteRace(::Game::Race *) const;

  private:
    enum	TokenRaceCreation {
      RC_NAME,
      RC_RACE,
      RC_NB
    };

    std::map<TokenRaceCreation, std::string>	_raceCreation;
    std::vector<std::string>			_vRaceCreation;
    std::map<std::string, std::vector< std::pair<std::string, unsigned int> > >	_raceCreationData;

    bool	createRaceCreationData(const Json::Value &);

    enum TokenUnitOnMap {
      UOM_NAME,
      UOM_NB,
      UOM_X,
      UOM_Y
    };

  public:
    struct unitOnMapData {
      unsigned int nb;
      double x;
      double y;
    };

  private:

    std::map<TokenUnitOnMap, std::string>	_unitOnMap;
    std::vector<std::string>			_vUnitOnMap;
    std::map<std::string, unitOnMapData>		_unitOnMapData;

    bool	createUnitOnMapData(const Json::Value &);

  protected:
    virtual void	emptyContainer(bool);

  public:
    Race(void);
    virtual ~Race(void);

  };
}

#endif
