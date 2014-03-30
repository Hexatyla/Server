//
// Settings.cpp for ex in /home/deleme/pfa
// 
// Made by anthony delemer
// Login   <deleme_a@epitech.net>
// 
// Started on  Tue Jan 21 11:26:27 2014 anthony delemer
// Last update Fri Feb  7 21:54:31 2014 gautier lefebvre
//

#include		"Room/Settings.hpp"

Room::Settings::Settings(bool lt, bool cheat, bool chat) : lockedTeams(lt), cheat(cheat), chat(chat) {}
Room::Settings::~Settings() {}


