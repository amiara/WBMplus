/*******************************************************************************
 * Copyright (c) 2005, 2007 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/
#include<cm.h>
#include<MF.h>
#include<MD.h>
 
// Input
static int _MDInDischCalculatedID = MFUnset;
static int _MDInDischReferenceID  = MFUnset;
static int _MDInResCapacityID     = MFUnset;
// Output
static int _MDOutResStorageID     = MFUnset;
static int _MDOutResStorageChgID  = MFUnset;
static int _MDOutResReleaseID     = MFUnset;

static void _MDReservoir (int itemID) {

// Input
	float discharge;      // Current discharge [m3/s]
	float meanDischarge;  // Long-term mean annual discharge [m3/s]
	float resCapacity;    // Reservoir capacity [km3]
// Output
	float resStorage;     // Reservoir storage [km3]
	float resStorageChg;  // Reservoir storage change [km3/dt]
	float resRelease;     // Reservoir release [m3/s] 
// local
	float prevResStorage; // Reservoir storage from the previous time step [km3]
	float dt;             // Time step length [s]
// Parameters
	float drySeasonPct = .6;
	float wetSeasonPct = 0.16;
	
	if (MFVarTestMissingVal (_MDInDischCalculatedID, itemID) ||
	    MFVarTestMissingVal (_MDInDischReferenceID,  itemID)) discharge = meanDischarge = 0.0;
	else {
		discharge     = MFVarGetFloat (_MDInDischCalculatedID,  itemID);
		meanDischarge = MFVarGetFloat (_MDInDischReferenceID,   itemID);
	}
	if (MFVarTestMissingVal (_MDInResCapacityID,     itemID) ||
	    ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID)) <= 0.0)) { 
		MFVarSetFloat (_MDOutResStorageID,    itemID, 0.0); 
		MFVarSetFloat (_MDOutResStorageChgID, itemID, 0.0); 
		MFVarSetFloat (_MDOutResReleaseID,    itemID, discharge);
		return;
	}

	dt = MFModelGet_dt ();
	if (MFVarTestMissingVal (_MDOutResStorageID, itemID)) prevResStorage = 0.0;
	else prevResStorage = MFVarGetFloat(_MDOutResStorageID, itemID);

	resRelease = discharge > meanDischarge ?
		         wetSeasonPct * discharge  :
		         drySeasonPct * discharge + (meanDischarge - discharge);

 	resStorage = prevResStorage + (discharge - resRelease) * 86400.0 / 1e9;
	if (resStorage > resCapacity) {
		resRelease = discharge * dt / 1e9 + prevResStorage - resCapacity;
		resRelease = resRelease * 1e9 / dt;
		resStorage = resCapacity;
	}
	else if (resStorage < 0.0) {
		resRelease = prevResStorage + discharge  * dt / 1e9;
		resRelease = resRelease * 1e9 / dt;
		resStorage=0;
	}
			
	resStorageChg = resStorage - prevResStorage;
	MFVarSetFloat (_MDOutResStorageID,    itemID, resStorage);
	MFVarSetFloat (_MDOutResStorageChgID, itemID, resStorageChg);
	MFVarSetFloat (_MDOutResReleaseID,    itemID, resRelease);
}

enum { MDhelp, MDnone, MDcalculate };

int MDReservoirDef () {
	int optID = MDnone;
	const char *optStr, *optName = MDModReservoirs;
	const char *options [] = { MDHelpStr, MDNoneStr, MDCalculateStr, (char *) NULL };
  
	if (_MDOutResReleaseID != CMfailed) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoirs");
	if (((optStr = MFOptionGet (MDModReservoirs)) != (char *) NULL) && (CMoptLookup (options, optStr, true) != MDcalculate)) {
		CMmsgPrint (CMmsgInfo,"Help [%s options]:",optName);
		for (optID = 1;options [optID] != (char *) NULL;++optID) CMmsgPrint (CMmsgInfo," %s",options [optID]);
		CMmsgPrint (CMmsgInfo,"\n");
		return (CMfailed);
	}

	if (((_MDInDischCalculateID = MDDischCalculatedDef ())) == CMfailed) ||
	    ((_MDInDischReferenceID = MDDischReferenceDef  ())) == CMfailed) ||
	    ((_MDInResCapacityID    = MFVarGetID (MDVarReservoirCapacity,      "km3",  MFInput,  MFState, false)) == CMfailed) ||
	    ((_MDOutResStorageID    = MFVarGetID (MDVarReservoirStorage,       "km3",  MFOutput, MFState, true))  == CMfailed) ||
	    ((_MDOutResStorageChgID = MFVarGetID (MDVarReservoirStorageChange, "km3",  MFOutput, MFState, true))  == CMfailed) ||
		((_MDOutResReleaseID    = MFVarGetID (MDVarReservoirRelease,       "m3/s", MFOutput, MFFlux,  false)) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Reservoirs");
	return (MFVarSetFunction(_MDOutResReleaseID,_MDReservoir)); 
}