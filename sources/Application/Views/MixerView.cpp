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
    lastWaveformDrawMs_=0;
    waveformPrimed_=false;
    for (int i=0; i<SONG_CHANNEL_COUNT; i++) {
        lastChannelNote_[i][0]='-';
        lastChannelNote_[i][1]='-';
        lastChannelNote_[i][2]=0;
    }
    for (int i=0; i<WAVEFORM_DRAW_COLUMNS; i++) {
        waveformLineTop_[i]=-1;
        waveformLineBottom_[i]=-1;
        waveformUpper_[i]=-1;
        waveformLower_[i]=-1;
    }
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

// Draw title

	SetColor(CD_NORMAL) ;

	Player *player=Player::GetInstance() ;
	
	std::ostringstream os;

	os << "Mixer " << ((player->GetSequencerMode()==SM_SONG)?"Song":"Live") ;

    std::string buffer(os.str());

	DrawString(pos._x,pos._y,buffer.c_str(),props) ;

#if defined(PLATFORM_RGNANO) || defined(PLATFORM_RGNANO_SIM)
    drawChannelMeters(true);
    drawWaveform(true);
#else
	// Now draw busses

	GUIPoint anchor=GetAnchor() ;
	char hex[3] ;
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
	
    drawWaveform(true) ;
    drawMap() ;
	drawNotes() ;
#endif
    
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

    drawChannelMeters() ;
    drawWaveform() ;

} ;

void MixerView::drawChannelMeters(bool force) {
#if defined(PLATFORM_RGNANO) || defined(PLATFORM_RGNANO_SIM)
    MixerService *mixer = MixerService::GetInstance();
    SDLGUIWindowImp *imp = (SDLGUIWindowImp *)w_.GetImpWindow();
    const int startCol = 2;
    const int labelRow = 4;
    const int meterRow = 7;
    const int noteRow = 10;
    const int cellWidth = 3;
    GUIColor panel(0x08, 0x12, 0x16);

    if (force) {
        imp->SetColor(panel);
        GUIRect area(0, 28, 240, 126);
        imp->DrawRect(area);
    }

    GUITextProperties props;
    props.invert_ = false;
    char label[3];
    char meterCell[4];
    for (int i=0; i<SONG_CHANNEL_COUNT; i++) {
        int col = startCol + i * cellWidth;
        int bus = Mixer::GetInstance()->GetBus(i);
        int level = mixer->GetBusPeakPercent(bus);
        if (level<0) level=0;
        if (level>100) level=100;
        bool isSelected = (i == viewData_->mixerCol_);
        bool isActive = Player::GetInstance()->IsChannelPlaying(i) || level > 0;

        SetColor(isSelected ? CD_HILITE2 : CD_NORMAL);
        hex2char(bus, label);
        DrawString(col, labelRow, label, props);

        meterCell[0]=' ';
        meterCell[1]=' ';
        meterCell[2]=0;
        if (level > 4) meterCell[0]='=';
        if (level > 38) meterCell[1]='=';
        if (level > 78) {
            meterCell[0]='!';
            meterCell[1]='!';
        }
        if (isSelected) {
            SetColor(CD_HILITE2);
        } else if (level > 78) {
            SetColor(CD_PLAY);
        } else if (isActive) {
            SetColor(CD_HILITE1);
        } else {
            SetColor(CD_NORMAL);
        }
        DrawString(col, meterRow, meterCell, props);

        SetColor(CD_NORMAL);
        DrawString(col, noteRow, "   ", props);
        if (Player::GetInstance()->IsRunning() && viewData_->playMode_ != PM_AUDITION) {
            const char *playedNote = Player::GetInstance()->GetPlayedNote(i);
            char noteChar = playedNote && playedNote[0] ? playedNote[0] : ' ';
            const char *playedOctave = Player::GetInstance()->GetPlayedOctive(i);
            if (noteChar != ' ') {
                lastChannelNote_[i][0] = noteChar;
                if (playedOctave && playedOctave[1] && playedOctave[1] != ' ') {
                    lastChannelNote_[i][1] = playedOctave[1];
                } else if (playedOctave && playedOctave[0] &&
                           playedOctave[0] != ' ') {
                    lastChannelNote_[i][1] = playedOctave[0];
                } else {
                    lastChannelNote_[i][1] = '-';
                }
            }
            SetColor(isSelected ? CD_HILITE2 : CD_HILITE1);
            DrawString(col, noteRow, lastChannelNote_[i], props);
        } else {
            lastChannelNote_[i][0]='-';
            lastChannelNote_[i][1]='-';
            SetColor(CD_NORMAL);
            DrawString(col, noteRow, lastChannelNote_[i], props);
        }
    }
    SetColor(CD_NORMAL);
#endif
}

void MixerView::drawWaveform(bool force) {
    MixerService *mixer = MixerService::GetInstance();

#if defined(PLATFORM_RGNANO) || defined(PLATFORM_RGNANO_SIM)
    unsigned int now=SDL_GetTicks();
    if (!force && lastWaveformDrawMs_!=0 && now-lastWaveformDrawMs_<33) {
        return;
    }
    lastWaveformDrawMs_=now;

    SDLGUIWindowImp *imp = (SDLGUIWindowImp *)w_.GetImpWindow();
    const int x = 16;
    const int y = 104;
    const int width = 192;
    const int height = 72;
    const int mid = y + (height / 2);
    const int columns = AudioMixer::WAVEFORM_SIZE;
    const int stepPx = 1;
    GUIColor scopeBackground(0x08,0x12,0x16);
    GUIColor scopeTrace(0x35,0xD3,0xCE);
    GUIColor scopeCenter(0x24,0x5E,0x62);
    int peakAbs = 1;
    for (int i=0; i<columns; i++) {
        int sample = mixer->GetMasterWaveformSample(i);
        int absSample = sample < 0 ? -sample : sample;
        if (absSample > peakAbs) peakAbs = absSample;
    }
    int gain = (peakAbs < 85) ? ((85 * 100) / peakAbs) : 100;
    if (gain > 520) gain = 520;

    imp->SetColor(scopeBackground);
    GUIRect clear(x, y, x + width, y + height);
    imp->DrawRect(clear);

    if (force || !waveformPrimed_) {
        for (int i=0; i<WAVEFORM_DRAW_COLUMNS; i++) {
            waveformLineTop_[i]=-1;
            waveformLineBottom_[i]=-1;
            waveformUpper_[i]=-1;
            waveformLower_[i]=-1;
        }
        waveformPrimed_=true;
    }

    imp->SetColor(scopeCenter);
    for (int col = 0; col < width; col += 12) {
        GUIRect center(x + col, mid, x + col + 2, mid + 1);
        imp->DrawRect(center);
    }

    for (int col = 0; col < width; col += stepPx) {
        int drawIndex = col / stepPx;
        int sampleIndex = (col * columns) / width;
        int prevIndex = sampleIndex > 0 ? sampleIndex - 1 : sampleIndex;
        int nextIndex = ((col + stepPx) * columns) / width;
        if (nextIndex >= columns) {
            nextIndex = columns - 1;
        }
        int prevSample = mixer->GetMasterWaveformSample(prevIndex);
        int sample = mixer->GetMasterWaveformSample(sampleIndex);
        int nextSample = mixer->GetMasterWaveformSample(nextIndex);

        int smoothed = (prevSample + (sample * 2) + nextSample) / 4;
        int waveY = mid - ((smoothed * gain * (height / 2 - 2)) / 10000);
        if (waveY < y + 2) waveY = y + 2;
        if (waveY >= y + height - 2) waveY = y + height - 3;

        imp->SetColor(scopeTrace);
        int lineTop = waveY;
        int lineBottom = waveY;
        GUIRect wave(x + col, waveY, x + col + stepPx, waveY + 1);
        imp->DrawRect(wave);

        if (sampleIndex>0) {
            int prevSmoothed = (prevSample + sample) / 2;
            int prevY = mid - ((prevSmoothed * gain * (height / 2 - 2)) / 10000);
            if (prevY < y + 2) prevY = y + 2;
            if (prevY >= y + height - 2) prevY = y + height - 3;
            if (prevY != waveY) {
                int delta = prevY > waveY ? prevY - waveY : waveY - prevY;
                int bridgeTop = waveY;
                int bridgeBottom = waveY;
                if (delta <= 4) {
                    bridgeTop = prevY < waveY ? prevY : waveY;
                    bridgeBottom = prevY > waveY ? prevY : waveY;
                } else if (prevY < waveY) {
                    bridgeTop = waveY - 2;
                } else {
                    bridgeBottom = waveY + 2;
                }
                if (bridgeTop < y + 2) bridgeTop = y + 2;
                if (bridgeBottom >= y + height - 2) bridgeBottom = y + height - 3;
                GUIRect bridge(x + col, bridgeTop, x + col + 1, bridgeBottom + 1);
                imp->DrawRect(bridge);
                lineTop = bridgeTop;
                lineBottom = bridgeBottom;
            }
        }

        waveformLineTop_[drawIndex]=lineTop;
        waveformLineBottom_[drawIndex]=lineBottom;
    }
#else
    GUITextProperties props;
    GUIPoint pos;
    pos._x = 3;
    pos._y = 72;
    DrawString(pos._x, pos._y, "scope unavailable", props);
#endif
}
