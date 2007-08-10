/******************************************************************************

GHAAS Water Balance/Transport Model V3.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2007, University of New Hampshire

MDDischarge.c

balazs.fekete@unh.edu

*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <cm.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInDischLevel1ID   = MFUnset;
static int _MDInDischObservedID = MFUnset;

// Output
static int _MDOutDischargeID     = MFUnset;

static void _MDDischarge (int itemID) {
	float discharge;

	discharge = MFVarGetFloat (_MDInDischLevel1ID,   itemID, 0.0);

	if (_MDInDischObservedID != MFUnset)
		 discharge = MFVarGetFloat (_MDInDischObservedID, itemID, discharge);

	MFVarSetFloat (_MDOutDischargeID, itemID, discharge);
}

enum { MDinput, MDsimulated, MDcorrected };

int MDDischargeDef() {
	int optID = MDinput;
	const char *optStr, *optName = MDOptDischarge;
	const char *options [] = { MDInputStr, "simulated", "corrected", (char *) NULL };

	MFDefEntering (MDOptDischarge);
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);
	switch (optID) {
		case MDinput: _MDOutDischargeID = MFVarGetID (MDVarDischarge,     "m3/s",  MFInput,  MFState, MFBoundary); break;
		case MDcorrected:
			if ((_MDInDischObservedID   = MFVarGetID (MDVarDischObserved, "m3/s",  MFInput,  MFState, MFBoundary)) == CMfailed)
				return (CMfailed);
		case MDsimulated:
			if (((_MDOutDischargeID     = MFVarGetID (MDVarDischarge,     "m3/s",  MFRoute,  MFState, MFBoundary)) == CMfailed) ||
				((_MDInDischLevel1ID    = MDDischLevel1Def ()) == CMfailed) ||
				(MFModelAddFunction (_MDDischarge) == CMfailed)) return (CMfailed);
			break;
		default: MFOptionMessage (optName, optStr, options); return (CMfailed);
	}
	MFDefLeaving ("Discharge");
	return (_MDOutDischargeID);
}
