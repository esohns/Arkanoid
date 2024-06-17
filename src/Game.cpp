#include "stdafx.h"

#include "Game.h"

#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "SDL_image.h"

#include "defines.h"
#include "MenuState.h"
#include "PlayingState.h"

Game::~Game ()
{
  delete music;
  delete fps_counter;
  delete game_state;

  closeSystems ();
}

Game::Game (int argc, char** argv)
 : running (true)
 , paused (false)
 , displayFPS (false)
 , sfxOn (true)
 , control_type (KEYBOARD)
 , current_state (MENU)
 , screen_w (ACE_OS::atoi (argv[1]))
 , screen_h (ACE_OS::atoi (argv[2]))
 , gameFPS (BASE_GAME_FPS)
 , game_state (NULL)
 , fps_counter (NULL)
 , screen (NULL)
 , music (NULL)
 , sound (NULL)
 , font (NULL)
{
  char buffer[MAX_PATH];
  ACE_OS::getcwd (buffer, sizeof (char[MAX_PATH]));

  if (initSystems () == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initSystems, aborting\n")));
    ACE_OS::exit (1);
  } // end IF

  std::string path_base = buffer;
  path_base += ACE_DIRECTORY_SEPARATOR_STR;
  path_base += RESOURCE_DIRECTORY;
  std::string file = path_base;
  file += ACE_DIRECTORY_SEPARATOR_STR;
  file += SOUNDS_DIRECTORY;
  file += ACE_DIRECTORY_SEPARATOR_STR;
  file += ACE_TEXT_ALWAYS_CHAR ("sfx.wav");
  sound = Mix_LoadWAV (file.c_str ());
  if (!sound)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Mix_LoadWAV: \"%s\", aborting\n"),
                ACE_TEXT (Mix_GetError ())));
    ACE_OS::exit (1);
  } // end IF

  SDL_WM_SetCaption (ACE_TEXT_ALWAYS_CHAR (WINDOW_CAPTION), NULL);

  file = path_base;
  file += ACE_DIRECTORY_SEPARATOR_STR;
  file += ACE_TEXT_ALWAYS_CHAR ("mainfont.ttf");
  font = TTF_OpenFont (file.c_str (), LARGE_FONT_SIZE);
  if (!font)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to TTF_OpenFont: \"%s\", aborting\n"),
                ACE_TEXT (TTF_GetError ())));
    ACE_OS::exit (1);
  } // end IF

  music = new Music (path_base);

  fps_counter = new FpsCounter (gameFPS);

  game_state = new MenuState ();
  game_state->InitState ();

  if (!music->isMusicOn ())
    SwitchMusic ();
}

int
Game::initSystems ()
{
  if (SDL_Init (SDL_INIT_EVERYTHING) < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SDL_Init: \"%s\", aborting\n"),
                ACE_TEXT (SDL_GetError ())));
    return -1;
  } // end IF
  screen = SDL_SetVideoMode (screen_w, screen_h, 32, SDL_SWSURFACE);

  if (TTF_Init () < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to TTF_Init: \"%s\", aborting\n"),
                ACE_TEXT (TTF_GetError ())));
    return -1;
  } // end IF

//#define ARKANOID_DEF_FREQUENCY 48000
  if (Mix_OpenAudio (MIX_DEFAULT_FREQUENCY,
                     //ARKANOID_DEF_FREQUENCY,
                     MIX_DEFAULT_FORMAT,
                     MIX_DEFAULT_CHANNELS,
                     4096) < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Mix_OpenAudio: \"%s\", aborting\n"),
                ACE_TEXT (Mix_GetError ())));
    return -1;
  } // end IF
  int mixer_flags_base_i = MIX_INIT_FLAC | MIX_INIT_MOD;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int mixer_flags_i = mixer_flags_base_i | MIX_INIT_MP3;
#else
  int mixer_flags_i = mixer_flags_base_i | MIX_INIT_OGG | MIX_INIT_FLUIDSYNTH;
#endif // ACE_WIN32 || ACE_WIN64
  int result = Mix_Init (mixer_flags_i);
  if (result != mixer_flags_i)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to Mix_Init(0x%x): 0x%x: \"%s\", continuing\n"),
                mixer_flags_i, result,
                ACE_TEXT (Mix_GetError ())));
  // debug info
  const int total = Mix_GetNumChunkDecoders ();
  for (int i = 0; i < total; i++)
    std::cerr << "Supported chunk decoder: " <<  Mix_GetChunkDecoder (i) << std::endl;

  return 0;
}

void
Game::closeSystems ()
{
  if (screen)
    SDL_FreeSurface (screen);
  if (font)
    TTF_CloseFont (font);
  if (sound)
    Mix_FreeChunk (sound);
  Mix_CloseAudio ();
  Mix_Quit ();
  TTF_Quit ();
  SDL_Quit ();
}

int
Game::Loop ()
{
  int render = -1;
  while (running)
  {
    render = fps_counter->measureFPS ();
    if (!render)
      continue;

    HandleEvents ();
    SDL_FillRect (screen, NULL, 0);

    game_state->UpdateState ();
    game_state->RenderState ();

    if (displayFPS)
    {
      char buffer_a[10] = {0};
      ::sprintf (buffer_a, "%d FPS", fps_counter->getFPS ());
      SDL_WM_SetCaption (buffer_a, NULL);
    } // end IF
    else
      SDL_WM_SetCaption (ACE_TEXT_ALWAYS_CHAR (WINDOW_CAPTION), NULL);

    SDL_Flip (screen);
  } // end WHILE

  return 0;
}

void
Game::HandleEvents ()
{
  union SDL_Event event;
  while (SDL_PollEvent (&event))
    if (event.type == SDL_QUIT)
      ShutDown ();
  Uint8* keystates = SDL_GetKeyState (NULL);
  if (keystates[SDLK_q])
    ShutDown ();

  game_state->HandleEvents (keystates, event, control_type);
}

void
ChangeState ()
{
  Game* game = g_GamePtr;
  if (game->current_state == PLAYING)
  {
    game->current_state = MENU;
    delete game->game_state;
    game->game_state = new MenuState ();
    game->game_state->InitState ();
  } // end IF
  else if (game->current_state == MENU)
  {
    game->current_state = PLAYING;
    delete game->game_state;
    game->game_state = new PlayingState ();
    game->game_state->InitState ();
  } // end ELSE IF
}

void
ShutDown ()
{
  g_GamePtr->running = false;
}

void
SwitchFPSVisibility ()
{
  g_GamePtr->displayFPS = !g_GamePtr->displayFPS;
  dynamic_cast<MenuState*> (g_GamePtr->GetState ())->UpdateInfo (SHOWFPS);
}

void
SwitchSfx ()
{
  g_GamePtr->sfxOn = !g_GamePtr->sfxOn;
  dynamic_cast<MenuState*> (g_GamePtr->GetState ())->UpdateInfo (SOUNDON);
}

void
Game::Draw (SDL_Surface* screen,
            SDL_Surface* source,
            int x, int y)
{
  SDL_Rect offset = {(Sint16)x, (Sint16)y, 0, 0};
  SDL_Rect clip = {0, 0, (Uint16)source->w, (Uint16)source->h};
  SDL_BlitSurface (source, &clip, screen, &offset);
}

std::string
IntToStr (int n)
{
  std::stringstream ss;
  ss << n;
  return ss.str ();
}

void
DisplayFinishText (unsigned int ms, const char* text)
{
  char buffer[MAX_PATH];
  ACE_OS::getcwd (buffer, sizeof (buffer));
  std::string path_base = buffer;
  path_base += ACE_DIRECTORY_SEPARATOR_STR;
  path_base += RESOURCE_DIRECTORY;
  std::string file = path_base;
  file += ACE_DIRECTORY_SEPARATOR_STR;
  file += ACE_TEXT_ALWAYS_CHAR ("font.ttf");
  TTF_Font* font = TTF_OpenFont (file.c_str (), HUGE_FONT_SIZE);
  if (!font)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to TTF_OpenFont: \"%s\", continuing\n"),
                ACE_TEXT (TTF_GetError ())));

  SDL_Surface* text_image = NULL;
  SDL_Surface* text_shade = NULL;
  SDL_Color color = {0x2b, 0xd7, 0xb7, 0};
  SDL_Color shade = {0xff, 0xff, 0xff, 0};
  if (font)
  {
    text_image = TTF_RenderText_Solid (font, text, color);
    text_shade = TTF_RenderText_Solid (font, text, shade);
  } // end IF

  int posX = g_GamePtr->GetScreen_W ()/2;
  int posY = g_GamePtr->GetScreen_H ()/2;
  Game::Draw (g_GamePtr->GetScreen (), text_shade, posX - text_shade->w/2 +2, posY - text_shade->h/2 +2);
  Game::Draw (g_GamePtr->GetScreen (), text_image, posX - text_image->w / 2, posY - text_image->h / 2);
  SDL_Flip (g_GamePtr->GetScreen ());

  unsigned int firstMeasure = SDL_GetTicks ();
  while (SDL_GetTicks () - firstMeasure <= ms);

  if (text_shade)
    SDL_FreeSurface (text_shade);
  if (text_image)
    SDL_FreeSurface (text_image);
  if (font)
    TTF_CloseFont (font);
}
