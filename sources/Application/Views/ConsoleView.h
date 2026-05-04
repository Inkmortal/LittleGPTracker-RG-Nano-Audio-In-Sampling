
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "BaseClasses/View.h"
#include "System/Console/Trace.h"

#define CONSOLE_WIDTH 40
#define CONSOLE_HEIGHT 25

class ConsoleView: public View,public Trace {
public:
	ConsoleView(GUIWindow &w,ViewData *viewData) ;

	// View implementation

	virtual void ProcessButtonMask(unsigned short mask,bool pressed);
	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType,unsigned int) {} ;
	virtual void OnFocus() {} ;
	virtual void CustomizeContextOverlay(const char *&name, const char *&where,
										 const char *&edit, const char *&field,
										 const char *&cmd1, const char *&cmd2,
										 const char *&cmd3, const char *&cmd4,
										 const char *&cmd5, const char *&cmd6,
										 const char *&cmd7) ;

	// Trace Implementation

	virtual void AddBuffer(char *buffer) ;

	// Additional

	bool IsDirty() ;

private:
	char lines_[CONSOLE_HEIGHT][CONSOLE_WIDTH] ;
	int currentLine_ ;
} ;

#endif
