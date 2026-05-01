#define SDL_MAIN_HANDLED
#include "Application/Application.h"
#include "Adapters/RGNANO_SIM/System/RGNanoSimSystem.h"
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"

#ifdef main
#undef main
#endif

int main(int argc,char *argv[])
{
	RGNanoSimSystem::Boot(argc,argv) ;

	SDLCreateWindowParams params ;
	params.title="LittleGPTracker RG Nano Simulator" ;
	params.cacheFonts_=true ;
	params.framebuffer_=false ;

	Application::GetInstance()->Init(params) ;

	int retval=RGNanoSimSystem::MainLoop() ;
	RGNanoSimSystem::Shutdown() ;
	return retval ;
}
