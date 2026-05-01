#include "MixerView.h"
#include "Application/Mixer/MixerService.h"
#include "Application/Model/Mixer.h"
#include "Application/Utils/char.h"
#if defined(PLATFORM_RGNANO) || defined(PLATFORM_RGNANO_SIM)
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
#endif
#include <string>
#include <iostream>
#include <sstream>

MixerView::MixerView(GUIWindow &w,ViewData *viewData):View(w,viewData) {
	clipboard_.active_=false ;
	clipboard_.data_=0 ;
	invertBatt_=false;
}

MixerView::~MixerView() {
} 


void MixerView::onStart() {
	Player *player=Player::GetInstance() ;
	unsigned char from=viewData_->songX_ ;
	unsigned char to=from ;
	//if (clipboard_.active_) {
	//	GUIRect r=getSelectionRect();
	//	from=r.Left() ;
	//	to=r.Right() ;
	//}
	player->OnStartButton(PM_SONG,from,false,to) ;
} ;

void MixerView::onStop() {
	Player *player=Player::GetInstance() ;
	unsigned char from=viewData_->songX_ ;
	unsigned char to=from ;
	player->OnStartButton(PM_SONG,from,true,to) ;
} ;

void MixerView::OnFocus() {
} ;

void MixerView::updateCursor(int dx,int dy) {
	int x=viewData_->mixerCol_ ;
	x+=dx ;
	if (x<0) x=0 ;
	if (x>7) x=7 ;
	viewData_->mixerCol_=x ;
	isDirty_=true;
}

void MixerView::ProcessButtonMask(unsigned short mask,bool pressed) {
	//if (!pressed) {
	//	if (viewMode_==VM_MUTEON) {
	//		if (mask&EPBM_R) {
	//			toggleMute() ;
	//		}
	//	} ;
	//	if (viewMode_==VM_SOLOON) {
	//		if (mask&EPBM_R) {
	//			switchSoloMode() ;
	//		}
	//	} ;
	//	return ;
	//} ;
	//
	
	if (clipboard_.active_) {
		viewMode_=VM_SELECTION ;
	} ;
	// Process selection related keys
	
	if (viewMode_==VM_SELECTION) {
        if (clipboard_.active_==false) {
            clipboard_.active_=true ;
            clipboard_.x_=viewData_->songX_ ;
            clipboard_.y_=viewData_->songY_ ;
            clipboard_.offset_=viewData_->songOffset_ ;
			saveX_=clipboard_.x_ ;
			saveY_=clipboard_.y_ ;
			saveOffset_=clipboard_.offset_ ;
        }
        processSelectionButtonMask(mask) ;
    } else {
	   
       // Switch back to normal mode

        viewMode_=VM_NORMAL ;
        processNormalButtonMask(mask) ;
    }
} ;


/******************************************************
 processNormalButtonMask:
        process button mask in the case there is no
        selection active
 ******************************************************/
 
void MixerView::processNormalButtonMask(unsigned int mask) {

	// B Modifier

	if (mask&EPBM_B) {
	} else {

	  // A modifier

	  if (mask&EPBM_A) {

	  } else {

		  // R Modifier

          	if (mask&EPBM_R) {
				if (mask&EPBM_UP) {
					ViewType vt=VT_SONG;
					ViewEvent ve(VET_SWITCH_VIEW,&vt) ;
					SetChanged();
					NotifyObservers(&ve) ;
				}	
				if (mask&EPBM_START) {
				    onStop() ;
                }
	    	} else {

			// L Modifier
			
				if (mask&EPBM_L) {

				} else {
					// No modif
					if (mask&EPBM_START) {
						onStart() ;
					}
				   if (mask&EPBM_LEFT) updateCursor(-1,0)  ;
				   if (mask&EPBM_RIGHT) updateCursor(1,0) ;
				}
		    }
	  } 
	}
} ;

/******************************************************
 processSelectionButtonMask:
        process button mask in the case there is a
        selection active
 ******************************************************/
 
void MixerView::processSelectionButtonMask(unsigned int mask) {

	// B Modifier

	if (mask&EPBM_B) {

    } else {

	  // A modifier

	  if (mask&EPBM_A) {

	  } else {

		  // R Modifier

          	if (mask&EPBM_R) {
 				if (mask&EPBM_START) {
				    onStop() ;
                }
	    	} else {

    			// No modifier
	          		if (mask&EPBM_START) {
					   onStart() ;
	    			}
		    }
	  } 
	}
}

void MixerView::DrawView() {

	Clear() ;

	GUITextProperties props ;
	GUIPoint pos=GetTitlePosition() ;
	GUIPoint anchor=GetAnchor() ;
	char hex[3] ;

// Draw title

	SetColor(CD_NORMAL) ;

	Player *player=Player::GetInstance() ;
	
	std::ostringstream os;

	os << "Mixer " << ((player->GetSequencerMode()==SM_SONG)?"Song":"Live") ;

	std::string buffer(os.str());

	DrawString(pos._x,pos._y,buffer.c_str(),props) ;

	// Now draw busses

	pos=anchor ;
	short dx=3;

	for (int i=0;i<8;i++) {
		if (i==viewData_->mixerCol_) {
			 props.invert_=true;
			 SetColor(CD_HILITE2) ;
		}
		int bus=Mixer::GetInstance()->GetBus(i) ;
		hex2char(bus,hex) ;
		DrawString(pos._x,pos._y,hex,props) ;
		pos._x+=dx ;	
		if (i==viewData_->mixerCol_) {
			 props.invert_=false;
			 SetColor(CD_NORMAL) ;
		}
	}; 
	
    drawWaveform() ;
    drawMap() ;
	drawNotes() ;
	drawMiniMeters() ;
    
	if (player->IsRunning()) {
		OnPlayerUpdate(PET_UPDATE) ;
	} ;
} ;

void MixerView::OnPlayerUpdate(PlayerEventType ,unsigned int tick) {

	Player *player=Player::GetInstance() ;

	// Draw clipping indicator & CPU usage

	GUIPoint anchor=GetAnchor() ;
	GUIPoint pos=anchor ;

	GUITextProperties props ;
	SetColor(CD_NORMAL) ;

    if (View::miniLayout_) {
      pos._y=0 ;
      pos._x=25 ;
    } else {
      pos=anchor ;
      pos._x+=25 ;
    }
    
	if (player->Clipped()) {
           DrawString(pos._x,pos._y,"clip",props); 
    } else {
           DrawString(pos._x,pos._y,"----",props); 
    }
	char strbuffer[10] ;

	pos._y+=1 ;
	sprintf(strbuffer,"%3.3d%%",player->GetPlayedBufferPercentage()) ; 
	DrawString(pos._x,pos._y,strbuffer,props) ;

    System *sys=System::GetInstance() ;
    int batt=sys->GetBatteryLevel() ;
    if (batt>=0) {
		if (batt<90) {
			SetColor(CD_HILITE2) ;
			invertBatt_=!invertBatt_ ;
		} else {
			invertBatt_=false ;
		} ;
		props.invert_=invertBatt_ ;

	    pos._y+=1 ;
    	sprintf(strbuffer,"%3.3d",batt) ; 
	    DrawString(pos._x,pos._y,strbuffer,props) ;
    }
	SetColor(CD_NORMAL) ;
	props.invert_=false ;
    int time=int(player->GetPlayTime()) ;
    int mi=time/60 ;
    int se=time-mi*60 ;
	sprintf(strbuffer,"%2.2d:%2.2d",mi,se) ; 
	pos._y+=1 ;	
	DrawString(pos._x,pos._y,strbuffer,props) ;

    drawNotes() ;
    drawMiniMeters() ;

} ;

void MixerView::drawWaveform() {
    MixerService *mixer = MixerService::GetInstance();

#if defined(PLATFORM_RGNANO) || defined(PLATFORM_RGNANO_SIM)
    SDLGUIWindowImp *imp = (SDLGUIWindowImp *)w_.GetImpWindow();
    const int x = 24;
    const int y = 72;
    const int width = 176;
    const int height = 48;
    const int mid = y + (height / 2);
    const int columns = AudioMixer::WAVEFORM_SIZE;

    SetColor(CD_HILITE2);
    for (int col = 0; col < width; col++) {
        int sampleIndex = (col * columns) / width;
        int nextIndex = ((col + 1) * columns) / width;
        if (nextIndex >= columns) {
            nextIndex = columns - 1;
        }
        int sample = mixer->GetMasterWaveformSample(sampleIndex);
        int nextSample = mixer->GetMasterWaveformSample(nextIndex);
        int minSample = mixer->GetMasterWaveformMin(sampleIndex);
        int maxSample = mixer->GetMasterWaveformMax(sampleIndex);
        int waveY = mid - ((sample * (height / 2 - 2)) / 100);
        int nextY = mid - ((nextSample * (height / 2 - 2)) / 100);
        if (waveY < y) waveY = y;
        if (waveY >= y + height) waveY = y + height - 1;
        if (nextY < y) nextY = y;
        if (nextY >= y + height) nextY = y + height - 1;
        int lineTop = waveY < nextY ? waveY : nextY;
        int lineBottom = waveY > nextY ? waveY : nextY;
        GUIRect wave(x + col, lineTop, x + col + 1, lineBottom + 1);
        imp->DrawRect(wave);

        if ((col % 4)==0) {
            int top = mid - ((maxSample * (height / 2 - 2)) / 100);
            int bottom = mid - ((minSample * (height / 2 - 2)) / 100);
            if (top < y) top = y;
            if (top >= y + height) top = y + height - 1;
            if (bottom < y) bottom = y;
            if (bottom >= y + height) bottom = y + height - 1;
            GUIRect upper(x + col, top, x + col + 1, top + 1);
            GUIRect lower(x + col, bottom, x + col + 1, bottom + 1);
            imp->DrawRect(upper);
            imp->DrawRect(lower);
        }
    }

    SetColor(CD_NORMAL);
    GUIRect center(x, mid, x + width, mid + 1);
    imp->DrawRect(center);
#else
    GUITextProperties props;
    GUIPoint pos;
    pos._x = 3;
    pos._y = 72;
    DrawString(pos._x, pos._y, "scope unavailable", props);
#endif
}
