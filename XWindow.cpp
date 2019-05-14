
// This program creates a basic X window application using Xlib.
#include <iostream> // preparedness for C++ later (not used in this program)
#include <stdio.h> // printf()
#include <stdlib.h> // exit()
#include <memory.h> // memset()

#include <X11/Xlib.h> // for native windowing
#include <X11/Xutil.h> // for visual and related api
#include <X11/XKBlib.h> // for XkbKeycodeToKeysym()
#include <X11/keysym.h> // for keyboard symbols

//namespaces
using namespace std; //preparedness for C++ later (not used in this program)

Display *gpDisplay = NULL; // 'Display' is a struct which will be used for communication with X-server. (This var will be mapped with the instance of Display on the server).

XVisualInfo *gpXVisualInfo = NULL; // XVisualInfo is like a 'Device Context' i.e. it is a struct which contains info about the drawing attributes of a device such as a display.

Colormap gColormap; // Color palette
Window gWindow;
bool gbFullscreen=false;
int giWindowWidth = 800;
int giWindowHeight = 600;

// entry point function
int main(void) // Note:- command line parameters can be used to give i/p address of the remote client
{
	// function prototypes
	void createWindow(void);
	void toggleFullscreen(void);
	void uninitialize();

	// variable declarations
	int winWidth = giWindowWidth;
	int winHeight = giWindowHeight;

	// code
	createWindow();

	// Message Loop
	XEvent event;
	KeySym keysym;

	while(1){ // Infinite loop
	
		XNextEvent(gpDisplay, &event); // XNextEvent() - Copies the first event from the event queue to the 'event' variable and then removes it from the event queue.
		
		switch(event.type){

			case MapNotify: // similar to WM_CREATE
			 	break;
			case KeyPress: // similar to WM_KEYDOWN - Keyboard button press
				keysym = XkbKeycodeToKeysym (gpDisplay, event.xkey.keycode, 0, 0); // 3rd param - locale (0=default 'en'), 					4th param - shift key status (0-shift not used).
			
				switch(keysym){
					case XK_Escape: // XK - X Keycode symbol
						uninitialize();
						exit(0); // successful exit status. Hence, 0
					case XK_F:
					case XK_f:
						if(gbFullscreen==false){
							toggleFullscreen();
							gbFullscreen=true;
						}
						else{
							toggleFullscreen();
							gbFullscreen=false;
						}
					break;
			
				}
				break;

			case MotionNotify: // WM_MOUSEMOVE
				break;

			case ConfigureNotify: // WM_RESIZE
				winWidth = event.xconfigure.width;
				winHeight = event.xconfigure.height;
				break;
			
			case Expose: // WM_PAINT
				break;
			
			case DestroyNotify: // WM_DESTROY
				break;

			case 33: // handles click on 'close' box || sys menu 'close'
				uninitialize(); // kill the window
				exit(0); // kill the process
			default:
				break;
		}
		

	}

	return (0); // This statement is never executed. The program exits from case 33 (written above).
}

void createWindow(void){
	
	// function prototypes
	void uninitialize(void);

	// variable declarations
	XSetWindowAttributes winAttribs;
	int styleMask;
	int defaultScreen;
	int defaultDepth;
	// Step 1: Make connection request to the X-server for further communication (i.e. get instance of 'Display' struct).
	gpDisplay = XOpenDisplay(NULL); // NULL - default connection of the local display
	if(gpDisplay==NULL){

		printf("ERROR: Unable to open X display.\nExiting now...\n");
		uninitialize();
		exit(1); // Abortive exit and hence a positive value is passed to exit().
	}
	
	// Step 2: Get the default screen (from 'Display') to which the Graphic card is displayed
	defaultScreen=XDefaultScreen(gpDisplay);

	// Step 3: Get the default bit depth of the root window of the defaultScreen.
	defaultDepth = XDefaultDepth(gpDisplay, defaultScreen);

	// Step 4: Allocate memory to XVisualInfo
	gpXVisualInfo = (XVisualInfo *) malloc (sizeof(XVisualInfo));
	if(gpXVisualInfo == NULL){
		printf("ERROR: Unable to allocate memory for the visual.\nExiting now...");
		uninitialize();
		exit(1);
	}	

	// Step 5: Get VisualInfo struct that best matches the defaultDepth and class (i.e. TrueColor)
	if(XMatchVisualInfo(gpDisplay, defaultScreen, defaultDepth, TrueColor, gpXVisualInfo)==0){
		
		printf("ERROR: Unable to get a visual.\nExiting now...");
		uninitialize();
		exit(1);
	}

	// Step 6: Set the attributes of the window
	winAttribs.border_pixel=0; // 0 - default border color
	winAttribs.background_pixmap = 0; // 0 - default imgs like cursor, icon, etc.
	// set the colormap for this window
	winAttribs.colormap = XCreateColormap(gpDisplay,
			      RootWindow(gpDisplay, gpXVisualInfo->screen), // Give me colormap of the root window
			      gpXVisualInfo->visual, 
	 	      AllocNone // don't allocate fixed memory	
			     );
	gColormap = winAttribs.colormap;
	
	winAttribs.background_pixel=XBlackPixel(gpDisplay, defaultScreen);
	// Specify the window events of our interest 
	winAttribs.event_mask = ExposureMask | // similar to WM_PAINT	
			 	VisibilityChangeMask | // similar to WM_CREATE
				ButtonPressMask | // for mouse button related events
				KeyPressMask | // for keyboard key press related events
				PointerMotionMask | // for mouse pointer related events
				StructureNotifyMask; // for 'ConfigureNotify' similar to WM_SIZE
	// set window style
	styleMask=CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

	// Step 9: Create the window
	gWindow=XCreateWindow(gpDisplay,
			      RootWindow(gpDisplay,gpXVisualInfo->screen),
			      0, // x
			      0, // y
			      giWindowWidth, // width
			      giWindowHeight, // height
			      0, // border width
				gpXVisualInfo->depth, // depth
				InputOutput, // type of window
				gpXVisualInfo->visual,
				styleMask,
				&winAttribs);
	if(!gWindow){
		printf("ERROR: Failed to create main window.\nExiting now...!");
		uninitialize();
		exit(1);
	}

	// Step 10: Name in the caption bar
	XStoreName(gpDisplay, gWindow, "First XWindow");

	// Step 11: Handle the close event. 
	// Note:- 'Close' events are handled by the window manager. In order to do it, perform the following steps:-
	// Step 11.1: Create an atom for the delete event.
	Atom windowManagerDelete = XInternAtom(gpDisplay,"WM_DELETE_WINDOW", True); // XInternAtom() returns the atom id associated with "WM_DELETE_WINDOW" atom string. True - create an atom (even if it exists).
	
	// Step 11.2: Add the atom created above as a window manager protocol.
	XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1); // 3rd param - array of atoms, 4th param - num of protocols to set

	// Step 12: Map this window to the screen
	XMapWindow(gpDisplay, gWindow);
}

void toggleFullscreen(void){

	//variable declarations
	Atom wm_state;
	Atom fullscreen;
	XEvent xev = {0};

	// code
	// Step 1: Get the current placement of the window (save the current state of the window in wm_state)
	wm_state = XInternAtom(gpDisplay,"_NET_WM_STATE", False); // Get atom id of the current state of the window. False - don't create this atom if it already exists
		
	// Step 2: Create a custom event / message
	// Step 2.1: Allocate 0 memory to the xevent variable
	memset(&xev,0,sizeof(xev));

	// Step 2.2: Set the values to the custom event / message
	xev.type = ClientMessage; // Since this is a custom message
	xev.xclient.window = gWindow;
	xev.xclient.message_type = wm_state; // message_type - atom created above
	xev.xclient.format=32; // 32-bit unsigned int
	xev.xclient.data.l[0]=gbFullscreen ? 0 : 1; // We are instructing the program to check what is the value in l[0] and then perform the corresponding action from l[1].

	fullscreen = XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN", False); // Get atom id of the fullscreen	atom. False, do not create if it already exists.

	xev.xclient.data.l[1]=fullscreen; // Add the fullscreen atom created above to l[1].

	// Step 3: Send the event created above to the event queue
	XSendEvent(gpDisplay, 
			RootWindow(gpDisplay, gpXVisualInfo->screen), // Propagate the event/message to me
			False, // Do not propagate the event/message to my child window
			StructureNotifyMask,
			&xev); // custom-event created above			
}

void uninitialize(){ // This func works like a destructor. i.e. in reverse order of creation
	
	if(gWindow){
		
		XDestroyWindow(gpDisplay, gWindow);
	}	
	
	if(gColormap){

		XFreeColormap(gpDisplay, gColormap);
	}

	if(gpXVisualInfo){

		free(gpXVisualInfo);
		gpXVisualInfo=NULL;
	}

	if(gpDisplay){

		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;

	}
}
