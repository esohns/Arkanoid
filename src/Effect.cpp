#include "stdafx.h"

#include "Effect.h"

#include "ace/OS.h"

#include "Animation.h"
#include "Ball.h"
#include "Game.h"
#include "Platform.h"
#include "PlayingState.h"

Effect::Effect (const char* filename,
                int maxFrame,
                int frameDelay,
                int frameWidth,
                int frameHeight,
                int animationColumns,
                int animationDirection )
 : GameObject (filename,
               maxFrame,
               frameDelay,
               frameWidth,
               frameHeight,
               animationColumns,
               animationDirection)
{
  SetID (EFFECT);
}

void
Effect::Destroy ()
{
  GameObject::Destroy ();
}

void
Effect::Init (int x, int y)
{
  GameObject::Init (static_cast<float> (x), static_cast<float> (y),
                    0.0f, static_cast<float> (ACE_OS::rand () % 2 + 1),
                    0, 1,
                    animation->GetFrameWidth () / 2.0f, animation->GetFrameHeight () / 2.0f);
  SetAlive (true);
}

int
Effect::Update ()
{
  if (isAlive ())
  {
    GameObject::Update ();
    if (animation)
      animation->Animate ();
  }

  return 0;
}

void
Effect::Render ()
{
  if (isAlive ())
  {
    GameObject::Render ();
    if (animation)
      animation->Draw (x - boundX, y - boundY);
  }
}

void
Effect::Collided (int ObjectID, enum col_dir dir)
{
  if (dir == NO_COLLISION)
    return;

  if (ObjectID == PLAYER)
  {
    if (effect_type == MAGNET)
    {
      PlayingState* ps = dynamic_cast<PlayingState*> (g_GamePtr->GetState ());
      ps->GetPlatform ()->MorphPlatform (effect_type);
      ps->GetBall ()->MorphBall (effect_type);
      SetAlive (false);
    }
    else if (effect_type == GUN)
    {
      PlayingState* ps = dynamic_cast<PlayingState*> (g_GamePtr->GetState ());
      ps->GetPlatform ()->MorphPlatform (effect_type);
      SetAlive (false);
    }
    else if (effect_type == SECONDBALL)
    {
      dynamic_cast<PlayingState*> (g_GamePtr->GetState ())->LaunchSecondBall ();
      PlayingState* ps = dynamic_cast<PlayingState*> (g_GamePtr->GetState ());
      ps->GetPlatform ()->MorphPlatform (effect_type);
      SetAlive (false);
    }
  }
}
