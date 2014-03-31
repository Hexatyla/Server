##
## Makefile for  in /home/geoffrey/Projects/pfa
## 
## Made by geoffrey bauduin
## Login   <baudui_g@epitech.net>
## 
## Started on  Wed Feb  5 14:00:43 2014 geoffrey bauduin
## Last update Mon Mar 31 23:34:28 2014 geoffrey bauduin
##

NAME=			server

SRCDIR=			src
SERVERDIR=		$(SRCDIR)/Server
NETWORKDIR=		$(SRCDIR)/Network
MYSQLDIR=		$(SRCDIR)/MySQL
ALGODIR=		$(SRCDIR)/Algo
ROOMDIR=		$(SRCDIR)/Room
FACTORYDIR=		$(SRCDIR)/Factory
PARSERDIR=		$(SRCDIR)/Parser

SRC_PARSER=		$(PARSERDIR)/Race.cpp			\
			$(PARSERDIR)/SQL.cpp

SRC_NETWORK=		$(NETWORKDIR)/UserJob.cpp		\
			$(NETWORKDIR)/Manager/Server.cpp

SRC_MYSQL=		$(MYSQLDIR)/Database.cpp		\
			$(MYSQLDIR)/Row.cpp			\
			$(MYSQLDIR)/Result.cpp			\
			$(MYSQLDIR)/Exception.cpp		\
			$(MYSQLDIR)/DBList.cpp

SRC_FACTORY=		$(FACTORYDIR)/Server.cpp		\
			$(FACTORYDIR)/Network.cpp

SRC_ALGO=		$(ALGODIR)/Pathfinding.cpp

SRC_ROOM=		$(ROOMDIR)/MapList.cpp			\
			$(ROOMDIR)/MapInfos.cpp			\
			$(ROOMDIR)/Type.cpp			\
			$(ROOMDIR)/Settings.cpp			\
			$(ROOMDIR)/ARoom.cpp			\
			$(ROOMDIR)/AQuickLaunch.cpp		\
			$(ROOMDIR)/OneVsOne.cpp			\
			$(ROOMDIR)/TwoVsTwo.cpp			\
			$(ROOMDIR)/ThreeVsThree.cpp

SRC_SERVER=		src/Server/Group.cpp			\
			src/Server/MatchMaking.cpp		\
			src/Server/main.cpp			\
			src/Server/Core.cpp			\
			src/Server/AItem.cpp			\
			src/Server/Event.cpp			\
			src/Server/HasEvent.cpp			\
			src/Server/HasUsers.cpp			\
			src/Server/Building.cpp			\
			src/Server/Unit.cpp			\
			src/Server/Hero.cpp			\
			src/Server/Projectile.cpp		\
			src/Server/Game.cpp			\
			src/Server/HasUnit.cpp			\
			src/Server/HasProjectile.cpp		\
			src/Server/HasBuilding.cpp		\
			src/Server/HasHero.cpp			\
			src/Server/HasObject.cpp		\
			src/Server/User.cpp			\
			src/Server/HasJobs.cpp			\
			src/Server/GamePlayer.cpp		\
			src/Server/HasProductionQueue.cpp	\
			src/Server/Object.cpp			\
			src/Server/RessourcesSpot.cpp		\
			src/Server/HasRessourcesSpot.cpp	\
			src/Server/Effect.cpp			\
			src/Server/Capacity.cpp			\
			src/Server/Controller.cpp		\
			src/Server/HasStatistics.cpp		\
			src/Server/GameLoader.cpp		\
			src/Server/Movable.cpp			\
			src/Server/HasPlayers.cpp		\
			src/Server/Map.cpp			\
			src/Server/Action.cpp			\
			src/Server/Waitlist.cpp			\
			src/Server/CanUseCapacity.cpp		\
			src/Server/CanBeDamaged.cpp		\
			src/Server/GroupMove.cpp		\
			src/Server/Spectator.cpp		\
			src/Server/Race.cpp

SRC=		$(SRC_PARSER)	\
		$(SRC_NETWORK)	\
		$(SRC_MYSQL)	\
		$(SRC_FACTORY)	\
		$(SRC_ALGO)	\
		$(SRC_ROOM)	\
		$(SRC_SERVER)

OBJ=		$(SRC:.cpp=.o)

COMMON=		common/hexatylaCommon.so

LDFLAGS=	-lssl -lcrypto -lpthread -L/usr/include/mysql -lmysqlclient -I/usr/include/mysql -lm common/libs/libjson.a -Wl,-rpath,./lib/ common/hexatylaCommon.so -L./common/libs -ljson

INCLUDES=	-Iincludes -Icommon/includes

CXXFLAGS=	$(INCLUDES) -std=c++11 -D__SERVER__ -Wall -Wextra

DEPS=		$(OBJ:.o=.deps)

CXX=		g++

all:		$(NAME)

$(NAME):	$(OBJ)
		cd common && $(MAKE)
		@$(CXX) -o $(NAME) $(OBJ) $(LDFLAGS) $(COMMON)

%.o:		%.cpp
		@$(CXX) -c $(CXXFLAGS) -MMD $< -o $@ -MF $(patsubst %.o, %.deps, $@) && echo "CXX\t$<"

clean:
		rm -f $(OBJ)

fclean:		clean
		rm -f $(NAME)

hardclean:	fclean
		@rm -f $(DEPS)

re:		hardclean all

-include $(DEPS)
