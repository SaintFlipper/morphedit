/*****************************************************************************
 File:			prinit.c
 Description:	Functions for initialising preset dialogs 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 11/4/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static LPCSTR pszPortamentoShapes[] = {"Linear", "Expo1", "Expo2", "Expo3", "Expo4", "Expo5", "Expo6", "Expo7", "Expo8"};
static LPCSTR pszPortamentoModes[] = {"Mono", "Poly 1 key", "Poly 2 key", "Poly 3 key", "Poly 4 key", "Poly 5 key"};
static LPCSTR pszPriorities[] = {"High", "Low", "First", "Last", "Drum"};
static LPCSTR pszSoloModes[] = {"Off", "Wind", "Synth"};
static LPCSTR pszLfoShapes[] = {"Random", "Triangle", "Sine", "Saw", "Square"};
static LPCSTR pszVelocityCurves[] = {"Off", "Curve 1", "Curve 2", "Curve 3", "Curve 4", "Global"};
static LPCSTR pszTunings[] = {"Equal", "Just C", "Vallotti", "19 Tone", "Gamelan", "User"};
static LPCSTR pszMixBus[] = {"Main", "Sub1", "FX A", "FX B"};
static LPCSTR pszXfadeMode[] = {"Off", "XFade", "XSwitch"};
static LPCSTR pszXfadeDirection[] = {"Pri->Sec", "Sec->Pri"};
static LPCSTR pszFootSwitchRoutings[] =
			{"Off", "Sustain", "SustainP", "SustainS", "AltEnv", "AltEnvP", "AltEnvS",
			"AltRel", "AltRelP", "AltRelS", "Xswitch", "PortOn", "PortOnP", "PortOnS"};
static LPCSTR pszNoteOnSources[] =
			{"Key", "Velocity", "Pitchwheel", "Control A", "Control B",
			"Control C", "Control D", "Mono pressure", "Free-run FG"};
static LPCSTR pszRealTimeSources[] =
			{"Pitchwheel", "Control A", "Control B", "Control C", "Control D", "Mono pressure",
			 "Poly pressure", "LFO 1", "LFO 2", "Aux env.", "FG 1", "FG 2", "Free-run FG"};
static LPCSTR pszPatchDests[] =
			{"Off", "Pitch", "Pitch P", "Pitch S", "Volume", "Volume P", "Volume S",
			 "Attack", "Attack P", "Attack S", "Decay", "Decay P", "Decay S",
			 "Release", "Release P", "Release S", "Xfade", "LFO1 amt", "LFO1 rate",
			 "LFO2 amt", "LFO2 rate", "Aux amt", "Aux att", "Aux dec", "Aux rel",
			 "Start", "Start P", "Start S", "Pan", "Pan P", "Pan S", "Tone",
			 "Tone P", "Tone S", "Morph", "Morph P", "Morph S", "Trans2", "Trans2 P", "Trans2 S",
			 "Port rt", "PortRt P", "PortRt S", "FG1 amt", "FG2 amt", "FiltLev", "FiltLev P",
			 "FiltLev S", "Freq trk", "Freq trk P", "Freq trk S"};

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

/*^L*/
/*****************************************************************************
 Function:		InitInstrumentsDialog
 Parameters:	HWND	hWnd			Dialog window
 Returns:		None.
 Description:	Initialises the instruments dialog
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 11/4/94	| Created.											| PW
*****************************************************************************/

VOID InitInstrumentsDialog (HWND hWnd)
{
	// If the instruments are on display
	if (IsPresetSubDialogDisplayed (VOICES))
	{
		// Update PRESET display with internal information
		UpdatePresetInternals ();

		/* Instrument 1 */

		// IDC_INSTR1_INSTRUMENT

		// IDC_INSTR1_LOW_KEY_EDITBOX
		// IDC_INSTR1_LOW_KEY_CONTROL
		InitUpDown (hWnd, IDC_INSTR1_LOW_KEY_CONTROL, 0, 127, 0,
					GetDlgItem (hWnd, IDC_INSTR1_LOW_KEY_EDITBOX), lpfnDisplayNote);

		// IDC_INSTR1_HIGH_KEY_EDITBOX
		// IDC_INSTR1_HIGH_KEY_CONTROL
		InitUpDown (hWnd, IDC_INSTR1_HIGH_KEY_CONTROL, 0, 127, 0,
					GetDlgItem (hWnd, IDC_INSTR1_HIGH_KEY_EDITBOX), lpfnDisplayNote);

		// IDC_INSTR1_COARSE_TUNE
		InitSlider (hWnd, IDC_INSTR1_COARSE_TUNE, -36, 36, 0);

		// IDC_INSTR1_FINE_TUNE
		InitSlider (hWnd, IDC_INSTR1_FINE_TUNE, -64, 64, 0);

		// IDC_INSTR1_KEY_TRANSPOSE
		InitSlider (hWnd, IDC_INSTR1_KEY_TRANSPOSE, -36, 36, 0);

		// IDC_INSTR1_DOUBLE_DETUNE
		InitSlider (hWnd, IDC_INSTR1_DOUBLE_DETUNE, 0, 15, 0);

		// IDC_INSTR1_SOUND_DELAY
		InitSlider (hWnd, IDC_INSTR1_SOUND_DELAY, 0, 127, 0);

		// IDC_INSTR1_SOUND_START
		InitSlider (hWnd, IDC_INSTR1_SOUND_START, 0, 127, 0);

		// IDC_INSTR1_LOOP_START_HIGH
		InitUpDown (hWnd, IDC_INSTR1_LOOP_START_HIGH, -1, 1, 0, (HWND)NULL, NULL);

		// IDC_INSTR1_LOOP_START_LOW
		InitUpDown (hWnd, IDC_INSTR1_LOOP_START_LOW, -1, 1, 0, (HWND)NULL, NULL);

		// IDC_INSTR1_LOOP_START_HIGH_VALUE
		// IDC_INSTR1_LOOP_START_LOW_VALUE
		DisplayPresetLoopStart (0);
		InterceptEditEnter (hWnd, IDC_INSTR1_LOOP_START_HIGH_VALUE);
		InterceptEditEnter (hWnd, IDC_INSTR1_LOOP_START_LOW_VALUE);

		// IDC_INSTR1_LOOP_LENGTH_HIGH
		InitUpDown (hWnd, IDC_INSTR1_LOOP_LENGTH_HIGH, -1, 1, 0, (HWND)NULL, NULL);

		// IDC_INSTR1_LOOP_LENGTH_LOW
		InitUpDown (hWnd, IDC_INSTR1_LOOP_LENGTH_LOW, -1, 1, 0, (HWND)NULL, NULL);

		// IDC_INSTR1_LOOP_LENGTH_HIGH_VALUE
		// IDC_INSTR1_LOOP_LENGTH_LOW_VALUE
		DisplayPresetLoopLength (0);
		InterceptEditEnter (hWnd, IDC_INSTR1_LOOP_LENGTH_HIGH_VALUE);
		InterceptEditEnter (hWnd, IDC_INSTR1_LOOP_LENGTH_LOW_VALUE);

		// IDC_INSTR1_REVERSE
		// IDC_INSTR1_NON_TRANSPOSE
		// IDC_INSTR1_LOOP_ENABLE

		// IDC_INSTR1_SOLO_MODE_CONTROL
		InitListbox (GetDlgItem (hWnd, IDC_INSTR1_SOLO_MODE_CONTROL), pszSoloModes, ARRAY_SIZE (pszSoloModes));

		// IDC_INSTR1_SOLO_PRIORITY_CONTROL
		InitListbox (GetDlgItem (hWnd, IDC_INSTR1_SOLO_PRIORITY_CONTROL), pszPriorities, ARRAY_SIZE (pszPriorities));

		// IDC_INSTR1_VOLUME
		InitSlider (hWnd, IDC_INSTR1_VOLUME, 0, 127, 0);

		/* Portamento 1 */

		// IDC_PORTAMENTO1_SHAPE
		InitListbox (GetDlgItem (hWnd, IDC_PORTAMENTO1_SHAPE), pszPortamentoShapes, ARRAY_SIZE (pszPortamentoShapes));

		// IDC_PORTAMENTO1_RATE
		InitSlider (hWnd, IDC_PORTAMENTO1_RATE, 0, 127, 0);

		/* Pan 1 */

		// IDC_PAN1_POSITION
		InitSlider (hWnd, IDC_PAN1_POSITION, -7, 7, 0);

		/* Filter 1 */

		// IDC_FILTER1_TYPE
		// IDC_FILTER1_REVERSE

		// IDC_FILTER1_LEVEL
		InitSlider (hWnd, IDC_FILTER1_LEVEL, 0, 255, 0);

		// IDC_FILTER1_MORPH
		InitSlider (hWnd, IDC_FILTER1_MORPH, 0, 255, 0);

		// IDC_FILTER1_FREQ_TRACK
		InitSlider (hWnd, IDC_FILTER1_FREQ_TRACK, 0, 255, 0);

		// IDC_FILTER1_TRANS2
		InitSlider (hWnd, IDC_FILTER1_TRANS2, 0, 255, 0);

		/* Alternate envelope 1 */

		// IDC_ALT_ENV1_ENABLE

		// IDC_ALT_ENV1_ATTACK
		InitSlider (hWnd, IDC_ALT_ENV1_ATTACK, 0, 99, 0);

		// IDC_ALT_ENV1_HOLD
		InitSlider (hWnd, IDC_ALT_ENV1_HOLD, 0, 99, 0);

		// IDC_ALT_ENV1_DECAY
		InitSlider (hWnd, IDC_ALT_ENV1_DECAY, 0, 99, 0);

		// IDC_ALT_ENV1_SUSTAIN
		InitSlider (hWnd, IDC_ALT_ENV1_SUSTAIN, 0, 99, 0);

		// IDC_ALT_ENV1_RELEASE
		InitSlider (hWnd, IDC_ALT_ENV1_RELEASE, 0, 99, 0);

		// IDC_ALT_ENV1_ENVELOPE

		/* Instrument 2 */

		// IDC_INSTR2_INSTRUMENT

		// IDC_INSTR2_LOW_KEY_EDITBOX
		// IDC_INSTR2_LOW_KEY_CONTROL
		InitUpDown (hWnd, IDC_INSTR2_LOW_KEY_CONTROL, 0, 127, 0,
					GetDlgItem (hWnd, IDC_INSTR2_LOW_KEY_EDITBOX), lpfnDisplayNote);

		// IDC_INSTR2_HIGH_KEY_EDITBOX
		// IDC_INSTR2_HIGH_KEY_CONTROL
		InitUpDown (hWnd, IDC_INSTR2_HIGH_KEY_CONTROL, 0, 127, 0,
					GetDlgItem (hWnd, IDC_INSTR2_HIGH_KEY_EDITBOX), lpfnDisplayNote);

		// IDC_INSTR2_COARSE_TUNE
		InitSlider (hWnd, IDC_INSTR2_COARSE_TUNE, -36, 36, 0);

		// IDC_INSTR2_FINE_TUNE
		InitSlider (hWnd, IDC_INSTR2_FINE_TUNE, -64, 64, 0);

		// IDC_INSTR2_KEY_TRANSPOSE
		InitSlider (hWnd, IDC_INSTR2_KEY_TRANSPOSE, -36, 36, 0);

		// IDC_INSTR2_DOUBLE_DETUNE
		InitSlider (hWnd, IDC_INSTR2_DOUBLE_DETUNE, 0, 15, 0);

		// IDC_INSTR2_SOUND_DELAY
		InitSlider (hWnd, IDC_INSTR2_SOUND_DELAY, 0, 127, 0);

		// IDC_INSTR2_SOUND_START
		InitSlider (hWnd, IDC_INSTR2_SOUND_START, 0, 127, 0);

		// IDC_INSTR2_LOOP_START_HIGH
		InitUpDown (hWnd, IDC_INSTR2_LOOP_START_HIGH, -1, 1, 0, (HWND)NULL, NULL);

		// IDC_INSTR2_LOOP_START_LOW
		InitUpDown (hWnd, IDC_INSTR2_LOOP_START_LOW, -1, 1, 0, (HWND)NULL, NULL);

		// IDC_INSTR2_LOOP_START_HIGH_VALUE
		// IDC_INSTR2_LOOP_START_LOW_VALUE
		DisplayPresetLoopStart (1);
		InterceptEditEnter (hWnd, IDC_INSTR2_LOOP_START_HIGH_VALUE);
		InterceptEditEnter (hWnd, IDC_INSTR2_LOOP_START_LOW_VALUE);

		// IDC_INSTR2_LOOP_LENGTH_HIGH
		InitUpDown (hWnd, IDC_INSTR2_LOOP_LENGTH_HIGH, -1, 1, 0, (HWND)NULL, NULL);

		// IDC_INSTR2_LOOP_LENGTH_LOW
		InitUpDown (hWnd, IDC_INSTR2_LOOP_LENGTH_LOW, -1, 1, 0, (HWND)NULL, NULL);

		// IDC_INSTR2_LOOP_LENGTH_HIGH_VALUE
		// IDC_INSTR2_LOOP_LENGTH_LOW_VALUE
		DisplayPresetLoopLength (1);
		InterceptEditEnter (hWnd, IDC_INSTR2_LOOP_LENGTH_HIGH_VALUE);
		InterceptEditEnter (hWnd, IDC_INSTR2_LOOP_LENGTH_LOW_VALUE);

		// IDC_INSTR2_REVERSE
		// IDC_INSTR2_NON_TRANSPOSE
		// IDC_INSTR2_LOOP_ENABLE

		// IDC_INSTR2_SOLO_MODE_CONTROL
		InitListbox (GetDlgItem (hWnd, IDC_INSTR2_SOLO_MODE_CONTROL), pszSoloModes, ARRAY_SIZE (pszSoloModes));

		// IDC_INSTR2_SOLO_PRIORITY_CONTROL
		InitListbox (GetDlgItem (hWnd, IDC_INSTR2_SOLO_PRIORITY_CONTROL), pszPriorities, ARRAY_SIZE (pszPriorities));

		// IDC_INSTR2_VOLUME
		InitSlider (hWnd, IDC_INSTR2_VOLUME, 0, 127, 0);

		/* Portamento 2 */

		// IDC_PORTAMENTO2_SHAPE
		InitListbox (GetDlgItem (hWnd, IDC_PORTAMENTO2_SHAPE), pszPortamentoShapes, ARRAY_SIZE (pszPortamentoShapes));

		// IDC_PORTAMENTO2_RATE
		InitSlider (hWnd, IDC_PORTAMENTO2_RATE, 0, 127, 0);

		/* Pan 2 */

		// IDC_PAN2_POSITION
		InitSlider (hWnd, IDC_PAN2_POSITION, -7, 7, 0);

		/* Filter 2 */

		// IDC_FILTER2_TYPE
		// IDC_FILTER2_REVERSE

		// IDC_FILTER2_LEVEL
		InitSlider (hWnd, IDC_FILTER2_LEVEL, 0, 255, 0);

		// IDC_FILTER2_MORPH
		InitSlider (hWnd, IDC_FILTER2_MORPH, 0, 255, 0);

		// IDC_FILTER2_FREQ_TRACK
		InitSlider (hWnd, IDC_FILTER2_FREQ_TRACK, 0, 255, 0);

		// IDC_FILTER2_TRANS2
		InitSlider (hWnd, IDC_FILTER2_TRANS2, 0, 255, 0);

		/* Alternate envelope 2 */

		// IDC_ALT_ENV2_ENABLE

		// IDC_ALT_ENV2_ATTACK
		InitSlider (hWnd, IDC_ALT_ENV2_ATTACK, 0, 99, 0);

		// IDC_ALT_ENV2_HOLD
		InitSlider (hWnd, IDC_ALT_ENV2_HOLD, 0, 99, 0);

		// IDC_ALT_ENV2_DECAY
		InitSlider (hWnd, IDC_ALT_ENV2_DECAY, 0, 99, 0);

		// IDC_ALT_ENV2_SUSTAIN
		InitSlider (hWnd, IDC_ALT_ENV2_SUSTAIN, 0, 99, 0);

		// IDC_ALT_ENV2_RELEASE
		InitSlider (hWnd, IDC_ALT_ENV2_RELEASE, 0, 99, 0);

		// IDC_ALT_ENV2_ENVELOPE
	}
}

/*^L*/
/*****************************************************************************
 Function:		InitModulatorsDialog
 Parameters:	HWND	hWnd			Dialog window
 Returns:		None.
 Description:	Initialises the modulators dialog
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94	| Created.											| PW
*****************************************************************************/

VOID InitModulatorsDialog (HWND hWnd)
{
	static LISTBOX_TAB_INFO	FG1ListboxTabs[] =
	{
		{IDC_FG1_LB_LABEL1,	0},
		{IDC_FG1_LB_LABEL2,	20},
		{IDC_FG1_LB_LABEL3,	50},
		{IDC_FG1_LB_LABEL4,	75},
		{IDC_FG1_LB_LABEL5,	110},
		{IDC_FG1_LB_LABEL6,	165},
		{IDC_FG1_LB_LABEL7,	190}
	};
	static LISTBOX_TAB_INFO	FG2ListboxTabs[] =
	{
		{IDC_FG2_LB_LABEL1,	0},
		{IDC_FG2_LB_LABEL2,	20},
		{IDC_FG2_LB_LABEL3,	50},
		{IDC_FG2_LB_LABEL4,	75},
		{IDC_FG2_LB_LABEL5,	110},
		{IDC_FG2_LB_LABEL6,	165},
		{IDC_FG2_LB_LABEL7,	190}
	};

	// If the FGs are on display
	if (IsPresetSubDialogDisplayed (FG))
	{
		// Set the Function Generator listbox tabs
		SetListBoxTabs (hWnd, IDC_FG1_LISTBOX, &FG1ListboxTabs[0], ARRAY_SIZE (FG1ListboxTabs));
		SetListBoxTabs (hWnd, IDC_FG2_LISTBOX, &FG2ListboxTabs[0], ARRAY_SIZE (FG2ListboxTabs));

		/* Function generator 1 */

		// IDC_FG1_AMOUNT
		InitSlider (hWnd, IDC_FG1_AMOUNT, -128, 127, 0);

		// IDC_FG1_LISTBOX

		/* Function generator 2 */

		// IDC_FG2_AMOUNT
		InitSlider (hWnd, IDC_FG2_AMOUNT, -128, 127, 0);

		// IDC_FG2_LISTBOX
	}

	// If the LFOs and AUXENV are on display
	if (IsPresetSubDialogDisplayed (LFO_AUXENV))
	{
		/* LFO 1 */

		// IDC_LFO1_SHAPE
		InitListbox (GetDlgItem (hWnd, IDC_LFO1_SHAPE), pszLfoShapes, ARRAY_SIZE (pszLfoShapes));

		// IDC_LFO1_RATE
		InitSlider (hWnd, IDC_LFO1_RATE, 0, 127, 0);

		// IDC_LFO1_DELAY
		InitSlider (hWnd, IDC_LFO1_DELAY, 0, 127, 0);

		// IDC_LFO1_VARIATION
		InitSlider (hWnd, IDC_LFO1_VARIATION, 0, 127, 0);

		// IDC_LFO1_AMOUNT
		InitSlider (hWnd, IDC_LFO1_AMOUNT, -128, 127, 0);

		/* LFO 2 */

		// IDC_LFO2_SHAPE
		InitListbox (GetDlgItem (hWnd, IDC_LFO2_SHAPE), pszLfoShapes, ARRAY_SIZE (pszLfoShapes));

		// IDC_LFO2_RATE
		InitSlider (hWnd, IDC_LFO2_RATE, 0, 127, 0);

		// IDC_LFO2_DELAY
		InitSlider (hWnd, IDC_LFO2_DELAY, 0, 127, 0);

		// IDC_LFO2_VARIATION
		InitSlider (hWnd, IDC_LFO2_VARIATION, 0, 127, 0);

		// IDC_LFO2_AMOUNT
		InitSlider (hWnd, IDC_LFO2_AMOUNT, -128, 127, 0);

		/* Auxilliary envelope */

		// IDC_AUX_ENV_AMOUNT
		InitSlider (hWnd, IDC_AUX_ENV_AMOUNT, -128, 127, 0);

		// IDC_AUX_ENV_DELAY
		InitSlider (hWnd, IDC_AUX_ENV_DELAY, 0, 127, 0);

		// IDC_AUX_ENV_ATTACK
		InitSlider (hWnd, IDC_AUX_ENV_ATTACK, 0, 99, 0);

		// IDC_AUX_ENV_HOLD
		InitSlider (hWnd, IDC_AUX_ENV_HOLD, 0, 99, 0);

		// IDC_AUX_ENV_DECAY
		InitSlider (hWnd, IDC_AUX_ENV_DECAY, 0, 99, 0);

		// IDC_AUX_ENV_SUSTAIN
		InitSlider (hWnd, IDC_AUX_ENV_SUSTAIN, 0, 99, 0);

		// IDC_AUX_ENV_RELEASE
		InitSlider (hWnd, IDC_AUX_ENV_RELEASE, 0, 99, 0);

		// IDC_AUX_ENV_ENVELOPE
	}

	// If the MAIN stuff is on display
	if (IsPresetSubDialogDisplayed (MAIN))
	{
		/* Keyboard */

		// IDC_KBD_PRESSURE_AMOUNT
		InitUpDown (hWnd, IDC_KBD_PRESSURE_AMOUNT, -128, 127, 0, GetDlgItem (hWnd, IDC_KBD_PRESSURE_AMOUNT_VALUE), lpfnDisplayNumber);

		// IDC_KBD_BEND_RANGE
		InitUpDown (hWnd, IDC_KBD_BEND_RANGE, 0, 13, 0, GetDlgItem (hWnd, IDC_KBD_BEND_RANGE_VALUE), lpfnDisplayBendRange);

		// IDC_KBD_VELOCITY_CURVE
		InitListbox (GetDlgItem (hWnd, IDC_KBD_VELOCITY_CURVE), pszVelocityCurves, ARRAY_SIZE (pszVelocityCurves));

		// IDC_KBD_TUNING
		InitListbox (GetDlgItem (hWnd, IDC_KBD_TUNING), pszTunings, ARRAY_SIZE (pszTunings));

		// IDC_PORTAMENTO_MODE
		InitListbox (GetDlgItem (hWnd, IDC_PORTAMENTO_MODE), pszPortamentoModes, ARRAY_SIZE (pszPortamentoModes));
	}
}

/*^L*/
/*****************************************************************************
 Function:		InitMiscDialog
 Parameters:	HWND	hWnd			Dialog window
 Returns:		None.
 Description:	Initialises the misc dialog
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94	| Created.											| PW
*****************************************************************************/

VOID InitMiscDialog (HWND hWnd)
{
	static LISTBOX_TAB_INFO	NoteOnModsListboxTabs[] =
	{
		{IDC_MISC_NOTE_ON_LB_LABEL1,	0},
		{IDC_MISC_NOTE_ON_LB_LABEL2,	60},
		{IDC_MISC_NOTE_ON_LB_LABEL3,	120},
	};
	static LISTBOX_TAB_INFO	RealTimeModsListboxTabs[] =
	{
		{IDC_MISC_REAL_TIME_LB_LABEL1,	0},
		{IDC_MISC_REAL_TIME_LB_LABEL2,	60},
		{IDC_MISC_REAL_TIME_LB_LABEL3,	120},
	};

	// Only if the complete preset is on display
	if (!fBigPresetDialog && IsPresetSubDialogDisplayed (MAIN))
	{
		// Set the Patchcords listbox tabs
		SetListBoxTabs (hWnd, IDC_MISC_NOTE_ON_PATCH_LISTBOX,
						&NoteOnModsListboxTabs[0], ARRAY_SIZE (NoteOnModsListboxTabs));
		SetListBoxTabs (hWnd, IDC_MISC_REAL_TIME_PATCH_LISTBOX,
						&RealTimeModsListboxTabs[0], ARRAY_SIZE (RealTimeModsListboxTabs));
	}

	// If the MAIN stuff is on display
	if (IsPresetSubDialogDisplayed (MAIN))
	{
		// IDC_MISC_PRESET_NAME

		// IDC_MISC_MIX_SELECT
		InitListbox (GetDlgItem (hWnd, IDC_MISC_MIX_SELECT), pszMixBus, ARRAY_SIZE (pszMixBus));

		// IDC_KBD_LOW_KEY
		// IDC_KBD_LOW_KEY_EDITBOX
		InitUpDown (hWnd, IDC_KBD_LOW_KEY, 0, 127, 0, GetDlgItem (hWnd, IDC_KBD_LOW_KEY_EDITBOX), lpfnDisplayNote);

		// IDC_KBD_HIGH_KEY
		// IDC_KBD_HIGH_KEY_EDITBOX
		InitUpDown (hWnd, IDC_KBD_HIGH_KEY, 0, 127, 0, GetDlgItem (hWnd, IDC_KBD_HIGH_KEY_EDITBOX), lpfnDisplayNote);

		// IDC_KBD_CENTRE
		// IDC_KBD_CENTRE_EDITBOX
		InitUpDown (hWnd, IDC_KBD_CENTRE, 0, 127, 0, GetDlgItem (hWnd, IDC_KBD_CENTRE_EDITBOX), lpfnDisplayNote);


		// IDC_MISC_FOOTSWITCH1_CONTROL
		InitListbox (GetDlgItem (hWnd, IDC_MISC_FOOTSWITCH1_CONTROL), pszFootSwitchRoutings, ARRAY_SIZE (pszFootSwitchRoutings));

		// IDC_MISC_FOOTSWITCH2_CONTROL
		InitListbox (GetDlgItem (hWnd, IDC_MISC_FOOTSWITCH2_CONTROL), pszFootSwitchRoutings, ARRAY_SIZE (pszFootSwitchRoutings));

		// IDC_MISC_FOOTSWITCH3_CONTROL
		InitListbox (GetDlgItem (hWnd, IDC_MISC_FOOTSWITCH3_CONTROL), pszFootSwitchRoutings, ARRAY_SIZE (pszFootSwitchRoutings));

		// IDC_MISC_MIDI_A_AMOUNT
		InitSlider (hWnd, IDC_MISC_MIDI_A_AMOUNT, -128, 127, 0);

		// IDC_MISC_MIDI_B_AMOUNT
		InitSlider (hWnd, IDC_MISC_MIDI_B_AMOUNT, -128, 127, 0);

		// IDC_MISC_MIDI_C_AMOUNT
		InitSlider (hWnd, IDC_MISC_MIDI_C_AMOUNT, -128, 127, 0);

		// IDC_MISC_MIDI_D_AMOUNT
		InitSlider (hWnd, IDC_MISC_MIDI_D_AMOUNT, -128, 127, 0);

		// IDC_MISC_NOTE_ON_PATCH_LISTBOX
		// IDC_MISC_REAL_TIME_PATCH_LISTBOX

		/* Xfade */

		// IDC_XFADE_MODE
		InitListbox (GetDlgItem (hWnd, IDC_XFADE_MODE), pszXfadeMode, ARRAY_SIZE (pszXfadeMode));

		// IDC_XFADE_DIRECTION
		InitListbox (GetDlgItem (hWnd, IDC_XFADE_DIRECTION), pszXfadeDirection, ARRAY_SIZE (pszXfadeDirection));

		// IDC_XFADE_BALANCE
		InitSlider (hWnd, IDC_XFADE_BALANCE, 0, 127, 0);

		// IDC_XFADE_AMOUNT
		InitSlider (hWnd, IDC_XFADE_AMOUNT, 0, 255, 0);

		// IDC_XFADE_SWITCH_POINT_CONTROL
		// IDC_XFADE_SWITCH_POINT_EDITBOX
		InitUpDown (hWnd, IDC_XFADE_SWITCH_POINT_CONTROL, 0, 127, 0,
					GetDlgItem (hWnd, IDC_XFADE_SWITCH_POINT_EDITBOX), lpfnDisplayNoteAndNumber);
	}
}

/*^L*/
/*****************************************************************************
 Function:		UpdatePresetInternals
 Parameters:	None.
 Returns:		None.
 Description:	Updates all the necessary preset controls with the new
				Morpheus internal information.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/4/94	| Created.											| PW
*****************************************************************************/

VOID UpdatePresetInternals (VOID)
{
	UINT		i, num;

	// If the instruments are on display
	if (IsPresetSubDialogDisplayed (VOICES))
	{
		// Instruments->instrument names
		SendDlgItemMessage (hwndCurrentPreset, IDC_INSTR1_INSTRUMENT, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage (hwndCurrentPreset, IDC_INSTR2_INSTRUMENT, CB_RESETCONTENT, 0, 0);
		num = ReadSysexWord (&pMorpheusInstrumentList->wNumInstruments);
		for (i=0 ; i<num ; i++)
		{
			SendDlgItemMessage (hwndCurrentPreset, IDC_INSTR1_INSTRUMENT, CB_ADDSTRING, 0,
								(LPARAM)(LPCSTR)&pMorpheusInstrumentList->Info[i].szName[0]);
			SendDlgItemMessage (hwndCurrentPreset, IDC_INSTR2_INSTRUMENT, CB_ADDSTRING, 0,
								(LPARAM)(LPCSTR)&pMorpheusInstrumentList->Info[i].szName[0]);
		}
		SendDlgItemMessage (hwndCurrentPreset, IDC_INSTR1_INSTRUMENT, CB_ADDSTRING, 0, (LPARAM)ResourceString (IDS_PR_INSTR_NONE));
		SendDlgItemMessage (hwndCurrentPreset, IDC_INSTR2_INSTRUMENT, CB_ADDSTRING, 0, (LPARAM)ResourceString (IDS_PR_INSTR_NONE));
		SendDlgItemMessage (hwndCurrentPreset, IDC_INSTR1_INSTRUMENT, CB_SETCURSEL, 0, 0);
		SendDlgItemMessage (hwndCurrentPreset, IDC_INSTR2_INSTRUMENT, CB_SETCURSEL, 0, 0);

		// Filters->filter types
		SendDlgItemMessage (hwndCurrentPreset, IDC_FILTER1_TYPE, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage (hwndCurrentPreset, IDC_FILTER2_TYPE, CB_RESETCONTENT, 0, 0);
		num = ReadSysexWord (&pMorpheusFilterList->wNumFilters);
		for (i=0 ; i<num ; i++)
		{
			SendDlgItemMessage (hwndCurrentPreset, IDC_FILTER1_TYPE, CB_ADDSTRING, 0,
								(LPARAM)(LPCSTR)&pMorpheusFilterList->Info[i].szName[0]);
			SendDlgItemMessage (hwndCurrentPreset, IDC_FILTER2_TYPE, CB_ADDSTRING, 0,
								(LPARAM)(LPCSTR)&pMorpheusFilterList->Info[i].szName[0]);
		}
		SendDlgItemMessage (hwndCurrentPreset, IDC_FILTER1_TYPE, CB_ADDSTRING, 0, (LPARAM)ResourceString (IDS_PR_INSTR_NONE));
		SendDlgItemMessage (hwndCurrentPreset, IDC_FILTER2_TYPE, CB_ADDSTRING, 0, (LPARAM)ResourceString (IDS_PR_INSTR_NONE));
		SendDlgItemMessage (hwndCurrentPreset, IDC_FILTER1_TYPE, CB_SETCURSEL, 0, 0);
		SendDlgItemMessage (hwndCurrentPreset, IDC_FILTER2_TYPE, CB_SETCURSEL, 0, 0);
	}
}

/*^L*/
/*****************************************************************************
 Function:		DisplayPreset
 Parameters:	None.
 Returns:		None.
 Description:	Displays the current edit preset.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/4/94	| Created.											| PW
*****************************************************************************/

VOID DisplayPreset (VOID)
{
	HWND	hWnd;
	char	szName[20], szPatchBuf[200];
	UINT	i;

	// Format the preset name
	for (i=0 ; i<ARRAY_SIZE(EditPreset.Name) ; i++)
	{
		szName[i] = WNUM (EditPreset.Name[i]);
	}
	szName[i] = '\0';

	// Blimey, this will take a while

	// If the MAIN sub-dialog is on display
	if (IsPresetSubDialogDisplayed (MAIN))
	{
		// Take a local copy of the dialog handle for the next lot of stuff
		hWnd = hwndCurrentPreset;

		// Keyboard
		SendDlgItemMessage (hWnd, IDC_KBD_PRESSURE_AMOUNT, UD_SET_CURRENT, WNUM (EditPreset.PressureAmount), 0);
		SendDlgItemMessage (hWnd, IDC_KBD_BEND_RANGE, UD_SET_CURRENT, WNUM (EditPreset.BendRange), 0);
		SendDlgItemMessage (hWnd, IDC_KBD_VELOCITY_CURVE, CB_SETCURSEL, WNUM (EditPreset.VelocityCurve), 0);
		SendDlgItemMessage (hWnd, IDC_KBD_TUNING, CB_SETCURSEL, WNUM (EditPreset.TuneTable), 0);
		SendDlgItemMessage (hWnd, IDC_PORTAMENTO_MODE, CB_SETCURSEL, WNUM (EditPreset.PortMode), 0);

		// Xfade
		SendDlgItemMessage (hWnd, IDC_XFADE_MODE, CB_SETCURSEL, WNUM (EditPreset.XFade.Mode), 0);
		SendDlgItemMessage (hWnd, IDC_XFADE_DIRECTION, CB_SETCURSEL, WNUM (EditPreset.XFade.Direction), 0);
		SendDlgItemMessage (hWnd, IDC_XFADE_BALANCE, SL_SET_CURRENT, WNUM (EditPreset.XFade.Balance), 0);
		SendDlgItemMessage (hWnd, IDC_XFADE_AMOUNT, SL_SET_CURRENT, WNUM (EditPreset.XFade.Amount), 0);
		SendDlgItemMessage (hWnd, IDC_XFADE_SWITCH_POINT_CONTROL, UD_SET_CURRENT, WNUM (EditPreset.XFade.SwitchPoint), 0);

		// Miscellaneous
		SendDlgItemMessage (hWnd, IDC_KBD_LOW_KEY, UD_SET_CURRENT, WNUM (EditPreset.LowKey), 0);
		SendDlgItemMessage (hWnd, IDC_KBD_HIGH_KEY, UD_SET_CURRENT, WNUM (EditPreset.HighKey), 0);
		SendDlgItemMessage (hWnd, IDC_KBD_CENTRE, UD_SET_CURRENT, WNUM (EditPreset.KeyboardCentre), 0);

		SendDlgItemMessage (hWnd, IDC_MISC_MIX_SELECT, CB_SETCURSEL, WNUM (EditPreset.MixBus), 0);
		SendDlgItemMessage (hWnd, IDC_MISC_FOOTSWITCH1_CONTROL, CB_SETCURSEL, WNUM (EditPreset.FootSwitchDestination[0]), 0);
		SendDlgItemMessage (hWnd, IDC_MISC_FOOTSWITCH2_CONTROL, CB_SETCURSEL, WNUM (EditPreset.FootSwitchDestination[1]), 0);
		SendDlgItemMessage (hWnd, IDC_MISC_FOOTSWITCH3_CONTROL, CB_SETCURSEL, WNUM (EditPreset.FootSwitchDestination[2]), 0);
		SendDlgItemMessage (hWnd, IDC_MISC_MIDI_A_AMOUNT, SL_SET_CURRENT, WNUM (EditPreset.CtrlAAmount), 0);
		SendDlgItemMessage (hWnd, IDC_MISC_MIDI_B_AMOUNT, SL_SET_CURRENT, WNUM (EditPreset.CtrlBAmount), 0);
		SendDlgItemMessage (hWnd, IDC_MISC_MIDI_C_AMOUNT, SL_SET_CURRENT, WNUM (EditPreset.CtrlCAmount), 0);
		SendDlgItemMessage (hWnd, IDC_MISC_MIDI_D_AMOUNT, SL_SET_CURRENT, WNUM (EditPreset.CtrlDAmount), 0);
		SendDlgItemMessage (hWnd, IDC_MISC_PRESET_NAME, WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szName[0]);
	}

	// If the instruments are on display
	if (IsPresetSubDialogDisplayed (VOICES))
	{
		// Take a local copy of the dialog handle for the next lot of stuff
		hWnd = hwndCurrentPreset;

		// Instrument 1
		SendDlgItemMessage (hWnd, IDC_INSTR1_INSTRUMENT, CB_SETCURSEL,
							InstrumentNumberToIndex (WNUM (EditPreset.Layer[0].LayerInstrument)), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_LOW_KEY_CONTROL, UD_SET_CURRENT, WNUM (EditPreset.Layer[0].LowKey), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_HIGH_KEY_CONTROL, UD_SET_CURRENT, WNUM (EditPreset.Layer[0].HighKey), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_COARSE_TUNE, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].CoarseTune), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_FINE_TUNE, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].FineTune), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_KEY_TRANSPOSE, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].KeyXpose), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_DOUBLE_DETUNE, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].DoubleDetune), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_SOUND_DELAY, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].SoundDelay), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_SOUND_START, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].SoundStart), 0);
		DisplayPresetLoopStart (0);
		DisplayPresetLoopLength (0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_REVERSE, BM_SETCHECK, WNUM (EditPreset.Layer[0].SoundReverse), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_NON_TRANSPOSE, BM_SETCHECK, WNUM (EditPreset.Layer[0].NonTranspose), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_LOOP_ENABLE, BM_SETCHECK, WNUM (EditPreset.Layer[0].Loop.Enable), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_SOLO_MODE_CONTROL, CB_SETCURSEL, WNUM (EditPreset.Layer[0].SoloMode), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_SOLO_PRIORITY_CONTROL, CB_SETCURSEL, WNUM (EditPreset.Layer[0].SoloPriority), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR1_VOLUME, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].Volume), 0);

		// Filter 1
		SendDlgItemMessage (hWnd, IDC_FILTER1_TYPE, CB_SETCURSEL,
							FilterNumberToIndex (WNUM (EditPreset.Layer[0].Filter.Type)), 0);
		SendDlgItemMessage (hWnd, IDC_FILTER1_REVERSE, BM_SETCHECK, WNUM (EditPreset.Layer[0].Filter.Reverse), 0);
		SendDlgItemMessage (hWnd, IDC_FILTER1_LEVEL, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].Filter.Level), 0);
		SendDlgItemMessage (hWnd, IDC_FILTER1_MORPH, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].Filter.Morph), 0);
		SendDlgItemMessage (hWnd, IDC_FILTER1_FREQ_TRACK, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].Filter.FreqTrack), 0);
		SendDlgItemMessage (hWnd, IDC_FILTER1_TRANS2, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].Filter.Transform2), 0);

		// Portamento 1
		SendDlgItemMessage (hWnd, IDC_PORTAMENTO1_SHAPE, CB_SETCURSEL, WNUM (EditPreset.Layer[0].PortamentoShape), 0);
		SendDlgItemMessage (hWnd, IDC_PORTAMENTO1_RATE, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].PortamentoRate), 0);

		// Pan 1
		SendDlgItemMessage (hWnd, IDC_PAN1_POSITION, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].Pan), 0);

		// Alternate envelope 1
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENABLE, BM_SETCHECK, WNUM (EditPreset.Layer[0].AltEnv.Enable), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ATTACK, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].AltEnv.Attack), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENVELOPE, ENV_SET_ATTACK, WNUM (EditPreset.Layer[0].AltEnv.Attack), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_HOLD, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].AltEnv.Hold), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENVELOPE, ENV_SET_HOLD, WNUM (EditPreset.Layer[0].AltEnv.Hold), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_DECAY, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].AltEnv.Decay), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENVELOPE, ENV_SET_DECAY, WNUM (EditPreset.Layer[0].AltEnv.Decay), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_SUSTAIN, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].AltEnv.Sustain), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENVELOPE, ENV_SET_SUSTAIN, WNUM (EditPreset.Layer[0].AltEnv.Sustain), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_RELEASE, SL_SET_CURRENT, WNUM (EditPreset.Layer[0].AltEnv.Release), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENVELOPE, ENV_SET_RELEASE, WNUM (EditPreset.Layer[0].AltEnv.Release), 0);

		// Instrument 2
		hWnd = hwndCurrentPreset;
		SendDlgItemMessage (hWnd, IDC_INSTR2_INSTRUMENT, CB_SETCURSEL,
							InstrumentNumberToIndex (WNUM (EditPreset.Layer[1].LayerInstrument)), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_LOW_KEY_CONTROL, UD_SET_CURRENT, WNUM (EditPreset.Layer[1].LowKey), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_HIGH_KEY_CONTROL, UD_SET_CURRENT, WNUM (EditPreset.Layer[1].HighKey), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_COARSE_TUNE, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].CoarseTune), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_FINE_TUNE, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].FineTune), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_KEY_TRANSPOSE, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].KeyXpose), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_DOUBLE_DETUNE, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].DoubleDetune), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_SOUND_DELAY, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].SoundDelay), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_SOUND_START, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].SoundStart), 0);
		DisplayPresetLoopStart (1);
		DisplayPresetLoopLength (1);
		SendDlgItemMessage (hWnd, IDC_INSTR2_REVERSE, BM_SETCHECK, WNUM (EditPreset.Layer[1].SoundReverse), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_NON_TRANSPOSE, BM_SETCHECK, WNUM (EditPreset.Layer[1].NonTranspose), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_LOOP_ENABLE, BM_SETCHECK, WNUM (EditPreset.Layer[1].Loop.Enable), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_SOLO_MODE_CONTROL, CB_SETCURSEL, WNUM (EditPreset.Layer[1].SoloMode), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_SOLO_PRIORITY_CONTROL, CB_SETCURSEL, WNUM (EditPreset.Layer[1].SoloPriority), 0);
		SendDlgItemMessage (hWnd, IDC_INSTR2_VOLUME, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].Volume), 0);

		// Filter 2
		SendDlgItemMessage (hWnd, IDC_FILTER2_TYPE, CB_SETCURSEL,
							FilterNumberToIndex (WNUM (EditPreset.Layer[1].Filter.Type)), 0);
		SendDlgItemMessage (hWnd, IDC_FILTER2_REVERSE, BM_SETCHECK, WNUM (EditPreset.Layer[1].Filter.Reverse), 0);
		SendDlgItemMessage (hWnd, IDC_FILTER2_LEVEL, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].Filter.Level), 0);
		SendDlgItemMessage (hWnd, IDC_FILTER2_MORPH, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].Filter.Morph), 0);
		SendDlgItemMessage (hWnd, IDC_FILTER2_FREQ_TRACK, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].Filter.FreqTrack), 0);
		SendDlgItemMessage (hWnd, IDC_FILTER2_TRANS2, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].Filter.Transform2), 0);

		// Portamento 2
		SendDlgItemMessage (hWnd, IDC_PORTAMENTO2_SHAPE, CB_SETCURSEL, WNUM (EditPreset.Layer[1].PortamentoShape), 0);
		SendDlgItemMessage (hWnd, IDC_PORTAMENTO2_RATE, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].PortamentoRate), 0);

		// Pan 2
		SendDlgItemMessage (hWnd, IDC_PAN2_POSITION, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].Pan), 0);

		// Alternate envelope 2
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENABLE, BM_SETCHECK, WNUM (EditPreset.Layer[1].AltEnv.Enable), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ATTACK, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].AltEnv.Attack), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENVELOPE, ENV_SET_ATTACK, WNUM (EditPreset.Layer[1].AltEnv.Attack), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_HOLD, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].AltEnv.Hold), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENVELOPE, ENV_SET_HOLD, WNUM (EditPreset.Layer[1].AltEnv.Hold), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_DECAY, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].AltEnv.Decay), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENVELOPE, ENV_SET_DECAY, WNUM (EditPreset.Layer[1].AltEnv.Decay), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_SUSTAIN, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].AltEnv.Sustain), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENVELOPE, ENV_SET_SUSTAIN, WNUM (EditPreset.Layer[1].AltEnv.Sustain), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_RELEASE, SL_SET_CURRENT, WNUM (EditPreset.Layer[1].AltEnv.Release), 0);
		SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENVELOPE, ENV_SET_RELEASE, WNUM (EditPreset.Layer[1].AltEnv.Release), 0);
	}

	// If the LFOs and ANXENV are on display
	if (IsPresetSubDialogDisplayed (LFO_AUXENV))
	{
		// Take a local copy of the dialog handle for the next lot of stuff
		hWnd = hwndCurrentPreset;

		// LFO 1
		SendDlgItemMessage (hWnd, IDC_LFO1_SHAPE, CB_SETCURSEL, WNUM (EditPreset.Lfo[0].Shape), 0);
		SendDlgItemMessage (hWnd, IDC_LFO1_RATE, SL_SET_CURRENT, WNUM (EditPreset.Lfo[0].Rate), 0);
		SendDlgItemMessage (hWnd, IDC_LFO1_DELAY, SL_SET_CURRENT, WNUM (EditPreset.Lfo[0].Delay), 0);
		SendDlgItemMessage (hWnd, IDC_LFO1_VARIATION, SL_SET_CURRENT, WNUM (EditPreset.Lfo[0].Variation), 0);
		SendDlgItemMessage (hWnd, IDC_LFO1_AMOUNT, SL_SET_CURRENT, WNUM (EditPreset.Lfo[0].Amount), 0);

		// LFO 2
		SendDlgItemMessage (hWnd, IDC_LFO2_SHAPE, CB_SETCURSEL, WNUM (EditPreset.Lfo[1].Shape), 0);
		SendDlgItemMessage (hWnd, IDC_LFO2_RATE, SL_SET_CURRENT, WNUM (EditPreset.Lfo[1].Rate), 0);
		SendDlgItemMessage (hWnd, IDC_LFO2_DELAY, SL_SET_CURRENT, WNUM (EditPreset.Lfo[1].Delay), 0);
		SendDlgItemMessage (hWnd, IDC_LFO2_VARIATION, SL_SET_CURRENT, WNUM (EditPreset.Lfo[1].Variation), 0);
		SendDlgItemMessage (hWnd, IDC_LFO2_AMOUNT, SL_SET_CURRENT, WNUM (EditPreset.Lfo[1].Amount), 0);

		// Auxilliary envelope
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_DELAY, SL_SET_CURRENT, WNUM (EditPreset.AuxEnv.Delay), 0);
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_ATTACK, SL_SET_CURRENT, WNUM (EditPreset.AuxEnv.Attack), 0);
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_HOLD, SL_SET_CURRENT, WNUM (EditPreset.AuxEnv.Hold), 0);
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_DECAY, SL_SET_CURRENT, WNUM (EditPreset.AuxEnv.Decay), 0);
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_SUSTAIN, SL_SET_CURRENT, WNUM (EditPreset.AuxEnv.Sustain), 0);
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_RELEASE, SL_SET_CURRENT, WNUM (EditPreset.AuxEnv.Release), 0);
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_AMOUNT, SL_SET_CURRENT, WNUM (EditPreset.AuxEnv.Amount), 0);

		SendDlgItemMessage (hWnd, IDC_AUX_ENV_ENVELOPE, ENV_SET_ATTACK, WNUM (EditPreset.AuxEnv.Attack), 0);
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_ENVELOPE, ENV_SET_HOLD, WNUM (EditPreset.AuxEnv.Hold), 0);
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_ENVELOPE, ENV_SET_DECAY, WNUM (EditPreset.AuxEnv.Decay), 0);
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_ENVELOPE, ENV_SET_SUSTAIN, WNUM (EditPreset.AuxEnv.Sustain), 0);
		SendDlgItemMessage (hWnd, IDC_AUX_ENV_ENVELOPE, ENV_SET_RELEASE, WNUM (EditPreset.AuxEnv.Release), 0);
	}

	// If the FGs are on display
	if (IsPresetSubDialogDisplayed (FG))
	{
		// Take a local copy of the dialog handle for the next lot of stuff
		hWnd = hwndCurrentPreset;

		// Function generator 1
		SendDlgItemMessage (hWnd, IDC_FG1_AMOUNT, SL_SET_CURRENT, WNUM (EditPreset.FunctionGenerator[0].Amount), 0);
		DisplayFuncGenList (GetDlgItem (hWnd, IDC_FG1_LISTBOX), &EditPreset.FunctionGenerator[0].Segments);

		// Function generator 2
		SendDlgItemMessage (hWnd, IDC_FG2_AMOUNT, SL_SET_CURRENT, WNUM (EditPreset.FunctionGenerator[1].Amount), 0);
		DisplayFuncGenList (GetDlgItem (hWnd, IDC_FG2_LISTBOX), &EditPreset.FunctionGenerator[1].Segments);
	}

	// If the sub-preset MAIN/Modulators dialog is on display
	if (!fBigPresetDialog && IsPresetSubDialogDisplayed (MAIN))
	{
		// Take a local copy of the dialog handle for the next lot of stuff
		hWnd = hwndCurrentPreset;

		// Note-on patch list
		SendDlgItemMessage (hWnd, IDC_MISC_NOTE_ON_PATCH_LISTBOX, LB_RESETCONTENT, 0, 0);
		for (i=0 ; i<NUM_NOTE_ON_PATCHES ; i++)
		{
			FormatPatchLine (NO, i, &szPatchBuf[0]);
			SendDlgItemMessage (hWnd, IDC_MISC_NOTE_ON_PATCH_LISTBOX, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szPatchBuf[0]);
		}

		// Real-time patch list
		SendDlgItemMessage (hWnd, IDC_MISC_REAL_TIME_PATCH_LISTBOX, LB_RESETCONTENT, 0, 0);
		for (i=0 ; i<NUM_REAL_TIME_PATCHES ; i++)
		{
			FormatPatchLine (RT, i, &szPatchBuf[0]);
			SendDlgItemMessage (hWnd, IDC_MISC_REAL_TIME_PATCH_LISTBOX, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szPatchBuf[0]);
		}
	}

	// Display the patchcords
	DisplayNoteOnPatch ();
	DisplayRealTimePatch ();

	// Set all 3 editing window titles
	if (fBigPresetDialog)
	{
		wsprintf (&szPatchBuf[0], ResourceString (IDS_PR_ALL_WINDOW_TITLE), (LPSTR)&szName[0]);
	}
	else if (IsPresetSubDialogDisplayed (MAIN))
	{
		wsprintf (&szPatchBuf[0], ResourceString (IDS_PR_MAIN_WINDOW_TITLE), (LPSTR)&szName[0]);
	}
	else if (IsPresetSubDialogDisplayed (VOICES))
	{
		wsprintf (&szPatchBuf[0], ResourceString (IDS_PR_VOICE_WINDOW_TITLE), (LPSTR)&szName[0]);
	}
	else if (IsPresetSubDialogDisplayed (LFO_AUXENV))
	{
		wsprintf (&szPatchBuf[0], ResourceString (IDS_PR_LFO_WINDOW_TITLE), (LPSTR)&szName[0]);
	}
	else if (IsPresetSubDialogDisplayed (FG))
	{
		wsprintf (&szPatchBuf[0], ResourceString (IDS_PR_FG_WINDOW_TITLE), (LPSTR)&szName[0]);
	}
	SetWindowText (hwndCurrentPreset, &szPatchBuf[0]);

	wsprintf (&szPatchBuf[0], ResourceString (IDS_PR_NO_WINDOW_TITLE), (LPSTR)&szName[0]);
	SetWindowText (hwndNoteOnPatches, &szPatchBuf[0]);
	wsprintf (&szPatchBuf[0], ResourceString (IDS_PR_RT_WINDOW_TITLE), (LPSTR)&szName[0]);
	SetWindowText (hwndRealTimePatches, &szPatchBuf[0]);
}

/*^L*/
/*****************************************************************************
 Function:		FormatPatchLine
 Parameters:	int		Type			RT or NO.
				int		nPatch			Patch number.
				char	*pszBuffer		Output buffer.
 Returns:		None.
 Description:	Formats a line of patchcord information. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 2/5/94		| Created.											| PW
*****************************************************************************/

VOID FormatPatchLine (int Type, int nPatch, char *pszBuffer)
{
	// Handle real-time mods
	if (Type == RT)
	{
		wsprintf (pszBuffer, "%s\t%s\t%d",
					pszRealTimeSources[EditPreset.RealTimeModulation[nPatch].Source],
					pszPatchDests[EditPreset.RealTimeModulation[nPatch].Destination],
					WNUM (EditPreset.RealTimeModulation[nPatch].Amount));
	}
	else
	// Must be note-on
	{
		wsprintf (pszBuffer, "%s\t%s\t%d",
					pszNoteOnSources[EditPreset.NoteOnModulation[nPatch].Source],
					pszPatchDests[EditPreset.NoteOnModulation[nPatch].Destination],
					WNUM (EditPreset.NoteOnModulation[nPatch].Amount));
	}
}

/*^L*/
/*****************************************************************************
 Function:		DisplayPresetLoopStart
 Parameters:	int		nLayer			Preset layer (0 or 1).
 Returns:		None.
 Description:	Updates the loop start edit box. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/6/94	| Created.											| PW
*****************************************************************************/

VOID DisplayPresetLoopStart (int nLayer)
{
	int				nHigh, nLow;
	PPRESET_PARAMS	pPreset;
	char			szNumBuf[40];
	LONG			lVal;

	// If no preset is on display
	if (nDisplayedPreset == INDEX_NONE)
	{
		// Go no further
		return;
	}
	pPreset = pMorpheusPresets[nDisplayedPreset];

	// Get the current MS and LS
	nHigh	= WNUM (EditPreset.Layer[nLayer].Loop.StartMS);
	nLow	= WNUM (EditPreset.Layer[nLayer].Loop.StartLS);
	lVal	= (LONG)nHigh * 1000L + (LONG)nLow;

	// Format and output
	wsprintf (&szNumBuf[0], (lVal < 0) ? "-%03d" : "%03d", abs (nHigh));
	SendDlgItemMessage (hwndCurrentPreset,
						nLayer == 0 ? IDC_INSTR1_LOOP_START_HIGH_VALUE : IDC_INSTR2_LOOP_START_HIGH_VALUE,
						WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szNumBuf[0]);

	// Format and output
	wsprintf (&szNumBuf[0], "%03d", abs (nLow));
	SendDlgItemMessage (hwndCurrentPreset,
						nLayer == 0 ? IDC_INSTR1_LOOP_START_LOW_VALUE : IDC_INSTR2_LOOP_START_LOW_VALUE,
						WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szNumBuf[0]);
}

/*^L*/
/*****************************************************************************
 Function:		DisplayPresetLoopLength
 Parameters:	int		nLayer			Preset layer (0 or 1).
 Returns:		None.
 Description:	Updates the loop length edit box. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/6/94	| Created.											| PW
*****************************************************************************/

VOID DisplayPresetLoopLength (int nLayer)
{
	int				nHigh, nLow;
	PPRESET_PARAMS	pPreset;
	char			szNumBuf[40];
	LONG			lVal;

	// If no preset is on display
	if (nDisplayedPreset == INDEX_NONE)
	{
		// Go no further
		return;
	}
	pPreset = pMorpheusPresets[nDisplayedPreset];

	// Get the current MS and LS
	nHigh	= WNUM (EditPreset.Layer[nLayer].Loop.SizeOffsetMS);
	nLow	= WNUM (EditPreset.Layer[nLayer].Loop.SizeOffsetLS);
	lVal	= (LONG)nHigh * 1000L + (LONG)nLow;

	// Format and output
	wsprintf (&szNumBuf[0], (lVal < 0) ? "-%03d" : "%03d", abs (nHigh));
	SendDlgItemMessage (hwndCurrentPreset,
						nLayer == 0 ? IDC_INSTR1_LOOP_LENGTH_HIGH_VALUE : IDC_INSTR2_LOOP_LENGTH_HIGH_VALUE,
						WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szNumBuf[0]);

	// Format and output
	wsprintf (&szNumBuf[0], "%03d", abs (nLow));
	SendDlgItemMessage (hwndCurrentPreset,
						nLayer == 0 ? IDC_INSTR1_LOOP_LENGTH_LOW_VALUE : IDC_INSTR2_LOOP_LENGTH_LOW_VALUE,
						WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szNumBuf[0]);
}

