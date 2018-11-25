/*===============*\
| BINDINGS SOURCE |
\*===============*/

#include "main.h"
#include "global.h"
#include "graphics.h"
#include "input.h"
#include "maths.h"
#include "fileio.h"
#include "text.h"
#include "binds.h"
#include "audio.h"

//////////
// MAIN //
/////////{

SQInteger sqWait(HSQUIRRELVM v){
	SQInteger a;

	sq_getinteger(v, 2, &a);

	xyWait((int) a);

	return 0;
};

SQInteger sqUpdate(HSQUIRRELVM v){
	xyUpdate();

	return 0;
};

SQInteger sqGetOS(HSQUIRRELVM v){
	switch(xyGetOS()){
		case 0:
			sq_pushstring(v, "windows", 7);
			break;
		case 1:
			sq_pushstring(v, "mac", 3);
			break;
		case 2:
			sq_pushstring(v, "linux", 5);
			break;
		case 3:
			sq_pushstring(v, "gcwz", 4);
			break;
		case 4:
			sq_pushstring(v, "pandora", 7);
			break;
		case 5:
			sq_pushstring(v, "android", 7);
			break;
		case 6:
			sq_pushstring(v, "ios", 7);
			break;
	};

	return 1;
};

SQInteger sqGetTicks(HSQUIRRELVM v){
	sq_pushinteger(v, SDL_GetTicks());

	return 1;
};

SQInteger sqGetFPS(HSQUIRRELVM v){
	sq_pushinteger(v, gvFPS);

	return 1;
};

SQInteger sqSetFPS(HSQUIRRELVM v){
	SQInteger iMax;

    sq_getinteger(v, 2, &iMax);

    if(iMax < 0){
    	xyPrint(0, "Maximum FPS cannot be negative.");
		return 0;
    } else gvMaxFPS = iMax;

	return 0;
};

SQInteger sqSetWindowTitle(HSQUIRRELVM v){
	const char* t;

	sq_getstring(v, 2, &t);
	SDL_SetWindowTitle(gvWindow, t);

	return 0;
};

//}

/////////////
// FILE IO //
////////////{

SQInteger sqImport(HSQUIRRELVM v){
	const char* a;

	sq_getstring(v, 2, &a);

	string b = gvAppDir;
	b += "xylib/";
	b += a;
	b += ".nut";

	sqstd_dofile(gvSquirrel, b.c_str(), 0, 1);

	return 0;
};

SQInteger sqDoNut(HSQUIRRELVM v){
	const char* a;

	sq_getstring(v, 2, &a);

	string b = "";
	char c[256];
	if(getcwd(c, sizeof(c)) == 0) return 0;
	b += c;
	b += "/";
	b += a;

	xyPrint(0, "Running %s...", b.c_str());
	sqstd_dofile(gvSquirrel, b.c_str(), 0, 1);

	return 0;
};

SQInteger sqFileExists(HSQUIRRELVM v){
	const char* file;

	sq_getstring(v, 2, &file);

	sq_pushbool(v, xyFileExists(file));

	return 1;
};

SQInteger sqGetDir(HSQUIRRELVM v){
	return 1;
};

SQInteger sqSetDir(HSQUIRRELVM v){
	return 0;
};

//}

//////////////
// GRAPHICS //
/////////////{

SQInteger sqClearScreen(HSQUIRRELVM v){
	SDL_RenderClear(gvRender);

	return 0;
};

SQInteger sqSetDrawTarget(HSQUIRRELVM v){
	SQInteger tex;

	sq_getinteger(v, 2, &tex);

	xySetDrawTarget(tex);

	return 0;
};

SQInteger sqResetDrawTarget(HSQUIRRELVM v){
	xyResetDrawTarget();

	return 0;
};

SQInteger sqDrawImage(HSQUIRRELVM v){
	SQInteger x, y, img;

	sq_getinteger(v, 2, &img);
	sq_getinteger(v, 3, &x);
	sq_getinteger(v, 4, &y);

	xyDrawImage(img, x, y);

	return 0;
};

SQInteger sqSetDrawColor(HSQUIRRELVM v){
	Uint32 c;

	sq_getinteger(v, 2, (SQInteger*)&c);

	xySetDrawColor(c);

	return 0;
};

SQInteger sqLoadImage(HSQUIRRELVM v){
	const char* file;

	sq_getstring(v, 2, &file);

	sq_pushinteger(v, xyLoadImage(file));

	return 1;
};

SQInteger sqLoadImageKeyed(HSQUIRRELVM v){
	const char* file;
	SQInteger key;

	sq_getstring(v, 2, &file);
	sq_getinteger(v, 3, &key);

	sq_pushinteger(v, xyLoadImageKeyed(file, key));

	return 1;
};

SQInteger sqSetBackgroundColor(HSQUIRRELVM v){
	SQInteger color;

	sq_getinteger(v, 2, &color);

	xySetBackgroundColor(color);

	return 0;
};

SQInteger sqSetScalingFilter(HSQUIRRELVM v){
	SQInteger hint;

	sq_getinteger(v, 2, &hint);
	if(hint > 2) hint = 2;
	if(hint < 0) hint = 0;

	string strHint;
	switch(hint){
		case 0:
			strHint = "0";
			break;
		case 1:
			strHint = "1";
			break;
		case 2:
			strHint = "2";
			break;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, strHint.c_str());

	return 0;
};

SQInteger sqSetResolution(HSQUIRRELVM v){
	SQInteger w, h;

	sq_getinteger(v, 2, &w);
	sq_getinteger(v, 3, &h);

	if(w < 1 || h < 1){
		xyPrint(0, "Window dimensions cannot be 0.");
		return 0;
	};

	SDL_Rect screensize;
	screensize.x = 0;
	screensize.y = 0;
	screensize.w = w;
	screensize.h = h;
	SDL_RenderSetViewport(gvRender, &screensize);
	SDL_RenderSetLogicalSize(gvRender, w, h);
	SDL_SetWindowSize(gvWindow, w, h);

	return 0;
};

//}

/////////////
// SPRITES //
////////////{

SQInteger sqNewSprite(HSQUIRRELVM v){
	SQInteger w, h, m, p, px, py, f;
	const char* i;

	sq_getstring(v, 2, &i);
	sq_getinteger(v, 3, &w);
	sq_getinteger(v, 4, &h);
	sq_getinteger(v, 5, &m);
	sq_getinteger(v, 6, &p);
	sq_getinteger(v, 7, &px);
	sq_getinteger(v, 8, &py);
	sq_getinteger(v, 9, &f);

	xySprite* newSprite = new xySprite(i, w, h, m, p, px, py, f);

	sq_pushinteger(v, newSprite->getnum());
	return 1;
};

SQInteger sqDrawSprite(HSQUIRRELVM v){
	SQInteger i, f, x, y;

	sq_getinteger(v, 2, &i);
	sq_getinteger(v, 3, &f);
	sq_getinteger(v, 4, &x);
	sq_getinteger(v, 5, &y);

	if(vcSprites.size() <= i) return 0;
	if(vcSprites[i] == 0) return 0;

	vcSprites[i]->draw(f, x, y);

	return 0;
};

SQInteger sqDrawSpriteEx(HSQUIRRELVM v){
	SQInteger i, f, x, y, a, l;
	float sx, sy;

	sq_getinteger(v, 2, &i);
	sq_getinteger(v, 3, &f);
	sq_getinteger(v, 4, &x);
	sq_getinteger(v, 5, &y);
	sq_getinteger(v, 6, &a);
	sq_getinteger(v, 7, &l);
	sq_getfloat(v, 8, &sx);
	sq_getfloat(v, 9, &sy);

	if(vcSprites.size() <= i) return 0;
	if(vcSprites[i] == 0) return 0;

	vcSprites[i]->drawex(f, x, y, a, static_cast<SDL_RendererFlip>(l), sx, sy);

	return 0;
};

SQInteger sqDeleteSprite(HSQUIRRELVM v){
	SQInteger i;

	sq_getinteger(v, 2, &i);

	if(i >= vcSprites.size()) return 0;
	if(vcSprites[i] != 0) delete vcSprites[i];

	return 0;
};

//}

///////////
// INPUT //
//////////{

SQInteger sqKeyPress(HSQUIRRELVM v){
	SQInteger a;

	sq_getinteger(v, 2, &a);
	sq_pushinteger(v, xyKeyPress(a));

	return 1;
};

SQInteger sqKeyDown(HSQUIRRELVM v){
	SQInteger a;

	sq_getinteger(v, 2, &a);
	sq_pushbool(v, xyKeyDown(a));

	return 1;
};

SQInteger sqKeyRelease(HSQUIRRELVM v){
	SQInteger a;

	sq_getinteger(v, 2, &a);
	sq_pushbool(v, xyKeyRelease(a));

	return 1;
};

SQInteger sqMouseDown(HSQUIRRELVM v){
	SQInteger i;

	sq_getinteger(v, 2, &i);

	sq_pushinteger(v, xyMouseButton(i));

	return 1;
};

SQInteger sqMousePress(HSQUIRRELVM v){
	SQInteger i;

	sq_getinteger(v, 2, &i);

	sq_pushinteger(v, xyMousePress(i));

	return 1;
};

SQInteger sqMouseRelease(HSQUIRRELVM v){
	SQInteger i;

	sq_getinteger(v, 2, &i);

	sq_pushinteger(v, xyMouseRelease(i));

	return 1;
};

SQInteger sqMouseX(HSQUIRRELVM v){
	sq_pushinteger(v, gvMouseX);
	return 1;
};

SQInteger sqMouseY(HSQUIRRELVM v){
	sq_pushinteger(v, gvMouseY);
	return 1;
};

SQInteger sqGetQuit(HSQUIRRELVM v){
	sq_pushbool(v, gvQuit);

	return 1;
};

//}

///////////
// MATHS //
//////////{

SQInteger sqRandomFloat(HSQUIRRELVM v){
	float a;

	sq_getfloat(v, 2, &a);
	sq_pushinteger(v, xyRandomFloat(a));

	return 1;
};

SQInteger sqRandomInt(HSQUIRRELVM v){
	SQInteger a;

	sq_getinteger(v, 2, &a);
	sq_pushinteger(v, xyRandomInt(a));

	return 1;
};

SQInteger sqDistance2(HSQUIRRELVM v){
	SQInteger x1, y1, x2, y2;

	sq_getinteger(v, 2, &x1);
	sq_getinteger(v, 3, &y1);
	sq_getinteger(v, 4, &x2);
	sq_getinteger(v, 5, &y2);

	sq_pushinteger(v, xyDistance(x1, y1, x2, y2));

	return 1;
};

SQInteger sqWrap(HSQUIRRELVM v){
	SQInteger x, mx, mn;

	sq_getinteger(v, 2, &x);
	sq_getinteger(v, 3, &mx);
	sq_getinteger(v, 4, &mn);

	sq_pushinteger(v, xyWrap(x, mn, mx));

	return 1;
};

SQInteger sqFloor(HSQUIRRELVM v){
	float f;

	sq_getfloat(v, 2, &f);
	sq_pushinteger(v, floor(f));

	return 1;
};

SQInteger sqPointAngle(HSQUIRRELVM v){
	float x1, y1, x2, y2;

	sq_getfloat(v, 2, &x1);
	sq_getfloat(v, 3, &y1);
	sq_getfloat(v, 4, &x2);
	sq_getfloat(v, 5, &y2);

	sq_pushfloat(v, xyPointAngle(x1, y1, x2, y2));

	return 1;
};

//}

//////////
// TEXT //
/////////{

SQInteger sqNewFont(HSQUIRRELVM v){
	SQInteger i, c, t;
	SQBool m;

	sq_getinteger(v, 2, &i);
	sq_getinteger(v, 3, &c);
	sq_getinteger(v, 4, &t);
	sq_getbool(v, 5, &m);

	//Sanitize inputs
	if(i < 0) i = 0;
	if(t > 255) t = 255;
	if(c > 255) c = 255;

	xyFont* newfont = new xyFont(i, (char)c, t, m);

	sq_pushinteger(v, newfont->getnum());

	return 1;
};

SQInteger sqDrawText(HSQUIRRELVM v){
	SQInteger f, x, y;
	const char* s;

	sq_getinteger(v, 2, &f);
	sq_getinteger(v, 3, &x);
	sq_getinteger(v, 4, &y);
	sq_getstring(v, 5, &s);

	if(f >= vcFonts.size()) return 0;
	if(vcFonts[f] == 0) return 0;

	vcFonts[f]->draw(x, y, s);

	return 0;
};

//}

///////////
// AUDIO //
//////////{

SQInteger sqLoadSound(HSQUIRRELVM v){
	const char* s;

	sq_getstring(v, 2, &s);

	sq_pushinteger(v, xyLoadSound(s));

	return 1;
};

SQInteger sqLoadMusic(HSQUIRRELVM v){
	const char* m;

	sq_getstring(v, 2, &m);

	sq_pushinteger(v, xyLoadMusic(m));

	return 1;
};

SQInteger sqPlaySound(HSQUIRRELVM v){
	SQInteger s, l;

	sq_getinteger(v, 2, &s);
	sq_getinteger(v, 3, &l);

	sq_pushinteger(v, xyPlaySound(s, l));

	return 1;
};

SQInteger sqPlayMusic(HSQUIRRELVM v){
	SQInteger m, l;

	sq_getinteger(v, 2, &m);
	sq_getinteger(v, 3, &l);

	xyPlayMusic(m, l);

	return 0;
};

SQInteger sqDeleteSound(HSQUIRRELVM v){
	SQInteger i;

	sq_getinteger(v, 2, &i);

	xyDeleteSound(i);

	return 0;
};

SQInteger sqDeleteMusic(HSQUIRRELVM v){
	SQInteger i;

	sq_getinteger(v, 2, &i);

	xyDeleteMusic(i);

	return 0;
};

//}

/////////////
// SDL_GFX //
////////////{

//}
