#include "NullView.h"

NullView::NullView(GUIWindow &w,ViewData *viewData):View(w,viewData) {
}

NullView::~NullView() {
} 

void NullView::ProcessButtonMask(unsigned short mask,bool pressed) {

} ;

void NullView::CustomizeContextOverlay(
	const char *&name, const char *&where, const char *&edit,
	const char *&field, const char *&cmd1, const char *&cmd2,
	const char *&cmd3, const char *&cmd4, const char *&cmd5,
	const char *&cmd6, const char *&cmd7) {
	name="STARTUP";
	where="Project picker opens";
	edit="Wait/load project";
	field="Startup holding screen";
	cmd1="Project picker is next";
	cmd2="Power opens menu";
	cmd3="R+Select helper";
	cmd4="No song loaded yet";
	cmd5="Load or create song";
	cmd6="Then edit tracks";
	cmd7="R+Select close";
}

void NullView::DrawView() {

	Clear() ;

	// Calculate screen dimensions dynamically (30x30 chars for RG Nano, 40x30 for standard)
	GUIRect rect = w_.GetRect();
	int screenWidth = rect.Width() / 8;
	int screenHeight = rect.Height() / 8;

	GUITextProperties props;
	SetColor(CD_HILITE2) ;

	char buildString[80] ;
	sprintf(buildString,"Piggy build %s.%s.%s",PROJECT_NUMBER,PROJECT_RELEASE,BUILD_COUNT) ;
	GUIPoint pos ;
	pos._y = screenHeight - 2;  // Position 2 rows from bottom
	pos._x = (screenWidth - strlen(buildString)) / 2;
	DrawString(pos._x,pos._y,buildString,props) ;

} ;

void NullView::OnPlayerUpdate(PlayerEventType ,unsigned int tick) {

} ;

void NullView::OnFocus() {
} ;
