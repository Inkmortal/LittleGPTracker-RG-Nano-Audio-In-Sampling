
#include "SDLEventManager.h"
#include "Application/Application.h"
#include "UIFramework/BasicDatas/GUIEvent.h"
#include "SDLGUIWindowImp.h"
#include "Application/Model/Config.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "Application/Views/BaseClasses/ViewEvent.h"
#include <fstream>
#include <sstream>

bool SDLEventManager::finished_=false ;
bool SDLEventManager::dumpEvent_=false ;
bool SDLEventManager::showPowerMenu_=false ;
int SDLEventManager::powerMenuSelection_=0;
bool SDLEventManager::showExitConfirm_=false ;
int SDLEventManager::exitConfirmSelection_=0;
bool SDLEventManager::showDebugScreen_=false ;
int SDLEventManager::debugScreenSelection_=0;

SDLEventManager::SDLEventManager() 
{
#ifdef PLATFORM_RGNANO_SIM
	simCommandIndex_=0;
	simNextCommandTime_=0;
	simPendingReleaseKey_=0;
	simScriptActive_=false;
#endif
}

SDLEventManager::~SDLEventManager() 
{
}

bool SDLEventManager::Init() 
{
	EventManager::Init() ;
	
	if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_TIMER) < 0 )
  {
		return false;
	}
  
	SDL_EnableUNICODE(1);
	SDL_ShowCursor(SDL_DISABLE);
	
	atexit(SDL_Quit) ;
	
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);

	int joyCount=SDL_NumJoysticks() ;
	joyCount=(joyCount>MAX_JOY_COUNT)?MAX_JOY_COUNT:joyCount ;

	keyboardCS_=new KeyboardControllerSource("keyboard") ;
	const char *dumpIt=Config::GetInstance()->GetValue("DUMPEVENT") ;
	if ((dumpIt)&&(!strcmp(dumpIt,"YES")))
  {
		dumpEvent_=true ;
	}

#ifdef PLATFORM_RGNANO_SIM
	LoadSimScript();
#endif

	for (int i=0;i<MAX_JOY_COUNT;i++) 
  {
		joystick_[i]=0 ;
		buttonCS_[i]=0 ;
		joystickCS_[i]=0 ;
	}
    
	for (int i=0;i<joyCount;i++) 
  {
		char sourceName[128] ;
		joystick_[i]=SDL_JoystickOpen(i) ;
    Trace::Log("EVENT","joystick[%d]=%x",i,joystick_[i]) ;
		Trace::Log("EVENT","Number of axis:%d",SDL_JoystickNumAxes(joystick_[i])) ;
		Trace::Log("EVENT","Number of buttons:%d",SDL_JoystickNumButtons(joystick_[i])) ;
		Trace::Log("EVENT","Number of hats:%d",SDL_JoystickNumHats(joystick_[i])) ;
		sprintf(sourceName,"buttonJoy%d",i) ;
		buttonCS_[i]=new ButtonControllerSource(sourceName) ;
		sprintf(sourceName,"axisJoy%d",i) ;
		joystickCS_[i]=new JoystickControllerSource(sourceName) ;
		sprintf(sourceName,"hatJoy%d",i) ;
		hatCS_[i]=new HatControllerSource(sourceName) ;
	}
  
	for (int i=0;i<SDLK_LAST;i++) 
  {
		keyname_[i]=SDL_GetKeyName((SDLKey)i) ;
	} 
  
	return true ;
} 

int SDLEventManager::MainLoop()
{
	GUIWindow *appWindow=Application::GetInstance()->GetWindow() ;
	SDLGUIWindowImp *sdlWindow=(SDLGUIWindowImp *)appWindow->GetImpWindow() ;

	while (!finished_)
	{
		SDL_Event event;
#ifdef PLATFORM_RGNANO_SIM
		bool hasEvent = (SDL_PollEvent(&event) != 0);
#else
		bool hasEvent = (SDL_WaitEvent(&event) != 0);
#endif
		if (hasEvent)
    {
			switch (event.type) {
				case SDL_KEYDOWN:
					if (dumpEvent_)
          {
						Trace::Log("EVENT","key(%s):%d",keyname_[event.key.keysym.sym],1) ;
					}

#ifdef PLATFORM_RGNANO
					// Check for power menu toggle (SDLK_q)
					if (event.key.keysym.sym == SDLK_q) {
						showPowerMenu_ = !showPowerMenu_;
						if (showPowerMenu_) {
							powerMenuSelection_ = 0;  // Reset to first option
						}
						break;
					}
#endif

					// If menu is active, handle menu input
					if (showPowerMenu_) {
						HandlePowerMenuInput(event.key.keysym.sym);
						break;  // Don't pass to game when menu is active
					}

					// If debug screen is active, handle debug input
					if (showDebugScreen_) {
						HandleDebugScreenInput(event.key.keysym.sym);
						break;  // Don't pass to game when debug screen is active
					}

					keyboardCS_->SetKey((int)event.key.keysym.sym,true) ;
					break ;

				case SDL_KEYUP:
					if (dumpEvent_) 
          {
						Trace::Log("EVENT","key(%s):%d",keyname_[event.key.keysym.sym],0) ;
					}
					keyboardCS_->SetKey((int)event.key.keysym.sym,false) ;
					break ;


				case SDL_JOYBUTTONDOWN:
					buttonCS_[event.jbutton.which]->SetButton(event.jbutton.button,true) ;
					break ;
				case SDL_JOYBUTTONUP:
					if (dumpEvent_) {
						Trace::Log("EVENT","but(%d):%d",event.button.which,event.jbutton.button) ;
					}
					buttonCS_[event.jbutton.which]->SetButton(event.jbutton.button,false) ;
					break ;
				case SDL_JOYAXISMOTION:
					if (dumpEvent_) {
						Trace::Log("EVENT","joy(%d)::%d=%d",event.jaxis.which,event.jaxis.axis,event.jaxis.value) ;
					}
					joystickCS_[event.jaxis.which]->SetAxis(event.jaxis.axis,float(event.jaxis.value)/32767.0f) ;
					break ;
				case SDL_JOYHATMOTION:
					if (dumpEvent_)
          {
						for (int i=0;i<4;i++)
            {
							int mask = 1<<i ;
							if (event.jhat.value&mask)
              {
								Trace::Log("EVENT","hat(%d)::%d::%d",event.jhat.which,event.jhat.hat,i) ;
							}
						}
					}
					hatCS_[event.jhat.which]->SetHat(event.jhat.hat,event.jhat.value) ;
					break ;
				case SDL_JOYBALLMOTION:
					if (dumpEvent_)
          {
						Trace::Log("EVENT","ball(%d)::%d=(%d,%d)",event.jball.which,event.jball.ball,event.jball.xrel,event.jball.yrel) ;
					}
					break ;
		}

			switch (event.type) 
			{

				case SDL_QUIT:
					sdlWindow->ProcessQuit() ;
					break ;
				case SDL_VIDEOEXPOSE:
					sdlWindow->ProcessExpose() ;

					break ;
				case SDL_USEREVENT:
					sdlWindow->ProcessUserEvent(event) ;
					break ;
			}
		}
#ifdef PLATFORM_RGNANO_SIM
		ProcessSimScript(sdlWindow);
		SDL_Delay(10);
#endif
	}
	return 0 ;
} ;

#ifdef PLATFORM_RGNANO_SIM
void SDLEventManager::LoadSimScript()
{
	const char *scriptPath=Config::GetInstance()->GetValue("RGNANOSIM_SCRIPT");
	if (!scriptPath) {
		return;
	}

	std::ifstream file(scriptPath);
	if (!file.is_open()) {
		Trace::Error("RGNANO_SIM failed to open script %s", scriptPath);
		return;
	}

	std::string line;
	while (std::getline(file,line)) {
		if (line.empty() || line[0]=='#') {
			continue;
		}
		std::istringstream iss(line);
		SimCommand command;
		command.value=0;
		iss >> command.op;
		if (command.op.empty()) {
			continue;
		}
		if (command.op=="wait") {
			iss >> command.value;
		} else if (command.op=="press") {
			iss >> command.arg >> command.value;
			if (command.value<=0) {
				command.value=80;
			}
		} else if (command.op=="down" || command.op=="up" || command.op=="screenshot" || command.op=="log") {
			iss >> command.arg;
		}
		simCommands_.push_back(command);
	}

	simCommandIndex_=0;
	simNextCommandTime_=System::GetInstance()->GetClock()+250;
	simPendingReleaseKey_=0;
	simScriptActive_=!simCommands_.empty();
	Trace::Log("RGNANO_SIM","Loaded %d script commands from %s",(int)simCommands_.size(),scriptPath);
}

void SDLEventManager::ProcessSimScript(SDLGUIWindowImp *window)
{
	if (!simScriptActive_) {
		return;
	}

	unsigned long now=System::GetInstance()->GetClock();
	if (now<simNextCommandTime_) {
		return;
	}

	if (simPendingReleaseKey_>0) {
		keyboardCS_->SetKey(simPendingReleaseKey_,false);
		Trace::Log("RGNANO_SIM","auto up key:%d",simPendingReleaseKey_);
		simPendingReleaseKey_=0;
		simCommandIndex_++;
		simNextCommandTime_=now+40;
		return;
	}

	if (simCommandIndex_>=simCommands_.size()) {
		simScriptActive_=false;
		Trace::Log("RGNANO_SIM","Script complete");
		return;
	}

	SimCommand &command=simCommands_[simCommandIndex_];
	if (command.op=="wait") {
		simCommandIndex_++;
		simNextCommandTime_=now+(unsigned long)command.value;
		return;
	}

	if (command.op=="press") {
		int key=GetKeyCode(command.arg.c_str());
		if (key>0) {
			keyboardCS_->SetKey(key,true);
			simPendingReleaseKey_=key;
			simNextCommandTime_=now+(unsigned long)command.value;
			Trace::Log("RGNANO_SIM","press %s",command.arg.c_str());
			return;
		}
		Trace::Error("RGNANO_SIM unknown key %s", command.arg.c_str());
	} else if (command.op=="down") {
		int key=GetKeyCode(command.arg.c_str());
		if (key>0) {
			keyboardCS_->SetKey(key,true);
		}
	} else if (command.op=="up") {
		int key=GetKeyCode(command.arg.c_str());
		if (key>0) {
			keyboardCS_->SetKey(key,false);
		}
	} else if (command.op=="screenshot") {
		SaveSimScreenshot(window,command.arg);
	} else if (command.op=="quit") {
		PostQuitMessage();
	} else if (command.op=="log") {
		Trace::Log("RGNANO_SIM","%s",command.arg.c_str());
	}

	simCommandIndex_++;
	simNextCommandTime_=now+40;
}

void SDLEventManager::SaveSimScreenshot(SDLGUIWindowImp *window, const std::string &path)
{
	if (!window || path.empty()) {
		return;
	}
	window->Flush();
	if (SDL_SaveBMP(window->GetSurface(),path.c_str())==0) {
		Trace::Log("RGNANO_SIM","Saved screenshot %s",path.c_str());
	} else {
		Trace::Error("RGNANO_SIM failed screenshot %s",path.c_str());
	}
}
#endif



void SDLEventManager::PostQuitMessage()
{
  Trace::Log("EVENT","SDEM:PostQuitMessage()") ;
	finished_=true  ;
} ; 


int SDLEventManager::GetKeyCode(const char *key)
{
	for (int i=0;i<SDLK_LAST;i++)
  {
		if (!strcmp(key,keyname_[i]))
    {
			return i ;
		}
	}
	return -1 ;
}

void SDLEventManager::RenderPowerMenu(SDL_Surface *screen)
{
	if (!showPowerMenu_ || !screen) return;

	// Darken background
	SDL_Rect fullScreen = {0, 0, screen->w, screen->h};
	SDL_FillRect(screen, &fullScreen, SDL_MapRGB(screen->format, 40, 40, 40));

	if (showExitConfirm_) {
		// Exit confirmation dialog
		int menuWidth = 180;
		int menuHeight = 80;
		SDL_Rect menuBox = {
			(screen->w - menuWidth) / 2,
			(screen->h - menuHeight) / 2,
			menuWidth,
			menuHeight
		};
		SDL_FillRect(screen, &menuBox, SDL_MapRGB(screen->format, 240, 240, 240));

		int itemHeight = 25;
		int itemY = menuBox.y + 20;

		// Yes option
		SDL_Rect yesRect = {menuBox.x + 10, itemY, menuWidth - 20, itemHeight};
		Uint32 yesColor = (exitConfirmSelection_ == 0) ?
			SDL_MapRGB(screen->format, 80, 120, 200) :
			SDL_MapRGB(screen->format, 200, 200, 200);
		SDL_FillRect(screen, &yesRect, yesColor);

		// No option
		SDL_Rect noRect = {menuBox.x + 10, itemY + itemHeight + 5, menuWidth - 20, itemHeight};
		Uint32 noColor = (exitConfirmSelection_ == 1) ?
			SDL_MapRGB(screen->format, 80, 120, 200) :
			SDL_MapRGB(screen->format, 200, 200, 200);
		SDL_FillRect(screen, &noRect, noColor);
	} else {
		// Main power menu
		int menuWidth = 180;
		int menuHeight = 80;
		SDL_Rect menuBox = {
			(screen->w - menuWidth) / 2,
			(screen->h - menuHeight) / 2,
			menuWidth,
			menuHeight
		};
		SDL_FillRect(screen, &menuBox, SDL_MapRGB(screen->format, 240, 240, 240));

		int itemHeight = 25;
		int itemY = menuBox.y + 10;

		// Exit option
		SDL_Rect exitRect = {menuBox.x + 10, itemY, menuWidth - 20, itemHeight};
		Uint32 exitColor = (powerMenuSelection_ == 0) ?
			SDL_MapRGB(screen->format, 80, 120, 200) :
			SDL_MapRGB(screen->format, 200, 200, 200);
		SDL_FillRect(screen, &exitRect, exitColor);

		// Debug option
		SDL_Rect debugRect = {menuBox.x + 10, itemY + itemHeight + 5, menuWidth - 20, itemHeight};
		Uint32 debugColor = (powerMenuSelection_ == 1) ?
			SDL_MapRGB(screen->format, 80, 120, 200) :
			SDL_MapRGB(screen->format, 200, 200, 200);
		SDL_FillRect(screen, &debugRect, debugColor);
	}
}

void SDLEventManager::HandlePowerMenuInput(SDLKey key)
{
	if (showExitConfirm_) {
		// Handle exit confirmation dialog
		switch (key) {
			case SDLK_u:  // UP
				exitConfirmSelection_--;
				if (exitConfirmSelection_ < 0) exitConfirmSelection_ = 1;
				break;

			case SDLK_d:  // DOWN
				exitConfirmSelection_++;
				if (exitConfirmSelection_ > 1) exitConfirmSelection_ = 0;
				break;

			case SDLK_a:  // A button - confirm
			case SDLK_RETURN:
				if (exitConfirmSelection_ == 0) {
					// Yes - quit
					showPowerMenu_ = false;
					showExitConfirm_ = false;
					PostQuitMessage();
				} else {
					// No - back to main menu
					showExitConfirm_ = false;
					exitConfirmSelection_ = 0;
				}
				break;

			case SDLK_b:  // B button - cancel
			case SDLK_ESCAPE:
				// Back to main menu
				showExitConfirm_ = false;
				exitConfirmSelection_ = 0;
				break;

			default:
				break;
		}
	} else {
		// Handle main power menu
		switch (key) {
			case SDLK_u:  // UP
				powerMenuSelection_--;
				if (powerMenuSelection_ < 0) powerMenuSelection_ = 1;
				break;

			case SDLK_d:  // DOWN
				powerMenuSelection_++;
				if (powerMenuSelection_ > 1) powerMenuSelection_ = 0;
				break;

			case SDLK_a:  // A button - confirm
			case SDLK_RETURN:
				if (powerMenuSelection_ == 0) {
					// Exit - show confirmation
					showExitConfirm_ = true;
					exitConfirmSelection_ = 1;  // Default to "No"
				} else if (powerMenuSelection_ == 1) {
					// Debug screen - open it
					showPowerMenu_ = false;
					showDebugScreen_ = true;
					debugScreenSelection_ = 0;
				}
				break;

			case SDLK_b:  // B button - cancel
			case SDLK_ESCAPE:
			case SDLK_q:  // Menu button again - close
				showPowerMenu_ = false;
				break;

			default:
				break;
		}
	}
}

void SDLEventManager::HandleDebugScreenInput(SDLKey key)
{
	switch (key) {
		case SDLK_u:  // UP
			debugScreenSelection_--;
			if (debugScreenSelection_ < 0) debugScreenSelection_ = 2;
			break;

		case SDLK_d:  // DOWN
			debugScreenSelection_++;
			if (debugScreenSelection_ > 2) debugScreenSelection_ = 0;
			break;

		case SDLK_a:  // A button - select
		case SDLK_RETURN:
			if (debugScreenSelection_ == 0) {
				// Audio Input Monitor - TODO: implement
				Trace::Log("DEBUG", "Audio Input Monitor selected");
			} else if (debugScreenSelection_ == 1) {
				// System Info - TODO: implement
				Trace::Log("DEBUG", "System Info selected");
			} else if (debugScreenSelection_ == 2) {
				// Exit Debug
				showDebugScreen_ = false;
				debugScreenSelection_ = 0;
			}
			break;

		case SDLK_b:  // B button - back
		case SDLK_ESCAPE:
			showDebugScreen_ = false;
			debugScreenSelection_ = 0;
			break;

		default:
			break;
	}
}

void SDLEventManager::RenderDebugScreen(SDL_Surface *screen)
{
	if (!showDebugScreen_ || !screen) return;

	// Fill entire screen with dark background
	SDL_Rect fullScreen = {0, 0, screen->w, screen->h};
	SDL_FillRect(screen, &fullScreen, SDL_MapRGB(screen->format, 20, 20, 20));

	// Menu box
	int menuWidth = 200;
	int menuHeight = 140;
	SDL_Rect menuBox = {
		(screen->w - menuWidth) / 2,
		(screen->h - menuHeight) / 2,
		menuWidth,
		menuHeight
	};
	SDL_FillRect(screen, &menuBox, SDL_MapRGB(screen->format, 240, 240, 240));

	// Title bar
	SDL_Rect titleBar = {menuBox.x, menuBox.y, menuWidth, 30};
	SDL_FillRect(screen, &titleBar, SDL_MapRGB(screen->format, 60, 60, 60));

	// Menu items
	int itemHeight = 25;
	int itemY = menuBox.y + 40;

	const char* items[3] = {
		"Audio Input Monitor",
		"System Info",
		"Exit Debug"
	};

	for (int i = 0; i < 3; i++) {
		SDL_Rect itemRect = {menuBox.x + 10, itemY + i * (itemHeight + 5), menuWidth - 20, itemHeight};
		Uint32 itemColor = (debugScreenSelection_ == i) ?
			SDL_MapRGB(screen->format, 80, 120, 200) :
			SDL_MapRGB(screen->format, 200, 200, 200);
		SDL_FillRect(screen, &itemRect, itemColor);
	}
}
