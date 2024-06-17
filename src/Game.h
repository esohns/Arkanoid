#ifndef __Arkanoid__Game__
#define __Arkanoid__Game__

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#include "State.h"
#include "Singleton.h"
#include "FpsCounter.h"
#include "Music.h"

#define g_Game        Game::GetSingleton ()
#define g_GamePtr     Game::GetSingletonPtr ()

enum GAME_STATE {MENU, PLAYING};

class PlayingState;
class MenuState;

//helper function for conversion
std::string IntToStr (int n);
//helper function for Displaying Finish Text
void DisplayFinishText (unsigned int ms, const char* text);

//forward declaration for use as callback functions
void SwitchFPSVisibility ();
void SwitchSfx ();
void ChangeState ();
void ShutDown ();

class Game
 : public Singleton<Game>
{
 public:
  Game (int argc, char** argv);
  virtual ~Game ();
  int Loop ();
  void HandleEvents ();

  inline void SetScreen_W (int screen_w_in) { screen_w = screen_w_in; }
  inline int GetScreen_W () { return screen_w; }

  inline void SetScreen_H (int screen_h_in) { screen_h = screen_h_in; }
  inline int GetScreen_H () { return screen_h; }

  inline bool isSfxOn () { return sfxOn; }

  inline bool isFPSVisible () { return displayFPS; }
  inline void setFPSVisile () { displayFPS = true; }

  inline Music* GetMusic () { return music; }
  inline Mix_Chunk* GetSfx () { return sound; }
  inline SDL_Surface* GetScreen () { return screen; }
  inline State* GetState () { return game_state; }
  inline TTF_Font* GetMainFont () { return font; }

  friend void ChangeState ();
  friend void ShutDown ();
  friend void SwitchFPSVisibility ();
  friend void SwitchSfx ();

  static void Draw (SDL_Surface* screen, SDL_Surface* source, int x, int y);  // helper function for drawing

 private:
  int initSystems ();
  void closeSystems ();

  bool running;
  bool paused;

  bool displayFPS;
  bool sfxOn;

  int control_type;

  int current_state;

  int screen_w;
  int screen_h;
  int gameFPS;

  State* game_state;

  FpsCounter* fps_counter;

  SDL_Surface* screen;
  Music* music;
  Mix_Chunk* sound;
  TTF_Font* font;
};

#endif /* defined(__Arkanoid__Game__) */
