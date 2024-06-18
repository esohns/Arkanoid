#include "stdafx.h"

#include "Platform.h"

#include "ace/OS.h"
#include "ace/Log_Msg.h"

#include "Animation.h"
#include "defines.h"
#include "Game.h"
#include "Projectile.h"
#include "PlayingState.h"

Platform::Platform (const char* filename,
                    int maxFrame,
                    int frameDelay,
                    int frameWidth,
                    int frameHeight,
                    int animationColumns,
                    int animationDirection)
 : GameObject (filename,
               maxFrame,
               frameDelay,
               frameWidth,
               frameHeight,
               animationColumns,
               animationDirection)
 , has_effect (-1)
{
  //setting ID to PLAYER and calling superclass constructor
  SetID (PLAYER);
}

void
Platform::Destroy ()
{
  //calling superclass destructor
  GameObject::Destroy ();
}

void
Platform::Init ()
{
  // Initializing ball
  GameObject::Init (g_Game.GetScreen_W () / 2.0f,
                    static_cast<float> (g_Game.GetScreen_H ()) - 10.0f,
                    PLATFORM_BASE_SPEED,
                    0.0f,
                    0,
                    0,
                    animation->GetFrameWidth () / 2.0f,
                    animation->GetFrameHeight () / 2.0f);
  SetAlive (true);

  lives = 5;
  score = 0;
}

int
Platform::Update ()
{
  if (isAlive ())
  {
    GameObject::Update ();
    //Performing boundry checking
    if (x > g_Game.GetScreen_W () - boundX)
      x = g_Game.GetScreen_W () - boundX;
    else if (x < boundX)
      x = boundX;
    //Updating of animation exists
    if (animation)
      animation->Animate ();
  }

  if (!GetLives ())
  {
    PlayingState* playing_state = dynamic_cast<PlayingState*> (g_GamePtr->GetState ());
    playing_state->SetChangingStateFlag (true);
    char buffer_a[BUFSIZ];
    ACE_OS::memset (&buffer_a, 0, sizeof (char[BUFSIZ]));
    char* username = buffer_a;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    DWORD buffer_size = sizeof (char[BUFSIZ]);
    if (!GetUserName (buffer_a, &buffer_size))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to GetUserName: %d, continuing\n"),
                  GetLastError ()));
#else
    username = ACE_OS::getenv ("USER");
#endif // ACE_WIN32 || ACE_WIN64
    playing_state->PushScore (username, GetScore ());
  }
  return 0;
}

void
Platform::Render ()
{
  if (isAlive ())
  {
    GameObject::Render ();
    //Rendering if animation exists
    if (animation)
      animation->Draw (x - boundX, y - boundY);
  }
}

void
Platform::MoveLeft ()
{
  dirX = -1;
}

void
Platform::MoveRight ()
{
  dirX = 1;
}

void
Platform::StopMoving ()
{
  dirX = 0;
}

void
Platform::Collided (int objectID, col_dir dir)
{
  if (dir == NO_COLLISION)
    return;

  if (objectID == BALL)
  {
    if (!(has_effect == MAGNET)) // if platform is under effect of Magnet then we dont want to add points constantly
      AddPoint ();
  }
}

void
Platform::Shoot ()
{
  if (has_effect == GUN)
  {
    Projectile** projectiles_temp = dynamic_cast<PlayingState*> (g_GamePtr->GetState ())->GetProjectiles ();
    for (int i = 0; i < DEFAULT_PROJECTILES_MAX; i++)
      if (!projectiles_temp[i]->isAlive ())
      {
        projectiles_temp[i]->Init (static_cast<int> (x), static_cast<int> (y),
                                   12);
        return;
      }
  }
}

void
Platform::MorphPlatform (int effect_type)
{
  has_effect = effect_type;
  animation->SetFrame (effect_type + 1);
}
