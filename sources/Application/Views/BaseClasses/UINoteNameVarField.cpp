#include "UINoteNameVarField.h"
#include "Application/AppWindow.h"
#include "Application/Utils/char.h"

UINoteNameVarField::UINoteNameVarField(GUIPoint &position,Variable &v,Variable *noteMode,const char *format,int min,int max,int xOffset,int yOffset)
               :UIIntVarField(position,v,format,min,max,xOffset,yOffset), noteMode_(noteMode) {
} ;

void UINoteNameVarField::Draw(GUIWindow &w,int offset) {

	GUITextProperties props ;
	GUIPoint position=GetPosition() ;
	position._y+=offset ;

	if (focus_) {
		((AppWindow&)w).SetColor(CD_HILITE2) ;
		props.invert_=true ;
	} else {
		((AppWindow&)w).SetColor(CD_NORMAL) ;
	}

	int mode = noteMode_ ? noteMode_->GetInt() : getNoteNameMode();
	char buffer[80] ;
	int note = src_.GetInt();
	const char *name = note<0 ? "--" : getNoteName(note,mode);
	sprintf(buffer,format_,name) ;
	w.DrawString(buffer,position,props) ;
} ;
