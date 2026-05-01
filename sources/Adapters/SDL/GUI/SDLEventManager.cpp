
#include "SDLEventManager.h"
#include "Application/Application.h"
#include "Application/AppWindow.h"
#include "Application/Instruments/InstrumentBank.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "System/FileSystem/FileSystem.h"
#include "UIFramework/BasicDatas/GUIEvent.h"
#include "SDLGUIWindowImp.h"
#include "Application/Model/Config.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "Application/Views/BaseClasses/ViewEvent.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>

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
	simMouseKey_=0;
	simScriptActive_=false;
	simScriptFailed_=false;
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
#ifdef PLATFORM_RGNANO_SIM
	const char *skin=Config::GetInstance()->GetValue("RGNANOSIM_SKIN") ;
	if ((skin)&&(!strcmp(skin,"YES"))) {
		SDL_ShowCursor(SDL_ENABLE);
	}
#endif
	
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
#ifdef PLATFORM_RGNANO_SIM
			if (HandleSimMouse(sdlWindow,event)) {
				continue;
			}
#endif
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
#ifdef PLATFORM_RGNANO_SIM
					if (sdlWindow->IsRGNanoSkinEnabled()) {
						sdlWindow->SetRGNanoButtonPressed((int)event.key.keysym.sym,true);
					}
#endif
					break ;

				case SDL_KEYUP:
					if (dumpEvent_) 
          {
						Trace::Log("EVENT","key(%s):%d",keyname_[event.key.keysym.sym],0) ;
					}
					keyboardCS_->SetKey((int)event.key.keysym.sym,false) ;
#ifdef PLATFORM_RGNANO_SIM
					if (sdlWindow->IsRGNanoSkinEnabled()) {
						sdlWindow->SetRGNanoButtonPressed((int)event.key.keysym.sym,false);
					}
#endif
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
#ifdef PLATFORM_RGNANO_SIM
	return simScriptFailed_ ? 1 : 0;
#else
	return 0 ;
#endif
} ;

#ifdef PLATFORM_RGNANO_SIM
#include "Services/Audio/AudioDriver.h"

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
		command.arg="";
		command.arg2="";
		command.arg3="";
		command.value=0;
		command.value2=0;
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
		} else if (command.op=="down" || command.op=="up" || command.op=="screenshot" || command.op=="log" || command.op=="expect_file" || command.op=="expect_project_sample" || command.op=="expect_view") {
			iss >> command.arg;
		} else if (command.op=="expect_screen_text") {
			std::getline(iss,command.arg);
			if (!command.arg.empty() && command.arg[0]==' ') {
				command.arg.erase(0,1);
			}
		} else if (command.op=="click") {
			iss >> command.value >> command.value2 >> command.arg;
			if (command.arg.empty()) {
				command.arg="80";
			}
		} else if (command.op=="expect_log") {
			iss >> command.arg;
			std::getline(iss,command.arg2);
			if (!command.arg2.empty() && command.arg2[0]==' ') {
				command.arg2.erase(0,1);
			}
		} else if (command.op=="expect_no_error" || command.op=="reset_audio_stats" || command.op=="end_audio_capture") {
		} else if (command.op=="expect_colors") {
			iss >> command.value;
		} else if (command.op=="expect_audio_activity") {
			iss >> command.value;
		} else if (command.op=="start_audio_capture") {
			iss >> command.arg;
		} else if (command.op=="expect_audio_capture_bytes") {
			iss >> command.value;
		} else if (command.op=="expect_song_chain" || command.op=="expect_chain_phrase") {
			iss >> command.value >> command.value2 >> command.arg;
		} else if (command.op=="expect_phrase_row_count") {
			iss >> command.value >> command.value2;
		} else if (command.op=="sim_set_tempo") {
			iss >> command.value;
		} else if (command.op=="sim_import_sample_to_instrument") {
			iss >> command.value >> command.arg;
		} else if (command.op=="sim_set_song_chain") {
			iss >> command.value >> command.value2 >> command.arg;
		} else if (command.op=="sim_set_chain_phrase") {
			iss >> command.value >> command.value2 >> command.arg >> command.arg2;
		} else if (command.op=="sim_set_phrase_note") {
			iss >> command.value >> command.value2 >> command.arg >> command.arg2;
		} else if (command.op=="expect_size") {
			iss >> command.value >> command.value2;
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
		SetSimKey(window,simPendingReleaseKey_,false);
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
			SetSimKey(window,key,true);
			simPendingReleaseKey_=key;
			simNextCommandTime_=now+(unsigned long)command.value;
			Trace::Log("RGNANO_SIM","press %s",command.arg.c_str());
			return;
		}
		FailSimScript("unknown key in press command");
		return;
	} else if (command.op=="down") {
		int key=GetKeyCode(command.arg.c_str());
		if (key>0) {
			SetSimKey(window,key,true);
		} else {
			FailSimScript("unknown key in down command");
			return;
		}
	} else if (command.op=="up") {
		int key=GetKeyCode(command.arg.c_str());
		if (key>0) {
			SetSimKey(window,key,false);
		} else {
			FailSimScript("unknown key in up command");
			return;
		}
	} else if (command.op=="click") {
		int key=GetSimButtonAt(command.value,command.value2);
		if (key>0) {
			SetSimKey(window,key,true);
			simPendingReleaseKey_=key;
			simNextCommandTime_=now+(unsigned long)atoi(command.arg.c_str());
			Trace::Log("RGNANO_SIM","click %d %d",command.value,command.value2);
			return;
		}
		FailSimScript("click missed simulated button");
		return;
	} else if (command.op=="screenshot") {
		SaveSimScreenshot(window,command.arg);
	} else if (command.op=="quit") {
		PostQuitMessage();
	} else if (command.op=="expect_file") {
		if (!ExpectSimFile(command.arg)) {
			FailSimScript("file assertion failed");
			return;
		}
	} else if (command.op=="expect_project_sample") {
		if (!ExpectSimProjectSample(command.arg)) {
			FailSimScript("project sample assertion failed");
			return;
		}
	} else if (command.op=="expect_log") {
		std::string needle=command.arg;
		if (!command.arg2.empty()) {
			needle+=" "+command.arg2;
		}
		if (!ExpectSimLog(needle)) {
			FailSimScript("log assertion failed");
			return;
		}
	} else if (command.op=="expect_no_error") {
		if (!ExpectSimNoError()) {
			FailSimScript("log error assertion failed");
			return;
		}
	} else if (command.op=="reset_audio_stats") {
		AudioDriver::ResetSimAudioStats();
		Trace::Log("RGNANO_SIM","reset_audio_stats");
	} else if (command.op=="start_audio_capture") {
		if (!AudioDriver::BeginSimAudioCapture(command.arg.c_str())) {
			FailSimScript("audio capture start failed");
			return;
		}
	} else if (command.op=="end_audio_capture") {
		AudioDriver::EndSimAudioCapture();
	} else if (command.op=="expect_view") {
		if (!ExpectSimView(command.arg)) {
			FailSimScript("view assertion failed");
			return;
		}
	} else if (command.op=="expect_audio_activity") {
		if (!ExpectSimAudioActivity(command.value)) {
			FailSimScript("audio activity assertion failed");
			return;
		}
	} else if (command.op=="expect_audio_capture_bytes") {
		if (!ExpectSimAudioCaptureBytes(command.value)) {
			FailSimScript("audio capture assertion failed");
			return;
		}
	} else if (command.op=="expect_screen_text") {
		if (!ExpectSimScreenText(command.arg)) {
			FailSimScript("screen text assertion failed");
			return;
		}
	} else if (command.op=="expect_song_chain") {
		if (!ExpectSimSongChain(command.value,command.value2,command.arg)) {
			FailSimScript("song chain assertion failed");
			return;
		}
	} else if (command.op=="expect_chain_phrase") {
		if (!ExpectSimChainPhrase(command.value,command.value2,command.arg)) {
			FailSimScript("chain phrase assertion failed");
			return;
		}
	} else if (command.op=="expect_phrase_row_count") {
		if (!ExpectSimPhraseRowCount(command.value,command.value2)) {
			FailSimScript("phrase row count assertion failed");
			return;
		}
	} else if (command.op=="sim_set_tempo") {
		if (!SimSetTempo(command.value)) {
			FailSimScript("tempo setup failed");
			return;
		}
	} else if (command.op=="sim_import_sample_to_instrument") {
		if (!SimImportSampleToInstrument(command.value,command.arg)) {
			FailSimScript("sample import setup failed");
			return;
		}
	} else if (command.op=="sim_set_song_chain") {
		if (!SimSetSongChain(command.value,command.value2,atoi(command.arg.c_str()))) {
			FailSimScript("song setup failed");
			return;
		}
	} else if (command.op=="sim_set_chain_phrase") {
		if (!SimSetChainPhrase(command.value,command.value2,atoi(command.arg.c_str()),atoi(command.arg2.c_str()))) {
			FailSimScript("chain setup failed");
			return;
		}
	} else if (command.op=="sim_set_phrase_note") {
		if (!SimSetPhraseNote(command.value,command.value2,atoi(command.arg.c_str()),atoi(command.arg2.c_str()))) {
			FailSimScript("phrase setup failed");
			return;
		}
	} else if (command.op=="expect_colors") {
		if (!ExpectSimScreenColors(window,command.value)) {
			FailSimScript("screen color assertion failed");
			return;
		}
	} else if (command.op=="expect_size") {
		if (!ExpectSimScreenSize(window,command.value,command.value2)) {
			FailSimScript("screen size assertion failed");
			return;
		}
	} else if (command.op=="log") {
		Trace::Log("RGNANO_SIM","%s",command.arg.c_str());
	} else {
		FailSimScript("unknown script command");
		return;
	}

	simCommandIndex_++;
	simNextCommandTime_=now+40;
}

bool SDLEventManager::HandleSimMouse(SDLGUIWindowImp *window, SDL_Event &event)
{
	if (!window || !window->IsRGNanoSkinEnabled()) {
		return false;
	}
	if (event.type==SDL_MOUSEBUTTONDOWN) {
		int key=GetSimButtonAt(event.button.x,event.button.y);
		if (key>0) {
			keyboardCS_->SetKey(key,true);
			window->SetRGNanoButtonPressed(key,true);
			simMouseKey_=key;
			Trace::Log("RGNANO_SIM","mouse down key:%d",key);
			return true;
		}
	} else if (event.type==SDL_MOUSEBUTTONUP) {
		if (simMouseKey_>0) {
			keyboardCS_->SetKey(simMouseKey_,false);
			window->SetRGNanoButtonPressed(simMouseKey_,false);
			Trace::Log("RGNANO_SIM","mouse up key:%d",simMouseKey_);
			simMouseKey_=0;
			return true;
		}
	}
	return false;
}

void SDLEventManager::SetSimKey(SDLGUIWindowImp *window, int key, bool pressed)
{
	keyboardCS_->SetKey(key,pressed);
	if (window && window->IsRGNanoSkinEnabled()) {
		window->SetRGNanoButtonPressed(key,pressed);
	}
}

int SDLEventManager::GetSimButtonAt(int x, int y)
{
	struct ButtonRect {
		int x;
		int y;
		int w;
		int h;
		int key;
	};
	ButtonRect buttons[] = {
		{76,374,28,28,SDLK_u},
		{76,434,28,28,SDLK_d},
		{46,404,28,28,SDLK_l},
		{106,404,28,28,SDLK_r},
		{250,444,30,30,SDLK_a},
		{286,408,30,30,SDLK_b},
		{214,408,30,30,SDLK_x},
		{250,372,30,30,SDLK_y},
		{88,498,62,18,SDLK_s},
		{210,498,62,18,SDLK_q},
		{18,16,88,22,SDLK_m},
		{18,16,22,42,SDLK_m},
		{254,16,88,22,SDLK_n},
		{320,16,22,42,SDLK_n}
	};
	for (int i=0;i<(int)(sizeof(buttons)/sizeof(buttons[0]));i++) {
		if (x>=buttons[i].x && x<buttons[i].x+buttons[i].w &&
			y>=buttons[i].y && y<buttons[i].y+buttons[i].h) {
			return buttons[i].key;
		}
	}
	return 0;
}

void SDLEventManager::FailSimScript(const char *message)
{
	simScriptFailed_=true;
	Trace::Error("RGNANO_SIM %s",message);
	PostQuitMessage();
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

bool SDLEventManager::ExpectSimFile(const std::string &path)
{
	if (path.empty()) {
		Trace::Error("RGNANO_SIM expect_file missing path");
		return false;
	}
	std::ifstream file(path.c_str(),std::ios::binary);
	bool exists=file.good();
	Trace::Log("RGNANO_SIM","expect_file %s => %s",path.c_str(),exists?"exists":"missing");
	return exists;
}

bool SDLEventManager::ExpectSimProjectSample(const std::string &sampleName)
{
	if (sampleName.empty()) {
		Trace::Error("RGNANO_SIM expect_project_sample missing sample name");
		return false;
	}
	std::string aliasPath="samples:";
	aliasPath+=sampleName;
	Path samplePath(aliasPath.c_str());
	Path resolvedPath(samplePath.GetPath());
	bool exists=resolvedPath.Exists();
	Trace::Log("RGNANO_SIM","expect_project_sample %s => %s (%s)",sampleName.c_str(),exists?"exists":"missing",resolvedPath.GetPath().c_str());
	return exists;
}

bool SDLEventManager::ExpectSimLog(const std::string &needle)
{
	const char *logPath=Config::GetInstance()->GetValue("RGNANOSIM_LOG");
	if (!logPath) {
		logPath="rgnano-sim.log";
	}
	std::ifstream file(logPath);
	if (!file.is_open()) {
		Trace::Error("RGNANO_SIM expect_log failed to open %s",logPath);
		return false;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string haystack=buffer.str();
	bool found=haystack.find(needle)!=std::string::npos;
	Trace::Log("RGNANO_SIM","expect_log %s in %s => %s",needle.c_str(),logPath,found?"found":"missing");
	return found;
}

bool SDLEventManager::ExpectSimNoError()
{
	const char *logPath=Config::GetInstance()->GetValue("RGNANOSIM_LOG");
	if (!logPath) {
		logPath="rgnano-sim.log";
	}
	std::ifstream file(logPath);
	if (!file.is_open()) {
		Trace::Error("RGNANO_SIM expect_no_error failed to open %s",logPath);
		return false;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string haystack=buffer.str();
	bool clean=haystack.find("[*ERROR*]")==std::string::npos && haystack.find("ERROR") == std::string::npos;
	Trace::Log("RGNANO_SIM","expect_no_error in %s => %s",logPath,clean?"clean":"error found");
	return clean;
}

bool SDLEventManager::ExpectSimView(const std::string &viewName)
{
	GUIWindow *guiWindow=Application::GetInstance()->GetWindow();
	AppWindow *appWindow=(AppWindow *)guiWindow;
	const char *current=appWindow ? appWindow->GetCurrentViewName() : "unknown";
	bool matches=viewName==current;
	Trace::Log("RGNANO_SIM","expect_view %s => %s",viewName.c_str(),current);
	if (!matches) {
		Trace::Error("RGNANO_SIM expected view %s but found %s",viewName.c_str(),current);
	}
	return matches;
}

bool SDLEventManager::ExpectSimAudioActivity(int minPeak)
{
	if (minPeak<=0) {
		minPeak=64;
	}
	int peak=AudioDriver::GetSimAudioPeak();
	unsigned long nonSilentBytes=AudioDriver::GetSimAudioNonSilentBytes();
	bool active=peak>=minPeak && nonSilentBytes>0;
	Trace::Log("RGNANO_SIM","expect_audio_activity peak=%d nonSilentBytes=%lu minPeak=%d => %s",peak,nonSilentBytes,minPeak,active?"active":"silent");
	if (!active) {
		Trace::Error("RGNANO_SIM audio activity assertion failed");
	}
	return active;
}

bool SDLEventManager::ExpectSimAudioCaptureBytes(int minBytes)
{
	if (minBytes<=0) {
		minBytes=44;
	}
	unsigned long bytes=AudioDriver::GetSimAudioCaptureBytes();
	bool captured=bytes>=(unsigned long)minBytes;
	Trace::Log("RGNANO_SIM","expect_audio_capture_bytes bytes=%lu minBytes=%d => %s",bytes,minBytes,captured?"captured":"short");
	if (!captured) {
		Trace::Error("RGNANO_SIM audio capture assertion failed");
	}
	return captured;
}

bool SDLEventManager::ExpectSimScreenText(const std::string &needle)
{
	GUIWindow *guiWindow=Application::GetInstance()->GetWindow();
	AppWindow *appWindow=(AppWindow *)guiWindow;
	bool found=appWindow && appWindow->ScreenContains(needle.c_str());
	Trace::Log("RGNANO_SIM","expect_screen_text %s => %s",needle.c_str(),found?"found":"missing");
	if (!found) {
		Trace::Error("RGNANO_SIM expected screen text %s",needle.c_str());
	}
	return found;
}

static bool ParseSimExpectedByte(const std::string &expected, unsigned char actual)
{
	if (expected=="any") {
		return actual!=0xFF;
	}
	char *end=0;
	long value=strtol(expected.c_str(),&end,16);
	return end && *end==0 && value>=0 && value<=0xFF && actual==(unsigned char)value;
}

bool SDLEventManager::ExpectSimSongChain(int row, int channel, const std::string &expected)
{
	GUIWindow *guiWindow=Application::GetInstance()->GetWindow();
	AppWindow *appWindow=(AppWindow *)guiWindow;
	ViewData *viewData=appWindow ? appWindow->GetViewData() : 0;
	if (!viewData || !viewData->song_ || row<0 || row>=SONG_ROW_COUNT || channel<0 || channel>=SONG_CHANNEL_COUNT) {
		Trace::Error("RGNANO_SIM expect_song_chain invalid args row=%d channel=%d",row,channel);
		return false;
	}
	unsigned char actual=*(viewData->song_->data_ + channel + SONG_CHANNEL_COUNT * row);
	bool matches=ParseSimExpectedByte(expected,actual);
	Trace::Log("RGNANO_SIM","expect_song_chain row=%d channel=%d actual=%02X expected=%s => %s",row,channel,actual,expected.c_str(),matches?"match":"mismatch");
	return matches;
}

bool SDLEventManager::ExpectSimChainPhrase(int chain, int row, const std::string &expected)
{
	GUIWindow *guiWindow=Application::GetInstance()->GetWindow();
	AppWindow *appWindow=(AppWindow *)guiWindow;
	ViewData *viewData=appWindow ? appWindow->GetViewData() : 0;
	if (!viewData || !viewData->song_ || chain<0 || chain>=CHAIN_COUNT || row<0 || row>=16) {
		Trace::Error("RGNANO_SIM expect_chain_phrase invalid args chain=%d row=%d",chain,row);
		return false;
	}
	unsigned char actual=*(viewData->song_->chain_->data_ + 16 * chain + row);
	bool matches=ParseSimExpectedByte(expected,actual);
	Trace::Log("RGNANO_SIM","expect_chain_phrase chain=%d row=%d actual=%02X expected=%s => %s",chain,row,actual,expected.c_str(),matches?"match":"mismatch");
	return matches;
}

bool SDLEventManager::ExpectSimPhraseRowCount(int phrase, int minRows)
{
	GUIWindow *guiWindow=Application::GetInstance()->GetWindow();
	AppWindow *appWindow=(AppWindow *)guiWindow;
	ViewData *viewData=appWindow ? appWindow->GetViewData() : 0;
	if (!viewData || !viewData->song_ || phrase<0 || phrase>=PHRASE_COUNT || minRows<0) {
		Trace::Error("RGNANO_SIM expect_phrase_row_count invalid args phrase=%d minRows=%d",phrase,minRows);
		return false;
	}
	int count=0;
	for (int row=0; row<16; row++) {
		unsigned char note=*(viewData->song_->phrase_->note_ + 16 * phrase + row);
		unsigned char instr=*(viewData->song_->phrase_->instr_ + 16 * phrase + row);
		if (note!=0xFF && instr!=0xFF) {
			count++;
		}
	}
	bool matches=count>=minRows;
	Trace::Log("RGNANO_SIM","expect_phrase_row_count phrase=%d count=%d min=%d => %s",phrase,count,minRows,matches?"match":"mismatch");
	return matches;
}

static ViewData *GetSimViewData()
{
	GUIWindow *guiWindow=Application::GetInstance()->GetWindow();
	AppWindow *appWindow=(AppWindow *)guiWindow;
	return appWindow ? appWindow->GetViewData() : 0;
}

bool SDLEventManager::SimSetTempo(int bpm)
{
	ViewData *viewData=GetSimViewData();
	if (!viewData || !viewData->project_) {
		Trace::Error("RGNANO_SIM sim_set_tempo has no project");
		return false;
	}
	Variable *tempo=viewData->project_->FindVariable(VAR_TEMPO);
	if (!tempo) {
		Trace::Error("RGNANO_SIM sim_set_tempo missing tempo variable");
		return false;
	}
	tempo->SetInt(bpm);
	Trace::Log("RGNANO_SIM","sim_set_tempo %d",bpm);
	return true;
}

bool SDLEventManager::SimImportSampleToInstrument(int instrument, const std::string &sampleName)
{
	ViewData *viewData=GetSimViewData();
	if (!viewData || !viewData->project_ || instrument<0 || instrument>=MAX_SAMPLEINSTRUMENT_COUNT || sampleName.empty()) {
		Trace::Error("RGNANO_SIM sim_import_sample_to_instrument invalid args instrument=%d sample=%s",instrument,sampleName.c_str());
		return false;
	}
	std::string aliasPath="./rgnano-sim-data/samples/";
	aliasPath+=sampleName;
	Path samplePath(aliasPath.c_str());
	Path resolvedPath(samplePath.GetPath());
	if (!resolvedPath.Exists()) {
		Trace::Error("RGNANO_SIM sim_import_sample_to_instrument missing %s (%s)",sampleName.c_str(),resolvedPath.GetPath().c_str());
		return false;
	}
	int sampleIndex=SamplePool::GetInstance()->ImportSample(samplePath);
	if (sampleIndex<0) {
		return false;
	}
	InstrumentBank *bank=viewData->project_->GetInstrumentBank();
	SampleInstrument *sampleInstrument=(SampleInstrument *)bank->GetInstrument(instrument);
	sampleInstrument->AssignSample(sampleIndex);
	Trace::Log("RGNANO_SIM","sim_import_sample_to_instrument inst=%d sample=%s index=%d",instrument,sampleName.c_str(),sampleIndex);
	return true;
}

bool SDLEventManager::SimSetSongChain(int row, int channel, int chain)
{
	ViewData *viewData=GetSimViewData();
	if (!viewData || !viewData->song_ || row<0 || row>=SONG_ROW_COUNT || channel<0 || channel>=SONG_CHANNEL_COUNT || chain<0 || chain>=CHAIN_COUNT) {
		Trace::Error("RGNANO_SIM sim_set_song_chain invalid args row=%d channel=%d chain=%d",row,channel,chain);
		return false;
	}
	*(viewData->song_->data_ + channel + SONG_CHANNEL_COUNT * row)=(unsigned char)chain;
	viewData->song_->chain_->SetUsed((unsigned char)chain);
	Trace::Log("RGNANO_SIM","sim_set_song_chain row=%d channel=%d chain=%02X",row,channel,chain);
	return true;
}

bool SDLEventManager::SimSetChainPhrase(int chain, int row, int phrase, int transpose)
{
	ViewData *viewData=GetSimViewData();
	if (!viewData || !viewData->song_ || chain<0 || chain>=CHAIN_COUNT || row<0 || row>=16 || phrase<0 || phrase>=PHRASE_COUNT || transpose<0 || transpose>255) {
		Trace::Error("RGNANO_SIM sim_set_chain_phrase invalid args chain=%d row=%d phrase=%d transpose=%d",chain,row,phrase,transpose);
		return false;
	}
	*(viewData->song_->chain_->data_ + 16 * chain + row)=(unsigned char)phrase;
	*(viewData->song_->chain_->transpose_ + 16 * chain + row)=(unsigned char)transpose;
	viewData->song_->chain_->SetUsed((unsigned char)chain);
	viewData->song_->phrase_->SetUsed((unsigned char)phrase);
	Trace::Log("RGNANO_SIM","sim_set_chain_phrase chain=%02X row=%d phrase=%02X transpose=%02X",chain,row,phrase,transpose);
	return true;
}

bool SDLEventManager::SimSetPhraseNote(int phrase, int row, int note, int instrument)
{
	ViewData *viewData=GetSimViewData();
	if (!viewData || !viewData->song_ || phrase<0 || phrase>=PHRASE_COUNT || row<0 || row>=16 || note<0 || note>119 || instrument<0 || instrument>=MAX_INSTRUMENT_COUNT) {
		Trace::Error("RGNANO_SIM sim_set_phrase_note invalid args phrase=%d row=%d note=%d instrument=%d",phrase,row,note,instrument);
		return false;
	}
	*(viewData->song_->phrase_->note_ + 16 * phrase + row)=(unsigned char)note;
	*(viewData->song_->phrase_->instr_ + 16 * phrase + row)=(unsigned char)instrument;
	viewData->song_->phrase_->SetUsed((unsigned char)phrase);
	Trace::Log("RGNANO_SIM","sim_set_phrase_note phrase=%02X row=%d note=%d instrument=%02X",phrase,row,note,instrument);
	return true;
}

bool SDLEventManager::ExpectSimScreenSize(SDLGUIWindowImp *window, int width, int height)
{
	if (!window) {
		Trace::Error("RGNANO_SIM expect_size has no window");
		return false;
	}
	SDL_Surface *surface=window->GetSurface();
	if (!surface) {
		Trace::Error("RGNANO_SIM expect_size has no screen surface");
		return false;
	}
	int appSurfaceWidth=surface->w;
	int appSurfaceHeight=surface->h;
	if (window->IsRGNanoSkinEnabled()) {
		appSurfaceWidth=240*window->GetScale();
		appSurfaceHeight=240*window->GetScale();
	}
	Trace::Log("RGNANO_SIM","expect_size found %dx%d app surface in %dx%d window, expected %dx%d",appSurfaceWidth,appSurfaceHeight,surface->w,surface->h,width,height);
	return appSurfaceWidth==width && appSurfaceHeight==height;
}

bool SDLEventManager::ExpectSimScreenColors(SDLGUIWindowImp *window, int minColors)
{
	if (!window) {
		Trace::Error("RGNANO_SIM expect_colors has no window");
		return false;
	}
	if (minColors<=0) {
		minColors=2;
	}
	window->Flush();
	int colorCount=CountSurfaceColors(window->GetSurface(),minColors);
	Trace::Log("RGNANO_SIM","expect_colors found %d colors, expected at least %d",colorCount,minColors);
	if (colorCount<minColors) {
		Trace::Error("RGNANO_SIM screen color assertion failed");
		return false;
	}
	return true;
}

int SDLEventManager::CountSurfaceColors(SDL_Surface *surface, int maxColors)
{
	if (!surface || maxColors<=0) {
		return 0;
	}
	std::vector<Uint32> colors;
	if (SDL_MUSTLOCK(surface)) {
		if (SDL_LockSurface(surface)<0) {
			Trace::Error("RGNANO_SIM failed to lock screen surface");
			return 0;
		}
	}
	for (int y=0;y<surface->h;y++) {
		for (int x=0;x<surface->w;x++) {
			Uint32 pixel=ReadSurfacePixel(surface,x,y);
			bool seen=false;
			for (size_t i=0;i<colors.size();i++) {
				if (colors[i]==pixel) {
					seen=true;
					break;
				}
			}
			if (!seen) {
				colors.push_back(pixel);
				if ((int)colors.size()>=maxColors) {
					if (SDL_MUSTLOCK(surface)) {
						SDL_UnlockSurface(surface);
					}
					return (int)colors.size();
				}
			}
		}
	}
	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}
	return (int)colors.size();
}

Uint32 SDLEventManager::ReadSurfacePixel(SDL_Surface *surface, int x, int y)
{
	int bpp=surface->format->BytesPerPixel;
	Uint8 *p=(Uint8 *)surface->pixels + y*surface->pitch + x*bpp;
	switch (bpp) {
		case 1: return *p;
		case 2: return *(Uint16 *)p;
		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				return p[0]<<16 | p[1]<<8 | p[2];
			}
			return p[0] | p[1]<<8 | p[2]<<16;
		default: return *(Uint32 *)p;
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
