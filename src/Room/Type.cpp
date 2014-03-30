//
// Type.cpp for pfa in /home/gautier/Projets/pfa
// 
// Made by gautier lefebvre
// Login   <lefebv_n@epitech.net>
// 
// Started on  Fri Jan 24 17:02:07 2014 gautier lefebvre
// Last update Fri Jan 24 17:03:15 2014 gautier lefebvre
//

#include	"Utils/Undefined.hpp"
#include	"Room/Type.hpp"

const std::string&	Room::RoomTypeToString(::Room::Type t) {
  for (int i = 0 ; i < ::Room::NB_ROOMTYPE ; ++i) {
    if (RoomTypeStringTab[i].type == t) {
      return (RoomTypeStringTab[i].string);
    }
  }
  return (undefined);
}
