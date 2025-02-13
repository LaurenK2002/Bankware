#ifndef DATA_H
#define DATA_H
#include "GL/gl.h"
#define NUM_TEXTURES 12

enum ObjectTypes : GLuint {
	SHIP=0,
	BACKGROUND1,
	BACKGROUND2,
	BACKGROUND3,
	BACKGROUND4,
	ENEMY,
	BLADE,
	BULLET,
	EXPLOSION,
	SHIELD,
	POWERUP,
	BUOY
};

#endif
