#ifndef _SDL_EVENT_MANAGER_
#define _SDL_EVENT_MANAGER_

#include "Foundation/T_Singleton.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"
#include "Services/Controllers/ButtonControllerSource.h"
#include "Services/Controllers/JoystickControllerSource.h"
#include "Services/Controllers/HatControllerSource.h"
#include "Services/Controllers/KeyboardControllerSource.h"
#include <SDL/SDL.h>

#include <string>
#include <vector>

#define MAX_JOY_COUNT 4

class SDLGUIWindowImp;


class SDLEventManager: public T_Singleton<SDLEventManager>,public EventManager {
public:
	SDLEventManager() ;
	~SDLEventManager() ;
	virtual bool Init() ;
	virtual int MainLoop() ;
	virtual void PostQuitMessage() ;
	virtual int GetKeyCode(const char *name) ;
	void RenderPowerMenu(SDL_Surface *screen) ;
	void RenderDebugScreen(SDL_Surface *screen) ;

private:
	static bool finished_ ;
	static bool dumpEvent_ ;
	static bool showPowerMenu_ ;
	static int powerMenuSelection_ ;
	static bool showExitConfirm_ ;
	static int exitConfirmSelection_ ;
	static bool showDebugScreen_ ;
	static int debugScreenSelection_ ;
	const char *keyname_[SDLK_LAST] ;
	SDL_Joystick *joystick_[MAX_JOY_COUNT];
	ButtonControllerSource *buttonCS_[MAX_JOY_COUNT] ;
	JoystickControllerSource *joystickCS_[MAX_JOY_COUNT] ;
	HatControllerSource *hatCS_[MAX_JOY_COUNT] ;
	KeyboardControllerSource *keyboardCS_ ;

	void HandlePowerMenuInput(SDLKey key) ;
	void HandleDebugScreenInput(SDLKey key) ;

#ifdef PLATFORM_RGNANO_SIM
	struct SimCommand {
		std::string op;
		std::string arg;
		std::string arg2;
		int value;
		int value2;
	};
	std::vector<SimCommand> simCommands_;
	size_t simCommandIndex_;
	unsigned long simNextCommandTime_;
	int simPendingReleaseKey_;
	int simMouseKey_;
	bool simScriptActive_;
	bool simScriptFailed_;
	void LoadSimScript();
	void ProcessSimScript(SDLGUIWindowImp *window);
	bool HandleSimMouse(SDLGUIWindowImp *window, SDL_Event &event);
	void SetSimKey(SDLGUIWindowImp *window, int key, bool pressed);
	int GetSimButtonAt(int x, int y);
	void SaveSimScreenshot(SDLGUIWindowImp *window, const std::string &path);
	void FailSimScript(const char *message);
	bool ExpectSimFile(const std::string &path);
	bool ExpectSimProjectSample(const std::string &sampleName);
	bool ExpectSimLog(const std::string &needle);
	bool ExpectSimNoError();
	bool ExpectSimView(const std::string &viewName);
	bool ExpectSimScreenSize(SDLGUIWindowImp *window, int width, int height);
	bool ExpectSimScreenColors(SDLGUIWindowImp *window, int minColors);
	int CountSurfaceColors(SDL_Surface *surface, int maxColors);
	Uint32 ReadSurfacePixel(SDL_Surface *surface, int x, int y);
#endif
} ;
#endif
