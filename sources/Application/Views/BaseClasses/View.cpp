#include "View.h"
#include "System/Console/Trace.h"
#include "Application/Player/Player.h"
#include "Application/Mixer/MixerService.h"
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
bool View::contextOverlay_ = false;
int View::contextOverlayPage_ = 0;

View::View(GUIWindow &w,ViewData *viewData):
	w_(w),
	modalView_(0),
	modalViewCallback_(0),
	hasFocus_(false)
{
	suppressPlaybackScope_=false;
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
	viewType_=VT_SONG ;
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

void View::drawContextOverlay() {
	GUITextProperties props;
	props.invert_=false;
	GUIRect rect=w_.GetRect();
	int width=rect.Width()/8;
	int height=rect.Height()/8;
	int x=ultraCompactLayout_ ? 1 : 6;
	int y=ultraCompactLayout_ ? 2 : 3;
	int boxW=ultraCompactLayout_ ? width-2 : width-12;
	int boxH=ultraCompactLayout_ ? height-4 : height-6;
	if (boxW<24) boxW=24;
	if (boxH<18) boxH=18;

	SetColor(CD_BACKGROUND);
	ClearRect(x,y,boxW,boxH);
	SetColor(CD_BORDER);
	for (int i=0;i<boxW;i++) {
		DrawString(x+i,y,"-",props);
		DrawString(x+i,y+boxH-1,"-",props);
	}
	for (int j=0;j<boxH;j++) {
		DrawString(x,y+j,"|",props);
		DrawString(x+boxW-1,y+j,"|",props);
	}
	DrawString(x,y,"+",props);
	DrawString(x+boxW-1,y,"+",props);
	DrawString(x,y+boxH-1,"+",props);
	DrawString(x+boxW-1,y+boxH-1,"+",props);

	const char *name="SONG";
	const char *where="R+Right Chain";
	const char *edit="A+Dpad edit";
	const char *field="Rows=time Cols=tracks";
	const char *cmd1="Dpad move cursor";
	const char *cmd2="A paste/new item";
	const char *cmd3="A+Dpad edit value";
	const char *cmd4="Start play";
	const char *cmd5="A+R solo  B+R mute";
	const char *cmd6="R+Dpad change view";
	const char *cmd7="R+Select helper";
	switch(viewType_) {
		case VT_SONG:
			name="SONG";
			where="R+Right Chain R+Down Mix";
			edit="A+Dpad chain Start play";
			field="Rows=time Cols=tracks";
			cmd1="Dpad move row/track";
			cmd2="A paste/new chain";
			cmd3="A+Dpad edit chain";
			cmd4="Start play song row";
			cmd5="L+Dpad jump/tempo";
			cmd6="A+R solo  B+R mute";
			cmd7="R+Right Chain";
			break;
		case VT_CHAIN:
			name="CHAIN";
			where="R+Left Song R+Right Phr";
			edit="A+Dpad phrase/trans";
			field="Rows=phrase playlist";
			cmd1="Dpad move row/field";
			cmd2="A paste/new phrase";
			cmd3="A+Dpad edit value";
			cmd4="Start play cur ch";
			cmd5="R+Start song ctx";
			cmd6="A+R solo  B+R mute";
			cmd7="R+Right Phrase";
			break;
		case VT_PHRASE:
			name="PHRASE";
			where="R+Left Chain R+Right Inst";
			edit="A+Dpad notes/cmds";
			field="Rows=steps Cols=event";
			cmd1="Dpad move note/cmd";
			cmd2="A note preview/paste";
			cmd3="A+Dpad edit value";
			cmd4="L+Dpad free note";
			cmd5="Select command picker";
			cmd6="Start play cur ch";
			cmd7="R+Start song ctx";
			break;
		case VT_PROJECT:
			name="PROJECT";
			where="R+Up Song";
			edit="Dpad field A+Dpad val";
			field="Tempo key scale render";
			cmd1="Dpad choose field";
			cmd2="A activate field";
			cmd3="A+Dpad edit value";
			cmd4="B secondary action";
			cmd5="Set tempo/key/scale";
			cmd6="Render/save here";
			cmd7="R+Up Song";
			break;
		case VT_INSTRUMENT:
			name="INSTR";
			where="R+Left Phrase A sample";
			edit="L+LR page L+UD mark";
			field="Sample lab + markers";
			cmd1="Dpad choose field";
			cmd2="A activate/sample";
			cmd3="A+Dpad edit value";
			cmd4="L+A+LR nudge mark";
			cmd5="R+A arrows audition";
			cmd6="Sel root detect/accept";
			cmd7="R+Left Phrase";
			break;
		case VT_TABLE:
			name="TABLE";
			where="R+Up Phrase";
			edit="A+Dpad command/value";
			field="Command mini-sequence";
			cmd1="Dpad move cmd/param";
			cmd2="Select command picker";
			cmd3="A+Dpad edit value";
			cmd4="A paste last cmd";
			cmd5="B+R interpolate";
			cmd6="Start play cur ch";
			cmd7="R+Right Inst Table";
			break;
		case VT_TABLE2:
			name="INST TABLE";
			where="R+Up Instrument";
			edit="A+Dpad command/value";
			field="Instrument motion";
			cmd1="Dpad move cmd/param";
			cmd2="Select command picker";
			cmd3="A+Dpad edit value";
			cmd4="A paste last cmd";
			cmd5="B+R interpolate";
			cmd6="Start play phrase";
			cmd7="R+Left Table";
			break;
		case VT_GROOVE:
			name="GROOVE";
			where="R+Up Phrase";
			edit="A+Dpad tick values";
			field="Timing/swing table";
			cmd1="Dpad move tick cell";
			cmd2="A+Dpad edit ticks";
			cmd3="B+Dpad prev/next";
			cmd4="Start play cur ch";
			cmd5="Groove changes feel";
			cmd6="Use GROV command";
			cmd7="R+Down Phrase";
			break;
		case VT_MIXER:
			name="MIXER";
			where="R+Up Song";
			edit="Left/Right select ch";
			field="Meters + master wave";
			cmd1="Left/Right select ch";
			cmd2="Start play song";
			cmd3="R+Start stop";
			cmd4="Watch channel meters";
			cmd5="Master wave below";
			cmd6="Check level activity";
			cmd7="R+Up Song";
			break;
		default:
			break;
	}
	CustomizeContextOverlay(name,where,edit,field,cmd1,cmd2,cmd3,cmd4,cmd5,cmd6,cmd7);

	SetColor(CD_HILITE2);
	DrawString(x+2,y+1,name,props);
	SetColor(CD_NORMAL);
	if (contextOverlayPage_ < 0 || contextOverlayPage_ > 1) {
		contextOverlayPage_ = 0;
	}
	if (contextOverlayPage_ == 0) {
		DrawString(x+2,y+3,"MAP",props);
		DrawString(x+2,y+4,"Project",props);
		DrawString(x+2,y+5,"  |",props);
		DrawString(x+2,y+6,"Song>Chain>Phrase",props);
		DrawString(x+2,y+7,"             |",props);
		DrawString(x+2,y+8,"          Instrument",props);
		DrawString(x+2,y+9,"          Table",props);
		DrawString(x+2,y+10,"Mixer< Song  Groove",props);

		SetColor(CD_HILITE1);
		DrawString(x+2,y+12,field,props);
		SetColor(CD_NORMAL);
		DrawString(x+2,y+14,where,props);
		DrawString(x+2,y+15,edit,props);
	} else {
		DrawString(x+2,y+3,"COMMANDS",props);
		SetColor(CD_HILITE1);
		DrawString(x+2,y+4,field,props);
		SetColor(CD_NORMAL);
		DrawString(x+2,y+6,cmd1,props);
		DrawString(x+2,y+7,cmd2,props);
		DrawString(x+2,y+8,cmd3,props);
		DrawString(x+2,y+9,cmd4,props);
		DrawString(x+2,y+10,cmd5,props);
		DrawString(x+2,y+11,cmd6,props);
		DrawString(x+2,y+12,cmd7,props);
		SetColor(CD_HILITE1);
		DrawString(x+2,y+14,where,props);
		SetColor(CD_NORMAL);
		DrawString(x+2,y+15,edit,props);
	}
	SetColor(CD_HILITE2);
	DrawString(x+2,y+boxH-2,"Up/Dn page R+Sel close",props);
	SetColor(CD_NORMAL);
}

void View::CustomizeContextOverlay(const char *&name, const char *&where,
                                   const char *&edit, const char *&field,
                                   const char *&cmd1, const char *&cmd2,
                                   const char *&cmd3, const char *&cmd4,
                                   const char *&cmd5, const char *&cmd6,
                                   const char *&cmd7) {
}

void View::drawPlaybackScope() {
	Player *player=Player::GetInstance();
	const char *label="STOP";
	if (player && player->IsRunning()) {
		switch(viewData_->playMode_) {
			case PM_SONG:
				label="PLAY:SONG";
				break;
			case PM_CHAIN:
				label="PLAY:CHAIN";
				break;
			case PM_PHRASE:
				label="PLAY:PHR";
				break;
			case PM_LIVE:
				label="PLAY:LIVE";
				break;
			case PM_AUDITION:
				label="AUDITION";
				break;
			default:
				label="PLAY:----";
				break;
		}
	}

	GUIRect rect=w_.GetRect();
	int width=rect.Width()/8;
	int x=width-10;
	if (x<0) x=0;
	int y=ultraCompactLayout_ ? 3 : 1;
	GUITextProperties props;
	props.invert_=false;
	SetColor((player && player->IsRunning()) ? CD_PLAY : CD_HILITE1);
	DrawString(x,y,"          ",props);
	DrawString(x,y,label,props);
	SetColor(CD_NORMAL);
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

void View::drawMiniMeters() {
	if (!ultraCompactLayout_) {
		return;
	}
	Player *player=Player::GetInstance();
	if (!player || !player->IsRunning() || viewData_->playMode_ == PM_AUDITION) {
		return;
	}

	GUIPoint anchor=GetAnchor();
	int y = anchor._y + View::songRowCount_ + 4;
	if (y > 29) {
		return;
	}

	GUITextProperties props;
	props.invert_=true;
	int x = anchor._x;
	int masterLevel = MixerService::GetInstance()->GetMasterPeakPercent();
	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
		bool isPlaying = player->IsChannelPlaying(i);
		int level = MixerService::GetInstance()->GetBusPeakPercent(i);
		if (isPlaying && level < masterLevel) {
			level = masterLevel;
		}
		int width = 0;
		if (level > 4) width = 1;
		if (level > 28) width = 2;
		if (level > 62) width = 3;
		if (i==viewData_->songX_) {
			SetColor(CD_HILITE2);
		} else if (level > 62) {
			SetColor(CD_PLAY);
		} else {
			SetColor(CD_HILITE1);
		}
		if (!isPlaying) {
			DrawString(x,y,"   ",props);
		} else if (width==1) {
			DrawString(x,y,"=  ",props);
		} else if (width==2) {
			DrawString(x,y,"== ",props);
		} else {
			DrawString(x,y,"===",props);
		}
		x+=3;
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
		if (!suppressPlaybackScope_) {
			drawPlaybackScope();
		}
		if (!suppressPlaybackScope_ && contextOverlay_) {
			drawContextOverlay();
		}
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

	if (pressed && !modalView_) {
		if (mask == (EPBM_R|EPBM_SELECT)) {
			contextOverlay_ = !contextOverlay_;
			if (contextOverlay_) {
				contextOverlayPage_ = 0;
			}
			isDirty_ = true;
			((AppWindow &)w_).SetDirty();
			return;
		}
		if (contextOverlay_) {
			if (mask == EPBM_DOWN || mask == EPBM_UP) {
				contextOverlayPage_ = contextOverlayPage_ == 0 ? 1 : 0;
				isDirty_ = true;
				((AppWindow &)w_).SetDirty();
			}
			return;
		}
	}
	if (!pressed && contextOverlay_ && !modalView_) {
		return;
	}

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
		if (pressed && (mask & EPBM_START)) {
			isDirty_=true;
		}
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
