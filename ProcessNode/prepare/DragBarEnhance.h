#pragma once

class CDragBarEnhance
{
public:
	CDragBarEnhance(void);
	~CDragBarEnhance(void);

	BOOL process(int nID,ConstImageBuf original, ImageBuf processed,float* fBarPos,bool bIsDrag,bool bIsROI, RECT* mROIRect);

	bool LigConProcCpu(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect);
	bool DetailProcCpu(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect);

	BOOL LigConProcIpp(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect);
	BOOL DetailProcIpp(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect);

	int Bisearch(WORDTYPE *data,int x,int begin,int last);
	bool DefaultEnhance(ConstImageBuf original, ImageBuf processed,float * fBarPos,RECT* mROIRect);

	bool flag;
	WORDTYPE nGrayThreshold;
	float fDifThreshold;
	int nRadius;
	float fDetailDegree;//
	float fEdgeDegree;//边缘增强参数
};
