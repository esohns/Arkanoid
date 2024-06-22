#include "stdafx.h"

#include "PlayingState.h"

#include <fstream>
#include <string>

#include "ace/OS.h"
#include "ace/Log_Msg.h"

#include "Ball.h"
#include "Block.h"
#include "defines.h"
#include "Effect.h"
#include "Game.h"
#include "GameObject.h"
#include "Gui.h"
#include "MapLoader.h"
#include "Platform.h"
#include "Projectile.h"

PlayingState::PlayingState ()
 : inherited ()
 , changingstate (false)
 , gobjects ()
 , map_loader (NULL)
 , ball (NULL)
 , second_ball (NULL)
 , platform (NULL)
 , effects (NULL)
 , projectiles (NULL)
 , gui (NULL)
 , second_ball_flag (false) // setting up flag , second wasnt intialized yet ( pushed on the list )
{
  char buffer_a[PATH_MAX];
  ACE_OS::getcwd (buffer_a, sizeof (char[PATH_MAX]));
  std::string path_base = buffer_a;
  path_base += ACE_DIRECTORY_SEPARATOR_STR_A;
  path_base += RESOURCE_DIRECTORY;
  path_base += ACE_DIRECTORY_SEPARATOR_STR_A;
  std::string graphics_base = path_base;
  graphics_base += ACE_TEXT_ALWAYS_CHAR (GRAPHICS_DIRECTORY);
  graphics_base += ACE_DIRECTORY_SEPARATOR_STR_A;
  std::string file = graphics_base;
  file += ACE_TEXT_ALWAYS_CHAR ("platformw.png");
  platform = new Platform (file.c_str (), 3, 0, 66, 18, 4, 0);
  file = graphics_base;
  file += ACE_TEXT_ALWAYS_CHAR ("ball.png");
  ball = new Ball (file.c_str (), 0, 1, 16, 16, 1, 1);
  second_ball = new Ball (file.c_str (), 0, 1, 16, 16, 1, 1);

  //pushing ball and platform on the list
  gobjects.push_back (platform);
  gobjects.push_back (ball);

  //loading blocks
  file = path_base;
  file += ACE_DIRECTORY_SEPARATOR_STR_A;
  file += ACE_TEXT_ALWAYS_CHAR ("config.cfg");
  map_loader = new MapLoader (file.c_str ());
  file = path_base;
  file += ACE_DIRECTORY_SEPARATOR_STR_A;
  file += ACE_TEXT_ALWAYS_CHAR (DEFAULT_MAP_FILE);
  gobjects.splice (gobjects.end (), map_loader->LoadMap (file.c_str ()));

  //loading projectiles
  file = graphics_base;
  file += ACE_TEXT_ALWAYS_CHAR ("effect.png");
  projectiles = new Projectile*[DEFAULT_PROJECTILES_MAX]; // Creating 10 projectile pointers as arbitrary value
  for (int i = 0; i < DEFAULT_PROJECTILES_MAX; i++)
  {
    projectiles[i] = new Projectile (file.c_str (), 23, 4, 16, 14, 24, 1);
    gobjects.push_back (projectiles[i]);
  }

  effects = new Effect*[3];
  effects[0] = new Effect (file.c_str (), 23, 4, 16, 14, 24, 1);
  effects[0]->SetEffectType (GUN);
  file = graphics_base;
  file += ACE_TEXT_ALWAYS_CHAR ("effect2.png");
  effects[1] = new Effect (file.c_str (), 10, 7, 34, 29, 11, 1);
  effects[1]->SetEffectType (MAGNET);
  file = graphics_base;
  file += ACE_TEXT_ALWAYS_CHAR ("effect3.png");
  effects[2] = new Effect (file.c_str (), 59, 1, 60, 60, 8, 1);
  effects[2]->SetEffectType (SECONDBALL);

  // we push effects as last elem. beacause they have to be rendered as last objs
  for (int i=0; i<3; i++)
    gobjects.push_back (effects[i]);

  gui = new Gui ();
  gui->Init (g_GamePtr->GetScreen_W () - 135, g_GamePtr->GetScreen_H () - 35);
}

PlayingState::~PlayingState ()
{
  delete map_loader;
  for (std::list<GameObject*>::iterator iter = gobjects.begin (); iter != gobjects.end (); iter++)
    (*iter)->Destroy ();
  delete gui;

  SaveHighscores (); // we save list of highscores to file
}

void
PlayingState::RenderState ()
{
  for (std::list<GameObject*>::iterator iter = gobjects.begin (); iter != gobjects.end (); iter++)
    (*iter)->Render ();

  gui->Render ();

  if (changingstate)
  {
    DisplayFinishText (3000, levelcomplete ? ACE_TEXT_ALWAYS_CHAR ("Level complete") : ACE_TEXT_ALWAYS_CHAR ("Game over"));
    changingstate = false;
    ChangeState ();
  }
}

void
PlayingState::UpdateState ()
{
  for (std::list<GameObject*>::iterator iter = gobjects.begin (); iter != gobjects.end (); iter++)
    for (std::list<GameObject*>::iterator iter2 = gobjects.begin (); iter2 != gobjects.end (); iter2++)
      (*iter)->Collided ((*iter2)->GetID (), (*iter)->detectCollision (*iter2));

  gui->Update ();

  for (std::list<GameObject*>::iterator iter = gobjects.begin (); iter != gobjects.end (); iter++)
    if ((*iter)->Update () == -1)
      break;  // if an update function returns -1 then we break from loop because we are about to change state

  levelcomplete = true;
  for (std::list<GameObject*>::iterator iter = gobjects.begin(); iter != gobjects.end(); iter++)
  {
    if ((*iter)->GetID () != BLOCK)
      continue;
    Block* block_p = static_cast<Block*> (*iter);
    if (block_p->GetHealth () > 0)
    {
      levelcomplete = false;
      break;
    }
  }
  if (levelcomplete)
  {
    changingstate = true;

    // *TODO*: load next level !!!
    char buffer_a[BUFSIZ];
    ACE_OS::memset (&buffer_a, 0, sizeof (char[BUFSIZ]));
    char* username = buffer_a;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    DWORD buffer_size = sizeof (char[BUFSIZ]);
    if (!GetUserNameA (buffer_a, &buffer_size))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to GetUserName: %d, continuing\n"),
                  GetLastError ()));
#else
    username = ACE_OS::getenv ("USER");
#endif // ACE_WIN32 || ACE_WIN64
    PushScore (username, platform->GetScore ());
  } // end IF
}

void
PlayingState::HandleEvents (Uint8* keystates, const SDL_Event& event, int control_type)
{
#if defined (SDL1_USE)
  if (keystates[SDLK_ESCAPE])
#elif defined (SDL2_USE)
  if (keystates[SDL_SCANCODE_ESCAPE])
#endif // SDL1_USE || SDL2_USE
  {
    ChangeState ();
    return;
  }

  // *TODO*: remove these cheats
#if defined (_DEBUG)
  if (event.key.keysym.sym == SDLK_F1)
  {
    platform->MorphPlatform (MAGNET);
    ball->MorphBall (MAGNET);
    second_ball->MorphBall (MAGNET);
  }
  else if (event.key.keysym.sym == SDLK_F2)
  {
    platform->MorphPlatform (GUN);
    ball->LoseEffect ();
    second_ball->LoseEffect ();
  }
  else if (event.key.keysym.sym == SDLK_F3)
  {
    LaunchSecondBall ();
    platform->MorphPlatform (SECONDBALL);
    ball->LoseEffect ();
    second_ball->LoseEffect ();
  }
  else if (event.key.keysym.sym == SDLK_F10)
    platform->AddLife ();
#endif // _DEBUG

  // Movement controls with keyboard
  if (control_type == KEYBOARD)
  {
#if defined (SDL1_USE)
    if (keystates[SDLK_SPACE])
#elif defined (SDL2_USE)
    if (keystates[SDL_SCANCODE_SPACE])
#endif // SDL1_USE || SDL2_USE
    {
      if (!ball->isAlive () ||
          ball->IsOnPlatform ())
        ball->StartFlying ();
      else if (second_ball->isAlive () &&
               second_ball->IsOnPlatform ())
        second_ball->StartFlying ();
    }

#if defined (SDL1_USE)
    if (keystates[SDLK_LEFT])
#elif defined (SDL2_USE)
    if (keystates[SDL_SCANCODE_LEFT])
#endif // SDL1_USE || SDL2_USE
      platform->MoveLeft ();
#if defined (SDL1_USE)
    else if (keystates[SDLK_RIGHT])
#elif defined (SDL2_USE)
    else if (keystates[SDL_SCANCODE_RIGHT])
#endif // SDL1_USE || SDL2_USE
      platform->MoveRight ();
    else
      platform->StopMoving ();

#if defined (SDL1_USE)
    if (keystates[SDLK_x])
#elif defined (SDL2_USE)
    if (keystates[SDL_SCANCODE_X])
#endif // SDL1_USE || SDL2_USE
      platform->Shoot ();
  }
  else if (control_type == MOUSE)
  {
    int x; // mouse x coordinate position
    SDL_GetMouseState (&x, NULL);
    if (x - 20 > platform->GetX ())
      platform->MoveRight ();
    else if (x + 20 < platform->GetX ())
      platform->MoveLeft ();
    else
      platform->StopMoving ();

    if (event.button.type == SDL_MOUSEBUTTONDOWN)
    {
      if (event.button.button == SDL_BUTTON_LEFT)
        platform->Shoot ();
      else if (event.button.button == SDL_BUTTON_MIDDLE)
      {
        if (!ball->isAlive () ||
            ball->IsOnPlatform ())
          ball->StartFlying ();
        else if (second_ball->isAlive () &&
                 second_ball->IsOnPlatform ())
          second_ball->StartFlying ();
      }
    }
  }
}

void
PlayingState::InitState ()
{
  for (std::list<GameObject*>::iterator iter = gobjects.begin (); iter != gobjects.end (); iter++)
    (*iter)->Init ();
}

void
PlayingState::SaveHighscores ()
{
  char buffer_a[PATH_MAX];
  ACE_OS::getcwd (buffer_a, sizeof (char[PATH_MAX]));
  std::string path_base = buffer_a;
  path_base += ACE_DIRECTORY_SEPARATOR_STR_A;
  path_base += RESOURCE_DIRECTORY;
  path_base += ACE_DIRECTORY_SEPARATOR_STR_A;
  std::string filename = path_base + ACE_TEXT_ALWAYS_CHAR ("highscores");
  std::ofstream file;
  file.open (filename.c_str (), ios::binary);

  for (std::list<std::pair<std::string, int> >::iterator iter = highsco_list.begin (); iter != highsco_list.end (); iter++)
    file << iter->first << ACE_TEXT_ALWAYS_CHAR (", ") << iter->second << ACE_TEXT_ALWAYS_CHAR ("\r\n"); // write windows(TM)-style newlines

  file.close ();
}

void
PlayingState::LaunchSecondBall ()
{
  int t_dirX = (ACE_OS::rand () % 2 + 1) * 2 - 3;
  int t_dirY = (ACE_OS::rand () % 2 + 1) * 2 - 3;

  second_ball->GameObject::Init (ball->GetX (), ball->GetY (),
                                 static_cast<float> (ACE_OS::rand () % 2 + 3), static_cast<float> (ACE_OS::rand () % 2 + 3),
                                 t_dirX, t_dirY,
                                 ball->GetBoundX (), ball->GetBoundY ());
  second_ball->SetAlive (true);

  if (!second_ball_flag)
  {
    gobjects.push_back (second_ball);
    second_ball_flag = true;
  }
}

void
PlayingState::SwitchBalls ()
{
  GameObject* temp_p = second_ball;
  second_ball = ball;
  ball = static_cast<Ball*> (temp_p);

  //ACE_ASSERT (second_ball_flag);
  //std::list<GameObject*>::iterator iter_2 = gobjects.end ();
  //for (std::list<GameObject*>::iterator iter = gobjects.begin (); iter != gobjects.end (); iter++)
  //  if (*iter == temp_p)
  //  {
  //    iter_2 = iter;
  //    break;
  //  } // end IF
  //ACE_ASSERT (iter_2 != gobjects.end ());
  //gobjects.erase (iter_2);
  //second_ball_flag = false;
}
