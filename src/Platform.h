#ifndef __Arkanoid__Platform__
#define __Arkanoid__Platform__

#include "GameObject.h"

class Platform
 : public GameObject
{
 public:
  Platform (const char* = NULL, int = 0, int = 0, int = 0, int = 0, int = 0, int = 0);

  void Destroy ();

  void Init ();
  int Update ();
  void Render ();

  void MoveLeft ();
  void MoveRight ();
  void StopMoving ();

  void Shoot ();

  void MorphPlatform (int);

  inline int GetScore () { return score; }
  inline int GetLives () { return lives; }

  inline void AddLife () { ++lives; }
  inline void LoseLife () { --lives; }

  inline void AddPoint () { ++score; }

  void Collided (int, enum col_dir);

 private:
  int lives;
  int score;
  int has_effect;
};

#endif /* defined(__Arkanoid__Platform__) */
