#ifndef __Arkanoid__PlayingState__
#define __Arkanoid__PlayingState__

#include <list>

#include "SDL.h"

#include "State.h"

// forward declarations
class GameObject;
class Platform;
class Ball;
class Effect;
class Projectile;
class MapLoader;
class Gui;

class PlayingState
 : public State
{
 public:
  PlayingState ();
  ~PlayingState ();

  virtual void InitState ();
  virtual void RenderState ();
  virtual void UpdateState ();
  virtual void HandleEvents (Uint8*, const SDL_Event&, int);

  inline void SetChangingStateFlag (bool flag) { changingstate = flag; }
  inline bool GetChangingStateFlag () { return changingstate; }

  inline Effect** GetEffects () { return effects; }
  inline Projectile** GetProjectiles () { return projectiles; }
  inline Platform* GetPlatform () { return platform; }
  inline Ball* GetBall () { return ball; }

  void LaunchSecondBall ();

 private:
  typedef State inherited;

  bool                   changingstate;

  std::list<GameObject*> gobjects;
  MapLoader*             map_loader;

  Ball*                  ball;
  Ball*                  second_ball;
  Platform*              platform;
  Effect**               effects;
  Projectile**           projectiles;
  Gui*                   gui;

  int                    second_ball_flag;

  void SaveHighscores ();
};

#endif /* defined(__Arkanoid__PlayingState__) */
