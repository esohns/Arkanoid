#include "stdafx.h"

#include "Block.h"

#include "ace/OS.h"

#include "Animation.h"
#include "Game.h"
#include "Effect.h"
#include "PlayingState.h"

Block::Block (const char* filename,
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
 , health (0)
 , maxhealth (0)
{
  //setting ID to PLAYER and calling superclass constructor
  SetID (BLOCK);
}

Block::Block (SDL_Surface* image,
              int maxFrame,
              int frameDelay,
              int frameWidth,
              int frameHeight,
              int animationColumns,
              int animationDirection)
 : GameObject (image,
               maxFrame,
               frameDelay,
               frameWidth,
               frameHeight,
               animationColumns,
               animationDirection)
 , health (0)
 , maxhealth (0)
{
  //setting ID to PLAYER and calling superclass constructor
  SetID (BLOCK);
}

void
Block::Destroy ()
{
  //calling superclass destructor
  GameObject::Destroy ();
}

void
Block::Init (float x,
             float y,
             int speed,
             int dirX,
             int health)
{
  // Initializing block
  GameObject::Init (x, y,
                    static_cast<float> (speed), 0.0f,
                    dirX, 0,
                    animation->GetFrameWidth () / 2.0f, animation->GetFrameHeight () / 2.0f);
  SetAlive (true);

  Block::health = health;
  Block::maxhealth = health;
}

int
Block::Update ()
{
  if (isAlive ())
  {
    GameObject::Update ();

    //Performing boundry checking
    if (x > g_Game.GetScreen_W () - boundX || x < boundX)
      dirX *= -1;

    //Updating of animation exists
    if (animation)
      animation->Animate ();
  }

  return 0;
}

void
Block::Render ()
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
Block::Collided (int objectID, col_dir dir)
{
  if (dir == NO_COLLISION)
    return;

  if (objectID == BALL || objectID == PROJECTILE)
  {
    if (!--health)
    {
      Effect** effs = dynamic_cast<PlayingState*> (g_GamePtr->GetState ())->GetEffects ();
      if (ACE_OS::rand () % 2)
      {
        int index = ACE_OS::rand () % 3;
        if (!effs[index]->isAlive ())
          effs[index]->Init (static_cast<int> (x), static_cast<int> (y));
      }
      SetAlive (false);
    }
    else if (animation->IsAutoAnimation ())
      animation->SetFrame (maxhealth - health);

    if (g_GamePtr->isSfxOn ())
      Mix_PlayChannel(-1, g_GamePtr->GetSfx (), 0);
  }
  else if (objectID == BLOCK)
  {
    if (dir == LEFT)
      dirX = -1;
    else if (dir == RIGHT)
      dirX = 1;
  }
}
