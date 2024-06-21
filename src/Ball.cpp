#include "stdafx.h"

#include "Ball.h"

#include "ace/OS.h"

#include "Animation.h"
#include "Game.h"
#include "Platform.h"
#include "PlayingState.h"

Ball::Ball (const char* filename,
            int maxFrame,
            int frameDelay,
            int frameWidth,
            int frameHeight,
            int animationColumns,
            int animationDirection)
 : inherited (filename,
              maxFrame,
              frameDelay,
              frameWidth,
              frameHeight,
              animationColumns,
              animationDirection)
 , has_effect (-1)
 , stand_on_platform (false)
{
  //setting ID and calling superclass constructor
  SetID (BALL);
}

void
Ball::Destroy ()
{
  // object destructor calling superclass destructor
  inherited::Destroy ();
}

void
Ball::Init ()
{
  Platform* platform = dynamic_cast<PlayingState*> (g_GamePtr->GetState ())->GetPlatform ();
  float posX = platform->GetX ();
  float posY = platform->GetY ();
  int t_dirX = (ACE_OS::rand () % 2 + 1) * 2 - 3; //  picking random direction either left or right
  inherited::Init (posX, posY - (animation->GetFrameHeight () / 2.0f),
                   static_cast<float> (ACE_OS::rand () % 1 + 3), static_cast<float> (ACE_OS::rand () % 1 + 3),
                   t_dirX, -1,
                   animation->GetFrameWidth () / 2.0f, animation->GetFrameHeight () / 2.0f);
}

void
Ball::Render ()
{
  //if object is alive we draw it 
  if (isAlive ())
  {
    inherited::Render ();
    //If there is an animation, we draw it so that the centre of it is at (x,y)
    if (animation)
      animation->Draw (x - boundX, y - boundY);
  }
}

int
Ball::Update ()
{
  //if object is alive we update it
  if (!isAlive ())
    return 0;

  if (stand_on_platform)
  {
    Platform* platform = dynamic_cast<PlayingState*> (g_GamePtr->GetState ())->GetPlatform ();
    x = platform->GetX ();
    y = platform->GetY () - 20.0f;
  }
  else
  {
    inherited::Update ();

    // we do a boundary checking
    if ((x >= (g_Game.GetScreen_W () - boundX)) || (x <= boundX))
      dirX *= -1;
    else if (y <= boundY)
      dirY *= -1;
    
    if (y >= g_Game.GetScreen_H ())
    {
      SetAlive (false);
      LoseEffect ();

      PlayingState* playing_state_p = dynamic_cast<PlayingState*> (g_GamePtr->GetState ());
      ACE_ASSERT (playing_state_p);
      Ball* second_ball_p = playing_state_p->GetSecondBall ();
      if (this == second_ball_p)
        return 0;

      if (second_ball_p->isAlive ())
        playing_state_p->SwitchBalls ();
      else
      {
        Platform* platform = playing_state_p->GetPlatform ();
        platform->LoseLife ();
        platform->MorphPlatform (-1); // calling platform to lose it's effect because (last) ball has just died
      }
    }
  }
  
  // we also update its animation if it exists
  if (animation)
    animation->Animate ();

  return 0;
}

bool
Ball::IsOnPlatform ()
{
  // sanity check(s): initial situation ?
  PlayingState* playing_state_p = dynamic_cast<PlayingState*> (g_GamePtr->GetState());
  ACE_ASSERT (playing_state_p);
  Ball* ball_p = playing_state_p->GetBall ();
  if (ball_p == this && !this->isAlive ())
    return true;

  return stand_on_platform;
  //  return false;

  //Platform* platform_p = playing_state_p->GetPlatform ();
  //ACE_ASSERT (platform_p);

  //return inherited::y <= platform_p->GetY () - 20.0f;
}

void
Ball::StartFlying ()
{
  // starting the ball, if it's not alive we dont allow for starting it again, until it dies
  if (!isAlive ())
  {
    SetAlive (true);
    Init ();
  }
  else if (stand_on_platform)
  {
    stand_on_platform = false;
    Update (); //*TODO*: make sure that it will 'escape' platform
    Update ();
  }
}

void
Ball::Collided (int ObjectID, enum col_dir dir)
{
  if (dir == NO_COLLISION)
    return;

  //checking for collision with PLAYER
  if (ObjectID == PLAYER)
  {
    if (has_effect == MAGNET && stand_on_platform == false)
      stand_on_platform = true;
    else
    {
      switch (dir)
      {
        case LEFT:
          dirX = -1;
          break;
        case RIGHT:
          dirX = 1;
          break;
        case TOP:
          dirY = -1;
          break;
        case TLCOR:
          dirX = -1;
          dirY = -1;
          break;
        case TRCOR:
          dirX = 1;
          dirY = -1;
          break;
      }

      if (stand_on_platform == false)
      {
        if (velX < 0)
          velX -= 0.2f;
        else
          velX += 0.2f;
        if (velY < 0)
          velY -= 0.2f;
        else
          velY += 0.2f;

        dynamic_cast<PlayingState*> (g_GamePtr->GetState ())->GetPlatform ()->AddPoint ();

        if (g_GamePtr->isSfxOn ())
          Mix_PlayChannel (-1, g_GamePtr->GetSfx (), 0);
      } // end IF
    }
  }
  else if (ObjectID == BLOCK)
  {
    switch (dir)
    {
      case LEFT:
        dirX = -1;
        break;
      case RIGHT:
        dirX = 1;
        break;
      case TOP:
        dirY = -1;
        break;
      case BOTTOM:
        dirY = 1;
        break;
      case TLCOR:
        dirX = -1;
        dirY = -1;
        break;
      case TRCOR:
        dirX = 1;
        dirY = -1;
        break;
      case BLCOR:
        dirX = -1;
        dirY = 1;
        break;
      case BRCOR:
        dirX = 1;
        dirY = 1;
        break;
    }
  }
}
