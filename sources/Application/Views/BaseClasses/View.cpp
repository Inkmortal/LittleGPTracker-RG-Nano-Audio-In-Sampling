#include "View.h"
#include "System/Console/Trace.h"
#include "Application/Player/Player.h"
#include "Application/Utils/char.h"
#include "Application/AppWindow.h"
#include "Application/Model/Config.h"
#include "ModalView.h"

bool View::initPrivate_=false ;

int View::margin_=0 ;
int View::songRowCount_; //=21 sets screen height among other things
bool View::miniLayout_=false ;
bool View::ultraCompactLayout_=false ;
int View::altRowNumber_ = 4;
int View::cursorAnimFrame_ = 0;

View::View(GUIWindow &w,ViewData *viewData):
	w_(w),
	modalView_(0),
	modalViewCallback_(0),
	hasFocus_(false)
{
  if (!initPrivate_)
  {
	   GUIRect rect=w.GetRect() ;
     miniLayout_=(rect.Width()<320);
     ultraCompactLayout_=(rect.Width()<=240 && rect.Height()<=240);
	   View::margin_=0 ;
		songRowCount_ = ultraCompactLayout_ ? 18 : (miniLayout_ ? 16 : 22); // RG Nano can fit more rows now

		const char *altRowStr = Config::GetInstance()->GetValue("ALTROWNUMBER");
		if (altRowStr) {
			altRowNumber_ = atoi(altRowStr);
		}

     initPrivate_=true ;
  }
	mask_=0 ;
	viewMode_=VM_NORMAL ;
	locked_=false ;
	viewData_=viewData;
	NOTIFICATION_TIMEOUT = 1000;
	displayNotification_ = "";
} ;

GUIPoint View::GetAnchor() {
	// Calculate width based on actual screen dimensions
	// Each character is 8 pixels wide, so width in chars = screen width in pixels / 8
	GUIRect rect = w_.GetRect();
	int width = rect.Width() / 8;   // 30 for RG Nano (240px), 40 for standard (320px)
	int height = rect.Height() / 8; // 30 for both RG Nano and standard (240px)

	int anchorX = (width-SONG_CHANNEL_COUNT*3)/2+2;

	// On mini layouts (no map), we have more width available
	// Shift left by 1 to use more screen space
	if (miniLayout_) {
		// 8 channels × 3 chars = 24 chars, plus 3 for row numbers = 27 total
		// On 240px (30 chars): shift slightly left for better use of space
		anchorX = ((width - SONG_CHANNEL_COUNT*3 - 3) / 2) + 3 - 1;
	}

#ifdef PLATFORM_RGNANO
	Trace::Log("RGNANO","GetAnchor: rect.Width()=%d, width=%d, anchorX=%d", rect.Width(), width, anchorX);
#endif

	return GUIPoint(anchorX, (height-View::songRowCount_)/2 - 1) ;
}

GUIPoint View::GetTitlePosition() {
#ifndef PLATFORM_CAANOO
	return GUIPoint(0,0) ;
#else
	return GUIPoint(0,1) ;
#endif
} ;

bool View::Lock() {
	if (locked_) return false ;
	locked_=true ;
	return true ;
} ;

void View::WaitForObject() {
	while (locked_) {} ;
}

void View::Unlock() {
	locked_=false ;
}

void View::drawMap() {
    if (!miniLayout_) {
        GUIPoint anchor=GetAnchor() ;
		GUIPoint pos(View::margin_,anchor._y);
    	GUITextProperties props ;

		//draw entire map
		SetColor(CD_HILITE1) ;
    	char buffer[5] ;
		props.invert_=true ;
		//row1
		sprintf(buffer,"P G ");
        DrawString(pos._x,pos._y,buffer,props) ;
		pos._y++ ;		
		//row2
		sprintf(buffer,"SCPI");
        DrawString(pos._x,pos._y,buffer,props) ;
		pos._y++ ;		
		//row3
		sprintf(buffer,"  TT");
        DrawString(pos._x,pos._y,buffer,props) ;

		//draw current screen on map
		SetColor(CD_HILITE2) ;
		pos._y = anchor._y;
		switch(viewType_)
		{
		case VT_CHAIN:
			pos._x+=1;
			pos._y+=1;
	        DrawString(pos._x,pos._y,"C",props) ;
			break;
		case VT_PHRASE:
			pos._x+=2;
			pos._y+=1;
	        DrawString(pos._x,pos._y,"P",props) ;
			break;
		case VT_PROJECT:
	        DrawString(pos._x,pos._y,"P",props) ;
			break;
		case VT_INSTRUMENT:
			pos._x+=3;
			pos._y+=1;
	        DrawString(pos._x,pos._y,"I",props) ;
			break;
		case VT_TABLE: //under phrase
			pos._x+=2;
			pos._y+=2;
	        DrawString(pos._x,pos._y,"T",props) ;
			break;
		case VT_TABLE2: //under instrument
			pos._x+=3;
			pos._y+=2;
	        DrawString(pos._x,pos._y,"T",props) ;
			break;
		case VT_GROOVE:
			pos._x+=2;
	        DrawString(pos._x,pos._y,"G",props) ;
			break;
		default: //VT_SONG
			pos._y+=1;
	        DrawString(pos._x,pos._y,"S",props) ;
			int foo=0;
		}

	}//!minilayout
}

void View::drawNotes() {

	GUIPoint anchor=GetAnchor() ;
		int initialX = anchor._x ;  // Align with song columns
		int initialY = anchor._y + View::songRowCount_ + 1 ;  // Just below song grid
		GUIPoint pos(initialX,initialY) ;
		GUITextProperties props ;

        Player *player=Player::GetInstance() ;
		
		//column banger refactor
		props.invert_= true;
        for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
			if (i==viewData_->songX_) {
				SetColor(CD_HILITE2) ;
			} else {
				SetColor(CD_HILITE1) ;
			}
			if (player->IsRunning() && viewData_->playMode_ != PM_AUDITION) {
				DrawString(pos._x,pos._y,player->GetPlayedNote(i),props) ; //row for the note values
				pos._y++ ;
				DrawString(pos._x,pos._y,player->GetPlayedOctive(i),props) ; //row for the octive values
				pos._y++ ;
				DrawString(pos._x,pos._y,player->GetPlayedInstrument(i),props) ; //draw instrument number
			} else {
				DrawString(pos._x,pos._y,"  ",props) ; //row for the note values
				pos._y++ ;
				DrawString(pos._x,pos._y,"  ",props) ; //row for the octive values
				pos._y++ ;
				DrawString(pos._x,pos._y,"  ",props) ; //draw instrument number
			}
			pos._y = initialY ;
			pos._x+= 3;
		}
}

void View::DoModal(ModalView *view,ModalViewCallback cb) {
	modalView_=view ;
	modalView_->OnFocus() ;
	modalViewCallback_=cb ;
	isDirty_=true ;
} ;

void View::Redraw() {
	if (modalView_) {
		if (isDirty_) {
			DrawView() ;
		}
		modalView_->Redraw() ;
	} else {
		DrawView() ;
	}
	isDirty_=false ;
} ;

void View::SetDirty(bool isDirty) {
	isDirty_=true ;
} ;

void View::ProcessButton(unsigned short mask, bool pressed) {
	isDirty_=false ;

	// Increment cursor animation frame
	cursorAnimFrame_++;
	if (cursorAnimFrame_ > 60) cursorAnimFrame_ = 0;

	if (modalView_) {
		modalView_->ProcessButton(mask,pressed);
		modalView_->isDirty_;
		if (modalView_->IsFinished()) {
			// process callback sending the modal dialog
			if (modalViewCallback_) {
				modalViewCallback_(*this,*modalView_) ;
			}
			SAFE_DELETE(modalView_) ;
			isDirty_=true ;
		}
	} else {
		ProcessButtonMask(mask,pressed);
	}
	if (isDirty_) ((AppWindow &)w_).SetDirty() ;
} ;

void View::OnPlayerUpdate(PlayerEventType type, unsigned int currentTick) {
	// Propagate player updates to modal view if one is active
	if (modalView_) {
		modalView_->OnPlayerUpdate(type, currentTick);
	}
	// Subclasses can override this and add their own logic
}

void View::Clear() {
	((AppWindow &)w_).Clear() ;
}

void View::SetColor(ColorDefinition cd) {
	((AppWindow &)w_).SetColor(cd) ;
} ;

void View::ClearRect(int x,int y,int w,int h) {
	GUIRect rect(x,y,(x+w),(y+h)) ;
	w_.ClearRect(rect) ;
} ;

void View::DrawString(int x,int y,const char *txt,GUITextProperties &props) {
	GUIPoint pos(x,y) ;
	w_.DrawString(txt,pos,props) ;
} ;

/*
	Displays the saved notification for 1 second
*/
void View::EnableNotification() {
	if ((SDL_GetTicks() - notificationTime_) <= NOTIFICATION_TIMEOUT) {
		SetColor(CD_NORMAL);
		GUITextProperties props;
        int xOffset = 4;
        DrawString(xOffset, notiDistY_, displayNotification_.c_str(), props);
    } else {
		displayNotification_ = "";
	}
}

/*
    Set displayed notification
    Saves the current time
    Optionally set display y offset if not in a project (default == 2)
    Allows negative offsets, use with care!
*/
void View::SetNotification(const char *notification, int offset) {
    notificationTime_ = SDL_GetTicks();
    displayNotification_ = notification;
    notiDistY_ = offset;
    isDirty_ = true;
}
