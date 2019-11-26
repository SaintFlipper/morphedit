/*****************************************************************************
 File:			mdata.c
 Description:	Just Morpheus data.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/4//94	| Created.											| PW
*****************************************************************************/

#include <windowsx.h>
#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								User editable								*/
/****************************************************************************/

// Presets (allocated)
PPRESET_PARAMS				pMorpheusPresets[MAX_PRESETS] = {NULL};

// Hyperpresets (allocated)
PHYPERPRESET_PARAMS			pMorpheusHyperPresets[MAX_HYPERPRESETS] = {NULL};

// Midimaps (allocated)
PMIDIMAP_PARAMS				pMorpheusMidimaps[MAX_MIDIMAPS] = {NULL};

// Tuning table (static)
TUNING_TABLE_PARAMS			MorpheusTuningTable = {0};

// Program map (static)
PROGRAM_MAP_PARAMS			MorpheusProgramMaps[NUM_PROGRAM_MAPS] = {0};

// Preset list (allocated)
PPRESET_LIST_PARAMS			pMorpheusPresetList;

// Hyperpreset list (allocated)
PHYPERPRESET_LIST_PARAMS	pMorpheusHyperpresetList;

// Midimap list (allocated)
PMIDIMAP_LIST_PARAMS		pMorpheusMidimapList;

// Master settings (allocated)
LPBYTE						pMasterSettings;
WORD						nMasterSettingsLength;

/****************************************************************************/
/*								Fixed configuration							*/
/****************************************************************************/

// Version data (static)
VERSION_PARAMS				MorpheusVersionData = {0};

// Configuration data (static)
CONFIGURATION_DATA_PARAMS	MorpheusConfigurationData = {0};

// Instrument list (allocated)
PINSTRUMENT_LIST_PARAMS		pMorpheusInstrumentList = NULL;
WORD						nMorpheusInstrumentListLength;

// Filter list (allocated)
PFILTER_LIST_PARAMS			pMorpheusFilterList = NULL;
WORD						nMorpheusFilterListLength;

// Effect list (allocated)
LPVOID						pMorpheusEffectList = NULL;
WORD						nMorpheusEffectListLength;
