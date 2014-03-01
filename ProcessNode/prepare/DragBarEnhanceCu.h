#pragma once

#include "ExportFunction.h"

typedef struct _DetailPara
{
	WORDTYPE wBackgroundThreshold;
	int nRadius;
	float fDetailDegree;
	WORDTYPE nGrayThreshold;
	float fDifThreshold;
	float fEdgeDegree;
}DetailPara;

BOOL InitCUDA(void);
bool LigConProcCuda(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect);
bool DetailProcCuda(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect,DetailPara depara);

