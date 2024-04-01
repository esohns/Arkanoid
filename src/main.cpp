#include "stdafx.h"

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/OS.h"

#include "SDL.h"

#include "Game.h"

extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
main (int argc_in,
      char** argv_in)
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE::init ();
#endif // ACE_WIN32 || ACE_WIN64

  ACE_OS::srand (SDL_GetTicks ());

  Game game (argc_in, argv_in);
  int status = game.Loop ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE::fini ();
#endif // ACE_WIN32 || ACE_WIN64

  return status;
}
//#if defined(ACE_WIN32) || defined(ACE_WIN64)
#ifdef __cplusplus
}
#endif /* __cplusplus */
