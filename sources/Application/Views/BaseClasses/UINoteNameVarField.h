#ifndef _UI_NOTE_NAME_VAR_FIELD_H_
#define _UI_NOTE_NAME_VAR_FIELD_H_

#include "UIIntVarField.h"

class UINoteNameVarField: public UIIntVarField {

public:
	UINoteNameVarField(GUIPoint &position,Variable &v,Variable *noteMode,const char *format,int min,int max,int xOffset,int yOffset) ;
	virtual void Draw(GUIWindow &w,int offset=0) ;

private:
	Variable *noteMode_;
} ;

#endif
