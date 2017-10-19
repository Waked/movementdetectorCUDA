#pragma once
#define _AFXDLL
#include <afxwin.h>
#include <mmsystem.h>

BOOL PlayResource(LPSTR lpName)
{
	BOOL bRtn;
	LPSTR lpRes;
	HANDLE hRes;
	HRSRC hResInfo;
	HINSTANCE Nl = AfxGetInstanceHandle();

	/* Find the WAVE resource. */
	hResInfo = FindResource(Nl, lpName, "WAVE");
	if (hResInfo == NULL)
		return FALSE;
	/* Load the WAVE resource. */

	hRes = LoadResource(Nl, hResInfo);
	if (hRes == NULL)
		return FALSE;

	/* Lock the WAVE resource and play it. */
	lpRes = (LPSTR)LockResource(hRes);
	if (lpRes == NULL)
		return FALSE;

	bRtn = sndPlaySound(lpRes, SND_MEMORY | SND_SYNC);
	if (bRtn == NULL)
		return FALSE;

	/* Free the WAVE resource and return success or failure. */
	FreeResource(hRes);
	return TRUE;
}