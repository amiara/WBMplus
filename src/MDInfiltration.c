/******************************************************************************

GHAAS Water Balance Model Library V1.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2007, University of New Hampshire

MDInfiltration.c

balazs.fekete@unh.edu

*******************************************************************************/

#include<stdio.h>
#include<string.h>
#include<cm.h>
#include<MF.h>
#include<MD.h>

/* Input */
// Only from non-irrigated Areas!
static int _MDInIrrAreaID;

static int _MDInWaterSurplusID;


/* Output */
static int _MDOutSurfaceROID;
static int _MDOutInfiltrationID = CMfailed;

static float _MDInfiltrationFrac = 0.5;

static void _MDInfiltrationSimple (int itemID) {

	float surplus;
	if (MFVarTestMissingVal (_MDInIrrAreaID,      itemID)) MFVarSetFloat(_MDInIrrAreaID,itemID, 0.0);

	if (MFVarTestMissingVal (_MDInWaterSurplusID, itemID) ||
	    MFVarTestMissingVal (_MDInIrrAreaID,      itemID)) {
		MFVarSetMissingVal (_MDOutSurfaceROID, itemID);
		MFVarSetMissingVal (_MDOutInfiltrationID, itemID);
// 		printf ("Missing vares in MDinfiltration\n");		
	}
	else {
		surplus = MFVarGetFloat(_MDInWaterSurplusID, itemID);
		MFVarSetFloat (_MDOutSurfaceROID,    itemID, (1.0 - _MDInfiltrationFrac) * surplus);
	 	MFVarSetFloat (_MDOutInfiltrationID, itemID,  _MDInfiltrationFrac        * surplus);
	}
}

enum { MDhelp, MDsimple, MDvarying };


int MDInfiltrationDef () {
	int  optID = MDsimple;
	const char *optStr, *optName = "Infiltration";
	const char *options [] = { MDHelpStr, "simple", "varying", (char *) NULL };
	float par;

	if (_MDOutInfiltrationID != CMfailed) return (_MDOutInfiltrationID);
	
	MFDefEntering ("Infiltration");
 	
	const char *optIrrStr, *optIrrName = "Irrigation";
	const char *optionsIrr [] = { MDHelpStr, MDInputStr, "none", MDCalculateStr, (char *) NULL };
	int optIrrID ;
 	if ((optIrrStr = MFOptionGet (optIrrName)) != (char *) NULL) optIrrID = CMoptLookup (optionsIrr,optIrrStr,true);
 	if (strcmp(optIrrStr, "calculate") == 0) 
	{
	 if ((_MDInIrrAreaID       = MFVarGetID (MDVarIrrAreaFraction,        "%",  MFInput,  MFState, false))  == CMfailed) return (CMfailed);
 	}
	if ((_MDInWaterSurplusID = MDWaterSurplusDef ()) == CMfailed) return (CMfailed);
	
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
		
	switch (optID) {
		case MDsimple: 		
			if (((optStr = MFOptionGet (MDParInfiltrationFrac))  != (char *) NULL) && (sscanf (optStr,"%f",&par) == 1))
				_MDInfiltrationFrac = par;
			if (((_MDOutSurfaceROID    = MFVarGetID (MDVarSurfaceRO,    "mm", MFOutput, MFFlux, false)) == CMfailed) ||
		       ((_MDOutInfiltrationID = MFVarGetID (MDVarInfiltration, "mm", MFOutput, MFFlux, false)) == CMfailed))
				return (CMfailed);
			_MDOutInfiltrationID = MFVarSetFunction (_MDOutInfiltrationID,_MDInfiltrationSimple);
		case MDvarying:
			break;
		default:
			CMmsgPrint (CMmsgInfo,"Help [%s options]:",optName);
			for (optID = 1;options [optID] != (char *) NULL;++optID) CMmsgPrint (CMmsgInfo," %s",options [optID]);
			CMmsgPrint (CMmsgInfo,"\n");
			return (CMfailed);
	}
	MFDefLeaving  ("Infiltration");
	return (_MDOutInfiltrationID);
}