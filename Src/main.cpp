// ============================================================================
// [Include Section]
// ============================================================================
#include <string>


#include <Fog/Core.h>
#include <Fog/G2d.h>

#include <SDL2/SDL.h>

using namespace Fog;
// ============================================================================
// [Tools]
// ============================================================================
// ----------------------------------------------------------------------------


static Fog::TransformF createRotationMatrix(Fog::PointF center, float rotation)
{
	Fog::TransformF m;
	Fog::PointF negcenter(-center.getX(),-center.getY());
	
	m.translate(negcenter, Fog::MATRIX_ORDER_APPEND);
	m.rotate(rotation, Fog::MATRIX_ORDER_APPEND);
	m.translate(center, Fog::MATRIX_ORDER_APPEND);
	
	return m;
}

// ============================================================================
// [SdlApplication]
// ============================================================================

// SdlApplication is nothing more than thin wrapper to SDL library. You need
// just to instantiate it and call run() to enter the SDL event loop.
struct SdlApplication
{
	SdlApplication();
	~SdlApplication();
	
	// Application state (just convenience instead of 0, 1, ...).
	enum APP_STATE
	{
		APP_OK = 0,
		APP_FAILED = 1
	};
	
	// Initialize application, called by run(), don't call manually.
	int init(int width, int height);
	
	// Destroy application, called by destructor, don't call manually.
	void destroy();
	
	// Run application, called by your code.
	int run(int width, int height);
	
	// Called to process SDL event.
	void onEvent(SDL_Event* ev);
	
	// Called on timer event.
	static Uint32 _onTimerCb(Uint32 interval, void* param);
	void onTimer();
	
	// Called to render content into buffer.
	void onRender();
	
	// Called to render content using Fog::Painter, called by onRender().
	void onPaint(Fog::Painter& p);
	
	// SDL screen surface, the surface we will paint to.
	SDL_Surface* _screen;
	
	// SDL timer.
	SDL_TimerID _timer;
	
	// Delay between frames.
	int _interval;
	
	// Some stuff we work with.
	double _rotate;
	
	// Whether the application is in event loop.
	bool _running;
	SDL_Window *win;
};

SdlApplication::SdlApplication() :
_screen(NULL),
_timer(0),
_interval(25),
_rotate(0.0),
_running(false)
{
}

SdlApplication::~SdlApplication()
{
	//SDL_DestroyWindow(win);
	destroy();
}

int SdlApplication::init(int width, int height)
{
	// Initialize the SDL library.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
		return APP_FAILED;
	}
	
	win = SDL_CreateWindow("SDL2 & FOG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	_screen = SDL_GetWindowSurface(win);
	// Create main surface.
	if (_screen  == NULL)
	{
		fprintf(stderr, "SDL_GetWindowSurface() failed: %s\n", SDL_GetError());
		return APP_FAILED;
	}
	
	
	
	// Create timer
	_timer = SDL_AddTimer(_interval, SdlApplication::_onTimerCb, reinterpret_cast<void*>(this));
	
	// Success.
	return APP_OK;
}

void SdlApplication::destroy()
{
	if (_screen)
	{
		SDL_RemoveTimer(_timer);
		SDL_FreeSurface(_screen);
		SDL_DestroyWindow(win);
		SDL_Quit();
		
		_screen = NULL;
	}
}

int SdlApplication::run(int width, int height)
{
	// Initialize application.
	int state = init(width, height);
	if (state != APP_OK) return state;
	
	// Enter to the SDL event loop.
	SDL_Event ev;
	_running = true;
	
	while (SDL_WaitEvent(&ev))
	{
		onEvent(&ev);
		if (_running == false)
		{
			break;
		}
	}
	
	return APP_OK;
}

void SdlApplication::onEvent(SDL_Event* ev)
{
	switch (ev->type)
	{
		case SDL_QUIT:
			_running = false;
			break;
			
		case SDL_USEREVENT:
			// This is our timer event.
			onTimer();
			break;
	}
}

Uint32 SdlApplication::_onTimerCb(Uint32 interval, void* param)
{
	// On timer callback is called from different thread, we just send custom event
	// back to the main one.
	//SdlApplication* app = reinterpret_cast<SdlApplication*>(param);
	
	SDL_UserEvent e;
	memset(&e, 0, sizeof(e));
	e.type = SDL_USEREVENT;
	SDL_PushEvent(reinterpret_cast<SDL_Event*>(&e));
	
	return interval;
}

void SdlApplication::onTimer()
{
	// Do some stuff...
	_rotate += 0.01;
	
	// Render new frame.
	onRender();
}

void SdlApplication::onRender()
{
	// Lock surface pixels.
	SDL_LockSurface(_screen);
	
	// Create Fog::Painter instance mapped to the SDL surface data.
	Fog::Painter p;
	
	// Setup image buffer for painter.
	Fog::ImageBits buf;
	
	buf.setData(Fog::SizeI(_screen->w, _screen->h), Fog::IMAGE_FORMAT_XRGB32, _screen->pitch, (reinterpret_cast<uint8_t*>(_screen->pixels)));
	
	// Call our paint handler.
	if (p.begin(buf) == Fog::ERR_OK) onPaint(p);
	
	// Never forget to call p.end(). Painting can be asynchronous and
	// SDL_UnlockSurface() can invalidate the surface->pixels pointer.
	p.end();
	
	// Unlock surface pixels.
	SDL_UnlockSurface(_screen);
	
	// Flip buffer.
	//SDL_Flip(_screen);
	SDL_UpdateWindowSurface(win);
}

void SdlApplication::onPaint(Fog::Painter& p)
{
	// Just use the Fog-Framework ;-)
	
	// Create some variable we will work with.
	float w = float(_screen->w);
	float h = float(_screen->h);
	
	float roundw = 100.0f;
	float roundh = 100.0f;
	
	Fog::PointF cp(w / 2.0f, h / 2.0f);
	
	// Clear the entire screen.
	p.setSource(Fog::Argb32(0xFF000000));
	p.fillAll();
	
	p.save();
	
	// Rotate across the screen center point.
	p.transform(createRotationMatrix(cp, _rotate));
	
	// And draw something...
	Fog::LinearGradientF gradient;
	
	//gradient
	gradient.setStart(cp.x - roundw / 2.0f, cp.y - roundh / 2.0f);
	gradient.setEnd(cp.x + roundw / 2.0f, cp.y + roundh / 2.0f);
	gradient.addStop(0.0f, Fog::Argb32(0xFFFFFFFF));
	gradient.addStop(0.5f, Fog::Argb32(0xFFFFFF00));
	gradient.addStop(1.0f, Fog::Argb32(0xFFFF0000));
	p.setSource(gradient);
	
	//p.setSource(Fog::Argb32(0xFFFF0000));

	
	
	p.fillRound(Fog::RoundF(
							Fog::RectF(cp.x - roundw / 2.0f, cp.y - roundh / 2.0f, roundw, roundh),
							Fog::PointF(20.0f, 20.0f)));
	
	p.restore();
}

// ============================================================================
// [Entry-Point]
// ============================================================================

// SDL may redeclare main to something else, cut it.
//#undef main

int main(int argc, char* argv[])
{
	// ----------------------------------------------------------------------------
	// This makes relative paths work in C++ in Xcode by changing directory to the one right where the app is located
#ifdef __APPLE__
	
	CFURLRef resourcesURL = CFBundleCopyExecutableURL( CFBundleGetMainBundle() );
	char pathtofile[PATH_MAX];
	if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)pathtofile, PATH_MAX))
	{
		// error!
	}
	CFRelease(resourcesURL);
	
	std::string longpath(pathtofile);
	std::string desiredpath;
	size_t pos;
	
	pos = longpath.rfind(".app/");
	desiredpath = longpath.substr(0,pos);
	// trim off the rest
	pos = desiredpath.find_last_of('/');
	desiredpath.erase(pos);
	
	chdir(desiredpath.c_str());
#endif
	
	SdlApplication app;
	return app.run(640, 480);
}