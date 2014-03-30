//
// main.cpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Fri Oct 11 16:11:09 2013 geoffrey bauduin
// Last update Mon Mar 24 17:08:57 2014 geoffrey bauduin
//

#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	"Logger.hpp"
#include	"Server/Core.hpp"
#include	"Protocol/Job.hpp"

static Server::Core *core = NULL;

static void	gere_sig(int signal) {
  if (signal == SIGINT) {
    exit(EXIT_SUCCESS);
  }
}

static int	usage(char *prog) {
  std::cerr << "Usage: " << prog << " PORT" << std::endl;
  return (1);
}

int		main(int ac, char **av) {
  int v = 1;
  if (ac < 2) {
    return (usage(av[0]));
  }
  else {
    srand(time(NULL));
    (void) Logger::getInstance();
    core = new Server::Core;
    if (core->init(ac, av)) {
      signal(SIGINT, &gere_sig);
      v = core->run();
    }
    delete core;
    Logger::deleteInstance();
  }
  return (v);
}
