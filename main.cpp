/*=============================================*\
|	PROJECT:	XYG Studio Runtime Environment	|
|	AUTHOR:		Nick Kovacs						|
|	DATE:		8-15-15							|
|	DESCRIPTION:Runtime environment used for	|
|		games and applications created using	|
|		the XYG Studio framework.				|
|	LICENSE:	You are free to use, modify, and|
|		redistribute this source code, in part,	|
|		or in full, provided proper attribution	|
|		is included and this information is not	|
|		modified or removed. Please include a	|
|		link to WWW.XYGSTUDIO.ORG in all		|
|		projects that use this source code.		|
\*=============================================*/

/*===========*\
| MAIN SOURCE |
\*===========*/

//Main header
#include "main.h"
#include "global.h"
#include "input.h"
#include "graphics.h"
#include "shapes.h"
#include "maths.h"
#include "fileio.h"
#include "binds.h"


/////////////////
//MAIN FUNCTION//
/////////////////

int main(int argc, char* args[]){
	//Initiate everything
	if(xyInit() == 0){
		xyPrint(0, "Failed to initiate XYG!");
		xyEnd();
		return 1;
	};

	//Load library
	sqstd_dofile(gvSquirrel, "libxyg.nut", 0, 0);

	//Run test app (until a better format is made)
	sqstd_dofile(gvSquirrel, "test.nut", 0, 0);

	//End game
	xyEnd();

	return 0;
};

//////////////
//EXCEPTIONS//
//////////////



///////////////////
//OTHER FUNCTIONS//
///////////////////

int xyInit(){
	//Initiate log file
	remove("log.txt");
	gvLog = fopen("log.txt", "w");
	xyPrint(0, "Initializing program...\n\n");

	//Initiate SDL
	if(SDL_Init(SDL_INIT_EVERYTHING < 0)){
		xyPrint(0, "Failed to initialize! %s", SDL_GetError());
		return 0;
	};

	//Create window
	gvWindow = SDL_CreateWindow("XYG Runtime Environment", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gvScrW, gvScrH, SDL_WINDOW_RESIZABLE);
	if(gvWindow == 0){
		xyPrint(0, "Window could not be created! SDL Error: %s\n", SDL_GetError());
		return 0;
	} else {
		//Create renderer for window
		gvRender = SDL_CreateRenderer(gvWindow, -1, SDL_RENDERER_ACCELERATED);
		if(gvRender == 0){
			xyPrint(0, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
			return 0;
		} else {
			//Initialize renderer color
			SDL_SetRenderDrawColor(gvRender, 0xFF, 0xFF, 0xFF, 0xFF);

			//Initialize PNG loading
			if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
				xyPrint(0, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
				return 0;
			};

			//Set up the viewport
			SDL_Rect screensize;
			screensize.x = 0;
			screensize.y = 0;
			screensize.w = gvScrW;
			screensize.h = gvScrH;
			SDL_RenderSetViewport(gvRender, &screensize);
			SDL_RenderSetLogicalSize(gvRender, gvScrW, gvScrH);

		};
	};

	xyInitInput();

	xyPrint(0, "SDL initialized successfully!");

	//Initiate Squirrel
	gvSquirrel = sq_open(1024);

	sq_setprintfunc(gvSquirrel, xyPrint, xyError);
	sq_pushroottable(gvSquirrel);

	xyBindAllFunctions(gvSquirrel);

	/*Error handler does not seem to print compile-time errors. I haven't
	been able to figure out why, as the same code works in my other apps,
	and is taken from the sq.c example included with Squirrel.*/
	sqstd_seterrorhandlers(gvSquirrel);

	xyPrint(0, "Squirrel initialized successfully!");

	//Initiate other
	vcTextures.push_back(0);
	vcSprites.push_back(0);

	//Return success
	return 1;
};

void xyError(HSQUIRRELVM v, const SQChar *s, ...){
	va_list args;
	va_start(args, s);
	fprintf(gvLog, "!ERROR! >:: ");
	vfprintf(gvLog, s, args);
	printf("!ERROR! >:: ");
	vprintf(s, args);
	printf("\n\n");
	fprintf(gvLog, "\n\n");
	va_end(args);
};

void xyEnd(){
	//Cleanup all resources
	xyPrint(0, "Cleaning up all resources...");
	for(int i = 0; i < vcTextures.size(); i++){
		xyDeleteImage(i);
	};

	for(int i = 0; i < vcSprites.size(); i++){
		delete vcSprites[i];
	};

	//Close Squirrel
	xyPrint(0, "Closing Squirrel...");
	int garbage = sq_collectgarbage(gvSquirrel);
	xyPrint(0, "Collected %i junk obects.", garbage);
	sq_pop(gvSquirrel, 1);
	sq_close(gvSquirrel);

	//Close SDL
	xyPrint(0, "Closing SDL...");
	SDL_DestroyRenderer(gvRender);
	SDL_DestroyWindow(gvWindow);
	IMG_Quit();
	SDL_Quit();

	//Close log file
	xyPrint(0, "System closed successfully!");
	fclose(gvLog);
};

void xyPrint(HSQUIRRELVM v, const SQChar *s, ...){
	va_list args;
	va_start(args, s);
	fprintf(gvLog, ">:: ");
	vfprintf(gvLog, s, args);
	printf(">:: ");
	vprintf(s, args);
	printf("\n\n");
	fprintf(gvLog, "\n\n");
	va_end(args);
};

void xyUpdateTime(){
	//Variables
	static Uint32 lastTicks = 0;
	Uint32 newTicks = SDL_GetTicks();

	//Set ticks
	if(newTicks >= lastTicks){
		gvTicks += newTicks - lastTicks;
	} else {
		lastTicks = newTicks;
	};

	//Update timers
	if(gvTicks >= 1000){
		gvSeconds += (gvTicks - gvTicks % 1000) / 1000;
		gvTicks -= gvTicks - gvTicks % 1000;
	};
	if(gvSeconds >= 60){
		gvMinutes += (gvSeconds - gvSeconds % 60) / 60;
		gvSeconds -= gvSeconds - gvSeconds % 60;
	};
	if(gvMinutes >= 60){
		gvHours += (gvMinutes - gvMinutes % 60) / 60;
		gvMinutes -= gvMinutes - gvMinutes % 60;
	};
};

void xyBindFunc(HSQUIRRELVM v, SQFUNCTION func, const SQChar *key){
	sq_pushroottable(v);
	sq_pushstring(v, key, -1);
	sq_newclosure(v, func, 0);
	sq_newslot(v, -3, false);
	sq_pop(v, 1);
};

void xyBindFunc(HSQUIRRELVM v, SQFUNCTION func, const SQChar *key, SQInteger nParams, const SQChar* sParams){
	sq_pushroottable(v);
	sq_pushstring(v, key, -1);
	sq_newclosure(v, func, 0);
	sq_setparamscheck(v, nParams, sParams);
	sq_newslot(v, -3, false);
	sq_pop(v, 1);
};

void xyBindAllFunctions(HSQUIRRELVM v){
	//Main
	xyPrint(0, "Embedding main...");
	xyBindFunc(v, sqUpdate, "xyUpdate");
	xyBindFunc(v, sqGetOS, "xyOS");

	//Graphics
	xyPrint(0, "Embedding graphics...");
	xyBindFunc(v, sqWait, "xyWait", 2, ".n");
	xyBindFunc(v, sqSetDrawTarget, "xySetDrawTarget", 2, ".n");
	xyBindFunc(v, sqClearScreen, "xyClearScreen");
	xyBindFunc(v, sqResetDrawTarget, "xyResetDrawTarget");
	xyBindFunc(v, sqDrawImage, "xyDrawImage", 4, ".nnn");
	xyBindFunc(v, sqSetDrawColor, "xySetDrawColor", 5, ".nnnn");
	xyBindFunc(v, sqUpdateScreen, "xyUpdateScreen");
	xyBindFunc(v, sqDrawRec, "xyDrawRec", 7, ".nnnnnn|b");
	xyBindFunc(v, sqLoadImage, "xyLoadImage", 2, ".s");
	xyBindFunc(v, sqLoadImageKeyed, "xyLoadImageKey", 3, ".sn");
	xyBindFunc(v, sqDrawImage, "xyDrawImage", 4, ".inn");
	xyBindFunc(v, sqSetBackgroundColor, "xySetBackgroundColor", 2, ".n");
	xyBindFunc(v, sqSetScalingFilter, "xySetScalingFilter", 2, ".i");

	//Sprites
	xyPrint(0, "Embedding sprites...");
	xyBindFunc(v, sqNewSprite, "xyNewSprite", 9, ".iiiiiiii");
	xyBindFunc(v, sqDrawSprite, "xyDrawSprite", 5, ".innn");
	xyBindFunc(v, sqDrawSpriteEx, "xyDrawSpriteEx", 9, ".innnninn");

	//Input
	xyPrint(0, "Embedding input...");
	xyBindFunc(v, sqUpdateInput, "xyUpdateInput");
	xyBindFunc(v, sqKeyPress, "xyKeyPress", 2, ".n");
	xyBindFunc(v, sqKeyRelease, "xyKeyRelease", 2, ".n");
	xyBindFunc(v, sqKeyDown, "xyKeyDown", 2, ".n");
	xyBindFunc(v, sqMouseX, "xyMouseX", 1, ".");
	xyBindFunc(v, sqMouseY, "xyMouseY", 1, ".");

	//Maths
	xyPrint(0, "Embedding maths...");
	xyBindFunc(v, sqRandomFloat, "xyRandFloat", 2, ".n");
	xyBindFunc(v, sqRandomInt, "xyRandInt", 2, ".n");

	//File IOxyPrint(0, "Embedding file I/O...");
	xyBindFunc(v, sqFileExists, "xyFileExists", 2, ".s");
};

int xyGetOS(){
#ifdef _OS_WINDOWS_
	return 0;
#endif
#ifdef _OS_MAC_
	return 1;
#endif
#ifdef _OS_LINUX_
	return 2;
#endif
#ifdef _OS_PANDORA_
	return 3;
#endif
#ifdef _OS_GCWZ_
	return 4;
#endif
#ifdef _OS_ANDROID_
	return 5;
#endif
#ifdef _OS_IOS_
	return 6;
#endif
};