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
	void RenderPowerMenu(SDL_Surface *screen, SDLGUIWindowImp *window=0) ;
	void RenderDebugScreen(SDL_Surface *screen, SDLGUIWindowImp *window=0) ;

private:
	static bool finished_ ;
	static bool dumpEvent_ ;
	static bool showPowerMenu_ ;
	static int powerMenuSelection_ ;
	static bool showExitConfirm_ ;
	static int exitConfirmSelection_ ;
	static bool showDebugScreen_ ;
	static int debugScreenSelection_ ;
	static bool menuInputHeld_[SDLK_LAST] ;
	static int powerMenuKey_ ;
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
		std::string arg3;
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
	bool AddSimScriptLine(const std::string &line, const char *scriptPath, int lineNumber);
	bool AppendSimRoute(const std::string &routeName, const char *scriptPath, int lineNumber);
	void ProcessSimScript(SDLGUIWindowImp *window);
	bool HandleSimMouse(SDLGUIWindowImp *window, SDL_Event &event);
	void SetSimKey(SDLGUIWindowImp *window, int key, bool pressed);
	int GetSimButtonAt(int x, int y);
	void SaveSimScreenshot(SDLGUIWindowImp *window, const std::string &path);
	void SaveSimAppScreenshot(SDLGUIWindowImp *window, const std::string &path);
	void FailSimScript(const char *message);
	bool ExpectSimFile(const std::string &path);
	bool ExpectSimProjectSample(const std::string &sampleName);
	bool ExpectSimLog(const std::string &needle);
	bool ExpectSimNoError();
	bool ExpectSimScreensDiffer(const std::string &firstPath, const std::string &secondPath);
	bool ExpectSimView(const std::string &viewName);
	bool ExpectSimSelectedText(const std::string &needle);
	bool ExpectSimPlayerRunning(const std::string &expected);
	bool ExpectSimPlayMode(const std::string &modeName);
	bool ExpectSimStreamingSample(const std::string &needle);
	bool ExpectSimPlayingChannel(int channel, const std::string &instrument);
	bool ExpectSimAudioActivity(int minPeak);
	bool ExpectSimAudioSilence(int maxPeak);
	bool ExpectSimAudioCaptureBytes(int minBytes);
	bool ExpectSimScreenText(const std::string &needle);
	bool ExpectSimSongChain(int row, int channel, const std::string &expected);
	bool ExpectSimChainPhrase(int chain, int row, const std::string &expected);
	bool ExpectSimPhraseRowCount(int phrase, int minRows);
	bool ExpectSimTempo(int bpm);
	bool ExpectSimInstrumentSample(int instrument, const std::string &sampleName);
	bool SimSetTempo(int bpm);
	bool SimSetNoteNames(const std::string &mode);
	bool SimSetScale(int scale);
	bool SimSetScaleKey(int key);
	bool SimImportSampleToInstrument(int instrument, const std::string &sampleName);
	bool SimSetSongChain(int row, int channel, int chain);
	bool SimSetChainPhrase(int chain, int row, int phrase, int transpose);
	bool SimSetPhraseNote(int phrase, int row, int note, int instrument);
	bool SimSaveProject();
	bool ExpectSimScreenSize(SDLGUIWindowImp *window, int width, int height);
	bool ExpectSimScreenColors(SDLGUIWindowImp *window, int minColors);
	bool ExpectSimSkinFrameClean(SDLGUIWindowImp *window);
	int CountSurfaceColors(SDL_Surface *surface, int maxColors);
	Uint32 ReadSurfacePixel(SDL_Surface *surface, int x, int y);
	void LogSimState(const char *label, bool includeScreen);
#endif
} ;
#endif
