//
// Spectator.hpp for  in /home/geoffrey/Projects/pfa
// 
// Made by geoffrey bauduin
// Login   <baudui_g@epitech.net>
// 
// Started on  Sat Mar  8 20:31:17 2014 geoffrey bauduin
// Last update Tue Mar 18 19:29:38 2014 geoffrey bauduin
//

#ifndef SERVER_SPECTATOR_HPP_
# define SERVER_SPECTATOR_HPP_

#include	<map>
#include	<vector>
#include	<list>
#include	"Server/IUpdatable.hpp"
#include	"IFromFactory.hpp"
#include	"Server/HasJobs.hpp"
#include	"Server/IHasReferenceToOtherObject.hpp"
#include	"Threading/Mutex.hpp"
#include	"Protocol/Job.hpp"

namespace	Server {

  class	Spectator: virtual public IFromFactory, public Server::IUpdatable, public Server::HasJobs,
		   public IHasReferenceToOtherObject {

  public:

    class AData : public IFromFactory {

    protected:
      Mutex	*_mutex;

    public:
      AData(void);
      virtual ~AData(void);

      virtual void	init(void) = 0;

      virtual void	addJob(Protocol::Job *) = 0;

    };

    class ItemData: public AData  {

    private:
      Protocol::Job				*_create;
      Protocol::Job				*_update;
      std::list<Protocol::Job *>		_effects;
      std::list<Protocol::Job *>		_actions;

      void	addEffectJob(Protocol::Job *);
      void	addActionJob(Protocol::Job *);

    public:
      ItemData(void);
      virtual ~ItemData(void);

      virtual void	init(void);
      virtual void	destroy(void);

      virtual void	addJob(Protocol::Job *);
      const std::vector<Protocol::Job *>	dump(void);

    };

    class PlayerData: public AData  {

    private:
      Protocol::Job				*_ressources;
      std::list<Protocol::Job *>		_ameliorations;

    public:
      PlayerData(void);
      virtual ~PlayerData(void);

      virtual void	init(void);
      virtual void	destroy(void);

      virtual void	addJob(Protocol::Job *);
      const std::vector<Protocol::Job *>	dump(bool);

    };

  private:
    class _data: public IFromFactory, public IHasReferenceToOtherObject {
    private:
      Mutex					*_itemMutex;
      Mutex					*_playerMutex;
      Mutex					*_gameDataMutex;
      std::map<Kernel::ID::id_t, PlayerData *>	_players;
      std::map<Kernel::ID::id_t, ItemData *>	_items;
      std::vector<Protocol::Job *>		_gameData;
      Protocol::Job				*_dayNight;      
      std::list<Kernel::ID::id_t>		_delete;

    public:
      _data(void);
      ~_data(void);

      void	init(void);
      virtual void	destroy(void);

      void	addJob(Kernel::ID::Type, Kernel::ID::id_t, Protocol::Job *);
      const std::vector<Protocol::Job *>	dump(Kernel::ID::id_t id = 0);
      virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);
      void		onPlayerQuit(Kernel::ID::id_t);

    };

    struct jobData {
      Kernel::ID::Type type;
      Kernel::ID::id_t id;
      Protocol::Job    *job;
    };

    std::map<unsigned int, std::vector<jobData> >	_toSend;
    _data					_game;
    _data					_spec;
    const Clock					*_clock;
    Mutex					*_sendMutex;


  public:
    Spectator(void);
    virtual ~Spectator(void);

    void	init(const Clock *);
    virtual void	destroy(void);

    virtual bool	update(const Clock *, double);

    void	addJob(Kernel::ID::Type, Kernel::ID::id_t, Protocol::Job *);

    virtual void	onItemDestroyed(Kernel::ID::id_t, ::Game::Type);
    void		onPlayerQuit(Kernel::ID::id_t);

    const std::vector<Protocol::Job *>	dump(Kernel::ID::id_t id = 0);

  };

}

#endif
