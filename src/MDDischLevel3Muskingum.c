/******************************************************************************

GHAAS Water Balance/Transport Model V3.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2007, University of New Hampshire

MDDichLevel3Muskingum.c

balazs.fekete@unh.edu

*******************************************************************************/

#include<stdio.h>
#include<math.h>
#include<cm.h>
#include<MF.h>
#include<MD.h>

// Input
static int _MDInMuskingumC0ID  = MFUnset;
static int _MDInMuskingumC1ID  = MFUnset;
static int _MDInMuskingumC2ID  = MFUnset;
static int _MDInRunoffVolumeID = MFUnset;
static int _MDInDischargeID    = MFUnset;
// Output
static int _MDOutDischAux0ID   = MFUnset;
static int _MDOutDischAux1ID   = MFUnset;
static int _MDOutDischLevel3ID = MFUnset;

static void _MDDischLevel3Muskingum (int itemID) {
// Input
	float C0;              // Muskingum C0 coefficient (current inflow)
	float C1;              // Muskingum C1 coefficient (previous inflow)
	float C2;              // MUskingum C2 coefficient (previous outflow) 
	float runoff;          // Runoff [mm/dt]
// Output
	float inDischCurrent;  // Upstream discharge at the current time step [m3/s]
	float outDisch;        // Downstream discharge [m3/s]
// Local
	float inDischPrevious; // Upstream discharge at the previous time step [m3/s]
	
	if (MFVarTestMissingVal (_MDInMuskingumC0ID,   itemID) ||
		MFVarTestMissingVal (_MDInMuskingumC1ID,   itemID) ||
		MFVarTestMissingVal (_MDInMuskingumC2ID,   itemID)) {
		MFVarSetFloat (_MDOutDischAux0ID,    itemID, 0.0);
		MFVarSetFloat (_MDOutDischAux1ID,    itemID, 0.0);
		MFVarSetFloat (_MDOutDischLevel3ID,  itemID, 0.0);
		return;
	}
	C0 = MFVarGetFloat (_MDInMuskingumC0ID,   itemID);
	C1 = MFVarGetFloat (_MDInMuskingumC1ID,   itemID);
	C2 = MFVarGetFloat (_MDInMuskingumC2ID,   itemID);

	if (MFVarTestMissingVal (_MDInRunoffVolumeID, itemID)) runoff          = 0.0;
 	else runoff          = MFVarGetFloat (_MDInRunoffVolumeID, itemID);
 	if (MFVarTestMissingVal (_MDOutDischAux0ID,   itemID)) inDischPrevious = 0.0;
	else inDischPrevious = MFVarGetFloat (_MDOutDischAux0ID,    itemID);
	if (MFVarTestMissingVal (_MDOutDischAux1ID,   itemID)) outDisch        = 0.0;
	else outDisch        = MFVarGetFloat (_MDOutDischAux1ID,  itemID);
	if (MFVarTestMissingVal (_MDInDischargeID,   itemID))  inDischCurrent  = 0.0;
	else inDischCurrent  = MFVarGetFloat (_MDInDischargeID,   itemID) + runoff;

	outDisch = C0 * inDischCurrent + C1 * inDischPrevious + C2 * outDisch;
	MFVarSetFloat (_MDOutDischAux0ID,    itemID, inDischCurrent);
	MFVarSetFloat (_MDOutDischAux1ID,    itemID, outDisch);
	MFVarSetFloat (_MDOutDischLevel3ID,  itemID, outDisch);
}

int MDDischLevel3MuskingumDef () {

	if (_MDOutDischLevel3ID != CMfailed) return (_MDOutDischLevel3ID);

	MFDefEntering ("Discharge Muskingum");

	if (((_MDInRunoffVolumeID  = MDRunoffVolumeDef ()) == CMfailed) ||
	    ((_MDInMuskingumC0ID   = MDDischLevel3MuskingumCoeffDef ()) == CMfailed) ||
	    ((_MDInMuskingumC1ID   = MFVarGetID (MDVarMuskingumC1,      MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
	    ((_MDInMuskingumC2ID   = MFVarGetID (MDVarMuskingumC2,      MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
	    ((_MDInDischargeID     = MFVarGetID (MDVarDischarge,          "m3/s", MFInput,  MFState, MFBoundary)) == CMfailed) ||
	    ((_MDOutDischLevel3ID  = MFVarGetID (MDVarDischLevel3,        "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed))
		return (CMfailed);
	MFDefLeaving ("Discharge Muskingum");
	return (MFVarSetFunction(_MDOutDischLevel3ID,_MDDischLevel3Muskingum));
}
