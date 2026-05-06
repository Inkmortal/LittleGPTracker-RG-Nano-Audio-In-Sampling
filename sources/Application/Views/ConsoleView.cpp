
#include "ConsoleView.h"
#include <string.h>

ConsoleView::ConsoleView(GUIWindow &w,ViewData *viewData):View(w,viewData) {
	currentLine_=0 ;
	for (int i=0;i<CONSOLE_HEIGHT;i++) {
		lines_[i][0]=0 ;
	}
	isDirty_=true ;
} ;

void ConsoleView::ProcessButtonMask(unsigned short mask,bool pressed) {
};

void ConsoleView::CustomizeContextOverlay(
	const char *&name, const char *&where, const char *&edit,
	const char *&field, const char *&cmd1, const char *&cmd2,
	const char *&cmd3, const char *&cmd4, const char *&cmd5,
	const char *&cmd6, const char *&cmd7) {
	name="CONSOLE";
	where="Diagnostic log";
	edit="Read only";
	field="Debug console";
	cmd1="RB+Select close";
	cmd2="Up/Down helper page";
	cmd3="No song edits here";
	cmd4="Power app menu";
	cmd5="Logs are internal";
	cmd6="Return via app flow";
	cmd7="RB+Select helper";
}

void ConsoleView::DrawView() {

	GUITextProperties props ;

	SetColor(CD_NORMAL) ;
	GUIPoint pos(0,0) ;	
	for (int i=0;i<CONSOLE_HEIGHT;i++) {
		w_.DrawString(lines_[(currentLine_+i)%CONSOLE_HEIGHT],pos,props) ;
		pos._y+=8 ;
	}
} ;

bool ConsoleView::IsDirty() {
	return isDirty_ ;
} ;

void ConsoleView::AddBuffer(char *buffer) 
{
} ;
