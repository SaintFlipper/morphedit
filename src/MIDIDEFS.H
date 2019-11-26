/*****************************************************************************
 File:			mididefs.h
 Description:	The include file containing Morpheus MIDI definitions.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 2/5/94		| Created.											| PW
*****************************************************************************/

#ifndef _INC_MIDIDEFS
#define _INC_MIDIDEFS

// Number of patches of each type
#define	NUM_NOTE_ON_PATCHES				10
#define	NUM_REAL_TIME_PATCHES			10

// Effect channel
#define	FXA								0
#define	FXB								1

// Note-on patchcord sources (nb. these are the actual MIDI values too)
#define	NOS_KEY							0
#define	NOS_VELOCITY					1
#define	NOS_PITCHWHEEL					2
#define	NOS_CONTROL_A					3 
#define	NOS_CONTROL_B					4
#define	NOS_CONTROL_C					5
#define	NOS_CONTROL_D					6
#define	NOS_MONO_PRESSURE				7
#define	NOS_FREE_RUN_FG					8

// Real-time patchcord source (also the MIDI values)
#define	RTS_PITCHWHEEL					0
#define	RTS_CONTROL_A					1
#define	RTS_CONTROL_B					2
#define	RTS_CONTROL_C					3
#define	RTS_CONTROL_D					4
#define	RTS_MONO_PRESSURE				5
#define	RTS_POLY_PRESSURE				6
#define	RTS_LFO1						7
#define	RTS_LFO2						8
#define	RTS_AUX_ENV						9
#define	RTS_FG1							10
#define	RTS_FG2							11
#define	RTS_FREE_RUN_FG					12
#define	RTS_PORT_1						13			// Internal
#define	RTS_PORT_2						14			// Internal
#define	RTS_AHDSR1						15			// Internal
#define	RTS_AHDSR2						16			// Internal
#define	RTS_XFADE						17			// Internal

// Patchcord destinations
#define D_OFF							0
#define	D_PITCH							1
#define	D_PITCHP						2
#define	D_PITCHS						3
#define	D_VOLUME						4
#define	D_VOLUMEP						5
#define	D_VOLUMES						6
#define	D_ATTACK						7
#define	D_ATTACKP						8
#define	D_ATTACKS						9
#define	D_DECAY							10
#define	D_DECAYP						11
#define	D_DECAYS						12
#define	D_RELEASE						13
#define	D_RELEASEP						14
#define	D_RELEASES						15
#define	D_XFADE							16
#define	D_LFO1_AMOUNT					17
#define	D_LFO1_RATE						18
#define	D_LFO2_AMOUNT					19
#define	D_LFO2_RATE						20
#define	D_AUX_AMOUNT					21
#define	D_AUX_ATTACK					22
#define	D_AUX_DECAY						23
#define	D_AUX_RELEASE					24
#define	D_START							25
#define	D_STARTP						26
#define	D_STARTS						27
#define	D_PAN							28
#define	D_PANP							29
#define	D_PANS							30
#define	D_TONE							31
#define	D_TONEP							32
#define	D_TONES							33
#define	D_MORPH							34
#define	D_MORPH_P						35
#define	D_MORPH_S						36
#define	D_TRANS2						37
#define	D_TRANS2_P						38
#define	D_TRANS2_S						39
#define	D_PORT_RATE						40
#define	D_PORT_RATE_P					41
#define	D_PORT_RATE_S					42
#define	D_FG1_AMOUNT					43
#define	D_FG2_AMOUNT					44
#define	D_FILTER_LEVEL					45
#define	D_FILTER_LEVEL_P				46
#define	D_FILTER_LEVEL_S				47
#define	D_FREQ_TRACK					48
#define	D_FREQ_TRACK_P					49
#define	D_FREQ_TRACK_S					50
#define	D_INSTR1_PORT					51	// Internal
#define	D_INSTR2_PORT					52	// Internal
#define	D_DCA1_VOL						53	// Internal
#define	D_DCA2_VOL						54	// Internal
#define D_DCA_XFADE						55	// Internal

// Patchcord destination types
#define	RT								1
#define	NO								2
#define	RTNO							3

// Global SYSEX flags
#define	SYSEX_START					0xF0
#define	SYSEX_END					0xF7

// Emu / Morpheus IDs
#define	EMU_MFG_ID					0x18
#define	MORPHEUS_PRODUCT_ID			0x0C

// Fixed SYSEX header length
#define	SYSEX_HEADER_LEN			5

// Number (or maximum number) of Morpheus parameters
#define	MAX_PRESETS					1024
#define	MAX_HYPERPRESETS			1024
#define	MAX_MIDIMAPS				256
#define	NUM_PROGRAM_MAPS			4

#define	NUM_FG_SEGMENT_SHAPES		63
#define	NUM_FG_JUMP_CONDITIONS		16

// System Exclusive message command IDs
#define	PRESET_REQUEST				0x00
#define	PRESET_DATA					0x01
#define	PARAMETER_REQUEST			0x02
#define	PARAMETER_DATA				0x03
#define	TUNE_TABLE_REQUEST      	0x04
#define	TUNE_TABLE_DATA				0x05
#define PROG_MAP_REQUEST			0x06
#define PROG_MAP_DATA				0x07
#define	MASTER_DATA_REQUEST			0x08
#define	VERSION_REQUEST				0x0A
#define VERSION_DATA				0x0B
#define	CONFIGURATION_REQUEST		0x0C
#define	CONFIGURATION_DATA			0x0D
#define	INSTRUMENT_LIST_REQUEST		0x0E
#define	INSTRUMENT_LIST_DATA		0x0F
#define	PRESET_LIST_REQUEST			0x12
#define	PRESET_LIST_DATA			0x13
#define HYPERPRESET_REQUEST			0x44
#define	HYPERPRESET_DATA			0x45
#define	MIDIMAP_REQUEST				0x46
#define	MIDIMAP_DATA				0x47
#define	HYPERPRESET_LIST_REQUEST	0x50
#define	HYPERPRESET_LIST_DATA		0x51
#define	MIDIMAP_LIST_REQUEST		0x52
#define	MIDIMAP_LIST_DATA			0x53
#define	EFFECT_LIST_REQUEST			0x54
#define	EFFECT_LIST_DATA			0x55
#define	FILTER_LIST_REQUEST			0x56
#define	FILTER_LIST_DATA			0x57

// Solo modes

#define	SOLO_MODE_OFF				0
#define	SOLO_MODE_WIND				1
#define	SOLO_MODE_SYNTH				2

// Solo priorities

#define	SOLO_PRIORITY_HIGH			0
#define	SOLO_PRIORITY_LOW			1
#define	SOLO_PRIORITY_FIRST			2
#define	SOLO_PRIORITY_LAST			3
#define	SOLO_PRIORITY_DRUM			4

// Parameter numbers
#define	MPARAM_BASIC_CHANNEL		256
#define	MPARAM_CHANNEL_VOLUME		257
#define	MPARAM_CHANNEL_PAN			258
#define	MPARAM_CURRENT_PROGRAM		259
#define	MPARAM_MASTER_TUNE			260
#define	MPARAM_MASTER_TRANSPOSE		261
#define	MPARAM_GLOBAL_BEND			262
#define	MPARAM_GLOBAL_VELCURVE		263
#define	MPARAM_MIDI_MODE			264
#define	MPARAM_CONTROL_A			266
#define	MPARAM_CONTROL_B			267
#define	MPARAM_CONTROL_C			268
#define	MPARAM_CONTROL_D			269
#define	MPARAM_FOOTSWITCH_1			270
#define	MPARAM_FOOTSWITCH_2			271
#define	MPARAM_FOOTSWITCH_3			272
#define	MPARAM_MIDI_MODECHANGE_ENABLE	273
#define	MPARAM_SYSEX_DEVICE_ID		274
#define	MPARAM_CURRENT_MIDIMAP		320
#define	MPARAM_AUTO_SELECT			323
#define	MPARAM_PACKET_DELAY			324
#define	MPARAM_PROTEUS_SYSEX		325
#define	MPARAM_COMPARE_MODE			326

#define	MPARAM_MIDIMAP_BASE			2048
#define	MPARAM_PRESET_BASE			8192
#define	MPARAM_HYPERPRESET_BASE		8704

#endif
