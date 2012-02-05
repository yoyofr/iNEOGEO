#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_GL_GL_H
#ifndef WII
#ifndef GP2X



#include "SDL.h"
#include "../emu.h"
#include "../screen.h"
#include "../video.h"
#include "../effect.h"
#include "../conf.h"
#include "../event.h"

//#include "glproc.h"

#include <OpenGLES/ES1/glext.h>
#include <OpenGLES/ES2/glext.h>


//#define TEXMIN256

static GLfloat vertices[4][2];  /* Holds Float Info For 4 Sets Of Vertices */
static GLfloat texcoords[4][2]; /* Holds Float Info For 4 Sets Of Texture coordinates. */
static GLuint txtnumber;


static float a;
static float b;
static float c;
static float d;

//static SDL_Surface *video_opengl;
static SDL_Window *video_opengl;
static SDL_GLContext ctx;
static SDL_Surface *tex_opengl;
static SDL_Rect glrectef;

extern int device_w,device_h;
extern int cur_width,cur_height;
extern t_touch_area *virtual_stick;
extern int virtual_stick_on,virtual_stick_pad;
extern Uint8 virtual_stick_buttons_alpha,virtual_stick_buttons_alpha2;
extern int virtual_stick_posx,virtual_stick_posy,virtual_stick_mindist,virtual_stick_maxdist;
extern int virtual_stick_mindist2,virtual_stick_maxdist2;

SDL_bool
blitter_opengl_init()
{
	Uint32 sdl_flags;
	Uint32 width = device_w;
	Uint32 height = device_h;
	
    //        if (load_glproc() == SDL_FALSE) return SDL_FALSE;
    
	sdl_flags = (fullscreen?SDL_FULLSCREEN:0)| SDL_DOUBLEBUF | SDL_HWSURFACE
    | SDL_HWPALETTE | SDL_OPENGL | SDL_RESIZABLE;
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    
	if ((effect[neffect].x_ratio!=2 || effect[neffect].y_ratio!=2) &&  
	    (effect[neffect].x_ratio!=1 || effect[neffect].y_ratio!=1) ) {
	    printf("Opengl support only effect with a ratio of 2x2 or 1x1\n");
	    return SDL_FALSE;
	}
    
	/*
     if (conf.res_x==304 && conf.res_y==224) {
     */
	if (scale < 2) {
	    width *=effect[neffect].x_ratio;
	    height*=effect[neffect].y_ratio;
	}
	width *= scale;
	height *= scale;
    
	//printf("%d %d %d %d %d\n",width,height,scale,visible_area.w,visible_area.h);
    /*
     } else {
     width = conf.res_x;
     height=conf.res_y;
     }
     
     */
	conf.res_x=width;
	conf.res_y=height;
	
    //	video_opengl = SDL_SetVideoMode(width, height, 16, sdl_flags);
    video_opengl = SDL_CreateWindow(0, 0, 0, width, height,SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	
	if ( video_opengl == NULL)
		return SDL_FALSE;
    
    ctx = SDL_GL_CreateContext(video_opengl);
    
    SDL_GetWindowSize(video_opengl, &width, &height);
    cur_width=width;
    cur_height=height;    
    
    virtual_stick_posx = virtual_stick_maxdist;
    virtual_stick_posy = cur_height-virtual_stick_maxdist;
    virtual_stick_maxdist2=virtual_stick_maxdist*virtual_stick_maxdist;
    virtual_stick_mindist2=virtual_stick_mindist*virtual_stick_mindist;
    
	
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glEnable(GL_TEXTURE_2D);
	glViewport(0, 0, width, height);
    
    glGenTextures(1, &txtnumber);               /* Create 1 Texture */	
    glBindTexture(GL_TEXTURE_2D, txtnumber);    /* Bind The Texture */
	/* Linear Filter */
	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	/*
     pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     */
	/* Texture Mode */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//GL_CLAMP);
	
    
	if (neffect == 0)
	{
		/* Texture limits */
        /*
         a = (240.0/304.0);
         b = (48.0/256.0);
         
         c = (240.0/256.0);
         */
	    tex_opengl= SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 256, 16, 0xF800, 0x7E0, 0x1F, 0);
        
	}
	else {		
	    /* Texture limits */
	    a = ((256.0/(float)visible_area.w) - 1.0f)*effect[neffect].x_ratio/2.0;
	    b = ((512.0/(float)visible_area.w) - 1.0f)*effect[neffect].x_ratio/2.0;
	    c = (((float)visible_area.h/256.0))*effect[neffect].y_ratio/2.0;
	    d = (((float)((visible_area.w<<1)-512)/256.0))*effect[neffect].y_ratio/2.0;
	    screen = SDL_CreateRGBSurface(SDL_SWSURFACE, visible_area.w<<1,  /*visible_area.h<<1*/512, 16, 0xF800, 0x7E0, 0x1F, 0);
	    //printf("[opengl] create_screen %p\n",screen);
	    tex_opengl= SDL_CreateRGBSurface(SDL_SWSURFACE, 1024, 512, 16, 0xF800, 0x7E0, 0x1F, 0);
	    if (visible_area.w==320) {
            glrectef.x=0;
            glrectef.y=0;
            glrectef.w=320*2;
            glrectef.h=224*2;
	    } else {
            glrectef.x=0;
            glrectef.y=0;
            glrectef.w=304*2;
            glrectef.h=224*2;
	    }
	}
    
    glBindTexture(GL_TEXTURE_2D, 0);    /* Bind The Texture */
	
	return SDL_TRUE;
}

SDL_bool
blitter_opengl_resize(int w,int h)
{
    Uint32 sdl_flags;
    
    sdl_flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_OPENGL | SDL_RESIZABLE;
    
    /*video_opengl = SDL_SetVideoMode(w, h, 16, sdl_flags);
     
     if ( video_opengl == NULL)
     return SDL_FALSE;
     
     glEnable(GL_TEXTURE_2D);
     glViewport(0, 0, w, h);*/
    
    return SDL_TRUE;
}

void 
blitter_opengl_update() {
    int width,height,rwidth,rheight;
    float ratio;
    if (neffect == 0) {
        SDL_BlitSurface(buffer, &visible_area, tex_opengl, NULL);
        
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        /*        for (int i=0;i<320;i++)
         for (int j=0;j<100;j++) ((unsigned short*)(tex_opengl->pixels))[i*(tex_opengl->pitch/2)+j]=i*i+j*j;
         */      
        glBindTexture(GL_TEXTURE_2D, txtnumber);    /* Bind The Texture */
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tex_opengl->pixels);
        
        /* Begin Drawing Quads, setup vertex and texcoord array pointers */
        glVertexPointer(2, GL_FLOAT, 0, vertices);
        glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
        
        /* Enable Vertex Pointer */
        glEnableClientState(GL_VERTEX_ARRAY);
        /* Enable Texture Coordinations Pointer */
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        texcoords[0][0]=(float)0/512.0f; texcoords[0][1]=(float)0/256.0f;
        texcoords[1][0]=(float)(visible_area.w)/512.0f; texcoords[1][1]=(float)0/256.0f;
        texcoords[2][0]=(float)0/512.0f; texcoords[2][1]=(float)(visible_area.h+visible_area.y)/256.0f;
        texcoords[3][0]=(float)(visible_area.w)/512.0f; texcoords[3][1]=(float)(visible_area.h+visible_area.y)/256.0f;
        
        
        SDL_GetWindowSize(video_opengl, &width, &height);
        cur_width=width;
        cur_height=height;
        
        virtual_stick_posx = virtual_stick_maxdist;
        virtual_stick_posy = cur_height-virtual_stick_maxdist;
        virtual_stick_maxdist2=virtual_stick_maxdist*virtual_stick_maxdist;
        virtual_stick_mindist2=virtual_stick_mindist*virtual_stick_mindist;
        
        
        switch (conf.rendermode) {
            case 0:
                ratio=(float)width/(float)height;
                if (ratio>4/3) {
                    rwidth=height*4/3;
                    rheight=height;
                } else {
                    rwidth=width;
                    rheight=width*3/4;
                }        
                break;
            case 1:
                rwidth=visible_area.w+visible_area.x;
                rheight=visible_area.h+visible_area.y;
                break;
            case 2:
                if (width>height) {
                    rwidth=width;
                    rheight=height;
                } else {
                    rwidth=width;
                    rheight=width*3/4;
                }
                break;
        }
        
        
        //update viewport to match current neogeo video res
        glViewport((width-rwidth)/2, height-rheight, rwidth, rheight);
        
        glColor4ub(255,255,255,255);
        
        vertices[0][0]=-1; vertices[0][1]=1;
        vertices[1][0]=1; vertices[1][1]=1;
        vertices[2][0]=-1; vertices[2][1]=-1;
        vertices[3][0]=1; vertices[3][1]=-1;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        /* Disable Vertex Pointer */
        glDisableClientState(GL_VERTEX_ARRAY);
        /* Disable Texture Coordinations Pointer */
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        glBindTexture(GL_TEXTURE_2D, 0);    /* Bind The Texture */
        
        if (virtual_stick_on) {
            //update viewport to match real device screen
            
            glViewport(0, 0, width, height);
            
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);            
            /* Enable Vertex Pointer */
            glEnableClientState(GL_VERTEX_ARRAY);        
            
            
            for (int i=0;i<VSTICK_NB_BUTTON;i++) {
                vertices[0][0]=(float)(virtual_stick[i].x)/cur_width;
                vertices[0][1]=(float)(virtual_stick[i].y)/cur_height;
                
                vertices[1][0]=vertices[0][0]+(float)(virtual_stick[i].w)/cur_width;
                vertices[1][1]=(float)(virtual_stick[i].y)/cur_height;
                
                vertices[2][0]=(float)(virtual_stick[i].x)/cur_width;
                vertices[2][1]=vertices[0][1]+(float)(virtual_stick[i].h)/cur_height;
                
                vertices[3][0]=vertices[0][0]+(float)(virtual_stick[i].w)/cur_width;
                vertices[3][1]=vertices[0][1]+(float)(virtual_stick[i].h)/cur_height;
                
                vertices[0][0]=vertices[0][0]*2-1;
                vertices[1][0]=vertices[1][0]*2-1;
                vertices[2][0]=vertices[2][0]*2-1;
                vertices[3][0]=vertices[3][0]*2-1;
                vertices[0][1]=-vertices[0][1]*2+1;
                vertices[1][1]=-vertices[1][1]*2+1;
                vertices[2][1]=-vertices[2][1]*2+1;
                vertices[3][1]=-vertices[3][1]*2+1;
                
                if (virtual_stick[i].finger_id) glColor4ub(virtual_stick[i].r,virtual_stick[i].g,virtual_stick[i].b,virtual_stick_buttons_alpha2);
                else glColor4ub(virtual_stick[i].r,virtual_stick[i].g,virtual_stick[i].b,virtual_stick_buttons_alpha);
                
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
            
            
            //now the stick
            for (int i=0;i<8;i++) {
            vertices[0][0]=(float)(virtual_stick_posx+0.9f*virtual_stick_maxdist*cosf(i*M_PI/4))/cur_width;
            vertices[0][1]=(float)(virtual_stick_posy-0.9f*virtual_stick_maxdist*sinf(i*M_PI/4))/cur_height;
            
            vertices[1][0]=(float)(virtual_stick_posx+0.6f*virtual_stick_maxdist*cosf(i*M_PI/4+M_PI/16))/cur_width;
            vertices[1][1]=(float)(virtual_stick_posy-0.6f*virtual_stick_maxdist*sinf(i*M_PI/4+M_PI/16))/cur_height;
            
            vertices[2][0]=(float)(virtual_stick_posx+0.6f*virtual_stick_maxdist*cosf(i*M_PI/4-M_PI/16))/cur_width;
            vertices[2][1]=(float)(virtual_stick_posy-0.6f*virtual_stick_maxdist*sinf(i*M_PI/4-M_PI/16))/cur_height;
            
            vertices[0][0]=vertices[0][0]*2-1;
            vertices[1][0]=vertices[1][0]*2-1;
            vertices[2][0]=vertices[2][0]*2-1;
            vertices[0][1]=-vertices[0][1]*2+1;
            vertices[1][1]=-vertices[1][1]*2+1;
            vertices[2][1]=-vertices[2][1]*2+1;
            
                
                if (virtual_stick_pad==i+1) glColor4ub(240,220,255,virtual_stick_buttons_alpha2);
                else glColor4ub(240,220,255,virtual_stick_buttons_alpha);
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
            }
            glDisable(GL_BLEND);
            glEnable(GL_TEXTURE_2D);
        }
    } 
	
    SDL_GL_SwapWindow(video_opengl);
}

void
blitter_opengl_close() {
	//if (screen != NULL)
	//	SDL_FreeSurface(screen);
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(video_opengl);
}

void
blitter_opengl_fullscreen() {
    //	SDL_WM_ToggleFullScreen(video_opengl);
}

#endif
#endif
#endif
