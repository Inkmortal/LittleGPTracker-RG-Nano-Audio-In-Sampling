#ifndef _NULL_VIEW_H_
#define _NULL_VIEW_H_

#include "BaseClasses/View.h"
#include "ViewData.h"

class NullView: public View {
public:
	NullView(GUIWindow &w,ViewData *viewData) ;
	~NullView() ;
	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;
	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType ,unsigned int tick=0) ;
	virtual void OnFocus() ;
	virtual void CustomizeContextOverlay(const char *&name, const char *&where,
										 const char *&edit, const char *&field,
										 const char *&cmd1, const char *&cmd2,
										 const char *&cmd3, const char *&cmd4,
										 const char *&cmd5, const char *&cmd6,
										 const char *&cmd7) ;
private:

} ;
#endif

