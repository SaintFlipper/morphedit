/*****************************************************************************
 File:			morpheus.h
 Description:	The main include file for the Morpheus editor.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 16/3/94	| Created.											| PW
*****************************************************************************/

#if defined (__WIN32__) || defined (_WIN32)
 #if !defined (WINDOWS32)
  #define	WINDOWS32
 #endif
#endif

// Standard C library headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Windows headers
#include <windowsx.h>
#include <windows.h>
#include <mmsystem.h>
#include <Winuser.h>
#include <ctl3d.h>

#if !defined (WINDOWS32)
 #include <toolhelp.h>
#endif

//#ifdef __BORLANDC__
// #include <dir.h>
//#endif

// Project headers
#include <aucntrl.h>
#include <morphdll.h>
#include "mididefs.h"

//#define	USE_SYSEX_DIALOG
//#define	ITSYBITS_WINDOWS

/****************************************************************************/
/*		Current editor version												*/	
/****************************************************************************/

/****************************************************************************/
/*		Resource strings													*/	
/****************************************************************************/

#define	IDS_GLOBAL						1000
#define	IDS_ATOZ						1001
#define	IDS_ERROR_TITLE					1002
#define	IDS_QUESTION_TITLE				1003
#define	IDS_WARNING_TITLE				1004
#define	IDS_OUT_OF_MEMORY				1005
#define	IDS_CDERR_FINDRESFAILURE		1006
#define	IDS_CDERR_INITIALIZATION		1007
#define	IDS_CDERR_LOADRESFAILURE		1008
#define	IDS_CDERR_LOCKRESFAILURE		1009
#define	IDS_CDERR_LOADSTRFAILURE		1010
#define	IDS_CDERR_MEMALLOCFAILURE		1011
#define	IDS_CDERR_MEMLOCKFAILURE		1012
#define	IDS_CDERR_NOHINSTANCE			1013
#define	IDS_CDERR_NOHOOK				1014
#define	IDS_CDERR_NOTEMPLATE			1015
#define	IDS_CDERR_REGISTERMSGFAIL		1016
#define	IDS_CDERR_STRUCTSIZE			1017
#define	IDS_CFERR_NOFONTS				1018
#define	IDS_CFERR_MAXLESSTHANMIN		1019
#define	IDS_FNERR_BUFFERTOOSMALL		1020
#define	IDS_FNERR_INVALIDFILENAME		1021
#define	IDS_FNERR_SUBCLASSFAILURE		1022
#define	IDS_FRERR_BUFFERLENGTHZERO		1023
#define	IDS_PDERR_CREATEICFAILURE		1024
#define	IDS_PDERR_DEFAULTDIFFERENT		1025
#define	IDS_PDERR_DNDMMISMATCH			1026
#define	IDS_PDERR_GETDEVMODEFAIL		1027
#define	IDS_PDERR_INITFAILURE			1028
#define	IDS_PDERR_LOADDRVFAILURE		1029
#define	IDS_PDERR_NODEFAULTPRN			1030
#define	IDS_PDERR_NODEVICES				1031
#define	IDS_PDERR_PARSEFAILURE			1032
#define	IDS_PDERR_PRINTERNOTFOUND		1033
#define	IDS_PDERR_RETDEFFAILURE			1034
#define	IDS_PDERR_SETUPFAILURE			1035
#define	IDS_CDERR_UNKNOWN				1036
#define	IDS_CDERR_TITLE					1037
#define	IDS_ERROR_FILE_OPEN				1038
#define	IDS_ERROR_FILE_READ				1039
#define	IDS_ERROR_WRONG_FILE_TYPE		1040
#define	IDS_ERROR_CORRUPT_FILE			1041
#define	IDS_NOT_CONNECTED_NO_HP			1042
#define	IDS_SAVE_HP_FILEBOX_TITLE		1043
#define	IDS_SAVE_HP_FILEBOX_MASK		1044
#define	IDS_SAVE_HP_ERROR_FETCHING		1045
#define	IDS_SAVE_HP_ERROR_DISK			1046
#define	IDS_LOAD_HP_ARE_YOU_SURE		1047
#define	IDS_LOAD_HP_FILEBOX_TITLE		1048
#define	IDS_LOAD_HP_FILEBOX_MASK		1049
#define	IDS_LOAD_HP_ERROR				1050
#define	IDS_NOT_CONNECTED_NO_MM			1051
#define	IDS_SAVE_MM_FILEBOX_TITLE		1052
#define	IDS_SAVE_MM_FILEBOX_MASK		1053
#define	IDS_SAVE_MM_ERROR_FETCHING		1054
#define	IDS_SAVE_MM_ERROR_DISK			1055
#define	IDS_LOAD_MM_ARE_YOU_SURE		1056
#define	IDS_LOAD_MM_FILEBOX_TITLE		1057
#define	IDS_LOAD_MM_FILEBOX_MASK		1058
#define	IDS_LOAD_MM_ERROR				1059
#define	IDS_NOT_CONNECTED_NO_PM			1060
#define	IDS_SAVE_PM_FILEBOX_TITLE		1061
#define	IDS_SAVE_PM_FILEBOX_MASK		1062
#define	IDS_SAVE_PM_ERROR_FETCHING		1063
#define	IDS_SAVE_PM_ERROR_DISK			1064
#define	IDS_LOAD_PM_ARE_YOU_SURE		1065
#define	IDS_LOAD_PM_FILEBOX_TITLE		1066
#define	IDS_LOAD_PM_FILEBOX_MASK		1067
#define	IDS_LOAD_PM_ERROR				1068
#define	IDS_NOT_CONNECTED_NO_PR			1069
#define	IDS_SAVE_PR_FILEBOX_TITLE		1070
#define	IDS_SAVE_PR_FILEBOX_MASK		1071
#define	IDS_SAVE_PR_ERROR_FETCHING		1072
#define	IDS_SAVE_PR_ERROR_DISK			1073
#define	IDS_LOAD_PR_ARE_YOU_SURE		1074
#define	IDS_LOAD_PR_FILEBOX_TITLE		1075
#define	IDS_LOAD_PR_FILEBOX_MASK		1076
#define	IDS_LOAD_PR_ERROR				1077
#define	IDS_SAVE_ALL_PRS_NOT_LOADED		1078
#define	IDS_SAVE_ALL_NOT_CON_PRS		1079
#define	IDS_SAVE_ALL_HPS_NOT_LOADED		1080
#define	IDS_SAVE_ALL_NOT_CON_HPS		1081
#define	IDS_SAVE_ALL_MMS_NOT_LOADED		1082
#define	IDS_SAVE_ALL_NOT_CON_MMS		1083
#define	IDS_SAVE_ALL_FILEBOX_TITLE		1084
#define	IDS_SAVE_ALL_FILEBOX_MASK		1085
#define	IDS_SAVE_ALL_ABORTED			1086
#define	IDS_LOAD_ALL_NO_CONFIG			1087
#define	IDS_LOAD_ALL_ARE_YOU_SURE		1088
#define	IDS_LOAD_ALL_FILEBOX_TITLE		1089
#define	IDS_LOAD_ALL_FILEBOX_MASK		1090
#define	IDS_LOAD_ALL_TOO_MANY_PRS		1091
#define	IDS_LOAD_ALL_TOO_FEW_PRS		1092
#define	IDS_LOAD_ALL_TOO_MANY_HPS		1093
#define	IDS_LOAD_ALL_TOO_FEW_HPS		1094
#define	IDS_LOAD_ALL_TOO_MANY_MMS		1095
#define	IDS_LOAD_ALL_TOO_FEW_MMS		1096
#define	IDS_NOT_CONNECTED_NO_UT			1097
#define	IDS_SAVE_UT_FILEBOX_TITLE		1098
#define	IDS_SAVE_UT_FILEBOX_MASK		1099
#define	IDS_SAVE_UT_ERROR_FETCHING		1100
#define	IDS_SAVE_UT_ERROR_DISK			1101
#define	IDS_LOAD_UT_ARE_YOU_SURE		1102
#define	IDS_LOAD_UT_FILEBOX_TITLE		1103
#define	IDS_LOAD_UT_FILEBOX_MASK		1104
#define	IDS_LOAD_UT_ERROR				1105
#define	IDS_EDIT_WINDOW_TITLE_MASK		1106
#define	IDS_NO_PRESET_NAME				1107
#define	IDS_FREE_RUN_FG_STEP_TITLE		1108
#define	IDS_HPS_LIST_WINDOW_TITLE		1109
#define	IDS_ERROR_CREATING_HPS_LIST		1110
#define	IDS_MODIFIED_HP_NOT_SAVED		1111
#define	IDS_ERROR_SENDING_HP			1112
#define	IDS_ERROR_OPENING_MIDI_DEVS		1113
#define	IDS_ERROR_REG_CLASSES			1114
#define	IDS_MAIN_WINDOW_TITLE			1115
#define	IDS_ERROR_CREATING_MAIN_WND		1116
#define	IDS_MIDI_STATUS_IDLE			1117
#define	IDS_ERROR_READING_MIDI_DATA		1118
#define	IDS_ERROR_STORING_INTERNALS		1119
#define	IDS_ERROR_CREATING_PS_WNDS		1120
#define	IDS_ERROR_CREATING_HPS_WNDS		1121
#define	IDS_ERROR_CREATING_MMS_WNDS		1122
#define	IDS_VERSION_MASK				1123
#define	IDS_YES_STRING					1124
#define	IDS_NO_STRING					1125
#define	IDS_CONNECT_QUERY				1126
#define	IDS_CONNECT_OVERWRITE_QUERY		1127
//#define	IDS_ERROR_STORING_INTERNALS		1128
#define	IDS_ERROR_READING_NAMELISTS		1129
#define	IDS_MIDI_STATUS_MASK			1130
#define	IDS_COMDLG_RET_MASK				1131
#define	IDS_MSTR_NOT_CONNECTED			1132
#define	IDS_MSTR_ERROR_READING			1133
#define	IDS_MSTR_ERROR_CREATING_WND		1134
#define	IDS_MIDI_ERR_BAD_DEVICE_ID		1135
#define	IDS_MIDI_ERR_ALREADY_USED		1136
#define	IDS_MIDI_ERR_OUT_OF_MEMORY		1137
#define	IDS_MIDI_ERR_NO_MIDI_MAP		1138
#define	IDS_MIDI_ERR_NO_DEV_PORT		1139
#define	IDS_MIDI_ERR_UNKNOWN_MASK		1140
#define	IDS_MIDI_ERR_MBOX_TITLE			1141
#define	IDS_SAVE_MIDI_PR_NOT_CON		1142
#define	IDS_SAVE_MIDI_PR_FBOX_TITLE		1143
#define	IDS_SAVE_MIDI_PR_FBOX_MASK		1144
#define	IDS_SAVE_MIDI_PR_FILE_MASK		1145
#define	IDS_SAVE_MIDI_HP_NOT_CON		1146
#define	IDS_SAVE_MIDI_HP_FBOX_TITLE		1147
#define	IDS_SAVE_MIDI_HP_FBOX_MASK		1148
#define	IDS_SAVE_MIDI_HP_FILE_MASK		1149
#define	IDS_MM_CREATING_WINDOW			1150
#define	IDS_MM_EDIT_CHANNEL_TITLE		1151
#define	IDS_MM_LIST_CREATING_WINDOW		1152
#define	IDS_MM_LIST_WINDOW_TITLE		1153
#define	IDS_MODIFIED_MM_NOT_SAVED		1154
#define	IDS_MM_ERROR_SENDING			1155
#define	IDS_MIDIRX_READING_CONFIG		1156
#define	IDS_MIDIRX_READING_VERSION		1157
#define	IDS_MIDIRX_READING_PRESET		1158
#define	IDS_MIDIRX_CHECKSUM_ERROR		1159
#define	IDS_MIDIRX_READING_HP			1160
#define	IDS_MIDIRX_READING_MM			1161
#define	IDS_MIDIRX_READING_INSLIST		1162
#define	IDS_MIDIRX_READING_FILTLIST		1163
#define	IDS_MIDIRX_READING_EFFLIST		1164
#define	IDS_MIDIRX_READING_PRLIST		1165
#define	IDS_MIDIRX_READING_HPLIST		1166
#define	IDS_MIDIRX_READING_MMLIST		1167
#define	IDS_MIDIRX_READING_UT			1168
#define	IDS_MIDIRX_READING_PM			1169
#define	IDS_MIDIRX_READING_MASTER		1170
#define	IDS_NO_DATA_CARD				1171
#define	IDS_MIDITX_SENDING_PR			1172
#define	IDS_MIDITX_SENDING_HP			1173
#define	IDS_MIDITX_SENDING_MM			1174
#define	IDS_MIDITX_SENDING_UT			1175
#define	IDS_MIDITX_SENDING_PM			1176
#define	IDS_UT_WINDOW_TITLE				1177
#define	IDS_UT_ERR_CREATING_WINDOW		1178
#define	IDS_UT_ERR_SENDING				1179
#define	IDS_NO_PATCH_WINDOW_TITLE		1180
#define	IDS_NO_PATCH_ERR_CREAT_WND		1181
#define	IDS_RT_PATCH_WINDOW_TITLE		1182
#define	IDS_RT_PATCH_ERR_CREAT_WND		1183
#define	IDS_KBD_WINDOW_TITLE			1184
#define	IDS_KBD_ERR_CREAT_WND			1185
#define	IDS_VELOCITY_SHORTHAND			1186
#define	IDS_PR_WINDOW_TITLE		  		1187
#define	IDS_PR_ERR_CREAT_WND			1188
#define	IDS_PR_NO_PATCH_WND_TITLE		1189
#define	IDS_PR_RT_PATCH_WND_TITLE		1190
#define	IDS_PR_FG_STEP_WND_TITLE		1191
#define	IDS_MACRO_ERR_OPEN				1192
#define	IDS_MACRO_ERR_READ				1193
#define	IDS_MACRO_ERR_WRONG_TYPE		1194
#define	IDS_MACRO_ERR_WRONG_VERSION		1195
#define	IDS_MACRO_OVERWRITE_Q			1196
#define	IDS_MACRO_ERR_CREATING			1197
#define	IDS_MACRO_ERR_WRITING			1198
#define	IDS_PRS_WINDOW_TITLE			1199
#define	IDS_PRS_ERR_CREAT_WND			1200
#define	IDS_MODIFIED_PR_NOT_SAVED		1201
#define	IDS_PRS_ERR_DISPLAYING_LIST		1202
#define	IDS_PR_ERR_SENDING				1203
#define	IDS_PR_INSTR_NONE				1204
#define	IDS_PR_ALL_WINDOW_TITLE			1205
#define	IDS_PR_MAIN_WINDOW_TITLE		1206
#define	IDS_PR_VOICE_WINDOW_TITLE		1207
#define	IDS_PR_LFO_WINDOW_TITLE			1208
#define	IDS_PR_FG_WINDOW_TITLE			1209
#define	IDS_PR_NO_WINDOW_TITLE			1210
#define	IDS_PR_RT_WINDOW_TITLE			1211
#define	IDS_PM_WINDOW_TITLE				1212
#define	IDS_PM_ERR_CREAT_WND			1213
#define	IDS_MODIFIED_PM_NOT_SAVED		1214
#define	IDS_PM_ERR_SENDING				1215
#define	IDS_SYSEX_ERR_CREAT_WND			1216
#define	IDS_SYSEX_ERR_SENDING			1217
										   
										   
/****************************************************************************/
/*		Menu items															*/	
/****************************************************************************/

// Main menu

#define	IDM_FILE_LOAD_ALL_SOUNDS					100
#define	IDM_FILE_LOAD_CURRENT_PRESET				101
#define	IDM_FILE_LOAD_CURRENT_HYPERPRESET			102
#define	IDM_FILE_LOAD_CURRENT_MIDIMAP				103
#define	IDM_FILE_LOAD_TUNING_TABLE					105
#define	IDM_FILE_LOAD_PROGRAM_MAP					106

#define	IDM_FILE_SAVE_ALL_SOUNDS					107
#define	IDM_FILE_SAVE_CURRENT_PRESET				108
#define	IDM_FILE_SAVE_CURRENT_HYPERPRESET			109
#define	IDM_FILE_SAVE_CURRENT_MIDIMAP				110
#define	IDM_FILE_SAVE_TUNING_TABLE					111
#define	IDM_FILE_SAVE_PROGRAM_MAP					112

#define	IDM_FILE_EXPORT_CURRENT_PRESET				113
#define	IDM_FILE_EXPORT_CURRENT_HYPERPRESET			114
#define	IDM_FILE_EXPORT_CURRENT_MIDIMAP				115

#define IDM_FILE_EXIT								130

#define	IDM_REFRESH_ALL								300
#define	IDM_REFRESH_INTERNALS						301

#define IDM_OPTIONS_MIDI							400
#define	IDM_OPTIONS_CONNECT_TO_MORPHEUS				401
#define	IDM_OPTIONS_BIG_PRESET_DIALOG				402

#define	IDM_HELP_INDEX								500
#define	IDM_HELP_ABOUT								501

#define	IDM_EDIT_PRESETS							600
#define	IDM_EDIT_HYPERPRESETS						601
#define	IDM_EDIT_MIDIMAPS							602
#define	IDM_EDIT_USER_TUNING						603
#define	IDM_EDIT_MASTER_SETTINGS					604
#define	IDM_EDIT_PROGRAM_MAPS						605

#define	IDM_WINDOW_ARRANGE_ICONS					700
#define	IDM_WINDOW_CASCADE							701

/****************************************************************************/
/*		Dialog controls														*/	
/****************************************************************************/

// From DLG_MIDI_OPTIONS

#define IDC_MIDI_INPUTS_LIST				100
#define IDC_MIDI_OUTPUTS_LIST				101
#define	IDC_DEVICE_ID						103
#define	IDC_TEST_KBD_BASE_OCTAVE			104
#define	IDC_TEST_KBD_MIDI_CHAN				105
#define	IDC_TEST_KBD_MIDI_A					106
#define	IDC_TEST_KBD_MIDI_B					107
#define	IDC_TEST_KBD_MIDI_C					108
#define	IDC_TEST_KBD_MIDI_D					109
#define	IDC_STORAGE_PATH					110
#define	IDC_BROWSE_STORAGE_PATH				111
#define	IDC_PRESET_BLOCK_PATH				112
#define	IDC_BROWSE_PRESET_BLOCK_PATH		113


// From DLG_PRESETS_LIST

#define	IDC_PRESETS_LIST					100
#define	IDC_PRESET_SAVE_CHANGES				101
#define	IDC_PRESET_ABANDON_CHANGES			102

// Er..
#define IDC_STATUS							100

// From DLG_ABOUT

#define	IDC_REVISION						100
#define	IDC_DATA_CARD_INSTALLED				101
#define	IDC_ABOUT_EDITOR_VERSION			102
#define	IDC_ABOUT_EXTRA1					104
#define	IDC_ABOUT_EXTRA2					105
#define	IDC_ABOUT_NAME						106

// From DLG_PATCH

#define	IDC_SOURCE							100
#define	IDC_DESTINATION						101
#define	IDC_AMOUNT							102

// From DLG_FUNCGEN

#define	IDC_FG_LEVEL						100
#define	IDC_FG_TIME							101
#define	IDC_FG_SHAPE						102
#define	IDC_FG_COND_JUMP					103
#define	IDC_FG_COND_VAL						104
#define	IDC_FG_COND_SEG						105
#define	IDC_FG_COND_SEG_VALUE				106
#define	IDC_FG_LEVEL_TYPE_0					107
#define	IDC_FG_LEVEL_TYPE_1					108
#define	IDC_FG_LEVEL_TYPE_2					109
#define	IDC_FG_LEVEL_TYPE_3					110

// From DLG_PATCH_SLIDER

#define	IDC_PATCH_LEVEL						100
#define	IDC_DELETE							101

/**************************************************************************/
/***					From DLG_PRESET									***/
/**************************************************************************/

// Instrument 1

#define	IDC_INSTR1_INSTRUMENT				100
#define IDC_INSTR1_LOW_KEY_EDITBOX			101
#define IDC_INSTR1_LOW_KEY_CONTROL			102
#define IDC_INSTR1_HIGH_KEY_EDITBOX			103
#define IDC_INSTR1_HIGH_KEY_CONTROL			104
#define IDC_INSTR1_COARSE_TUNE				105
#define IDC_INSTR1_COARSE_TUNE_VALUE		106
#define IDC_INSTR1_FINE_TUNE				107
#define IDC_INSTR1_FINE_TUNE_VALUE			108
#define IDC_INSTR1_KEY_TRANSPOSE			109
#define IDC_INSTR1_KEY_TRANSPOSE_VALUE		110
#define IDC_INSTR1_DOUBLE_DETUNE			111
#define IDC_INSTR1_DOUBLE_DETUNE_VALUE		112
#define IDC_INSTR1_SOUND_DELAY				113
#define IDC_INSTR1_SOUND_DELAY_VALUE		114
#define IDC_INSTR1_SOUND_START				115
#define IDC_INSTR1_SOUND_START_VALUE		116
#define IDC_INSTR1_LOOP_START_HIGH			117
#define IDC_INSTR1_LOOP_START_HIGH_VALUE	118
#define IDC_INSTR1_LOOP_START_LOW			119
#define IDC_INSTR1_LOOP_START_LOW_VALUE		120
#define IDC_INSTR1_LOOP_LENGTH_HIGH			121
#define IDC_INSTR1_LOOP_LENGTH_HIGH_VALUE	122
#define IDC_INSTR1_LOOP_LENGTH_LOW			123
#define IDC_INSTR1_LOOP_LENGTH_LOW_VALUE	124
#define IDC_INSTR1_REVERSE					125
#define IDC_INSTR1_NON_TRANSPOSE			126
#define IDC_INSTR1_LOOP_ENABLE				127
#define IDC_INSTR1_SOLO_MODE_EDITBOX		128
#define IDC_INSTR1_SOLO_MODE_CONTROL		129
#define IDC_INSTR1_SOLO_PRIORITY_EDITBOX	130
#define IDC_INSTR1_SOLO_PRIORITY_CONTROL	131
#define	IDC_INSTR1_VOLUME					132
#define	IDC_INSTR1_VOLUME_VALUE				133

// Filter 1

#define	IDC_FILTER1_TYPE					300
#define	IDC_FILTER1_REVERSE					301
#define	IDC_FILTER1_LEVEL					302
#define	IDC_FILTER1_LEVEL_VALUE				303
#define	IDC_FILTER1_MORPH					304
#define	IDC_FILTER1_MORPH_VALUE				305
#define	IDC_FILTER1_FREQ_TRACK				306
#define	IDC_FILTER1_FREQ_TRACK_VALUE		307
#define	IDC_FILTER1_TRANS2					308
#define	IDC_FILTER1_TRANS2_VALUE			309

// Portamento 1

#define	IDC_PORTAMENTO1_SHAPE				500
#define	IDC_PORTAMENTO1_RATE				501
#define	IDC_PORTAMENTO1_RATE_VALUE			502


// Pan 1

#define	IDC_PAN1_POSITION					800
#define	IDC_PAN1_POSITION_VALUE				801

// Alternate envelope 1

#define	IDC_ALT_ENV1_ENABLE					1600
#define	IDC_ALT_ENV1_ATTACK					1601
#define	IDC_ALT_ENV1_ATTACK_VALUE			1602
#define	IDC_ALT_ENV1_HOLD					1603
#define	IDC_ALT_ENV1_HOLD_VALUE				1604
#define	IDC_ALT_ENV1_DECAY					1605
#define	IDC_ALT_ENV1_DECAY_VALUE			1606
#define	IDC_ALT_ENV1_SUSTAIN				1607
#define	IDC_ALT_ENV1_SUSTAIN_VALUE			1608
#define	IDC_ALT_ENV1_RELEASE				1609
#define	IDC_ALT_ENV1_RELEASE_VALUE			1610
#define	IDC_ALT_ENV1_ENVELOPE				1611

// Instrument 2

#define	IDC_INSTR2_INSTRUMENT				200
#define IDC_INSTR2_LOW_KEY_EDITBOX			201
#define IDC_INSTR2_LOW_KEY_CONTROL			202
#define IDC_INSTR2_HIGH_KEY_EDITBOX			203
#define IDC_INSTR2_HIGH_KEY_CONTROL			204
#define IDC_INSTR2_COARSE_TUNE				205
#define IDC_INSTR2_COARSE_TUNE_VALUE		206
#define IDC_INSTR2_FINE_TUNE				207
#define IDC_INSTR2_FINE_TUNE_VALUE			208
#define IDC_INSTR2_KEY_TRANSPOSE			209
#define IDC_INSTR2_KEY_TRANSPOSE_VALUE		210
#define IDC_INSTR2_DOUBLE_DETUNE			211
#define IDC_INSTR2_DOUBLE_DETUNE_VALUE		212
#define IDC_INSTR2_SOUND_DELAY				213
#define IDC_INSTR2_SOUND_DELAY_VALUE		214
#define IDC_INSTR2_SOUND_START				215
#define IDC_INSTR2_SOUND_START_VALUE		216
#define IDC_INSTR2_LOOP_START_HIGH			217
#define IDC_INSTR2_LOOP_START_HIGH_VALUE	218
#define IDC_INSTR2_LOOP_START_LOW			219
#define IDC_INSTR2_LOOP_START_LOW_VALUE		220
#define IDC_INSTR2_LOOP_LENGTH_HIGH			221
#define IDC_INSTR2_LOOP_LENGTH_HIGH_VALUE	222
#define IDC_INSTR2_LOOP_LENGTH_LOW			223
#define IDC_INSTR2_LOOP_LENGTH_LOW_VALUE	224
#define IDC_INSTR2_REVERSE					225
#define IDC_INSTR2_NON_TRANSPOSE			226
#define IDC_INSTR2_LOOP_ENABLE				227
#define IDC_INSTR2_SOLO_MODE_EDITBOX		228
#define IDC_INSTR2_SOLO_MODE_CONTROL		229
#define IDC_INSTR2_SOLO_PRIORITY_EDITBOX	230
#define IDC_INSTR2_SOLO_PRIORITY_CONTROL	231
#define	IDC_INSTR2_VOLUME					232
#define	IDC_INSTR2_VOLUME_VALUE				233

// Filter 2

#define	IDC_FILTER2_TYPE					400
#define	IDC_FILTER2_REVERSE					401
#define	IDC_FILTER2_LEVEL					402
#define	IDC_FILTER2_LEVEL_VALUE				403
#define	IDC_FILTER2_MORPH					404
#define	IDC_FILTER2_MORPH_VALUE				405
#define	IDC_FILTER2_FREQ_TRACK				406
#define	IDC_FILTER2_FREQ_TRACK_VALUE		407
#define	IDC_FILTER2_TRANS2					408
#define	IDC_FILTER2_TRANS2_VALUE			409

// Portamento 2

#define	IDC_PORTAMENTO2_SHAPE				600
#define	IDC_PORTAMENTO2_RATE				601
#define	IDC_PORTAMENTO2_RATE_VALUE			602

// Pan 2

#define	IDC_PAN2_POSITION					900
#define	IDC_PAN2_POSITION_VALUE				901

// Alternate envelope 2

#define	IDC_ALT_ENV2_ENABLE					1700
#define	IDC_ALT_ENV2_ATTACK					1701
#define	IDC_ALT_ENV2_ATTACK_VALUE			1702
#define	IDC_ALT_ENV2_HOLD					1703
#define	IDC_ALT_ENV2_HOLD_VALUE				1704
#define	IDC_ALT_ENV2_DECAY					1705
#define	IDC_ALT_ENV2_DECAY_VALUE			1706
#define	IDC_ALT_ENV2_SUSTAIN				1707
#define	IDC_ALT_ENV2_SUSTAIN_VALUE			1708
#define	IDC_ALT_ENV2_RELEASE				1709
#define	IDC_ALT_ENV2_RELEASE_VALUE			1710
#define	IDC_ALT_ENV2_ENVELOPE				1711

// Xfade

#define	IDC_XFADE_MODE						700
#define	IDC_XFADE_DIRECTION					701
#define	IDC_XFADE_BALANCE					702
#define	IDC_XFADE_BALANCE_VALUE				703
#define	IDC_XFADE_AMOUNT					704
#define	IDC_XFADE_AMOUNT_VALUE				705
#define	IDC_XFADE_SWITCH_POINT_CONTROL		706
#define	IDC_XFADE_SWITCH_POINT_EDITBOX		707

// LFO 1

#define	IDC_LFO1_SHAPE						1000
#define	IDC_LFO1_RATE						1001
#define	IDC_LFO1_RATE_VALUE					1002
#define	IDC_LFO1_DELAY						1003
#define	IDC_LFO1_DELAY_VALUE				1004
#define	IDC_LFO1_VARIATION					1005
#define	IDC_LFO1_VARIATION_VALUE			1006
#define	IDC_LFO1_AMOUNT						1007
#define	IDC_LFO1_AMOUNT_VALUE				1008

// LFO 2

#define	IDC_LFO2_SHAPE						1100
#define	IDC_LFO2_RATE						1101
#define	IDC_LFO2_RATE_VALUE					1102
#define	IDC_LFO2_DELAY						1103
#define	IDC_LFO2_DELAY_VALUE				1104
#define	IDC_LFO2_VARIATION					1105
#define	IDC_LFO2_VARIATION_VALUE			1106
#define	IDC_LFO2_AMOUNT						1107
#define	IDC_LFO2_AMOUNT_VALUE				1108

// Function generator 1

#define	IDC_FG1_AMOUNT						1200
#define	IDC_FG1_AMOUNT_VALUE				1201
#define	IDC_FG1_LISTBOX						1202
#define	IDC_FG1_LB_LABEL1					1204
#define	IDC_FG1_LB_LABEL2					1205
#define	IDC_FG1_LB_LABEL3					1206
#define	IDC_FG1_LB_LABEL4					1207
#define	IDC_FG1_LB_LABEL5					1208
#define	IDC_FG1_LB_LABEL6					1209
#define	IDC_FG1_LB_LABEL7					1210

// Function generator 2

#define	IDC_FG2_AMOUNT						1300
#define	IDC_FG2_AMOUNT_VALUE				1301
#define	IDC_FG2_LISTBOX						1302
#define	IDC_FG2_LB_LABEL1					1304
#define	IDC_FG2_LB_LABEL2					1305
#define	IDC_FG2_LB_LABEL3					1306
#define	IDC_FG2_LB_LABEL4					1307
#define	IDC_FG2_LB_LABEL5					1308
#define	IDC_FG2_LB_LABEL6					1309
#define	IDC_FG2_LB_LABEL7					1310

// Auxilliary envelope

#define	IDC_AUX_ENV_DELAY					1800
#define	IDC_AUX_ENV_DELAY_VALUE				1801
#define	IDC_AUX_ENV_ATTACK					1802
#define	IDC_AUX_ENV_ATTACK_VALUE			1803
#define	IDC_AUX_ENV_HOLD					1804
#define	IDC_AUX_ENV_HOLD_VALUE				1805
#define	IDC_AUX_ENV_DECAY					1806
#define	IDC_AUX_ENV_DECAY_VALUE				1807
#define	IDC_AUX_ENV_SUSTAIN					1808
#define	IDC_AUX_ENV_SUSTAIN_VALUE			1809
#define	IDC_AUX_ENV_RELEASE					1810
#define	IDC_AUX_ENV_RELEASE_VALUE			1811
#define	IDC_AUX_ENV_ENVELOPE				1812
#define	IDC_AUX_ENV_AMOUNT					1813

// Keyboard

#define	IDC_KBD_PRESSURE_AMOUNT				1400
#define	IDC_KBD_PRESSURE_AMOUNT_VALUE		1401
#define	IDC_KBD_BEND_RANGE					1402
#define	IDC_KBD_BEND_RANGE_VALUE			1403
#define	IDC_KBD_VELOCITY_CURVE				1404
#define	IDC_KBD_TUNING						1405
#define	IDC_PORTAMENTO_MODE					1406

// Miscellaneous

#define	IDC_MISC_MIX_SELECT					1500
#define	IDC_MISC_FOOTSWITCH1_CONTROL		1501
#define	IDC_MISC_FOOTSWITCH2_CONTROL		1502
#define	IDC_MISC_FOOTSWITCH3_CONTROL		1503
#define	IDC_MISC_MIDI_A_AMOUNT				1504
#define	IDC_MISC_MIDI_A_AMOUNT_VALUE		1505
#define	IDC_MISC_MIDI_B_AMOUNT				1506
#define	IDC_MISC_MIDI_B_AMOUNT_VALUE		1507
#define	IDC_MISC_MIDI_C_AMOUNT				1508
#define	IDC_MISC_MIDI_C_AMOUNT_VALUE		1509
#define	IDC_MISC_MIDI_D_AMOUNT				1510
#define	IDC_MISC_MIDI_D_AMOUNT_VALUE		1511
#define	IDC_MISC_NOTE_ON_MODS_LISTBOX		1512
#define	IDC_MISC_REAL_TIME_MODS_LISTBOX		1513
#define	IDC_MISC_PRESET_NAME				1514
#define	IDC_MISC_NOTE_ON_PATCH_LISTBOX		1515
#define	IDC_MISC_REAL_TIME_PATCH_LISTBOX	1516
#define	IDC_KBD_LOW_KEY						1517
#define	IDC_KBD_LOW_KEY_EDITBOX				1518
#define	IDC_KBD_HIGH_KEY					1519
#define	IDC_KBD_HIGH_KEY_EDITBOX			1520
#define	IDC_KBD_CENTRE						1521
#define	IDC_KBD_CENTRE_EDITBOX				1522

#define	IDC_MISC_NOTE_ON_LB_LABEL1			1530
#define	IDC_MISC_NOTE_ON_LB_LABEL2			1531
#define	IDC_MISC_NOTE_ON_LB_LABEL3			1532
#define	IDC_MISC_REAL_TIME_LB_LABEL1		1533
#define	IDC_MISC_REAL_TIME_LB_LABEL2		1534
#define	IDC_MISC_REAL_TIME_LB_LABEL3		1535

// Preset sub-dialog 'pseudo-toolbar' buttons

#define	IDC_PRESET_TOOLBAR_MAIN				2000
#define	IDC_PRESET_TOOLBAR_VOICES			2001
#define	IDC_PRESET_TOOLBAR_LFO_AUXENV		2002
#define	IDC_PRESET_TOOLBAR_FG				2003

// Preset sub-block group boxes

#define	IDC_FG1_GROUPBOX					2500
#define	IDC_FG2_GROUPBOX					2501
#define	IDC_PRIMARY_GROUPBOX				2502
#define	IDC_SECONDARY_GROUPBOX				2503
#define	IDC_AUX_ENV_GROUPBOX				2504
#define	IDC_NOTE_ON_MODS_GROUPBOX			2505
#define	IDC_REAL_TIME_MODS_GROUPBOX			2506

/**************************************************************************/
/***					From DLG_HYPERPRESET							***/
/**************************************************************************/

#define	IDC_HYPER_NAME						100
#define	IDC_HYPER_PORT_MODE					101
#define	IDC_HYPER_FG_LISTBOX				102
#define	IDC_HYPER_ZONE_LISTBOX				103
#define	IDC_HYPER_FG_LB_LABEL1				104
#define	IDC_HYPER_FG_LB_LABEL2				105
#define	IDC_HYPER_FG_LB_LABEL3				106
#define	IDC_HYPER_FG_LB_LABEL4				107
#define	IDC_HYPER_FG_LB_LABEL5				108
#define	IDC_HYPER_FG_LB_LABEL6				109
#define	IDC_HYPER_FG_LB_LABEL7				110
#define	IDC_HYPER_ZONE_LB_LABEL1			111
#define	IDC_HYPER_ZONE_LB_LABEL2			112
#define	IDC_HYPER_ZONE_LB_LABEL3			113
#define	IDC_HYPER_ZONE_LB_LABEL4			114
#define	IDC_HYPER_ZONE_LB_LABEL5			115
#define	IDC_HYPER_ZONE_LB_LABEL6			116
#define	IDC_HYPER_ZONE_LB_LABEL7			117
#define	IDC_HYPER_ZONE_LB_LABEL8			118
#define	IDC_HYPER_ZONE_LB_LABEL9			119
#define	IDC_HYPER_ZONE_LB_LABEL10			120
#define	IDC_HYPER_ZONE_LB_LABEL11			121
#define	IDC_HYPER_ZONE_LB_LABEL12			122

/**************************************************************************/
/***					From DLG_HYPER_ZONE								***/
/**************************************************************************/

#define	IDC_ZONE_PRESET						100
#define	IDC_ZONE_VOLUME						101
#define	IDC_ZONE_PAN						102
#define	IDC_ZONE_LOW_KEY					103
#define	IDC_ZONE_LOW_KEY_EDITBOX			104
#define	IDC_ZONE_HIGH_KEY					105
#define	IDC_ZONE_HIGH_KEY_EDITBOX			106
#define	IDC_ZONE_LOW_VELOCITY				107
#define	IDC_ZONE_LOW_VELOCITY_EDITBOX		108
#define	IDC_ZONE_HIGH_VELOCITY				109
#define	IDC_ZONE_HIGH_VELOCITY_EDITBOX		110
#define	IDC_ZONE_VELOCITY_OFFSET			111
#define	IDC_ZONE_VELOCITY_OFFSET_EDITBOX	112
#define	IDC_ZONE_XPOSE						113
#define	IDC_ZONE_COARSE_TUNE				114
#define	IDC_ZONE_FINE_TUNE					115

/**************************************************************************/
/***					From DLG_MIDIMAP								***/
/**************************************************************************/

#define	IDC_MM_NAME							100
#define	IDC_MM_CHANNEL_LISTBOX				101
#define	IDC_MM_PROGRAM_MAP					102

#define	IDC_MM_FXA_TYPE						110
#define	IDC_MM_FXA_PARAM1					111
#define	IDC_MM_FXA_PARAM1_NAME				112
#define	IDC_MM_FXA_PARAM2					113
#define	IDC_MM_FXA_PARAM2_NAME				114
#define	IDC_MM_FXA_PARAM3					115
#define	IDC_MM_FXA_PARAM3_NAME				116
#define	IDC_MM_FXA_PARAM4					117
#define	IDC_MM_FXA_PARAM4_NAME				118
#define	IDC_MM_FXA_PARAM5					119
#define	IDC_MM_FXA_PARAM5_NAME				120
#define	IDC_MM_FXA_PARAM6					121
#define	IDC_MM_FXA_PARAM6_NAME				122
#define	IDC_MM_FXA_PARAM7					123
#define	IDC_MM_FXA_PARAM7_NAME				124
#define	IDC_MM_FXA_PARAM8					125
#define	IDC_MM_FXA_PARAM8_NAME				126
#define	IDC_MM_FXA_PARAM9					127
#define	IDC_MM_FXA_PARAM9_NAME				128
#define	IDC_MM_FXA_PARAM10					129
#define	IDC_MM_FXA_PARAM10_NAME				130

#define	IDC_MM_FXB_TYPE						140
#define	IDC_MM_FXB_PARAM1					141
#define	IDC_MM_FXB_PARAM1_NAME				142
#define	IDC_MM_FXB_PARAM2					143
#define	IDC_MM_FXB_PARAM2_NAME				144
#define	IDC_MM_FXB_PARAM3					145
#define	IDC_MM_FXB_PARAM3_NAME				146
#define	IDC_MM_FXB_PARAM4					147
#define	IDC_MM_FXB_PARAM4_NAME				148
#define	IDC_MM_FXB_PARAM5					149
#define	IDC_MM_FXB_PARAM5_NAME				150
#define	IDC_MM_FXB_PARAM6					151 
#define	IDC_MM_FXB_PARAM6_NAME				152
#define	IDC_MM_FXB_PARAM7					153
#define	IDC_MM_FXB_PARAM7_NAME				154
#define	IDC_MM_FXB_PARAM8					155
#define	IDC_MM_FXB_PARAM8_NAME				156
#define	IDC_MM_FXB_PARAM9					157
#define	IDC_MM_FXB_PARAM9_NAME				158
#define	IDC_MM_FXB_PARAM10					159
#define	IDC_MM_FXB_PARAM10_NAME				160

#define	IDC_MM_FXA_AMOUNT					170
#define	IDC_MM_FXB_AMOUNT					171
#define	IDC_MM_B_TO_A_AMOUNT				172
#define	IDC_MM_FXA_BUS						173
#define	IDC_MM_FXB_BUS						174

#define	IDC_MM_CHANNEL_LISTBOX_LABEL1		180
#define	IDC_MM_CHANNEL_LISTBOX_LABEL2		181
#define	IDC_MM_CHANNEL_LISTBOX_LABEL3		182
#define	IDC_MM_CHANNEL_LISTBOX_LABEL4		183
#define	IDC_MM_CHANNEL_LISTBOX_LABEL5		184
#define	IDC_MM_CHANNEL_LISTBOX_LABEL6		185

/**************************************************************************/
/***					From DLG_MM_CHANNEL								***/
/**************************************************************************/

#define	IDC_MM_CHANNEL_PROGRAM				100
#define	IDC_MM_CHANNEL_BANK					101
#define	IDC_MM_CHANNEL_VOLUME				102
#define	IDC_MM_CHANNEL_PAN					103
#define	IDC_MM_CHANNEL_MIX_BUS				104
#define	IDC_MM_CHANNEL_ENABLE				105
#define	IDC_MM_CHANNEL_PROGCHG_ENBL			106
#define	IDC_MM_CHANNEL_BANKCHG_ENBL			107
#define	IDC_MM_CHANNEL_VOLCTL_ENBL			108
#define	IDC_MM_CHANNEL_PANCTL_ENBL			109
#define	IDC_MM_CHANNEL_PWHL_ENBL			110
#define	IDC_MM_CHANNEL_MPRESS_ENBL			111
#define	IDC_MM_CHANNEL_PPRESS_ENBL			112
#define	IDC_MM_CHANNEL_CTLA_ENBL			113
#define	IDC_MM_CHANNEL_CTLB_ENBL			114
#define	IDC_MM_CHANNEL_CTLC_ENBL			115
#define	IDC_MM_CHANNEL_CTLD_ENBL			116
#define	IDC_MM_CHANNEL_FTSW1_ENBL			117
#define	IDC_MM_CHANNEL_FTSW2_ENBL			118
#define	IDC_MM_CHANNEL_FTSW3_ENBL			119

/**************************************************************************/
/***					From DLG_MESSAGE_BOX							***/
/**************************************************************************/

#define	IDC_MESSAGE_TEXT					100
#define	IDC_MESSAGE_BUTTON1					101
#define	IDC_MESSAGE_BUTTON2					102
#define	IDC_MESSAGE_BUTTON3					103
#define	IDC_MESSAGE_BITMAP					104

/**************************************************************************/
/***					From DLG_USER_TUNING							***/
/**************************************************************************/

#define	IDC_USER_TUNING_LISTBOX				100
#define	IDC_USER_TUNING_LABEL1				101
#define	IDC_USER_TUNING_LABEL2				102
#define	IDC_USER_TUNING_LABEL3				103
#define	IDC_USER_TUNING_COARSE				104
#define	IDC_USER_TUNING_FINE				105
#define	IDC_USER_TUNING_SEND				106
#define	IDC_USER_TUNING_ABORT				107

/**************************************************************************/
/***					From DLG_MASTER_SETTINGS						***/
/**************************************************************************/

#define	IDC_MASTER_TUNE						100
#define	IDC_MASTER_TRANSPOSE				101
#define	IDC_MASTER_BEND_RANGE				102
#define	IDC_MASTER_VELOCITY_CURVE			103
#define	IDC_MASTER_MIDI_MODE				104
#define	IDC_MASTER_MIDI_ID					105
#define	IDC_MASTER_MIDI_ID_TEXT				106
#define	IDC_MASTER_MIDI_MODE_CHANGE			107
#define	IDC_MASTER_PROGRAM_MAP				108
#define	IDC_MASTER_PROGRAM_MODE_TEXT		109
#define	IDC_MASTER_CHAN1_PROGRAM			110
#define	IDC_MASTER_CHAN1_VOLUME				111
#define	IDC_MASTER_CHAN1_PAN				112
#define	IDC_MASTER_CHAN1_VOLUME_TEXT		113
#define	IDC_MASTER_CHAN1_PAN_TEXT			114
#define	IDC_MASTER_CHAN2_PROGRAM			120
#define	IDC_MASTER_CHAN2_VOLUME				121
#define	IDC_MASTER_CHAN2_PAN				122
#define	IDC_MASTER_CHAN2_VOLUME_TEXT		123
#define	IDC_MASTER_CHAN2_PAN_TEXT			124
#define	IDC_MASTER_CHAN3_PROGRAM			130
#define	IDC_MASTER_CHAN3_VOLUME				131
#define	IDC_MASTER_CHAN3_PAN				132
#define	IDC_MASTER_CHAN3_VOLUME_TEXT		133
#define	IDC_MASTER_CHAN3_PAN_TEXT			134
#define	IDC_MASTER_CHAN4_PROGRAM			140
#define	IDC_MASTER_CHAN4_VOLUME				141
#define	IDC_MASTER_CHAN4_PAN				142
#define	IDC_MASTER_CHAN4_VOLUME_TEXT		143
#define	IDC_MASTER_CHAN4_PAN_TEXT			144
#define	IDC_MASTER_CHAN5_PROGRAM			150
#define	IDC_MASTER_CHAN5_VOLUME				151
#define	IDC_MASTER_CHAN5_PAN				152
#define	IDC_MASTER_CHAN5_VOLUME_TEXT		153
#define	IDC_MASTER_CHAN5_PAN_TEXT			154
#define	IDC_MASTER_CHAN6_PROGRAM			160
#define	IDC_MASTER_CHAN6_VOLUME				161
#define	IDC_MASTER_CHAN6_PAN				162
#define	IDC_MASTER_CHAN6_VOLUME_TEXT		163
#define	IDC_MASTER_CHAN6_PAN_TEXT			164
#define	IDC_MASTER_CHAN7_PROGRAM			170
#define	IDC_MASTER_CHAN7_VOLUME				171
#define	IDC_MASTER_CHAN7_PAN				172
#define	IDC_MASTER_CHAN7_VOLUME_TEXT		173
#define	IDC_MASTER_CHAN7_PAN_TEXT			174
#define	IDC_MASTER_CHAN8_PROGRAM			180
#define	IDC_MASTER_CHAN8_VOLUME				181
#define	IDC_MASTER_CHAN8_PAN				182
#define	IDC_MASTER_CHAN8_VOLUME_TEXT		183
#define	IDC_MASTER_CHAN8_PAN_TEXT			184
#define	IDC_MASTER_CHAN9_PROGRAM			190
#define	IDC_MASTER_CHAN9_VOLUME				191
#define	IDC_MASTER_CHAN9_PAN				192
#define	IDC_MASTER_CHAN9_VOLUME_TEXT		193
#define	IDC_MASTER_CHAN9_PAN_TEXT			194
#define	IDC_MASTER_CHAN10_PROGRAM			200
#define	IDC_MASTER_CHAN10_VOLUME			201
#define	IDC_MASTER_CHAN10_PAN				202
#define	IDC_MASTER_CHAN10_VOLUME_TEXT		203
#define	IDC_MASTER_CHAN10_PAN_TEXT			204
#define	IDC_MASTER_CHAN11_PROGRAM			210
#define	IDC_MASTER_CHAN11_VOLUME			211
#define	IDC_MASTER_CHAN11_PAN				212
#define	IDC_MASTER_CHAN11_VOLUME_TEXT		213
#define	IDC_MASTER_CHAN11_PAN_TEXT			214
#define	IDC_MASTER_CHAN12_PROGRAM			220
#define	IDC_MASTER_CHAN12_VOLUME			221
#define	IDC_MASTER_CHAN12_PAN				222
#define	IDC_MASTER_CHAN12_VOLUME_TEXT		223
#define	IDC_MASTER_CHAN12_PAN_TEXT			224
#define	IDC_MASTER_CHAN13_PROGRAM			230
#define	IDC_MASTER_CHAN13_VOLUME			231
#define	IDC_MASTER_CHAN13_PAN				232
#define	IDC_MASTER_CHAN13_VOLUME_TEXT		233
#define	IDC_MASTER_CHAN13_PAN_TEXT			234
#define	IDC_MASTER_CHAN14_PROGRAM			240
#define	IDC_MASTER_CHAN14_VOLUME			241
#define	IDC_MASTER_CHAN14_PAN				242
#define	IDC_MASTER_CHAN14_VOLUME_TEXT		243
#define	IDC_MASTER_CHAN14_PAN_TEXT			244
#define	IDC_MASTER_CHAN15_PROGRAM			250
#define	IDC_MASTER_CHAN15_VOLUME			251
#define	IDC_MASTER_CHAN15_PAN				252
#define	IDC_MASTER_CHAN15_VOLUME_TEXT		253
#define	IDC_MASTER_CHAN15_PAN_TEXT			254
#define	IDC_MASTER_CHAN16_PROGRAM			260
#define	IDC_MASTER_CHAN16_VOLUME			261
#define	IDC_MASTER_CHAN16_PAN				262
#define	IDC_MASTER_CHAN16_VOLUME_TEXT		263
#define	IDC_MASTER_CHAN16_PAN_TEXT			264
#define	IDC_MASTER_CONTROLLER_A				300
#define	IDC_MASTER_CONTROLLER_A_TEXT		301
#define	IDC_MASTER_CONTROLLER_B				302
#define	IDC_MASTER_CONTROLLER_B_TEXT		303
#define	IDC_MASTER_CONTROLLER_C				304
#define	IDC_MASTER_CONTROLLER_C_TEXT		305
#define	IDC_MASTER_CONTROLLER_D				306
#define	IDC_MASTER_CONTROLLER_D_TEXT		307
#define	IDC_MASTER_FOOTSWITCH_1				308
#define	IDC_MASTER_FOOTSWITCH_1_TEXT		309
#define	IDC_MASTER_FOOTSWITCH_2				310
#define	IDC_MASTER_FOOTSWITCH_2_TEXT		311
#define	IDC_MASTER_FOOTSWITCH_3				312
#define	IDC_MASTER_FOOTSWITCH_3_TEXT		313

/**************************************************************************/
/***					From DLG_PROGRAM_MAP							***/
/**************************************************************************/

#define	IDC_PM_LABEL1						100
#define	IDC_PM_LABEL2						101
#define	IDC_PM_LIST							102
#define	IDC_PM_OUTPUT_PROGRAM				103
#define	IDC_PM_OUTPUT_PROGRAM_TEXT			104
#define	IDC_PM_SEND							105
#define	IDC_PM_ABANDON						106
#define	IDC_PM_CURRENT_MAP					107

/**************************************************************************/
/***					From DLG_PRESET_MACRO							***/
/**************************************************************************/

#define	IDC_MACRO_LOAD						100
#define	IDC_MACRO_SAVE						101
#define	IDC_MACRO_NAME						102
#define	IDC_MACRO_NAME_LISTBOX				103

/****************************************************************************/
/*		Miscellaneous defines												*/
/****************************************************************************/

#if defined (WINDOWS32)
 #define MAXPATH							MAX_PATH
 #define MAXDIR								MAX_PATH
 #define MAXDRIVE							4
#endif

#define	EN_ENTER							(WM_USER + 0x200)
#define	WM_USER_SET_WINDOW_SIZE				(WM_USER + 0x201)

#ifdef	ITSYBITS_WINDOWS
 #define	MY_CAPTION_STYLE						IBS_HORZCAPTION
 #define	MY_DEFAULT_WINDOW_PROC					ibDefWindowProc
 #define	MY_CREATE_PROC							ibSetCaptionSize(hWnd, SMALL_CAPTION_SIZE);
#else
 #define	MY_CAPTION_STYLE						WS_CAPTION
 #define	MY_DEFAULT_WINDOW_PROC					DefWindowProc
 #define	MY_CREATE_PROC
#endif

#define	WINDOWS_SECTION_NAME					"Windows"

#define	MORPHEUS_MAIN_WINDOW_CLASS				"MorpheusEditor"
#define	MORPHEUS_HYPERPRESETS_WINDOW_CLASS		"MorpheusHyperpresets"
#define	MORPHEUS_MIDIMAPS_WINDOW_CLASS			"MorpheusMidimaps"
#define	MORPHEUS_PRESETS_WINDOW_CLASS			"MorpheusPresets"
#define	MORPHEUS_CURRENT_PRESET_WINDOW_CLASS	"MorpheusCurrentPreset"
#define	MORPHEUS_PATCHES_WINDOW_CLASS			"MorpheusPresetPatches"
#define	MORPHEUS_NOTE_ON_PATCH_CLASS			"MorpheusNoteOnPatchbay"
#define	MORPHEUS_REAL_TIME_PATCH_CLASS			"MorpheusRealTimePatchbay"

#define	MORPHEUS_SYSEX_WINDOW_CLASS				"MorpheusSysex"
#define	MORPHEUS_DIALOG_CLASS					"MorpheusDialog"
#define	STATIC_BITMAP_CLASS						"StaticBitmap"

#define NO_PARAM					-1		// Means 'optional parameter is unused'

// Dialog reformatting flags

#define RDF_CENTRE					0x0001
#define	RDF_THINFONT				0x0002

// Small caption bar font size

#define	SMALL_CAPTION_SIZE			16	// Whatever that means

// INI file name

#define	INI_FILE_NAME					"morphedt.ini"

// Invalid list index
#define	INDEX_NONE					-1

// Window classes

#define	WCNAME_PRESETS_LIST			"wndPresetsList"
#define	WCNAME_CURRENT_PRESET		"wndCurrentPreset"
#define	WCNAME_NOTE_ON_PATCHES		"wndNoteOnPatches"
#define	WCNAME_REAL_TIME_PATCHES	"wndRealTimePatches"
#define	WCNAME_HYPERS_LIST			"wndHypersList"
#define	WCNAME_CURRENT_HYPER		"wndCurrentHyper"
#define	WCNAME_MMS_LIST				"wndMMsList"
#define	WCNAME_CURRENT_MM			"wndCurrentMM"
#define	WCNAME_MASTER				"wndMaster"
#define	WCNAME_TUNING				"wndTuning"
#define	WCNAME_PROGRAM_MAP			"wndProgramMap"
#define	WCNAME_PIANO				"wndPiano"

#define	WCNAME_MOUSE_EATER			"wndMouseEater"

// Window names (INI file)

#define		MAIN_WINDOW					"MainWindow"
 #define	PRESETS_LIST_WINDOW			"PresetsListWindow"
 #define	CURRENT_PRESET_WINDOW		"CurrentPreset"
 #define	NOTE_ON_PATCHES_WINDOW		"NoteOnPatches"
 #define	REAL_TIME_PATCHES_WINDOW	"RealTimePatches"

 #define	HYPERPRESETS_LIST_WINDOW	"HyperPresetsListWindow"
 #define	CURRENT_HYPERPRESET_WINDOW	"CurrentHyperPresetWindow"

 #define	MIDIMAPS_LIST_WINDOW		"MidimapsListWindow"
 #define	CURRENT_MIDIMAP_WINDOW		"CurrentMidimapWindow"

 #define	USER_TUNING_WINDOW			"UserTuningWindow"
 #define	MASTER_SETTINGS_WINDOW		"MasterSettingsWindow"
 #define	PROGRAM_MAP_WINDOW			"ProgramMapWindow"

 #define	PIANO_WINDOW				"PianoWindow"

/*
	File load / save masks
 */

#define	HYPERPRESETS_FILE_MASK			"Hyperpresets\0*.hyp\0All files\0*.*\0\0"
#define	MIDIMAPS_FILE_MASK				"Midimaps\0*.mm\0All files\0*.*\0\0"
#define	PROGRAM_MAPS_FILE_MASK			"Program map\0*.pm\0All files\0*.*\0\0"
#define	PRESETS_FILE_MASK				"Presets\0*.pr\0All files\0*.*\0\0"
#define	ALL_SOUNDS_FILE_MASK			"All sounds\0*.sds\0All files\0*.*\0\0"
#define	USER_TUNING_FILE_MASK			"User tuning\0*.tun\0All files\0*.*\0\0"
#define	MIDI_FILES_FILE_MASK			"Midi files\0*.mid\0All files\0*.*\0\0"


/****************************************************************************/
/*		Morpheus defines													*/
/****************************************************************************/

#define	BASIC_SYSEX_REQUEST_LENGTH	6

// Regular MIDI defines
#define	NUM_MIDI_CHANNELS			16
#define	NUM_MIDI_NOTES				128

/****************************************************************************/
/*		Macros																*/
/****************************************************************************/

#define	WNUM(val)					(int)ReadSysexWord(&(val))
#define	ARRAY_SIZE(a)				(sizeof(a) / sizeof ((a)[0]))

// Macros

#if defined (WINDOWS32)

 #define	DllImport	__declspec( dllimport )
 #define	DllExport	__declspec( dllexport )
 #define	INTEXPORT	DllExport	int
 #define	INTIMPORT	DllImport	int
 #define	VOIDEXPORT	DllExport	VOID
 #define	VOIDIMPORT	DllImport	VOID
 #define	BOOLEXPORT	DllExport	BOOL
 #define	BOOLIMPORT	DllImport	BOOL
 #define	LONGEXPORT	DllExport	LONG
 #define	LONGIMPORT	DllImport	LONG

#else

 #define	INTEXPORT	int FAR PASCAL _export
 #define	INTIMPORT	int FAR PASCAL _export
 #define	VOIDEXPORT	VOID FAR PASCAL _export
 #define	VOIDIMPORT	VOID FAR PASCAL _export
 #define	BOOLEXPORT	BOOL FAR PASCAL _export
 #define	BOOLIMPORT	BOOL FAR PASCAL _export
 #define	LONGEXPORT	LONG FAR PASCAL _export
 #define	LONGIMPORT	LONG FAR PASCAL _export

#endif

#if defined (WINDOWS32)
 #define	WINDOW_PROC					LRESULT WINAPI CALLBACK
 #define	DIALOG_PROC					BOOL WINAPI CALLBACK
 #define	NCODE						HIWORD (wParam)
 #define	GETWNDID(hWnd)				GetWindowLong (hWnd, GWL_ID)
 #define	SETWNDID(hWnd,id)			SetWindowLong (hWnd, GWL_ID, id)
#else
 #define	WINDOW_PROC					LRESULT FAR PASCAL _export
 #define	DIALOG_PROC					BOOL FAR PASCAL _export
 #define	NCODE						HIWORD (lParam)
 #define	GETWNDID(hWnd)				GetWindowWord (hWnd, GWW_ID)
 #define	SETWNDID(hWnd,id)			SetWindowWord (hWnd, GWW_ID, id)
#endif

#if !defined(FIELDOFFSET)
 #define FIELDOFFSET(type, field)    ((int)(&((type NEAR*)1)->field)-1)
#endif

#if defined (__BORLANDC__)
 #define	SPLITPATH					fnsplit
#else
 #define	SPLITPATH					_splitpath
#endif

/****************************************************************************/
/*		Typedefs															*/	
/****************************************************************************/

/** Internal types ***/

#if defined (WINDOWS32)
 typedef BYTE *HPBYTE;
#else
 typedef BYTE huge	*HPBYTE;
#endif

typedef signed char	SBYTE;

typedef struct
{
	int		idLabel;						// Listbox label ID
	int		nTabPos;						// Tab position (in dialog units)
} LISTBOX_TAB_INFO;

typedef struct
{
	LPSTR	lpszName;						// Parameter name
	int		nMin;							// Parameter min value
	int		nMax;							// Parameter max value
	int		nDef;							// Parameter default value
} EFFECT_PARAM_INFO;

typedef enum {NO_SUBDIALOG, MAIN, VOICES, LFO_AUXENV, FG, ROUTINGS} SUBDIALOG;

/** Morpheus SYSEX message formats **/

// Function generator (for PRESET and HYPERPRESET)
typedef struct
{
	struct
	{
		WORD	Level;						// Function generator segment level
		WORD	Time;						// Function generator segment time
		WORD	Shape;						// Function generator segment shape
		WORD	CondJump;					// Function generator jump condition
		WORD	CondValue;					// Function generator conditional value
		WORD	DestSegment;				// Function generator conditional jump-to segment
	} Segment[8];
} FG_PARAMS, FAR *PFG_PARAMS;

// Preset data
typedef struct
{
	BYTE		bHeader[SYSEX_HEADER_LEN];	// SYSEX header
	BYTE		bVersion;					// Preset data version
	WORD		wNumber;					// Preset number

	WORD		Name[12];					// Name (NOT zero terminated)
	WORD		LowKey;						// Lowest MIDI key
	WORD		HighKey;					// Highest MIDI key
	WORD		BendRange;					// Pitch bend range
	WORD		VelocityCurve;				// Velocity curve
	WORD		KeyboardCentre;				// MIDI key keyboard centre point
	WORD		TuneTable;					// Which tuning table
	WORD		MixBus;						// Which output bus
	WORD		PortMode;					// Polyphony mode
	struct
	{
		WORD	Mode;						// Cross-fade mode
		WORD	Direction;					// Cross-fade direction
		WORD	Balance;					// Cross-fade balance
		WORD	Amount;						// Cross-fade amount
		WORD	SwitchPoint;				// Cross-fade switch point
	} XFade;
	struct
	{
		WORD	Source;						// Note-on modulation source
		WORD	Destination;				// Note-on modulation destination
		WORD   	Amount;						// Note-on modulation amount
	} NoteOnModulation[10];
	struct
	{
		WORD	Source;						// Real time modulation source
		WORD	Destination;				// Real time modulation destination
		WORD   	Amount;						// Real time modulation amount
	} RealTimeModulation[10];
	WORD		FootSwitchDestination[3];	// Foot switch destinations
	WORD		CtrlAAmount;				// Controller A amount
	WORD		CtrlBAmount;				// Controller B amount
	WORD		CtrlCAmount;				// Controller C amount
	WORD		CtrlDAmount;				// Controller D amount
	WORD		PressureAmount;				// Pressure amount
	struct
	{
		WORD	Shape;						// LFO shape
		WORD	Rate;						// LFO rate
		WORD	Delay;						// LFO delay
		WORD	Variation;					// LFO variation
		WORD	Amount;						// LFO amount
	} Lfo[2];
	struct
	{
		WORD		Amount;					// Function generator amount
		FG_PARAMS	Segments;				// Segments
	} FunctionGenerator[2];
	struct
	{
		WORD	Delay;						// Auxilliary envelope delay
		WORD	Attack;						// Auxilliary envelope attack
		WORD	Hold;						// Auxilliary envelope hold
		WORD	Decay;						// Auxilliary envelope decay
		WORD	Sustain;					// Auxilliary envelope sustain
		WORD	Release;					// Auxilliary envelope release
		WORD	Amount;						// Auxilliary envelope amount
	} AuxEnv;
	struct
	{
		WORD	LayerInstrument;			// Layer instrument
		WORD	LowKey;						// Layer MIDI lowest key
		WORD	HighKey;					// Layer MIDI highest key
		WORD	Volume;						// Layer volume
		WORD	Pan;						// Layer pan position
		WORD	CoarseTune;					// Layer coarse tuning
		WORD	KeyXpose;					// Layer key transposition
		WORD	FineTune;					// Layer fine tuning
		struct
		{
			WORD	Enable;					// Alternate envelope enable/disable
			WORD	Attack;					// Alternate envelope attack time
			WORD	Hold;					// Alternate envelope hold time
			WORD	Decay;					// Alternate envelope decay time
			WORD	Sustain;				// Alternate envelope sustain time
			WORD	Release;				// Alternate envelope release time
		} AltEnv;
		struct
		{
			WORD	Enable;					// Looping enable
			WORD	StartMS;				// Loop starting point MSB
			WORD	StartLS;				// Loop starting point LSB
			WORD	SizeOffsetMS;			// Loop size offset MSB
			WORD	SizeOffsetLS;			// Loop size offset LSB
		} Loop;
		WORD	SoundStart;					// Instrument start
		WORD	SoundDelay;					// Instrument delay
		WORD	SoloMode;					// Solo mode
		WORD	SoloPriority;				// Solo priority
		WORD	PortamentoRate;				// Portamento rate
		WORD	PortamentoShape;			// Portamento shape
		WORD	DoubleDetune;				// Double + detune levels
		WORD	SoundReverse;				// Instrument reverse
		WORD	NonTranspose;				// Non-transposition
		struct
		{
			WORD	Type;					// Filter type
			WORD	Reverse;				// Filter reverse
			WORD	Level;					// Filter level
			WORD	Morph;					// Filter morphing
			WORD	FreqTrack;				// Filter frequency tracking
			WORD	Transform2;				// Filter transform 2
		} Filter;
		WORD	Unused;						// Unused
	} Layer[2];
	BYTE		CheckSum;					// Data checksum
	BYTE		EndOfSysEx;					// 0xF7 EOX
} PRESET_PARAMS, FAR *PPRESET_PARAMS;

// The checksummable length of preset data
#define	PRESET_CHECKSUM_LENGTH	(FIELDOFFSET (PRESET_PARAMS, CheckSum) - FIELDOFFSET (PRESET_PARAMS, Name))

// Hyperpreset data
typedef struct
{
	BYTE		bHeader[SYSEX_HEADER_LEN];	// SYSEX header
	BYTE		bVersion;					// Hyperpreset data version
	WORD		wNumber;					// Preset number

	WORD		Name[12];					// Hyperpreset name
	WORD		PortMode;					// Port mode
	FG_PARAMS	FreeRunFG;					// Free-running function generator
	struct
	{
		WORD	Preset;						// Preset number
		WORD	Volume;						// Preset volume
		WORD	Pan;						// Preset pan
		WORD	LowKey;						// Preset MIDI low key
		WORD	HighKey;					// Preset MIDI high key
		WORD	LowVel;						// Preset MIDI low velocity
		WORD	HighVel;					// Preset MIDI high velocity
		WORD	VelOffset;					// Preset velocity offset
		WORD	Xpose;						// Preset transpose
		WORD	CoarseTune;					// Preset coarse tuning
		WORD	FineTune;					// Preset fine tuning
	} Zone[16];
	BYTE		CheckSum;					// Data checksum
	BYTE		EndOfSysEx;					// 0xF7 EOX
} HYPERPRESET_PARAMS, FAR *PHYPERPRESET_PARAMS;

// The checksummable length of hyperpreset data
#define	HYPERPRESET_CHECKSUM_LENGTH	(FIELDOFFSET (HYPERPRESET_PARAMS, CheckSum) - FIELDOFFSET (HYPERPRESET_PARAMS, Name))

// Midimap data
typedef struct
{
	BYTE		bHeader[SYSEX_HEADER_LEN];	// SYSEX header
	BYTE		bVersion;					// Midimap data version
	WORD		wNumber;					// Preset number

	WORD		Name[12];					// Midimap name
	struct									// Midimap channel setups
	{
		WORD	Program;					// Channel program number
		WORD	Bank;						// Channel bank number
		WORD	Volume;						// Channel volume
		WORD	Pan;						// Channel pan position
		WORD	MixBus;						// Channel mix bus
		WORD	Enable;						// Channel enable
		WORD	ProgChgEnbl;				// Channel program change enable
		WORD	BankChgEnbl;				// Channel bank change enable
		WORD	VolCtlEnbl;					// Channel volume change enable
		WORD	PanCtlEnbl;					// Channel pan control enable
		WORD	PWhlEnbl;					// Channel pitch wheel enable
		WORD	MPressEnbl;					// Channel mono pressure enable
		WORD	PPressEnbl;					// Channel poly pressure enable
        WORD	CtlAEnbl;					// Channel controller A enable
		WORD	CtlBEnbl;					// Channel controller B enable
		WORD	CtlCEnbl;					// Channel controller C enable
		WORD	CtlDEnbl;					// Channel controller D enable
		WORD	Ftsw1Enbl;					// Channel footswitch 1 enable
		WORD	Ftsw2Enbl;					// Channel footswitch 2 enable
		WORD	Ftsw3Enbl;					// Channel footswitch 3 enable
	} Channel[NUM_MIDI_CHANNELS];
	WORD		ProgMap;					// Program map (-1 = NONE, 0..3)
	WORD		FxAType;					// Effect A type
	WORD		FxAParmVals[10];			// Effect A params
	WORD		FxBType;					// Effect B type
	WORD		FxBParmVals[10];			// Effect B params
    WORD		FxAAmt;						// Effect A amount
	WORD		FxBAmt;						// Effect B amount
	WORD        BtoAAmt;					// Effect B to A amount %
	WORD		FxABus;						// Effect A mix bus
	WORD		FxBBus;						// Effect B mix bus

	BYTE		CheckSum;					// Data checksum
	BYTE		EndOfSysEx;					// 0xF7 EOX
} MIDIMAP_PARAMS, FAR *PMIDIMAP_PARAMS;

// The checksummable length of midimap data
#define	MIDIMAP_CHECKSUM_LENGTH	(FIELDOFFSET (MIDIMAP_PARAMS, CheckSum) - FIELDOFFSET (MIDIMAP_PARAMS, Name))

// Tuning table data
typedef struct
{
	BYTE		bHeader[SYSEX_HEADER_LEN];	// SYSEX header
	BYTE		Number;						// Table number (always 0)
	WORD		TuneValue[NUM_MIDI_NOTES];	// Tuning values
	BYTE		EndOfSysEx;					// 0xF7 EOX
} TUNING_TABLE_PARAMS, FAR *PTUNING_TABLE_PARAMS;

// Program map data
typedef struct
{
	BYTE		bHeader[SYSEX_HEADER_LEN];	// SYSEX header
	BYTE		Number;						// Map number (0..3)
	WORD		Program[128];				// Program numbers
	BYTE		EndOfSysEx;					// 0xF7 EOX
} PROGRAM_MAP_PARAMS, FAR *PPROGRAM_MAP_PARAMS;

// Version data
typedef struct
{
	BYTE		bHeader[SYSEX_HEADER_LEN];	// SYSEX header
	BYTE		VersionCode;				// (Should always be 2)
	BYTE		Revision[3];				// Revision number
	BYTE		EndOfSysEx;					// 0xF7 EOX
} VERSION_PARAMS, FAR *PVERSION_PARAMS;

// Instrument list
typedef struct
{
	BYTE	bHeader[SYSEX_HEADER_LEN];		// SYSEX header
	WORD	wNumInstruments;				// Number of instruments
	struct
	{
		WORD    wInstrumentNumber;			// Instrument number
		BYTE	szName[12];					// Instrument name
	} Info[1];								// NB. repeated a variable number of times
} INSTRUMENT_LIST_PARAMS, far *PINSTRUMENT_LIST_PARAMS;

// Filter list
typedef struct
{
	BYTE	bHeader[SYSEX_HEADER_LEN];		// SYSEX header
	WORD	wNumFilters;					// Number of filters
	struct
	{
		WORD    wFilterNumber;				// Filter number
		BYTE	szName[12];					// Filter name
		WORD	wTransformNumber;			// Transform number
	} Info[1];								// NB. repeated a variable number of times
} FILTER_LIST_PARAMS, far *PFILTER_LIST_PARAMS;

// Effects lists - nb. can't use the same NUM + Info[] format because of variable A and B lists
typedef struct
{
	BYTE	szName[11];						// Parameter name
	WORD	wMinValue;						// Minimum parameter value
	WORD	wMaxValue;						// Maximum parameter value
	WORD	wDefValue;						// Default parameter value
} EFFECT_PARAM;

typedef struct
{
	WORD	wID;							// Effect ID
	BYTE	szName[12];						// Effect name
	WORD	wNumParams;						// Number of parameters
} EFFECT_DATA_HDR;

typedef struct
{
	WORD		wNumEffects;				// Number of effects
} EFFECT_AB_HDR;

typedef struct
{
	BYTE		bHeader[SYSEX_HEADER_LEN];	// SYSEX header
} EFFECT_LIST_HDR;

// Configuration data
typedef struct
{
	BYTE	bHeader[SYSEX_HEADER_LEN];		// SYSEX header
	WORD	wNumPresets;					// Number of presets
	BYTE	bSetRankA;						// Sound set Rank A
	WORD	wNumInstrsRankA;				// Number of instruments in Rank A
	BYTE	bSetRankB;						// Sound set Rank B
	WORD	wNumInstrsRankB;				// Number of instruments in Rank B
	WORD	wNumHyperpresets;				// Number of hyperpresets
	WORD	wNumFilters;					// Number of filters
	WORD	wNumMidimaps;					// Number of midimaps
	WORD	wNumAEffects;					// Nunber of A effects
	WORD	wNumBEffects;					// Number of B effects
	BYTE	bEndOfSysex;
	BYTE	EndOfSysEx;						// 0xF7 EOX
} CONFIGURATION_DATA_PARAMS, far *PCONFIGURATION_DATA_PARAMS;

// List of preset names
typedef struct
{
	BYTE	bHeader[SYSEX_HEADER_LEN];		// SYSEX header
	WORD	wNumPresets;					// Number of presets
	struct
	{
		BYTE	szName[13];					// Preset name
	} Info[1];								// Repeated a variable number of times
} PRESET_LIST_PARAMS, FAR *PPRESET_LIST_PARAMS;

// List of hyperpreset names
typedef struct
{
	BYTE	bHeader[SYSEX_HEADER_LEN];		// SYSEX header
	WORD	wNumHyperpresets;				// Number of hyperpresets
	struct
	{
		BYTE	szName[13];					// Hyperpreset name
	} Info[1];								// Repeated a variable number of times
} HYPERPRESET_LIST_PARAMS, FAR *PHYPERPRESET_LIST_PARAMS;

// List of midimap names
typedef struct
{
	BYTE	bHeader[SYSEX_HEADER_LEN];		// SYSEX header
	WORD	wNumMidimaps;					// Number of midimaps
	struct
	{
		BYTE	szName[13];					// Midimap name
	} Info[1];								// Repeated a variable number of times
} MIDIMAP_LIST_PARAMS, FAR *PMIDIMAP_LIST_PARAMS;

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/***																	***/
/***						EXTERNALS									***/
/***																	***/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

// From MAIN.C

extern BOOL			fConnectedToMorpheus;	// TRUE if we in constant communication with Morpheus
extern BOOL			fYielding;				// TRUE if we're yielding during a long operation (eg. SYSEX)

	// Global options
extern int			idMidiInput;			// Input device index
extern int			idMidiOutput;			// Output device index
extern BYTE			bMidiDeviceID;			// Morpheus MIDI device ID
extern UINT			nKeyboardLeftKey;		// Test keyboard : key at left of window
extern BYTE			bKbdMidiChannel;		// Test keyboard : MIDI channel
extern int			nTestKbdControllers[4];	// Test keyboard : MIDI controller assignments
extern char			szPresetMacroDirectory[MAXPATH];	// Directory where preset macros are stored
extern char			szStorageDirectory[MAXPATH];		// Directory where sounds etc. are stored

extern HINSTANCE	hInst;					// My instance
extern char			szMyPath[MAXPATH];		// My path (with trailing '\')
extern char			szIniFileName[MAXPATH];	// INI file name

extern HWND			hwndMain;				// Main window
extern HWND			hwndClient;				// MDI client

 extern HWND		hwndPresetsList;		// Presets -> list of presets
 extern HWND		hwndCurrentPreset;		// Current preset window
 extern HWND		hwndNoteOnPatches;		// Current preset -> NOTE-ON patches
 extern HWND		hwndRealTimePatches;	// Current preset -> REAL-TIME patches

 extern HWND		hwndHypersList;			// Hyperpresets -> list window
 extern HWND		hwndCurrentHyper;		// Current hyperpreset editor

 extern HWND		hwndMMsList;			// Midimaps->list
 extern HWND		hwndCurrentMM;			// Current midimap editor

 extern HWND		hwndMaster;				// Master settings window
 extern HWND		hwndTuning;				// User tuning window
 extern HWND		hwndProgramMap;			// Program map window

 extern HWND		hwndPiano;				// Piano keyboard

 extern HWND		hwndLongOperation;		// Message box for long MIDI operations

extern VOID			SetMainScroll (VOID);
extern VOID			DisplayStatusText (int nIndex, LPCSTR lpszString);
extern VOID			DrawBitmap (HDC hDC, HBITMAP hBitmap, int x, int y);

// From SYSEX.C

extern BOOL 			SendMorpheusData (HWND hwndParent, LPBYTE lpData, UINT nLength);
extern BOOL				GetMorpheusData (HWND hwndParent, BYTE bType, int iByteParam,
											int iWordParam, HPBYTE *phpData, DWORD *pdwLength);
extern VOID				WriteSysexWord (LPVOID lpAddr, int iValue);
extern int				ReadSysexWord (LPVOID lpAddr);
#ifdef USE_SYSEX_DIALOG
extern DIALOG_PROC		SysexDumpWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
#else
extern WINDOW_PROC		SysexDumpWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
#endif

// From CONNECT.C

extern VOID				ConnectTopToBottom (HDC hDC, HPEN hPen, int x1, int y1, int x2, int y2);

// From PRESETS.C

extern int				nDisplayedPreset;	// Currently displayed preset (index)
extern PRESET_PARAMS	EditPreset;			// Copy of preset being edited
extern BOOL				fDirtyPreset;		// Whether the current preset has been modified

extern BOOL				CreateAllPresetWindows (VOID);
extern VOID				DestroyAllPresetWindows (VOID);
extern BOOL				CreatePresetsListWindow (VOID);
extern VOID				DestroyPresetsListWindow (VOID);
extern WINDOW_PROC		PresetsListDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern VOID				DisplayPresetsList (VOID);

// From PRESET2.C

extern BOOL				fBigPresetDialog;	// TRUE if whole preset is displayed in one big dialog

extern BOOL				CreateCurrentPresetWindow (VOID);
extern VOID				DestroyCurrentPresetWindow (VOID);

extern WINDOW_PROC		CurrentPresetDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern VOID				PresetSendParam (HWND hwndParent, LPVOID lpVar);

// From PRESET3.C

extern DWORD			InitPresetSubDialog (VOID);
extern VOID				TermPresetSubDialog (VOID);
extern DWORD			PresetSubDialog (int nToolbarButton);
extern BOOL				IsPresetSubDialogDisplayed (SUBDIALOG n);
extern DWORD			SetBigPresetDialog (BOOL fState);

// From PRESET4.C

extern VOID				PresetRightMouseButton (HWND hWnd, int x, int y);

// From PRINIT.C

extern VOID				InitInstrumentsDialog (HWND hWnd);
extern VOID				InitModulatorsDialog (HWND hWnd);
extern VOID				InitMiscDialog (HWND hWnd);
extern VOID				DisplayPreset (VOID);
extern VOID				UpdatePresetInternals (VOID);
extern VOID				FormatPatchLine (int Type, int nPatch, char *pszBuffer);

extern VOID				DisplayPresetLoopStart (int nLayer);
extern VOID				DisplayPresetLoopLength (int nLayer);

// From PATCHBNO.C

extern BOOL				CreateNoteOnPatchBayWindow (VOID);
extern VOID				DestroyNoteOnPatchBayWindow (VOID);
extern WINDOW_PROC		NoteOnPatchBayWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern VOID 			DisplayNoteOnPatch (VOID);

// From PATCHBRT.C

extern BOOL				CreateRealTimePatchBayWindow (VOID);
extern VOID				DestroyRealTimePatchBayWindow (VOID);
extern WINDOW_PROC		RealTimePatchBayWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern VOID				DisplayRealTimePatch (VOID);

// From HYPERS.C

extern int					nDisplayedHyper;	// Currently displayed preset (index)
extern BOOL					fDirtyHyper;		// Whether the current hyperpreset has been modified
extern HYPERPRESET_PARAMS	CurrentHyper;		// The hyperpreset being worked on (local copy)

extern WINDOW_PROC		HypersListDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern BOOL				CreateAllHyperWindows (VOID);
extern VOID				DestroyAllHyperWindows (VOID);
extern BOOL				CreateHypersListWindow (VOID);
extern VOID				DestroyHypersListWindow (VOID);
extern DIALOG_PROC		HyperssListDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern VOID				DisplayHypersList (VOID);

// From HYPER1.C

extern WINDOW_PROC		CurrentHyperDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern VOID				DisplayHyper (int idHyper);
extern BOOL				CreateCurrentHyperWindow (VOID);
extern VOID				DestroyCurrentHyperWindow (VOID);
extern VOID				FormatNote (LPSTR lpBuf, int Note);

// From MIDIMAPS.C

extern int					nDisplayedMM;		// Currently displayed midimap (index)
extern BOOL					fDirtyMM;			// Whether the current midimap has been modified
extern MIDIMAP_PARAMS		CurrentMM;			// The midimap being worked on (local copy)

extern WINDOW_PROC		MMsListDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern BOOL				CreateAllMMWindows (VOID);
extern VOID				DestroyAllMMWindows (VOID);
extern BOOL				CreateMMsListWindow (VOID);
extern VOID				DestroyMMsListWindow (VOID);
extern VOID				DisplayMMsList (VOID);

// From MIDIMAP1.C

extern WINDOW_PROC		CurrentMMDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern VOID				DisplayMM (int idMM);
extern BOOL				CreateCurrentMMWindow (VOID);
extern VOID				DestroyCurrentMMWindow (VOID);

// From DLGUTILS.C

extern HFONT					hDialogFont;
extern VALUE_DISPLAY_FUNCTION	lpfnDisplayNote;
extern VALUE_DISPLAY_FUNCTION	lpfnDisplayNumber;
extern VALUE_DISPLAY_FUNCTION	lpfnDisplayNoteAndNumber;
extern VALUE_DISPLAY_FUNCTION	lpfnDisplayBendRange;
extern VALUE_DISPLAY_FUNCTION	lpfnDisplayPresetPan;
extern BOOL						fAbortLongOperation;

extern VOID				DlgUtilsInit (VOID);
extern VOID				DlgUtilsTerm (VOID);
extern WINDOW_PROC		MyDialogWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern VOID				ReformatDialog (HWND hWnd, WORD wFlags);
extern VOID				SaveWindowPosition (LPCSTR lpszName, HWND hWnd);
extern VOID				GetWindowPosition (LPCSTR lpszName, HWND hWnd, BOOL fResize);
extern VOID				StartLongOperation (LPCSTR lpszMessage);
extern VOID				EndLongOperation (VOID);
extern BOOL				AbortLongOperation (VOID);
extern VOID				SetListBoxTabs (HWND hWnd, int idListBox, LISTBOX_TAB_INFO *pLTI, UINT nNumTabs);
extern BOOL				MyGetLoadFileName (HWND hWnd, LPCSTR lpszTitle, LPCSTR lpszFilter, LPSTR lpszFileName);
extern BOOL				MyGetSaveFileName (HWND hWnd, LPCSTR lpszTitle, LPCSTR lpszFilter, LPSTR lpszFileName);
extern VOID				InterceptEditEnter (HWND hwndParent, int idEditControl);

extern VOID				InitListbox (HWND hwndControl, LPCSTR *pszItems, int nNumItems);
extern VOID				InitSlider (HWND hwndParent, int idControl, int nMin, int nMax, int nDefault);
extern VOID				InitUpDown (HWND hwndParent, int idControl, int nMin, int nMax, int nDefault,
									HWND hwndPartner, VALUE_DISPLAY_FUNCTION pfnDisplay);


extern DWORD			DialogInWindow (HWND hWnd, LPCSTR lpszDialogName);
extern VOID				DestroyAllChildren (HWND hWnd);
extern VOID				GetCreatePosition (LPCSTR lpszName, BOOL fResize, LPWINDOWPLACEMENT lpWp);
extern WINDOW_PROC		InvisoWndProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern HWND				CreateMouseEater (HWND hwndParent);
extern VOID				EnableAllMenuItems (HWND hWnd, BOOL fEnable);
extern LPCSTR			ResourceString (int idString);

// From MIDI.C

extern VOID			MidiStatus (LPCSTR lpString);
extern VOID			CreateMidiStatusDialog (VOID);
extern VOID			DisplayMidiError (WORD wErrorCode);
extern BYTE			MidiChecksum (LPVOID lpData, UINT nLength);

// From MDATA.C

extern PPRESET_PARAMS				pMorpheusPresets[MAX_PRESETS];
extern PHYPERPRESET_PARAMS			pMorpheusHyperPresets[MAX_HYPERPRESETS];
extern PMIDIMAP_PARAMS				pMorpheusMidimaps[MAX_MIDIMAPS];
extern TUNING_TABLE_PARAMS			MorpheusTuningTable;
extern PROGRAM_MAP_PARAMS			MorpheusProgramMaps[NUM_PROGRAM_MAPS];
extern PPRESET_LIST_PARAMS			pMorpheusPresetList;
extern PHYPERPRESET_LIST_PARAMS		pMorpheusHyperpresetList;
extern PMIDIMAP_LIST_PARAMS			pMorpheusMidimapList;

extern VERSION_PARAMS				MorpheusVersionData;
extern CONFIGURATION_DATA_PARAMS	MorpheusConfigurationData;

extern PINSTRUMENT_LIST_PARAMS		pMorpheusInstrumentList;
extern WORD							nMorpheusInstrumentListLength;

extern PFILTER_LIST_PARAMS			pMorpheusFilterList;
extern WORD							nMorpheusFilterListLength;

extern LPVOID						pMorpheusEffectList;
extern WORD							nMorpheusEffectListLength;

extern LPBYTE						pMasterSettings;
extern WORD							nMasterSettingsLength;

// From MIDIRX.C

extern VOID	MidiRxFreeAll (VOID);
extern VOID	MidiRxFreeInternals (VOID);

extern BOOL	MorpheusGetInternals (VOID);
extern BOOL	MorpheusGetAll (VOID);
extern BOOL	MorpheusGetConfigurationData (VOID);
extern BOOL	MorpheusGetVersionData (VOID);
extern BOOL	MorpheusGetInstrumentList (VOID);
extern BOOL	MorpheusGetFilterList (VOID);
extern BOOL	MorpheusGetEffectList (VOID);
extern BOOL	MorpheusGetPresetsList (VOID);
extern BOOL	MorpheusGetHyperpresetsList (VOID);
extern BOOL	MorpheusGetMidimapsList (VOID);
extern BOOL	MorpheusGetPreset (int idPreset);
extern BOOL MorpheusGetHyper (int idHyper);
extern BOOL MorpheusGetMM (int idMM);
extern BOOL	MorpheusGetTuning (TUNING_TABLE_PARAMS *pTable);
extern BOOL	MorpheusGetMasterSettings (VOID);
extern BOOL MorpheusGetProgramMap (int nMap);

extern int	InstrumentNumberToIndex (int idInstrument);
extern int	IndexToInstrumentNumber (int nIndex);
extern int	FilterNumberToIndex (int idFilter);
extern int	IndexToFilterNumber (int nIndex);

extern int					EffectNumberToIndex (int idFx, int idEffect);
extern int					IndexToEffectNumber (int idFx, int idIndex);
extern EFFECT_PARAM_INFO	*GetEffectParamInfo (int idFx, int idIndex, int *pNumParams);
extern int					NumEffects (int idFx);
extern LPCSTR				EffectsName (int idFx, int idIndex);

extern LPCSTR				ProgramName (int idProgram);

// From MIDITX.C

extern BOOL	MorpheusSelectPreset (HWND hwndParent, int nPreset);
extern BOOL	MorpheusSelectHyper (HWND hwndParent, int nHyper);
extern BOOL	MorpheusSelectMM (HWND hwndParent, int nMM);
extern BOOL	MorpheusSetParameter (HWND hwndParent, int nParam, int nValue);
extern BOOL	MorpheusSendPreset (HWND hwndParent, int idPreset);
extern BOOL MorpheusSendHyper (HWND hwndParent, int idHyper);
extern BOOL MorpheusSendMM (HWND hwndParent, int idMM);
extern BOOL MorpheusSendTuning (HWND hwndParent, TUNING_TABLE_PARAMS *pTable);
extern BOOL MorpheusSendProgramMap (HWND hwndParent, int nMap);

extern BOOL	SendChannelProgram (HWND hWnd, int nChan, int nProgram);
extern BOOL	SendChannelVolume (HWND hWnd, int nChan, int nVolume);
extern BOOL	SendChannelPan (HWND hWnd, int nChan, int nPan);

// From FUNCGEN.C

extern VOID		DisplayFuncGenList (HWND hWnd, PFG_PARAMS pFgParams);
extern LPCSTR	FGShapeName (int n);
extern LPCSTR	FGConditionName (int n);

// From FILEINT.C

extern BOOL 	SaveInternals (VOID);
extern BOOL 	LoadInternals (VOID);
extern WORD		CalcChecksum (LPVOID lpData, WORD nLength);

extern BOOL		SaveMasterSettings (VOID);
extern BOOL		LoadMasterSettings (VOID);

// From FILEPRS.C

extern BOOL		LoadPreset (int idPreset);
extern BOOL		SavePreset (int idPreset);

// From FILEHPS.C

extern BOOL		LoadHyper (int idHyper);
extern BOOL		SaveHyper (int idHyper);

// From FILEMM.C

extern BOOL		SaveMM (int idMM);
extern BOOL		LoadMM (int idMM);

// From FILETUNE.C

extern BOOL		SaveTuning (VOID);
extern BOOL		LoadTuning (VOID);

// From FILEPM.C

extern BOOL		SaveProgramMap (int nMap);
extern BOOL		LoadProgramMap (int nMap);

// From FILES.C

extern BOOL		SaveAllSounds (VOID);
extern BOOL		LoadAllSounds (VOID);
extern VOID		CreatePresetNames (int nPreset);
extern VOID		CreateHyperNames (int nHyper);
extern VOID		CreateMMNames (int nMM);

// From MIDIFILE.C

extern BOOL		ExportPreset (int idPreset);
extern BOOL		ExportHyper (int idHyper);
extern BOOL		ExportMM (int idMM);

// From MSGBOX.C

extern int		MyResMessageBox (HWND hwndParent, int idText, int idTitle, UINT fuStyle);
extern int		MyMessageBox (HWND hwndParent, LPCSTR lpszText, LPCSTR lpszTitle, UINT fuStyle);

// From PIANO.C

extern WINDOW_PROC	PianoWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern BOOL		CreatePianoWindow (VOID);
extern VOID		DestroyPianoWindow (VOID);

// From MASTER.C

extern WINDOW_PROC	MasterDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern VOID		DisplayMasterSettings (VOID);
extern BOOL		CreateMasterWindow (VOID);
extern VOID		DestroyMasterWindow (VOID);

// From TUNING.C

extern WINDOW_PROC	TuningDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern BOOL		CreateUserTuningWindow (VOID);
extern VOID		DestroyUserTuningWindow (VOID);
extern VOID		DisplayTuning (VOID);

// From PROGMAP.C

extern BOOL		fDirtyProgramMap;
extern int		nSelectedProgramMap;

extern WINDOW_PROC	ProgramMapDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern BOOL		CreateProgramMapWindow (VOID);
extern VOID		DestroyProgramMapWindow (VOID);
extern VOID		DisplayProgramMap (VOID);

// From OPTIONS.C

extern DIALOG_PROC	MidiOptionsDialog (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
extern VOID 		LoadOptions (VOID);
extern VOID			StoreOptions (VOID);
extern VOID			SaveStoragePath (LPCSTR lpszPath);

// From BMPCTRL.C

extern BOOL		StaticBitmapControlInit (HINSTANCE hInst);
extern VOID		StaticBitmapControlTerm (HINSTANCE hInst);


