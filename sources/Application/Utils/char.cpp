#include "char.h"

char h2c__[16]= {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' } ;

const char *notes__[12] = {
"C ","C#","D ","D#","E ","F ","F#","G ","G#","A ","A#","B " 
} ;

const char *flatNotes__[12] = {
"C ","Db","D ","Eb","E ","F ","Gb","G ","Ab","A ","Bb","B "
} ;

static int noteNameMode__ = NOTE_NAME_SHARPS;

void setNoteNameMode(int mode) {
	if (mode<NOTE_NAME_SHARPS || mode>NOTE_NAME_FLATS) {
		mode=NOTE_NAME_SHARPS;
	}
	noteNameMode__=mode;
}

int getNoteNameMode() {
	return noteNameMode__;
}

const char *getNoteName(unsigned char note, int mode) {
	if (mode<NOTE_NAME_SHARPS || mode>NOTE_NAME_FLATS) {
		mode=noteNameMode__;
	}
	return mode==NOTE_NAME_FLATS ? flatNotes__[note%12] : notes__[note%12];
}


