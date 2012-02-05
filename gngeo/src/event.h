#ifndef GNEVENT_H
#define GNEVENT_H

#include "SDL.h"

typedef enum {
	GN_NONE=0,
    GN_RIGHT,	
    GN_UPRIGHT,
    GN_UP,
	GN_UPLEFT,
    GN_LEFT,
	GN_DOWNLEFT,    
	GN_DOWN,
	GN_DOWNRIGHT,
    GN_A,
	GN_B,
	GN_C,
	GN_D,
	GN_START,
	GN_SELECT_COIN,
	GN_MENU_KEY,
	GN_HOTKEY1,
	GN_HOTKEY2,
	GN_HOTKEY3,
	GN_HOTKEY4,
    GN_TURBO,
	GN_MAX_KEY,
}GNGEO_BUTTON;

#define VSTICK_NB_BUTTON 8
typedef struct {int button_id,x,y,w,h;Uint8 r,g,b;SDL_FingerID finger_id;} t_touch_area;

struct BUT_MAP {
	Uint8 player; /* 0=none 1=p1 2=p2 3=both */
	GNGEO_BUTTON map; /* Mapped button */
};
struct BUT_MAPJAXIS {
	Uint8 player; /* 0=none 1=p1 2=p2 3=both */
	GNGEO_BUTTON map; /* Mapped button */
	int dir; /* Only for joystick axis */
	int value;
};

typedef struct JOYMAP {
	struct BUT_MAP key[SDLK_LAST];
	struct BUT_MAP **jbutton;
	struct BUT_MAPJAXIS **jaxe;
	struct BUT_MAP **jhat;
}JOYMAP;

JOYMAP *jmap;
Uint8 joy_state[2][GN_MAX_KEY];


bool init_event(void);

bool create_joymap_from_string(int player,char *jconf);
int handle_event(void);
int wait_event(void);
void reset_event(void);

#endif
