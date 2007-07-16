/******************************************************************************

GHAAS Water Balance Model Library V1.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2007, University of New Hampshire

MDAvgNSteps.c

balazs.fekete@unh.edu

*******************************************************************************/

#include<stdio.h>
#include<cm.h>
#include<MF.h>
#include<MD.h>

static int _MDOutAvgNStepsID = CMfailed;

static void _MDAvgNSteps (int itemID) {
	int nStep;
	if (MFVarTestMissingVal (_MDOutAvgNStepsID, itemID))
		MFVarSetInt (_MDOutAvgNStepsID, itemID, 1);
	else {
		nStep = MFVarGetInt (_MDOutAvgNStepsID, itemID);
		MFVarSetInt (_MDOutAvgNStepsID, itemID, nStep + 1);
	}
}
int MDAvgNStepsDef () {

	if (_MDOutAvgNStepsID != CMfailed) return (_MDOutAvgNStepsID);
	MFDefEntering ("Average NSteps");
	if ((_MDOutAvgNStepsID = MFVarGetID (MDVarAvgNSteps, MFNoUnit, MFInt,    MFState,  true)) == CMfailed)
		return (CMfailed);
	MFDefLeaving ("Average NSteps");
	return (MFVarSetFunction(_MDOutAvgNStepsID,_MDAvgNSteps));
}