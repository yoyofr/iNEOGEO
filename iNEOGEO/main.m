/*
 *	rectangles.c
 *	written by Holmes Futrell
 *	use however you want
 */

#include "SDL.h"
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <UIKit/UIDevice.h>

int device_w,device_h,device_isSlow,device_isIpad;
int cur_width,cur_height;
extern int gnmain(int argc, char *argv[]);

//******************************************************************
// get_device_type
// return 1 if slow device (anything slower than 3GS)
//******************************************************************
int get_device_type() {
    size_t size;
    int device_type;
    // Set 'oldp' parameter to NULL to get the size of the data
    // returned so we can allocate appropriate amount of space
    sysctlbyname("hw.machine", NULL, &size, NULL, 0); 

    // Allocate the space to store name
    char *name = (char*)malloc(size);

    // Get the platform name
    sysctlbyname("hw.machine", name, &size, NULL, 0);
    
    /* iPhone Simulator == i386
	 iPhone == iPhone1,1             //Slow
	 3G iPhone == iPhone1,2          //Slow
	 3GS iPhone == iPhone2,1
	 4 iPhone == iPhone3,1
	 1st Gen iPod == iPod1,1         //Slow
	 2nd Gen iPod == iPod2,1
	 3rd Gen iPod == iPod3,1
	 */
	
	device_type=0;
    if (strstr(name,"iPhone1,")) device_type=1;
    if (strstr(name,"iPod1,")) device_type=1;

            
    free(name);
    return device_type;
}

        //******************************************************************
        // get_device_res
        // get current resolution
        //******************************************************************
void get_device_res() {
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
		device_isIpad=1; //ipad
		device_w=1024;
		device_h=768;
	}
	else {		
        device_isIpad=0; //iphone   (iphone 4 res currently not handled)
        device_w=480;
		device_h=320;
	}
}


int main(int argc, char *argv[])
{
#if 1
    char *gnargv[6]={"gngeo","--blitter=opengl","-f","--bench","viewpoin","--showfps"};
    
/*    iCadeRView=[[iCadeReaderView alloc] initWithFrame:CGRectZero];
    UIWindow *window=[[UIApplication sharedApplication] keyWindow];		
	[window addSubview:iCadeRView];    
    iCadeRView.active = YES;
  */  
    
    get_device_res();
    device_isSlow=get_device_type();
    
    gnmain(3,gnargv);
    /* shutdown SDL */
    SDL_Quit();
    exit(0);
#else
    
    SDL_Window *window;
	SDL_Renderer *renderer;
    int done;
    SDL_Event event;
    
    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
//        fatalError("Could not initialize SDL");
        exit(1);
    }
    
    /* seed random number generator */
    srand(time(NULL));
    
    /* create window and renderer */
    window =
    SDL_CreateWindow(NULL, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                     SDL_WINDOW_SHOWN);
    if (window == 0) {
//        fatalError("Could not initialize Window");
                exit(2);
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
//        fatalError("Could not create renderer");
                exit(3);
    }
    
    /* Fill screen with black */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    /* Enter render loop, waiting for user to quit */
    done = 0;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            }
        }
        render(renderer);
        SDL_Delay(1);
    }
    
    /* shutdown SDL */
    SDL_Quit();
#endif
    return 0;
}



#if TARGET_IPHONE_SIMULATOR

#include "btstack.h"
#include "wiimote.h"

int bt_send_cmd(const hci_cmd_t *cmd, ...) {
    va_list pvar;
	va_start(pvar, cmd);

    return 0;
}
void bt_send_l2cap(uint16_t local_cid, uint8_t *data, uint16_t len) {
    
}

// init BTstack library
int bt_open(void){
    return 0;
}
// stop using BTstack library
int bt_close(void) {
    return 0;
}

void run_loop_init(RUN_LOOP_TYPE type) {
    
}
void bt_flip_addr(bd_addr_t dest, bd_addr_t src) {
    
}

btstack_packet_handler_t bt_register_packet_handler(btstack_packet_handler_t handler) {
    return handler;
}

const hci_cmd_t btstack_set_power_mode;
const hci_cmd_t btstack_set_system_bluetooth_enabled;
const hci_cmd_t btstack_get_system_bluetooth_enabled;
const hci_cmd_t hci_inquiry;
const hci_cmd_t hci_remote_name_request;
const hci_cmd_t hci_remote_name_request_cancel;
const hci_cmd_t hci_inquiry_cancel;
const hci_cmd_t hci_delete_stored_link_key;
const hci_cmd_t hci_write_inquiry_mode;
const hci_cmd_t l2cap_create_channel;

#endif