/******************************************************************************

GHAAS Water Balance Model Library V1.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2004, University of New Hampshire

MDGrossRad.c

balazs.fekete@unh.edu

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cm.h>
#include <MF.h>
#include <MD.h>

#define  DTOR 0.01745329252
static float _MDGrossRadStdTAU = 1.0;

static int _MDOutGrossRadID = CMfailed;

static void _MDGrossRadianceStd (int itemID) {
/* Input */
	int   day;
	float lambda;
/* Output */
	float  grossRad;
/* Local */
	int   hour;
	double eta,sigma,sinphi,sp,sbb;

	day    = MFDateGetDayOfYear ();
   lambda = MFModelGetLatitude (itemID) * DTOR;

	sp = 1360.0 * 3600.0 * 24.0 * 0.041841 / 41860.0; /* FBM  0.041841 conversion from cal/cm2 to MJ/m2 */
	grossRad = 0;
	sigma = -23.4 * cos (2.0 * M_PI * (day + 11.0) / 365.0) * DTOR;
	for (hour = 0;hour < 24; hour++) {
		eta = (hour + 1) * M_PI / 12.0;
		sinphi = sin (lambda) * sin (sigma) + cos (lambda) * cos (sigma) * cos (eta);
		sbb = sp * sinphi * pow ((double) _MDGrossRadStdTAU,(double) (1.0 / sinphi));
		if (sbb > 0) grossRad += sbb;
	}
	MFVarSetFloat (_MDOutGrossRadID,  itemID, grossRad / 24.0);
}

static void _MDGrossRadianceOtto (int itemID) {
/* Input */
	int   day;
	float lambda;
/* Output */
	float  grossRad;
/* Local */
	int   hour;
	double eta, sigma,sinphi,sp,sbb,sotd;

	day    = MFDateGetDayOfYear ();
   lambda = MFModelGetLatitude (itemID) * DTOR;

	sp = 1360.0 * 3600.0 * 24.0 * 0.041841 / 41860.0; /* FBM  0.041841 conversion from cal/cm2 to MJ/m2 */
	grossRad = 0.0;
	sigma = -23.4856 * cos (2.0 * M_PI * (day + 11.0) / 365.25) * DTOR;
	for (hour = 0;hour < 24;hour++) {
		eta = (double) ((hour + 1)) * M_PI / 12.0;
		sinphi = sin (lambda) * sin (sigma) + cos (lambda) * cos (sigma) * cos (eta);
 		sotd = 1 - (0.016729 * cos (0.9856 * (day - 4.0) * DTOR));
		sbb = sp * sinphi / pow (sotd,2.0);
		if (sbb >= 0) grossRad += sbb;
	}
	MFVarSetFloat (_MDOutGrossRadID,  itemID, grossRad / 24.0);
}

enum { MDhelp, MDinput, MDstandard,  MDOtto }; 

int MDGrossRadDef () {
	int optID = MDinput;
	const char *optStr, *optName = MDVarGrossRadiance;
	const char *options [] = { MDHelpStr, "input", "standard", "Otto", (char *) NULL };
	float par;

	if (_MDOutGrossRadID != CMfailed) return (_MDOutGrossRadID);

	MFDefEntering ("Gross Radiance");
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);

	switch (optID) {
		case MDinput: _MDOutGrossRadID = MFVarGetID (MDVarGrossRadiance, "MJ/m^2", MFInput,  MFFlux,  false); break;
		case MDstandard:
			if ((_MDOutGrossRadID    = MFVarGetID (MDVarGrossRadiance, "MJ/m^2", MFOutput, MFFlux,  false)) == CMfailed)
				return (CMfailed);
			if (((optStr = MFOptionGet (MDParGrossRadTAU)) != (char *) NULL) && (sscanf (optStr,"%f",&par) == 1))
				_MDGrossRadStdTAU = par;
			_MDOutGrossRadID = MFVarSetFunction (_MDOutGrossRadID,_MDGrossRadianceStd);
			break;
		case MDOtto:
			if ((_MDOutGrossRadID    = MFVarGetID (MDVarGrossRadiance, "MJ/m^2", MFOutput, MFFlux,  false)) == CMfailed)
				return (CMfailed);
			_MDOutGrossRadID = MFVarSetFunction (_MDOutGrossRadID,_MDGrossRadianceOtto);
		
			break;
		default:
			CMmsgPrint (CMmsgInfo,"Help [%s options]:",optName);
			for (optID = 1;options [optID] != (char *) NULL;++optID) CMmsgPrint (CMmsgInfo," %s",options [optID]);
			CMmsgPrint (CMmsgInfo,"\n");
			return (CMfailed);
	}
	MFDefLeaving ("Gross Radiance");
	return (_MDOutGrossRadID);
}