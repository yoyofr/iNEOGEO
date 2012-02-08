#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdbool.h>

#define SLIDEY_CHANGE_RENDERMODE_MIN 10
#define SLIDEX_CHANGE_RENDERMODE_MAX 2

#include "SDL.h"
#include "SDL_events.h"
#include "screen.h"
#include "event.h"
#include "conf.h"
#include "emu.h"
#include "memory.h"

#include "wiimote.h"
int iOS_wiiDeadZoneValue;
int iOS_inGame;
int iOS_waysStick=0;
float joy_analog_x[4];
float joy_analog_y[4];

extern int device_w,device_h,device_isSlow,device_isIpad;
extern int cur_width,cur_height;

int virtual_stick_on=1;
int virtual_stick_pad=0;
Uint8 virtual_stick_buttons_alpha=128;
Uint8 virtual_stick_buttons_alpha2=200;

int wm_joy_pl1,wm_joy_pl2;
int wm_prev_joy_pl1=0;
int wm_prev_joy_pl2=0;

int virtual_stick_posx=70;
int virtual_stick_posy=320-70;
int virtual_stick_maxdist=70;
int virtual_stick_mindist=16;
int virtual_stick_maxdist2=70*70;
int virtual_stick_mindist2=16*16;
float virtual_stick_angle;
SDL_FingerID virtual_stick_padfinger;
int slide_detected;

t_touch_area virtual_stick_iphone_landscape[VSTICK_NB_BUTTON]={
    {GN_A,          480-64-10-64,   320-64-6-64,   64,64,0xFF,0x00,0x00,0},  //red
    {GN_B,          480-64,         320-64-6-64-10,   64,64,0xFF,0xFF,0x00,0},  //yellow
    {GN_C,          480-64-10-64,   320-64,         64,64,0x00,0xFF,0x00,0},  //green
    {GN_D,          480-64,         320-64-10,         64,64,0x00,0x00,0xFF,0},  //blue
    
    {GN_START,      480-48,         0,              48,48,0xFF,0xFF,0xFF,0},
    {GN_SELECT_COIN,480-48,         48,             48,48,0x8F,0x8F,0x8F,0},
    {GN_MENU_KEY,     0,            0,              48,48,0xEF,0xFF,0x7F,0},
    {GN_TURBO,        0,            48,             48,48,0xFF,0x7F,0xFF,0}
};

t_touch_area virtual_stick_iphone_portrait[VSTICK_NB_BUTTON]={
    {GN_A,          320-64-10-64,   480-2*64-6-20,   64,64,0xFF,0x00,0x00,0},  //red
    {GN_B,          320-64,         480-2*64-6-30,   64,64,0xFF,0xFF,0x00,0},  //yellow
    {GN_C,          320-64-10-64,   480-64-20,         64,64,0x00,0xFF,0x00,0},  //green
    {GN_D,          320-64,         480-64-30,         64,64,0x00,0x00,0xFF,0},  //blue
    
    {GN_START,      270,         240,               48,48,0xFF,0xFF,0xFF,0},
    {GN_SELECT_COIN,195,         240,               48,48,0x8F,0x8F,0x8F,0},
    {GN_MENU_KEY,     0,            240,            48,48,0xEF,0xFF,0x7F,0},
    {GN_TURBO,        75,            240,           48,48,0xFF,0x7F,0xFF,0}
};


t_touch_area virtual_stick_ipad_landscape[VSTICK_NB_BUTTON]={
    {GN_A,          1024-96*2-10,   768-96*2-6,    96,96,0xFF,0x00,0x00,0},  //red
    {GN_B,          1024-96,        768-96*2-6-20,    96,96,0xFF,0xFF,0x00,0},  //yellow
    {GN_C,          1024-96*2-10,   768-96,         96,96,0x00,0xFF,0x00,0},  //green
    {GN_D,          1024-96,        768-96-20,         96,96,0x00,0x00,0xFF,0},  //blue
    
    {GN_START,      1024-80,        0,              64,64,0xFF,0xFF,0xFF,0},
    {GN_SELECT_COIN,1024-80,        100,            64,64,0x8F,0x8F,0x8F,0},
    {GN_MENU_KEY,     0,            0,              64,64,0xEF,0xFF,0x7F,0},
    {GN_TURBO,        0,            100,            64,64,0xFF,0x7F,0xFF,0}
};

t_touch_area virtual_stick_ipad_portrait[VSTICK_NB_BUTTON]={
    {GN_A,          768-96*2-20,   1024-96*2-6-60,    96,96,0xFF,0x00,0x00,0},  //red
    {GN_B,          768-96,        1024-96*2-6-20-60,    96,96,0xFF,0xFF,0x00,0},  //yellow
    {GN_C,          768-96*2-20,   1024-96-60,         96,96,0x00,0xFF,0x00,0},  //green
    {GN_D,          768-96,        1024-96-20-60,         96,96,0x00,0x00,0xFF,0},  //blue
    
    {GN_START,      768-80,        600,             64,64,0xFF,0xFF,0xFF,0},
    {GN_SELECT_COIN,768-80-120,    600,             64,64,0x8F,0x8F,0x8F,0},
    {GN_MENU_KEY,     0,           600,             64,64,0xEF,0xFF,0x7F,0},
    {GN_TURBO,        120,         600,             64,64,0xFF,0x7F,0xFF,0}
};


t_touch_area *virtual_stick=virtual_stick_iphone_portrait;
int gTurboMode;

static int get_mapid(char *butid) {
    //	printf("Get mapid %s\n",butid);
	if (!strcmp(butid,"A")) return GN_A;
	if (!strcmp(butid,"B")) return GN_B;
	if (!strcmp(butid,"C")) return GN_C;
	if (!strcmp(butid,"D")) return GN_D;
    
	if (!strcmp(butid,"UP")) return GN_UP;
	if (!strcmp(butid,"DOWN")) return GN_DOWN;
	if (!strcmp(butid,"UPDOWN")) return GN_UP;
    
	if (!strcmp(butid,"LEFT")) return GN_LEFT;
	if (!strcmp(butid,"RIGHT")) return GN_RIGHT;
	if (!strcmp(butid,"LEFTRIGHT")) return GN_LEFT;
    
	if (!strcmp(butid,"JOY")) return GN_UP;
    
	if (!strcmp(butid,"START")) return GN_START;
	if (!strcmp(butid,"COIN")) return GN_SELECT_COIN;
    
	if (!strcmp(butid,"MENU")) return GN_MENU_KEY;
    
	if (!strcmp(butid,"HOTKEY1")) return GN_HOTKEY1;
	if (!strcmp(butid,"HOTKEY2")) return GN_HOTKEY2;
	if (!strcmp(butid,"HOTKEY3")) return GN_HOTKEY3;
	if (!strcmp(butid,"HOTKEY4")) return GN_HOTKEY4;
    
    if (!strcmp(butid,"TURBO")) return GN_TURBO;
    
	return GN_NONE;
}

bool create_joymap_from_string(int player,char *jconf) {
	char *v;
	char butid[32]={0,};
	char jevt;
	int code;
	int jid;
	int rc;
	char type;
	//printf("Jconf=%s\n",jconf);
	if (jconf==NULL) return 0;
	v=strdup(jconf);
	v=strtok(v,",");
	//printf("V1=%s\n",v);
	while(v) {
		rc=sscanf(v,"%[A-Z1-4]=%c%d%c%d",butid,&type,&jid,&jevt,&code);
		if (rc==3 && type=='K') { /* Keyboard */
			//printf("%s | keycode %d\n",butid,jid);
			code=jid;
			if (code<SDLK_LAST) {
				jmap->key[code].player=player;
				jmap->key[code].map=get_mapid(butid);
			}
			//printf("%d\n",get_mapid(butid));
		}
		if (rc==5 && type=='J') {
            //			printf("%d, %s | joy no %d | evt %c | %d\n",rc,butid,jid,jevt,code);
			if (jid<conf.nb_joy) {
				switch(jevt) {
                    case 'A':
                        if (code<SDL_JoystickNumAxes(conf.joy[jid])) {
                            jmap->jaxe[jid][code].player=player;
                            jmap->jaxe[jid][code].map=get_mapid(butid);
                            jmap->jaxe[jid][code].dir=1;
                        }
                        break;
                    case 'a': /* Inverted axis */
                        if (code<SDL_JoystickNumAxes(conf.joy[jid])) {
                            jmap->jaxe[jid][code].player=player;
                            jmap->jaxe[jid][code].map=get_mapid(butid);
                            jmap->jaxe[jid][code].dir=-1;
                        }
                        break;
                    case 'H':
                        if (code<SDL_JoystickNumHats(conf.joy[jid])) {
                            jmap->jhat[jid][code].player=player;
                            jmap->jhat[jid][code].map=get_mapid(butid);
                        }
                        break;
                    case 'B':
                        if (code<SDL_JoystickNumButtons(conf.joy[jid])) {
                            jmap->jbutton[jid][code].player=player;
                            jmap->jbutton[jid][code].map=get_mapid(butid);
                        }
                        break;
				}
			}
		}
        
		v=strtok(NULL,",");
	}
    
	return true;
}

bool init_event(void) {
	int i;
    //	printf("sizeof joymap=%d nb_joy=%d\n",sizeof(JOYMAP),conf.nb_joy);
	jmap=calloc(sizeof(JOYMAP),1);
    
#ifdef WII
	conf.nb_joy = 4;
#else	
	conf.nb_joy = SDL_NumJoysticks();
#endif
	if( conf.nb_joy>0) {
		if (conf.joy!=NULL) free(conf.joy);
		conf.joy=calloc(sizeof(SDL_Joystick*),conf.nb_joy);
		
		SDL_JoystickEventState(SDL_ENABLE);
		
		jmap->jbutton=calloc(conf.nb_joy,sizeof(struct BUT_MAP*));
		jmap->jaxe=   calloc(conf.nb_joy,sizeof(struct BUT_MAPJAXIS*));
		jmap->jhat=   calloc(conf.nb_joy,sizeof(struct BUT_MAP*));
		
        
		/* Open all the available joystick */
		for (i=0;i<conf.nb_joy;i++) {
			conf.joy[i]=SDL_JoystickOpen(i);
            /*			printf("joy \"%s\", axe:%d, button:%d\n",
             SDL_JoystickName(i),
             SDL_JoystickNumAxes(conf.joy[i])+ (SDL_JoystickNumHats(conf.joy[i]) * 2),
             SDL_JoystickNumButtons(conf.joy[i]));*/
			jmap->jbutton[i]=calloc(SDL_JoystickNumButtons(conf.joy[i]),sizeof(struct BUT_MAP));
			jmap->jaxe[i]=calloc(SDL_JoystickNumAxes(conf.joy[i]),sizeof(struct BUT_MAPJAXIS));
			jmap->jhat[i]=calloc(SDL_JoystickNumHats(conf.joy[i]),sizeof(struct BUT_MAP));
		}
	}
	create_joymap_from_string(1,CF_STR(cf_get_item_by_name("p1control")));
	create_joymap_from_string(2,CF_STR(cf_get_item_by_name("p2control")));
	return true;
}
int handle_pdep_event(SDL_Event *event) {
	switch (event->type) {
        case SDL_KEYDOWN:
            switch (event->key.keysym.sym) {
                case SDLK_ESCAPE:
                    return 1;
                    break;
                case SDLK_F12:
                    screen_fullscreen();
                    break;
                default:
                    break;
            }
            break;
		default:
			break;
	}
	return 0;
}

#define EVGAME 1
#define EVMENU 2

int icade_detected=0;

int vstick_update_status(int rx,int ry) {
    float angle;
    //compute distance    
    //    printf("%d %d / %d %d\n",rx,ry,virtual_stick_posx,virtual_stick_posy);
    float dist=(rx-virtual_stick_posx)*(rx-virtual_stick_posx)+(ry-virtual_stick_posy)*(ry-virtual_stick_posy);
    
    
    virtual_stick_pad=0; //Reset pad state
    if ((dist>virtual_stick_mindist2)&&(dist<virtual_stick_maxdist2)) {
        //compute angle
        //        float rdist=sqrtf(dist);
        float dx=rx-virtual_stick_posx;
        float dy=-ry+virtual_stick_posy;
        if (dx!=0) {
            
            angle=atanf(dy/dx);
            if ((dx>=0)&&(dy>=0)) { //TOP RIGHT
                
            } else if ((dx<=0)&&(dy>=0)) { //TOP LEFT
                angle=M_PI+angle;
            } else if ((dx<=0)&&(dy<=0)) { //BOTTOM LEFT
                angle=M_PI+angle;
            } else if ((dx>=0)&&(dy<=0)) { //BOTTOM RIGHT
                angle=M_PI*2+angle;
            }
        } else {
            if (dy>0) angle=M_PI/2;
            else angle=M_PI*3/2;
        }
        
        virtual_stick_angle=angle;
        if ( ((virtual_stick_angle<M_PI*2)&&(virtual_stick_angle>=M_PI*2-M_PI/8))||((virtual_stick_angle<M_PI/8)&&(virtual_stick_angle>=0))) { //Right
            virtual_stick_pad=GN_RIGHT;
        } else if ((virtual_stick_angle>=-M_PI/8+M_PI/4)&&(virtual_stick_angle<M_PI/8+M_PI/4)) { //Up&Right
            virtual_stick_pad=GN_UPRIGHT;
        } else if ((virtual_stick_angle>=-M_PI/8+2*M_PI/4)&&(virtual_stick_angle<M_PI/8+2*M_PI/4)) { //Up
            virtual_stick_pad=GN_UP;
        } else if ((virtual_stick_angle>=-M_PI/8+3*M_PI/4)&&(virtual_stick_angle<M_PI/8+3*M_PI/4)) { //Up&Left
            virtual_stick_pad=GN_UPLEFT;
        } else if ((virtual_stick_angle>=-M_PI/8+4*M_PI/4)&&(virtual_stick_angle<M_PI/8+4*M_PI/4)) { //Left
            virtual_stick_pad=GN_LEFT;
        } else if ((virtual_stick_angle>=-M_PI/8+5*M_PI/4)&&(virtual_stick_angle<M_PI/8+5*M_PI/4)) { //Left&Down
            virtual_stick_pad=GN_DOWNLEFT;
        } else if ((virtual_stick_angle>=-M_PI/8+6*M_PI/4)&&(virtual_stick_angle<M_PI/8+6*M_PI/4)) { //Down
            virtual_stick_pad=GN_DOWN;
        } else if ((virtual_stick_angle>=-M_PI/8+7*M_PI/4)&&(virtual_stick_angle<M_PI/8+7*M_PI/4)) { //Down&Right
            virtual_stick_pad=GN_DOWNRIGHT;
        }
        //    printf("angle: %f pad:%02X\n",angle*180/M_PI,virtual_stick_pad);
    }
    
    return virtual_stick_pad;
}

int handle_event(void) {
	SDL_Event event;
    SDL_Touch *state;
    //	int i;
    int rx,ry;
	int ret;
	int jaxis_threshold=10000;
    
    int wm_joy_pl1,wm_joy_pl2;
    wm_joy_pl1=wm_joy_pl2=0;
    
    if (device_isIpad) {
        if (cur_width>cur_height) virtual_stick=virtual_stick_ipad_landscape;
        else virtual_stick=virtual_stick_ipad_portrait;
    } else {
        if (cur_width>cur_height) virtual_stick=virtual_stick_iphone_landscape;
        else virtual_stick=virtual_stick_iphone_portrait;
    }
    
    if (num_of_joys>=2) {
        if (wm_joy_pl2=iOS_wiimote_check(&(joys[1]))) virtual_stick_on=0;
        joy_state[1][GN_UP]=(wm_joy_pl2&WII_JOY_UP?1:0);
        joy_state[1][GN_DOWN]=(wm_joy_pl2&WII_JOY_DOWN?1:0);
        joy_state[1][GN_LEFT]=(wm_joy_pl2&WII_JOY_LEFT?1:0);
        joy_state[1][GN_RIGHT]=(wm_joy_pl2&WII_JOY_RIGHT?1:0);
        joy_state[1][GN_A]=(wm_joy_pl2&WII_JOY_A?1:0);
        joy_state[1][GN_B]=(wm_joy_pl2&WII_JOY_B?1:0);
        joy_state[1][GN_C]=(wm_joy_pl2&WII_JOY_C?1:0);
        joy_state[1][GN_D]=(wm_joy_pl2&WII_JOY_D?1:0);
        joy_state[1][GN_SELECT_COIN]=(wm_joy_pl2&WII_JOY_SELECT?1:0);
        joy_state[1][GN_START]=(wm_joy_pl2&WII_JOY_START?1:0);
        joy_state[1][GN_MENU_KEY]=(wm_joy_pl2&WII_JOY_HOME?1:0);
        joy_state[1][GN_TURBO]=(wm_joy_pl2&WII_JOY_E?1:0);
    }
    if (num_of_joys>=1) {        
        if (wm_joy_pl1=iOS_wiimote_check(&(joys[0]))) virtual_stick_on=0;
        
        if (wm_joy_pl1!=wm_prev_joy_pl1) {
            wm_prev_joy_pl1=wm_joy_pl1;

        joy_state[0][GN_UP]=(wm_joy_pl1&WII_JOY_UP?1:0);
        joy_state[0][GN_DOWN]=(wm_joy_pl1&WII_JOY_DOWN?1:0);
        joy_state[0][GN_LEFT]=(wm_joy_pl1&WII_JOY_LEFT?1:0);
        joy_state[0][GN_RIGHT]=(wm_joy_pl1&WII_JOY_RIGHT?1:0);
        joy_state[0][GN_A]=(wm_joy_pl1&WII_JOY_A?1:0);
        joy_state[0][GN_B]=(wm_joy_pl1&WII_JOY_B?1:0);
        joy_state[0][GN_C]=(wm_joy_pl1&WII_JOY_C?1:0);
        joy_state[0][GN_D]=(wm_joy_pl1&WII_JOY_D?1:0);
        joy_state[0][GN_SELECT_COIN]=(wm_joy_pl1&WII_JOY_SELECT?1:0);
        joy_state[0][GN_START]=(wm_joy_pl1&WII_JOY_START?1:0);
        joy_state[0][GN_MENU_KEY]=(wm_joy_pl1&WII_JOY_HOME?1:0);
        joy_state[0][GN_TURBO]=(wm_joy_pl1&WII_JOY_E?1:0);
        }
    }
    
	while (SDL_PollEvent(&event)) {
	    if ((ret=handle_pdep_event(&event))!=0) {
	    	return ret;
	    }
    	switch (event.type) {
            case SDL_MOUSEMOTION:
                break;
            case SDL_MOUSEBUTTONDOWN:
                break;
            case SDL_MOUSEBUTTONUP:
                break;
            case SDL_FINGERMOTION:
                state = SDL_GetTouch(event.tfinger.touchId);
                rx = event.tfinger.x*cur_width / state->xres;
                ry = event.tfinger.y*cur_height / state->yres;
                
                //printf("delta: %d x %d\n",event.tfinger.dx*cur_width/ state->xres,event.tfinger.dy*cur_height/ state->yres);
                if ((event.tfinger.dy*100/ state->yres < -SLIDEY_CHANGE_RENDERMODE_MIN)&&
                    (abs(event.tfinger.dx*100/ state->xres) < SLIDEX_CHANGE_RENDERMODE_MAX)) {
                    slide_detected=1;
                }
                
                if (event.tfinger.fingerId==virtual_stick_padfinger) { //is it the finger on pad
                    if (vstick_update_status(rx,ry)==0) virtual_stick_padfinger=0;
                    joy_state[0][GN_UP]=(virtual_stick_pad==GN_UP?1:0);
                    joy_state[0][GN_DOWN]=(virtual_stick_pad==GN_DOWN?1:0);
                    joy_state[0][GN_LEFT]=(virtual_stick_pad==GN_LEFT?1:0);
                    joy_state[0][GN_RIGHT]=(virtual_stick_pad==GN_RIGHT?1:0);
                    joy_state[0][GN_UPRIGHT]=(virtual_stick_pad==GN_UPRIGHT?1:0);
                    joy_state[0][GN_DOWNRIGHT]=(virtual_stick_pad==GN_DOWNRIGHT?1:0);
                    joy_state[0][GN_UPLEFT]=(virtual_stick_pad==GN_UPLEFT?1:0);
                    joy_state[0][GN_DOWNLEFT]=(virtual_stick_pad==GN_DOWNLEFT?1:0);
                } else if (virtual_stick_padfinger==0) {
                    if (vstick_update_status(rx,ry)) virtual_stick_padfinger=event.tfinger.fingerId;
                    joy_state[0][GN_UP]=(virtual_stick_pad==GN_UP?1:0);
                    joy_state[0][GN_DOWN]=(virtual_stick_pad==GN_DOWN?1:0);
                    joy_state[0][GN_LEFT]=(virtual_stick_pad==GN_LEFT?1:0);
                    joy_state[0][GN_RIGHT]=(virtual_stick_pad==GN_RIGHT?1:0);
                    joy_state[0][GN_UPRIGHT]=(virtual_stick_pad==GN_UPRIGHT?1:0);
                    joy_state[0][GN_DOWNRIGHT]=(virtual_stick_pad==GN_DOWNRIGHT?1:0);
                    joy_state[0][GN_UPLEFT]=(virtual_stick_pad==GN_UPLEFT?1:0);
                    joy_state[0][GN_DOWNLEFT]=(virtual_stick_pad==GN_DOWNLEFT?1:0);
                }
                
                for (int i=0;i<VSTICK_NB_BUTTON;i++) {                    
                    //is there a button already pressed with this finger ?
                    if (virtual_stick[i].finger_id==event.tfinger.fingerId) {
                        //a button was pressed and finger moved
                        //check if finger is still in button area
                        if ((rx>virtual_stick[i].x)&&(rx<virtual_stick[i].x+virtual_stick[i].w)&&
                            (ry>virtual_stick[i].y)&&(ry<virtual_stick[i].y+virtual_stick[i].h)){
                            break;
                        } else {
                            //button not pressed anymore
                            //do not break to check if finger moved to a new button
                            virtual_stick[i].finger_id=0;
                            joy_state[0][virtual_stick[i].button_id]=0;                            
                        }
                    } else {
                        //did the finger move to a new button area ?
                        if ((rx>virtual_stick[i].x)&&(rx<virtual_stick[i].x+virtual_stick[i].w)&&
                            (ry>virtual_stick[i].y)&&(ry<virtual_stick[i].y+virtual_stick[i].h)){
                            joy_state[0][virtual_stick[i].button_id]=1;
                            virtual_stick[i].finger_id=event.tfinger.fingerId;                        
                        }
                    }
                }
                
                break;
            case SDL_FINGERDOWN:
                virtual_stick_on=1;
                state = SDL_GetTouch(event.tfinger.touchId);
                rx = event.tfinger.x*cur_width / state->xres;
                ry = event.tfinger.y*cur_height / state->yres;
                
                
                if (vstick_update_status(rx,ry)) { //finger is on pad
                    joy_state[0][GN_UP]=(virtual_stick_pad==GN_UP?1:0);
                    joy_state[0][GN_DOWN]=(virtual_stick_pad==GN_DOWN?1:0);
                    joy_state[0][GN_LEFT]=(virtual_stick_pad==GN_LEFT?1:0);
                    joy_state[0][GN_RIGHT]=(virtual_stick_pad==GN_RIGHT?1:0);
                    joy_state[0][GN_UPRIGHT]=(virtual_stick_pad==GN_UPRIGHT?1:0);
                    joy_state[0][GN_DOWNRIGHT]=(virtual_stick_pad==GN_DOWNRIGHT?1:0);
                    joy_state[0][GN_UPLEFT]=(virtual_stick_pad==GN_UPLEFT?1:0);
                    joy_state[0][GN_DOWNLEFT]=(virtual_stick_pad==GN_DOWNLEFT?1:0);
                    virtual_stick_padfinger=event.tfinger.fingerId;
                } else { //check if finger is on a button
                    for (int i=0;i<VSTICK_NB_BUTTON;i++) {
                        if ((rx>virtual_stick[i].x)&&(rx<virtual_stick[i].x+virtual_stick[i].w)&&
                            (ry>virtual_stick[i].y)&&(ry<virtual_stick[i].y+virtual_stick[i].h)){
                            joy_state[0][virtual_stick[i].button_id]=1;
                            virtual_stick[i].finger_id=event.tfinger.fingerId;
                            break;
                        }
                    }       
                }
                break;
            case SDL_FINGERUP:
                if (virtual_stick_padfinger==event.tfinger.fingerId) {
                    virtual_stick_pad=0;                    
                    joy_state[0][GN_UP]=0;
                    joy_state[0][GN_DOWN]=0;
                    joy_state[0][GN_LEFT]=0;
                    joy_state[0][GN_RIGHT]=0;
                    joy_state[0][GN_UPRIGHT]=0;
                    joy_state[0][GN_DOWNRIGHT]=0;
                    joy_state[0][GN_UPLEFT]=0;
                    joy_state[0][GN_DOWNLEFT]=0;
                } 
            
                    
                    for (int i=0;i<VSTICK_NB_BUTTON;i++) 
                        if (virtual_stick[i].finger_id==event.tfinger.fingerId) {
                            virtual_stick[i].finger_id=0;
                            joy_state[0][virtual_stick[i].button_id]=0;
                            break;
                        }
                if (slide_detected) {
                    slide_detected=0;
                    conf.rendermode++;
                    if (conf.rendermode>3) conf.rendermode=0;

                }
                break;
                
            case SDL_KEYUP:
                //printf("%d\n",jmap->key[event.key.keysym.sym].player);
                switch (jmap->key[event.key.keysym.sym].player) {
                    case 1:
                        joy_state[0][jmap->key[event.key.keysym.sym].map]=0;
                        break;
                    case 2:
                        joy_state[1][jmap->key[event.key.keysym.sym].map]=0;
                        break;
                    case 3:
                        joy_state[1][jmap->key[event.key.keysym.sym].map]=0;
                        joy_state[0][jmap->key[event.key.keysym.sym].map]=0;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_KEYDOWN:
                virtual_stick_on=0;
                icade_detected=1;
                //				printf("%d %c\n", event.key.keysym.sym,event.key.keysym.sym);
                switch (jmap->key[event.key.keysym.sym].player) {
                    case 1:
                        joy_state[0][jmap->key[event.key.keysym.sym].map]=1;
                        break;
                    case 2:
                        joy_state[1][jmap->key[event.key.keysym.sym].map]=1;
                        break;
                    case 3:
                        joy_state[1][jmap->key[event.key.keysym.sym].map]=1;
                        joy_state[0][jmap->key[event.key.keysym.sym].map]=1;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_JOYHATMOTION: /* Hat only support Joystick map */
            {
                int player=jmap->jhat[event.jhat.which][event.jhat.hat].player;
                int map=jmap->jhat[event.jhat.which][event.jhat.hat].map;
                int i;
                if (player && map==GN_UP) {
                    player-=1;
                    for(i=GN_UP;i<=GN_RIGHT;i++)
                        joy_state[player][i]=0;
                    if (event.jhat.value&SDL_HAT_UP) joy_state[player][GN_UP]=1;
                    if (event.jhat.value&SDL_HAT_DOWN) joy_state[player][GN_DOWN]=1;
                    if (event.jhat.value&SDL_HAT_LEFT) joy_state[player][GN_LEFT]=1;
                    if (event.jhat.value&SDL_HAT_RIGHT) joy_state[player][GN_RIGHT]=1;
                    
                }
                
                //printf("SDL_JOYHATMOTION  %d %d %d\n",event.jhat.which,
                //event.jhat.hat,event.jhat.value);
            }
                break;
            case SDL_JOYAXISMOTION:
            {
                int player=jmap->jaxe[event.jaxis.which][event.jaxis.axis].player;
                int map=jmap->jaxe[event.jaxis.which][event.jaxis.axis].map;
                int oldvalue=jmap->jaxe[event.jaxis.which][event.jaxis.axis].value;
                int value=0;
                //if (event.jaxis.axis!=6 &&event.jaxis.axis!=7 )
                //	printf("Axiw motions %d %d %d\n",event.jaxis.which,event.jaxis.axis,event.jaxis.value);
                if (player) {
                    player-=1;
                    
                    value=event.jaxis.value*jmap->jaxe[event.jaxis.which][event.jaxis.axis].dir;
                    
                    //printf("%d %d %d\n",player,map,value);
                    if (map==GN_UP || map==GN_DOWN) {
                        if (value>jaxis_threshold) {
                            joy_state[player][GN_UP]=1;
                            joy_state[player][GN_DOWN]=0;
                        }
                        if (value<-jaxis_threshold) {
                            joy_state[player][GN_DOWN]=1;
                            joy_state[player][GN_UP]=0;
                        }
                        if (oldvalue>jaxis_threshold && value<=jaxis_threshold && value>=-jaxis_threshold)
                            joy_state[player][GN_UP]=0;
                        if (oldvalue<-jaxis_threshold && value>=-jaxis_threshold && value<=jaxis_threshold)
                            joy_state[player][GN_DOWN]=0;
                        
                    }
                    if (map==GN_LEFT || map==GN_RIGHT) {
                        if (value>jaxis_threshold) {
                            joy_state[player][GN_RIGHT]=1;
                            joy_state[player][GN_LEFT]=0;
                        }
                        if (value<-jaxis_threshold) {
                            joy_state[player][GN_LEFT]=1;
                            joy_state[player][GN_RIGHT]=0;
                        }
                        if (oldvalue>jaxis_threshold && value<=jaxis_threshold && value>=-jaxis_threshold)
                            joy_state[player][GN_RIGHT]=0;
                        if (oldvalue<-jaxis_threshold && value>=-jaxis_threshold && value<=jaxis_threshold)
                            joy_state[player][GN_LEFT]=0;
                        
                    }
                    
                    jmap->jaxe[event.jaxis.which][event.jaxis.axis].value=value;
                    
                    
                }
                
                /*	if (abs(event.jaxis.value)>jaxis_threshold)
                 printf("SDL_JOYAXISMOTION %d %d %d %d\n",event.jaxis.which,
                 event.jaxis.axis,value,jmap->jaxe[event.jaxis.which][event.jaxis.axis].dir);
                 * */
            }
                break;
            case SDL_JOYBUTTONDOWN: 
            {
                int player=jmap->jbutton[event.jbutton.which][event.jbutton.button].player;
                int map=jmap->jbutton[event.jbutton.which][event.jbutton.button].map;
                //printf("player %d map %d\n",player,map);
                if (player) {
                    player-=1;
                    joy_state[player][map]=1;
                }
                
                //printf("SDL_JOYBUTTONDOWN %d %d\n",event.jbutton.which,event.jbutton.button);
            }
                break;
            case SDL_JOYBUTTONUP:
            {
                int player=jmap->jbutton[event.jbutton.which][event.jbutton.button].player;
                int map=jmap->jbutton[event.jbutton.which][event.jbutton.button].map;
                if (player) {
                    player-=1;
                    joy_state[player][map]=0;
                }
            }
                break;
            case SDL_VIDEORESIZE:
                conf.res_x=event.resize.w;
                conf.res_y=event.resize.h;
                screen_resize(event.resize.w, event.resize.h);
                break;
            case SDL_QUIT:
                return 1;
                break;
            default:
                break;
		}
	}
    /*
     for(i=0;i<GN_MAX_KEY;i++)
     printf("%d",joy_state[0][i]);
     printf("|");
     for(i=0;i<GN_MAX_KEY;i++)
     printf("%d",joy_state[1][i]);
     printf("\r");
     */
	/* Update coin data */
	memory.intern_coin = 0x7;
	if (joy_state[0][GN_SELECT_COIN])
		memory.intern_coin &= 0x6;
	if (joy_state[1][GN_SELECT_COIN])
		memory.intern_coin &= 0x5;
	/* Update start data TODO: Select */
	memory.intern_start = 0x8F;
	if (joy_state[0][GN_START])
		memory.intern_start &= 0xFE;
	if (joy_state[1][GN_START])
		memory.intern_start &= 0xFB;
    
    /* TURBO mode */
    if (joy_state[0][GN_TURBO]) {
        gTurboMode=1;
    } else gTurboMode=0;
    
	/* Update P1 */
	memory.intern_p1 = 0xFF;
	if ((joy_state[0][GN_UP]||joy_state[0][GN_UPLEFT]||joy_state[0][GN_UPRIGHT]) && ((!joy_state[0][GN_DOWN])||(!joy_state[0][GN_DOWNLEFT])||(!joy_state[0][GN_DOWNRIGHT])))
	    memory.intern_p1 &= 0xFE;
	if ((joy_state[0][GN_DOWN]||joy_state[0][GN_DOWNLEFT]||joy_state[0][GN_DOWNRIGHT]) && ((!joy_state[0][GN_UP])||(!joy_state[0][GN_UPLEFT])||(!joy_state[0][GN_UPRIGHT])))
	    memory.intern_p1 &= 0xFD;
	if ((joy_state[0][GN_LEFT]||joy_state[0][GN_UPLEFT]||joy_state[0][GN_DOWNLEFT]) && ((!joy_state[0][GN_RIGHT])||(!joy_state[0][GN_UPRIGHT])||(!joy_state[0][GN_DOWNRIGHT])))
	    memory.intern_p1 &= 0xFB;
	if ((joy_state[0][GN_RIGHT]||joy_state[0][GN_UPRIGHT]||joy_state[0][GN_DOWNRIGHT]) && ((!joy_state[0][GN_LEFT])||(!joy_state[0][GN_UPLEFT])||(!joy_state[0][GN_DOWNLEFT])))
	    memory.intern_p1 &= 0xF7;
	if (joy_state[0][GN_A])
	    memory.intern_p1 &= 0xEF;	// A
	if (joy_state[0][GN_B])
	    memory.intern_p1 &= 0xDF;	// B
	if (joy_state[0][GN_C])
	    memory.intern_p1 &= 0xBF;	// C
	if (joy_state[0][GN_D])
	    memory.intern_p1 &= 0x7F;	// D
    
	/* Update P1 */
	memory.intern_p2 = 0xFF;
	if (joy_state[1][GN_UP] && (!joy_state[1][GN_DOWN]))
	    memory.intern_p2 &= 0xFE;
	if (joy_state[1][GN_DOWN] && (!joy_state[1][GN_UP]))
	    memory.intern_p2 &= 0xFD;
	if (joy_state[1][GN_LEFT] && (!joy_state[1][GN_RIGHT]))
	    memory.intern_p2 &= 0xFB;
	if (joy_state[1][GN_RIGHT] && (!joy_state[1][GN_LEFT]))
	    memory.intern_p2 &= 0xF7;
	if (joy_state[1][GN_A])
	    memory.intern_p2 &= 0xEF;	// A
	if (joy_state[1][GN_B])
	    memory.intern_p2 &= 0xDF;	// B
	if (joy_state[1][GN_C])
	    memory.intern_p2 &= 0xBF;	// C
	if (joy_state[1][GN_D])
	    memory.intern_p2 &= 0x7F;	// D
    
#if defined(GP2X) || defined(WIZ)
	if (joy_state[0][GN_HOTKEY1] && joy_state[0][GN_HOTKEY2]
        && (joy_state[0][GN_START] || joy_state[0][GN_SELECT_COIN]))
		return 1;
#endif
    
	if(joy_state[0][GN_MENU_KEY]==1)
		return 1;
	else 
		return 0;
    
}

/*
 int handle_event(void) {
 return handle_event_inter(EVGAME);
 }
 */
static int last=-1;
static int counter=40;

void reset_event(void) {
	SDL_Event event;
	int i;
	for (i = 0; i < GN_MAX_KEY; i++)
		joy_state[0][i] = 0;
	while (SDL_PollEvent(&event));
	last=-1;
	counter=40;
    
    for (i=0;i<VSTICK_NB_BUTTON;i++) virtual_stick[i].finger_id=0;
    virtual_stick_padfinger=0;
    virtual_stick_pad=0;
    wm_prev_joy_pl1=wm_prev_joy_pl2=0;
    if (num_of_joys>=2) {
        if (wm_prev_joy_pl2=iOS_wiimote_check(&(joys[1]))) virtual_stick_on=0;
    }   
    if (num_of_joys>=1) {
        if (wm_prev_joy_pl1=iOS_wiimote_check(&(joys[0]))) virtual_stick_on=0;
    }
    slide_detected=0;
}

int wait_event(void) {
	SDL_Event event;
    SDL_Touch *state;
	int i,a,rx,ry;
	//static int counter;
	//static int last=-1;
	//int last=-1;
	//for(i=0;i<GN_MAX_KEY;i++)
	//	if (joy_state[0][i]) last=i;
	//SDL_WaitEvent(&event);
    
    //printf("wait event\n");
    int wm_joy_pl1,wm_joy_pl2;
    wm_joy_pl1=wm_joy_pl2=0;
    
    if (device_isIpad) {
        if (cur_width>cur_height) virtual_stick=virtual_stick_ipad_landscape;
        else virtual_stick=virtual_stick_ipad_portrait;
    } else {
        if (cur_width>cur_height) virtual_stick=virtual_stick_iphone_landscape;
        else virtual_stick=virtual_stick_iphone_portrait;
    }
    
    if (num_of_joys>=2) {
        if (wm_joy_pl2=iOS_wiimote_check(&(joys[1]))) virtual_stick_on=0;
        joy_state[1][GN_UP]=(wm_joy_pl2&WII_JOY_UP?1:0);
        joy_state[1][GN_DOWN]=(wm_joy_pl2&WII_JOY_DOWN?1:0);
        joy_state[1][GN_LEFT]=(wm_joy_pl2&WII_JOY_LEFT?1:0);
        joy_state[1][GN_RIGHT]=(wm_joy_pl2&WII_JOY_RIGHT?1:0);
        joy_state[1][GN_A]=(wm_joy_pl2&WII_JOY_A?1:0);
        joy_state[1][GN_B]=(wm_joy_pl2&WII_JOY_B?1:0);
        joy_state[1][GN_C]=(wm_joy_pl2&WII_JOY_C?1:0);
        joy_state[1][GN_D]=(wm_joy_pl2&WII_JOY_D?1:0);
        joy_state[1][GN_SELECT_COIN]=(wm_joy_pl2&WII_JOY_SELECT?1:0);
        joy_state[1][GN_START]=(wm_joy_pl2&WII_JOY_START?1:0);
        joy_state[1][GN_MENU_KEY]=(wm_joy_pl2&WII_JOY_HOME?1:0);
        joy_state[1][GN_TURBO]=(wm_joy_pl2&WII_JOY_E?1:0);
        
    }
    if (num_of_joys>=1) {        
        if (wm_joy_pl1=iOS_wiimote_check(&(joys[0]))) virtual_stick_on=0;
        
        if (wm_joy_pl1!=wm_prev_joy_pl1) {
            wm_prev_joy_pl1=wm_joy_pl1;
        joy_state[0][GN_UP]=(wm_joy_pl1&WII_JOY_UP?1:0);
        joy_state[0][GN_DOWN]=(wm_joy_pl1&WII_JOY_DOWN?1:0);
        joy_state[0][GN_LEFT]=(wm_joy_pl1&WII_JOY_LEFT?1:0);
        joy_state[0][GN_RIGHT]=(wm_joy_pl1&WII_JOY_RIGHT?1:0);
        joy_state[0][GN_A]=(wm_joy_pl1&WII_JOY_A?1:0);
        joy_state[0][GN_B]=(wm_joy_pl1&WII_JOY_B?1:0);
        joy_state[0][GN_C]=(wm_joy_pl1&WII_JOY_C?1:0);
        joy_state[0][GN_D]=(wm_joy_pl1&WII_JOY_D?1:0);
        joy_state[0][GN_SELECT_COIN]=(wm_joy_pl1&WII_JOY_SELECT?1:0);
        joy_state[0][GN_START]=(wm_joy_pl1&WII_JOY_START?1:0);
        joy_state[0][GN_MENU_KEY]=(wm_joy_pl1&WII_JOY_HOME?1:0);
        joy_state[0][GN_TURBO]=(wm_joy_pl1&WII_JOY_E?1:0);
        }
    }
    
	while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
                break;
            case SDL_MOUSEBUTTONDOWN:
                break;
            case SDL_MOUSEBUTTONUP:
                break;
            case SDL_FINGERMOTION:
                state = SDL_GetTouch(event.tfinger.touchId);
                rx = event.tfinger.x*cur_width / state->xres;
                ry = event.tfinger.y*cur_height / state->yres;
                
                //printf("delta: %d x %d\n",event.tfinger.dx*cur_width/ state->xres,event.tfinger.dy*cur_height/ state->yres);
                if ((event.tfinger.dy*100/ state->yres > SLIDEY_CHANGE_RENDERMODE_MIN)&&
                    (abs(event.tfinger.dx*100/ state->xres) < SLIDEX_CHANGE_RENDERMODE_MAX)) {
                    slide_detected=1;
                }
                
                if (event.tfinger.fingerId==virtual_stick_padfinger) { //is it the finger on pad
                    if (vstick_update_status(rx,ry)==0) virtual_stick_padfinger=0;
                    joy_state[0][GN_UP]=(virtual_stick_pad==GN_UP?1:0);
                    joy_state[0][GN_DOWN]=(virtual_stick_pad==GN_DOWN?1:0);
                    joy_state[0][GN_LEFT]=(virtual_stick_pad==GN_LEFT?1:0);
                    joy_state[0][GN_RIGHT]=(virtual_stick_pad==GN_RIGHT?1:0);
                    joy_state[0][GN_UPRIGHT]=(virtual_stick_pad==GN_UPRIGHT?1:0);
                    joy_state[0][GN_DOWNRIGHT]=(virtual_stick_pad==GN_DOWNRIGHT?1:0);
                    joy_state[0][GN_UPLEFT]=(virtual_stick_pad==GN_UPLEFT?1:0);
                    joy_state[0][GN_DOWNLEFT]=(virtual_stick_pad==GN_DOWNLEFT?1:0);
                } else if (virtual_stick_padfinger==0) {
                    if (vstick_update_status(rx,ry)) virtual_stick_padfinger=event.tfinger.fingerId;
                    joy_state[0][GN_UP]=(virtual_stick_pad==GN_UP?1:0);
                    joy_state[0][GN_DOWN]=(virtual_stick_pad==GN_DOWN?1:0);
                    joy_state[0][GN_LEFT]=(virtual_stick_pad==GN_LEFT?1:0);
                    joy_state[0][GN_RIGHT]=(virtual_stick_pad==GN_RIGHT?1:0);
                    joy_state[0][GN_UPRIGHT]=(virtual_stick_pad==GN_UPRIGHT?1:0);
                    joy_state[0][GN_DOWNRIGHT]=(virtual_stick_pad==GN_DOWNRIGHT?1:0);
                    joy_state[0][GN_UPLEFT]=(virtual_stick_pad==GN_UPLEFT?1:0);
                    joy_state[0][GN_DOWNLEFT]=(virtual_stick_pad==GN_DOWNLEFT?1:0);
                }
                
                for (int i=0;i<VSTICK_NB_BUTTON;i++) {                    
                    //is there a button already pressed with this finger ?
                    if (virtual_stick[i].finger_id==event.tfinger.fingerId) {
                        //a button was pressed and finger moved
                        //check if finger is still in button area
                        if ((rx>virtual_stick[i].x)&&(rx<virtual_stick[i].x+virtual_stick[i].w)&&
                            (ry>virtual_stick[i].y)&&(ry<virtual_stick[i].y+virtual_stick[i].h)){
                            break;
                        } else {
                            //button not pressed anymore
                            //do not break to check if finger moved to a new button
                            virtual_stick[i].finger_id=0;
                            joy_state[0][virtual_stick[i].button_id]=0;                            
                        }
                    } else {
                        //did the finger move to a new button area ?
                        if ((rx>virtual_stick[i].x)&&(rx<virtual_stick[i].x+virtual_stick[i].w)&&
                            (ry>virtual_stick[i].y)&&(ry<virtual_stick[i].y+virtual_stick[i].h)){
                            joy_state[0][virtual_stick[i].button_id]=1;
                            virtual_stick[i].finger_id=event.tfinger.fingerId;                        
                        }
                    }
                }
                
                break;
            case SDL_FINGERDOWN:
                virtual_stick_on=1;
                state = SDL_GetTouch(event.tfinger.touchId);
                rx = event.tfinger.x*cur_width / state->xres;
                ry = event.tfinger.y*cur_height / state->yres;
                
                
                if (vstick_update_status(rx,ry)) { //finger is on pad
                    joy_state[0][GN_UP]=(virtual_stick_pad==GN_UP?1:0);
                    joy_state[0][GN_DOWN]=(virtual_stick_pad==GN_DOWN?1:0);
                    joy_state[0][GN_LEFT]=(virtual_stick_pad==GN_LEFT?1:0);
                    joy_state[0][GN_RIGHT]=(virtual_stick_pad==GN_RIGHT?1:0);
                    joy_state[0][GN_UPRIGHT]=(virtual_stick_pad==GN_UPRIGHT?1:0);
                    joy_state[0][GN_DOWNRIGHT]=(virtual_stick_pad==GN_DOWNRIGHT?1:0);
                    joy_state[0][GN_UPLEFT]=(virtual_stick_pad==GN_UPLEFT?1:0);
                    joy_state[0][GN_DOWNLEFT]=(virtual_stick_pad==GN_DOWNLEFT?1:0);
                    virtual_stick_padfinger=event.tfinger.fingerId;
                } else { //check if finger is on a button
                    for (int i=0;i<VSTICK_NB_BUTTON;i++) {
                        if ((rx>virtual_stick[i].x)&&(rx<virtual_stick[i].x+virtual_stick[i].w)&&
                            (ry>virtual_stick[i].y)&&(ry<virtual_stick[i].y+virtual_stick[i].h)){
                            joy_state[0][virtual_stick[i].button_id]=1;
                            virtual_stick[i].finger_id=event.tfinger.fingerId;
                            break;
                        }
                    }       
                }
                break;
            case SDL_FINGERUP:
                if (virtual_stick_padfinger==event.tfinger.fingerId) {
                    virtual_stick_pad=0;                    
                    joy_state[0][GN_UP]=0;
                    joy_state[0][GN_DOWN]=0;
                    joy_state[0][GN_LEFT]=0;
                    joy_state[0][GN_RIGHT]=0;
                    joy_state[0][GN_UPRIGHT]=0;
                    joy_state[0][GN_DOWNRIGHT]=0;
                    joy_state[0][GN_UPLEFT]=0;
                    joy_state[0][GN_DOWNLEFT]=0;
                }
                    
                    for (int i=0;i<VSTICK_NB_BUTTON;i++) 
                        if (virtual_stick[i].finger_id==event.tfinger.fingerId) {
                            virtual_stick[i].finger_id=0;
                            joy_state[0][virtual_stick[i].button_id]=0;
                            break;
                        }
                
                if (slide_detected) {
                    slide_detected=0;
                    conf.rendermode++;
                    if (conf.rendermode>3) conf.rendermode=0;
                    
                }
                last=-1;
                counter=40;
                break;
            case SDL_KEYDOWN:
                virtual_stick_on=0;
                /* Some default keyboard standard key */
                switch (event.key.keysym.sym) {
                    case SDLK_TAB:
                        joy_state[0][GN_MENU_KEY]=1;
                        //last=GN_MENU_KEY;
                        //return GN_MENU_KEY;
                        break;	
                    case SDLK_UP:
                        joy_state[0][GN_UP]=1;
                        //last=GN_UP;
                        //return GN_UP;
                        break;	
                    case SDLK_DOWN:
                        joy_state[0][GN_DOWN]=1;
                        //last=GN_DOWN;
                        //return GN_DOWN;
                        break;	
                    case SDLK_LEFT:
                        joy_state[0][GN_LEFT]=1;
                        //last=GN_LEFT;
                        //return GN_LEFT;
                        break;	
                    case SDLK_RIGHT:
                        joy_state[0][GN_RIGHT]=1;
                        //last=GN_RIGHT;
                        //return GN_RIGHT;
                        break;	
                    case SDLK_ESCAPE:
                        joy_state[0][GN_B]=1;
                        //last=GN_A;
                        //return GN_A;
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        joy_state[0][GN_A]=1;
                        //last=GN_B;
                        //return GN_B;
                        break;
                    default:
                        SDL_PushEvent(&event);
                        handle_event();
                        break;
                }
                break;
            case SDL_KEYUP:
                //printf("KEYUPPPPP!!!\n");
                
                for(i=0;i<GN_MAX_KEY;i++)
                    joy_state[0][i]=0;
                last=-1;
                counter=40;
                break;
            default:
                SDL_PushEvent(&event);
				handle_event();
				/* Simulate keyup */
				a=0;
				for (i = 0; i < GN_MAX_KEY; i++)
					if (joy_state[0][i]) a++;
				if (a!=1) {
					for (i = 0; i < GN_MAX_KEY; i++)
                        joy_state[0][i] = 0;
                    last = -1;
                    counter = 40;
				}
                break;
        }
	}
    /*
     }
     SDL_PushEvent(&event);
     handle_event();
	 */
    
	if (last!=-1) {
		if (counter>0)
			counter--;
		if (counter==0) {
			counter=5;
			return last;
		}
	} else {
		for(i=0;i<GN_MAX_KEY;i++)
			if (joy_state[0][i]) {
				last=i;
				return i;
			}
	}
    /*
     for(i=0;i<GN_MAX_KEY;i++)
     if (joy_state[0][i] ) {
     if (i != last) {
     counter=30;
     last=i;
     return i;
     } else {
     counter--;
     if (counter==0) {
     counter=5;
     return i;
     }
     
     }
     
     
     }
     */
	return 0;
}
