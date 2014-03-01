#include "stdafx.h"

#include "DragBarEnhance.h"
#include "DragBarEnhanceCu.h"

#include "SpecEnh/libs/ippi.h"
#include "SpecEnh/libs/ippcore.h"

//����ȫ�ֱ���
extern bool g_bCudaAvailable;
extern float g_fDetailParam;
extern UINT g_unMovebyte;
//#define USE_IPP
DWORD g_nStartTime1, g_nDuration1, g_nStartTime2, g_nDuration2;
//#define OUTPUT_DURATION

CDragBarEnhance::CDragBarEnhance(void)
: flag(false)
{

	char strValue[25];
	int nStrSize = 25;
	GetPrivateProfileString("DragBarEnh","nGrayThreshold","55000",strValue,nStrSize,g_chModulePathName);	nGrayThreshold = atoi(strValue) /65535.0* MAX_GRAY;
	GetPrivateProfileString("DragBarEnh","fDifThreshold","0.0005",strValue,nStrSize,g_chModulePathName);	fDifThreshold = atof(strValue);
	GetPrivateProfileString("DragBarEnh","nRadius","2",strValue,nStrSize,g_chModulePathName);     nRadius=atoi(strValue);
	GetPrivateProfileString("DragBarEnh","fDetailDegree","5",strValue,nStrSize,g_chModulePathName);     fDetailDegree=atof(strValue);
    GetPrivateProfileString("DragBarEnh","fEdgeDegree","1",strValue,nStrSize,g_chModulePathName);     fEdgeDegree=atof(strValue);
}

CDragBarEnhance::~CDragBarEnhance(void)
{
}

BOOL CDragBarEnhance::process(int nID,ConstImageBuf original, ImageBuf processed,float* fBarPos,bool bIsDrag,bool bIsROI, RECT* mROIRect)
{
	int h = original.height;
	int w = original.width;

	const WORDTYPE *pOrg=original.ptr;
	WORDTYPE *pRes=processed.ptr;
	BOOL BackValue = FALSE;
	DetailPara depara = {g_wBackgroundThreshold,2,5,55000/65535.0*MAX_GRAY,0.0005,1};
   
    //g_fDetailParam=3;


#ifndef USE_IPP//�������IPP,���ж��Ƿ���CUDA,�������CUDA,����CPU
	switch(nID)
	{
	case 0:

		if(g_bCudaAvailable)
		{

#ifdef OUTPUT_DURATION
		    g_nStartTime1 = GetTickCount();
#endif
			BackValue=LigConProcCuda(pOrg, pRes, w, h, fBarPos,bIsDrag,bIsROI,mROIRect);
#ifdef OUTPUT_DURATION

			g_nDuration1 = GetTickCount()-g_nStartTime1;
			char sBuf1[64];
			sprintf_s(sBuf1,"(GPU)���ȶԱȶ�=%d(ms)",g_nDuration1);
			MessageBox(NULL,sBuf1,"��ʱ",MB_OK);
#endif
		}
		else
		{
#ifdef OUTPUT_DURATION
		    g_nStartTime1 = GetTickCount();
#endif

			BackValue=LigConProcCpu(pOrg, pRes, w, h, fBarPos,bIsDrag,bIsROI,mROIRect);
#ifdef OUTPUT_DURATION
			g_nDuration1 = GetTickCount()-g_nStartTime1;
			char sBuf1[64];
			sprintf_s(sBuf1,"(CPU)���ȶԱȶ�=%d(ms)",g_nDuration1);
			MessageBox(NULL,sBuf1,"��ʱ",MB_OK);
#endif

		}
		break;
	case 2:
#ifdef OUTPUT_DURATION
		g_nStartTime2 = GetTickCount();
#endif
		if(g_bCudaAvailable)
		{
#ifdef OUTPUT_DURATION
			g_nStartTime2 = GetTickCount();
#endif
			BackValue=DetailProcCuda(pOrg, pRes, w, h, fBarPos,bIsDrag,bIsROI,mROIRect,depara);
#ifdef OUTPUT_DURATION
			g_nDuration2 = GetTickCount()-g_nStartTime2;
			char sBuf2[64];
			sprintf_s(sBuf2,"(GPU)ϸ��=%d(ms)",g_nDuration2);
			MessageBox(NULL,sBuf2,"��ʱ",MB_OK);
#endif
		}
		else
		{
#ifdef OUTPUT_DURATION
			g_nStartTime2 = GetTickCount();
#endif
			BackValue=DetailProcCpu(pOrg, pRes, w, h, fBarPos,bIsDrag,bIsROI,mROIRect);
#ifdef OUTPUT_DURATION
			g_nDuration2 = GetTickCount()-g_nStartTime2;
			char sBuf2[64];
			sprintf_s(sBuf2,"(CPU)ϸ��=%d(ms)",g_nDuration2);
			MessageBox(NULL,sBuf2,"��ʱ",MB_OK);
#endif
		}

		break;
	default:
		break;
	}
#else   //else�����IPP
	switch(nID)
	{
	case 0:
#ifdef OUTPUT_DURATION
		g_nStartTime1 = GetTickCount();
#endif
		BackValue=LigConProcIpp(pOrg, pRes, w, h, fBarPos,bIsDrag,bIsROI,mROIRect);
#ifdef OUTPUT_DURATION
		g_nDuration1 = GetTickCount()-g_nStartTime1;
		char sBuf1[64];
		sprintf_s(sBuf1,"(IPP)���ȶԱȶ�=%d(ms)",g_nDuration1);
		MessageBox(NULL,sBuf1,"��ʱ",MB_OK);
#endif
		break;
	case 2:
#ifdef OUTPUT_DURATION
		g_nStartTime1 = GetTickCount();
#endif
		BackValue=DetailProcIpp(pOrg, pRes, w, h, fBarPos,bIsDrag,bIsROI,mROIRect);
#ifdef OUTPUT_DURATION
		g_nDuration1 = GetTickCount()-g_nStartTime1;
		sprintf_s(sBuf1,"(IPP)ϸ�ڶԱȶ�=%d(ms)",g_nDuration1);
		MessageBox(NULL,sBuf1,"��ʱ",MB_OK);
#endif
		break;
	default:
		break;
	}
#endif
	return BackValue;
}
int CDragBarEnhance:: Bisearch(WORDTYPE data[],int x,int begin,int last)
{
	
	//begin������last,�򷵻ؽ��
	if(begin>last)
	{
		return begin;
	}
	
	int mid=(begin+last)/2;
	//���x�������е�data[mid]��ֵ��ȣ��򷵻ؽ��
	if(x==data[mid])
	{
		return mid;
	}
	//��x<data[mid]��ʱ���������߲�������x�����õݹ鷽��ʵ��
	if(x<data[mid])
	{
		Bisearch(data,x,begin,mid-1);
	}
	//���x>data[mid]��ʱ��������ұ߲�������x,���õݹ鷽��ʵ��
	else	
	{
		Bisearch(data,x,mid+1,last);
	}
		
	
}

bool CDragBarEnhance::DefaultEnhance(ConstImageBuf original, ImageBuf processed,float * fBarPos,RECT* mROIRect)
{
	int i,j;
	const WORDTYPE *nOriginGray=original.ptr;
	WORDTYPE nMid,nMean,nMax=0,nMin=MAX_GRAY;
	double dTotal=0;
	int nNum=0;
	for(i=0;i<original.height;i++)
	{
		for(j=0;j<original.width;j++)
		{
			//if(i>mROIRect->top&& i<=mROIRect->bottom && j>=mROIRect->left && j<=mROIRect->right)
			//{
				WORDTYPE nTemp=(*nOriginGray)>>g_unMovebyte;
				if(nTemp>nMax)
					nMax=nTemp;
				if(nTemp<nMin)
					nMin=nTemp;
				dTotal+=nTemp;
				nNum++;
			//}
			nOriginGray++;
		}
	}
	
	nMean=dTotal/nNum;
	int nDalta1,nDalta2,nDalta3;
	float fFactor1,fFactor2;
	nDalta1=nMax-nMean;
	nDalta2=nMean-nMin;
	nDalta3=nDalta1-nDalta2;
	fFactor1=(float)nDalta1/nDalta2;
	fFactor2=1.0/fFactor1;

	if(nDalta3>0)//˵����ֵ����nMin������nMidҪС��nMean
	{
		nMid=nMean-nDalta2*0.3;
	}
	else//˵����ֵ����nMax������nMidҪ����nMean
	{
		if(nDalta1/nMean<0.01)
			nMid=nMean-nDalta1*0.01;
		else
			nMid=nMean+nDalta1*0.1;
	}
	//������ֵ����newlight��contrast************************
    WORDTYPE newlight;
	int nSign;
	if(nMid<MAX_GRAY/2)
	{
		nMax=1.05*nMid;
		newlight=2*nMid;
		nSign=1;
		fBarPos[1]=1;
	}
	else
	{
		nMax=0.5*MAX_GRAY+0.5*nMid;
		newlight=(MAX_GRAY-nMid)*2;
		fBarPos[1]=0.5;
		nSign=-1;
	}
	


	//�����̶ȱ������в��***************************
	int nStart,nStep,nTabLen=0;
#ifdef ISWORD
	nTabLen=512;
	nStart=50;
	nStep=10;
#else
	nTabLen=1000;
	nStart=800;
	nStep=160;
#endif
	WORDTYPE *pBarTab=new WORDTYPE[nTabLen];//�̶�ת����
	pBarTab[0] = nStart;
	for(i=1;i<nTabLen;i++)
	{
		if(pBarTab[i-1]+nStep>=MAX_GRAY)
		{
			pBarTab[i] = MAX_GRAY;
			break;
		}
		else
			pBarTab[i] = pBarTab[i-1]+nStep;
#ifdef ISWORD
		nStep++;
#else
		nStep+=2;
#endif
	}
	nTabLen = i;
	int pos;
	pos=Bisearch(pBarTab,newlight,0,nTabLen);
	fBarPos[0]=nSign*(1-(float)pos/nTabLen);
	process(0,original,processed,fBarPos, 0,0, mROIRect);
	delete []pBarTab;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//ʹ��CPU����ʵ���޼�����
bool CDragBarEnhance::LigConProcCpu(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect)
{
	float light=fBarPos[0],contrast=fBarPos[1];
	float k,nx,ny,db,nx1,ny1,pix;
	int i,nStart,nStep,nTabLen=0;

#ifdef ISWORD
	nTabLen=512;
	nStart=50;
	nStep=10;
#else
	nTabLen=1000;
	nStart=800;
	nStep=160;
#endif
	WORDTYPE *pTab=new WORDTYPE[MAX_GRAY+1];//�Ҷ�ת����
	WORDTYPE *pBarTab=new WORDTYPE[nTabLen];//�̶�ת����
	pBarTab[0] = nStart;
	for(i=1;i<nTabLen;i++)
	{
		if(pBarTab[i-1]+nStep>=MAX_GRAY)
		{
			pBarTab[i] = MAX_GRAY;
			break;
		}
		else
			pBarTab[i] = pBarTab[i-1]+nStep;
#ifdef ISWORD
		nStep++;
#else
		nStep+=2;
#endif
	}
	nTabLen = i;

	int newlight=pBarTab[(int)((1-fabs(light))*nTabLen)];


	ny=(float)MAX_GRAY/2;
	contrast *= 0.95;



	if(light>=0)
	{
		ny1=MAX_GRAY;
		nx=(float)newlight/2;

		if(contrast>=0)
		{
			
			nx1=nx*(1-contrast)+nx;
			ny1=MAX_GRAY;
		}
		else
		{
			ny1=-ny*contrast;
			nx1=0;
		}
	}
	else
	{
		nx=MAX_GRAY-(float)newlight/2;
		if(contrast>=0)
		{
			
			nx1=MAX_GRAY-contrast*newlight/2;
			ny1=MAX_GRAY;
		}
		else
		{
			ny1=MAX_GRAY-contrast*ny;
			nx1=MAX_GRAY;
		}
	}
	k=(ny-ny1)/(nx-nx1);
	db=ny-nx*k;

	for(i=0;i<=MAX_GRAY;i++)//������ǿ��ͨ��ƽ��ֱ��.�Աȶ���ǿ��ͨ������ֱ��б��
	{
		pix=i*k+db;  
		if(pix<0) pTab[i]=0;
		else if(pix>MAX_GRAY) pTab[i]=MAX_GRAY;
		else pTab[i]=(WORDTYPE)pix;
	}

	//���
	for(i=0; i<w*h; i++)
	{
		pRes[i] = pTab[pOrg[i]>>g_unMovebyte]<<g_unMovebyte;
	}
	delete []pTab;
	delete []pBarTab;
	return TRUE;
	
}

bool CDragBarEnhance::DetailProcCpu(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect)
{


	//����4�����ֵķ���

 
	int x,y,sx,ex,sy,ey,pos;
	float detail=fBarPos[0];

	double dTempSum, dTempDouble,dMean;
	double * pSum =  new double [h*w]; //��ͼ�����Ͻ���ÿһ�������ֵ�ۼӺ�
	memset(pSum,0,sizeof(double) *h*w);


	//�����Ե�ǰ��Ϊ���½ǵ������ۼӺ�
	pos = 0;
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			if(x>0 && y>0)
			{
				pSum[pos] = pSum[pos-w]+(pSum[pos-1]-pSum[pos-w-1])+(pOrg[pos]>>g_unMovebyte);
			}
			else if(x>0)//��һ��
			{
				pSum[pos] = pSum[pos-1]+(pOrg[pos]>>g_unMovebyte);
			}
			else if(y>0)//��һ��
			{
				pSum[pos] = pSum[pos-w]+(pOrg[pos]>>g_unMovebyte);
			}
			else//ԭ��
			{
				pSum[pos] = pOrg[pos]>>g_unMovebyte;
			}
			pos++;
		}
	}

	pos = 0;
	WORDTYPE OrgPos;
	for(y=0;y<h;y++)
	{
		sy = y-nRadius;
		if(sy<0) sy = 0;
		ey = y+nRadius;
		if(ey>h-1) ey = h-1;
		for(x=0;x<w;x++)
		{
			OrgPos=pOrg[pos]>>g_unMovebyte;
			if(OrgPos<g_wBackgroundThreshold>>g_unMovebyte)
			{
				sx = x-nRadius;
				if(sx<0) sx = 0;
				ex = x+nRadius;
				if(ex>w-1) ex = w-1;
				dTempSum = pSum[ey*w+ex];
				if(sy>0)
				{
					dTempSum -= pSum[(sy-1)*w+ex];
				}
				if(sx>0) 
				{
					dTempSum -= pSum[ey*w+sx-1];
				}
				if(sx>0&&sy>0) 
				{
					dTempSum += pSum[(sy-1)*w+sx-1];
				}
				dMean = dTempSum/(ey-sy+1)/(ex-sx+1);
				dTempDouble = OrgPos-dMean;
				if(detail>0)
				{
					float expIndex;
				     if(detail<0.5)
					{
						fDetailDegree=detail*10+5;
						fEdgeDegree=detail*4+1;
					}
					else if(detail<0.8)
					{
						fDetailDegree=detail*15+2.5;
						fEdgeDegree=detail*6;
					}
					else
					{
						fDetailDegree=detail*20-1.5;
						fEdgeDegree=detail*8-1.6;
					}	
				   
					if(OrgPos-dMean>=0)
						expIndex=3*detail*fDetailDegree;//��ϸ����ǿ�̶�
					else
						expIndex=4*detail*fDetailDegree;//��ϸ����ǿ�̶�
		
					float dif;
					if(OrgPos>nGrayThreshold)//�Ҷ���ֵ���Ҷȸ���һ������ֵ���򲻽���ϸ����ǿ��ֻ��΢���б�Ե��ǿ
					{
						dTempDouble=OrgPos+(OrgPos-dMean)*3; 
					}
					else
					{
						float curGray = (float)OrgPos/MAX_GRAY;
						dif	= curGray - (float)dMean/MAX_GRAY;
						if (fabs(dif)*curGray*curGray*curGray < fDifThreshold)  //0.002)  //0.0005)	
						{
							dif	*= 2;          //cos����x����ѹ��������Ĭ��ȡ1�����ǲ�����ѹ��
							dif = min(dif,1.0f);
							if (dif>=0)
							{	
								dTempDouble = OrgPos + expIndex* cos(PI/2*dif) * MAX_GRAY * 0.001 + (OrgPos-dMean)*4*fEdgeDegree*detail;                //����Ե��ǿ�̶�							
							}
							else
							{
								dTempDouble= OrgPos - expIndex * cos(PI/2*dif) * MAX_GRAY * 0.001 + (OrgPos-dMean)*8*fEdgeDegree*detail;                //����Ե��ǿ�̶�
							}
						}
						else
						{
							dif	*= 2;          //cos����x����ѹ��������Ĭ��ȡ1�����ǲ�����ѹ��
							dif = min(dif,1.0f);
							if (dif>=0)
							{
								dTempDouble =OrgPos +  expIndex * cos(PI/2*dif) * MAX_GRAY * 0.01 + (OrgPos-dMean)*4*fEdgeDegree*detail; 							
							}
							else
							{
								dTempDouble= OrgPos - expIndex* cos(PI/2*dif) * MAX_GRAY * 0.01 + (OrgPos-dMean)*8*fEdgeDegree*detail; 	
							}
						}						
					}
				}
				else
				{
					dTempDouble=OrgPos+dTempDouble*detail;
				}
			}
			else 
			{
				dTempDouble=OrgPos;

			}
			if(dTempDouble<0) dTempDouble = 0;
			if(dTempDouble>MAX_GRAY) dTempDouble = MAX_GRAY;
			pRes[pos] = ((WORDTYPE)dTempDouble)<<g_unMovebyte;
			pos++;
		}
	}
	delete [] pSum;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// ʹ��IPP��ʵ���޼�����
BOOL CDragBarEnhance::LigConProcIpp(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect)
{
	ippStaticInit();
	float light=fBarPos[0],contrast=fBarPos[1];
	float k,nx,ny,db,nx1,ny1;
	int i, nStep,nStart,nTabLen=0;
#ifdef ISWORD
	nTabLen=512;
	nStart=50;
	nStep=10;
#else
	nTabLen=1000;
	nStart=800;
	nStep=160;
#endif
	WORDTYPE *pTab=new WORDTYPE[MAX_GRAY+1];//�Ҷ�ת����
	WORDTYPE *pBarTab=new WORDTYPE[nTabLen];//�̶�ת����
	pBarTab[0] = nStart;
	for(i=1;i<nTabLen;i++)
	{
		if(pBarTab[i-1]+nStep>=MAX_GRAY)
		{
			pBarTab[i] = MAX_GRAY;
			break;
		}
		else
			pBarTab[i] = pBarTab[i-1]+nStep;
#ifdef ISWORD
		nStep++;
#else
		nStep+=2;
#endif
	}
	nTabLen = i;
	

	int newlight=pBarTab[(int)((1-fabs(light))*nTabLen)];
	ny=(double)MAX_GRAY/2;
	contrast *= 0.95;



		if(light>=0)
	{
		ny1=MAX_GRAY;
		nx=(float)newlight/2;

		if(contrast>=0)
		{
			
			nx1=nx*(1-contrast)+nx;
			ny1=MAX_GRAY;
		}
		else
		{
			ny1=-ny*contrast;
			nx1=0;
		}
	}
	else
	{
		nx=MAX_GRAY-(float)newlight/2;
		if(contrast>=0)
		{
			
			nx1=MAX_GRAY-contrast*newlight/2;
			ny1=MAX_GRAY;
		}
		else
		{
			ny1=MAX_GRAY-contrast*ny;
			nx1=MAX_GRAY;
		}
	}
	k=(ny-ny1)/(nx-nx1);
	db=ny-nx*k;
	Ipp32f* orgimagef1=new Ipp32f[h*w];
	IppiSize bigroi={w,h};
#ifdef ISWORD
	ippiConvert_16u32f_C1R(pOrg,sizeof(WORDTYPE)*w,orgimagef1,sizeof(Ipp32f)*w,bigroi);
#else
	Ipp32u* org=(Ipp32u*)pOrg;
	ippiConvert_32u32f_C1R(org,sizeof(WORDTYPE)*w,orgimagef1,sizeof(Ipp32f)*w,bigroi);
	ippiDivC_32f_C1IR(4096,orgimagef1,sizeof(Ipp32f)*w,bigroi);//����12λ�Ĺ���
#endif
	ippiMulC_32f_C1IR(k,orgimagef1,sizeof(Ipp32f)*w,bigroi);
	ippiAddC_32f_C1IR(db,orgimagef1,sizeof(Ipp32f)*w,bigroi);
#ifdef ISWORD
	ippiConvert_32f16u_C1R(orgimagef1,sizeof(Ipp32f)*w,pRes,sizeof(WORDTYPE)*w,bigroi,ippRndZero);
#else
	Ipp32u* res=(Ipp32u*)pRes;
	ippiThreshold_LTVal_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,bigroi,0,0);//С��0�Ķ���Ϊ0
	ippiThreshold_GTVal_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,bigroi,MAX_GRAY,MAX_GRAY);//����MAX_GRAY�Ķ���ΪMAX_GRAY
	ippiMulC_32f_C1IR(4096,orgimagef1,sizeof(WORDTYPE)*w,bigroi); //����12λ����
	ippiConvert_32f32u_C1RSfs(orgimagef1,sizeof(Ipp32f)*w,res,sizeof(WORDTYPE)*w,bigroi,ippRndZero,0);

#endif
	delete[] orgimagef1;
	delete[] pBarTab;
	return TRUE;

}
BOOL CDragBarEnhance::DetailProcIpp(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect)
{
	ippStaticInit();
	int r=nRadius;
	float expIndex;
	float detail=fBarPos[0];
	Ipp32f* orgimage=new Ipp32f[(h+2*r)*(w+2*r)];
	Ipp32f* orgimagef1=new Ipp32f[h*w];
	Ipp32f* imagem=new Ipp32f[h*w];
	Ipp32f* imgdiff=new Ipp32f[h*w];	
	IppiSize bigroi={w,h};
	IppiSize dstroi={w+2*r,h+2*r};
	IppiSize mask={2*r+1,2*r+1};
	IppiPoint anchor={r,r};

	if(detail<0.5)
	{
		fDetailDegree=detail*10+5;
		fEdgeDegree=detail*4+1;
	}
	else if(detail<0.8)
	{
		fDetailDegree=detail*15+2.5;
		fEdgeDegree=detail*6;
	}
	else
	{
		fDetailDegree=detail*20-1.5;
		fEdgeDegree=detail*8-1.6;
	}	

#ifdef ISWORD
	ippiConvert_16u32f_C1R(pOrg,sizeof(WORD)*w,orgimagef1,sizeof(Ipp32f)*w,bigroi);//orgimagef1���32fԭʼͼ��
#else
	Ipp32u* org=(Ipp32u*)pOrg;
	ippiConvert_32u32f_C1R(org,sizeof(WORDTYPE)*w,orgimagef1,sizeof(Ipp32f)*w,bigroi);//orgimagef1���32fԭʼͼ��
	ippiDivC_32f_C1IR(4096,orgimagef1,sizeof(Ipp32f)*w,bigroi);//����12λ�Ĺ���

#endif
	ippiCopyReplicateBorder_32f_C1R(orgimagef1,sizeof(Ipp32f)*w,bigroi,orgimage,sizeof(Ipp32f)*(w+2*r),dstroi,r,r);//orgimage��żӹ��ߵ�ͼ��
	ippiFilterBox_32f_C1R(orgimage+r*(w+2*r)+r,sizeof(Ipp32f)*(w+2*r),imagem,sizeof(Ipp32f)*w,bigroi,mask,anchor);//imagem��������ֵdMean
	ippiSub_32f_C1R(imagem,sizeof(Ipp32f)*w,orgimagef1,sizeof(Ipp32f)*w,imgdiff,sizeof(Ipp32f)*w,bigroi);//imgdiff=pOrg[pos]-dMean

	if(detail>0)
	{

		Ipp32f* pBackground=new Ipp32f[h*w];
		Ipp32f* pHighGrayregion=new Ipp32f[h*w];
		Ipp32f* pLowGrayregion=new Ipp32f[h*w];
		Ipp32f* pLowGrayregionlowdif=new Ipp32f[h*w];
		Ipp32f* pLowGrayregionhighdif=new Ipp32f[h*w];
		Ipp32f* pAbsdiff=new Ipp32f[h*w];
		Ipp32f* pLowdiffpo=new Ipp32f[h*w];
		Ipp32f* pLowdiffne=new Ipp32f[h*w];
		Ipp32f* pHighdiffpo=new Ipp32f[h*w];
		Ipp32f* pHighdiffne=new Ipp32f[h*w];
		Ipp32f* imgone=new Ipp32f[h*w];
		Ipp32f* rescos=new Ipp32f[h*w];
		Ipp32f* imgdiff1=new Ipp32f[h*w];
		Ipp32f* temp1=new Ipp32f[h*w];
		Ipp32f* temp2=new Ipp32f[h*w];

	

		ippiSet_32f_C1R(1.0f,imgone,sizeof(Ipp32f)*w,bigroi);
		ippiCopy_32f_C1R(imgdiff,sizeof(Ipp32f)*w,imgdiff1,sizeof(Ipp32f)*w,bigroi);//imgdiff1=pOrg[pos]-dMean

			//����ģ��+����
		ippiThreshold_LTVal_32f_C1R(orgimagef1,sizeof(Ipp32f)*w,pBackground,sizeof(Ipp32f)*w,bigroi,g_wBackgroundThreshold,0);//С�ڱ�����ֵ�Ķ���Ϊ0
		ippiThreshold_GTVal_32f_C1IR(pBackground,sizeof(Ipp32f)*w,bigroi,0,1);//���ڵ��ڱ�����ֵ�Ķ���Ϊ1

	    ippiMul_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,pBackground,sizeof(Ipp32f)*w,bigroi);//С�ڱ�����ֵ�Ķ�Ϊ0�����ڵ��ڱ�����ֵ�Ķ�Ϊ�Ҷ�ֵ

		//*****************************************************************************************************************************************

		//�߻Ҷ���ģ��+����
		ippiThreshold_LTValGTVal_32f_C1R(orgimagef1,sizeof(Ipp32f)*w,pHighGrayregion,sizeof(Ipp32f)*w,bigroi,nGrayThreshold+1,-1,g_wBackgroundThreshold-1,-1);//ǰ������ֵС��nGrayThreshold+1��Ϊ0��������Ϊ0
		ippiThreshold_GTVal_32f_C1IR(pHighGrayregion,sizeof(Ipp32f)*w,bigroi,-1,1);
		ippiThreshold_LTVal_32f_C1IR(pHighGrayregion,sizeof(Ipp32f)*w,bigroi,1,0);
		//dTempDouble=pOrg[pos]+(pOrg[pos]-dMean)*3;
		ippiMulC_32f_C1R(imgdiff,sizeof(Ipp32f)*w,3.0,temp1,sizeof(Ipp32f)*w,bigroi);
		ippiAdd_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);

		ippiMul_32f_C1IR(temp1,sizeof(Ipp32f)*w,pHighGrayregion,sizeof(Ipp32f)*w,bigroi);//pHighGrayregion�д��0��ֵ����Ч�ģ�������Ϊ�����ĻҶ�ֵ
		//************************************************************************************************************************************************

		//�ͻҶ���ģ��
		ippiThreshold_GTVal_32f_C1R(orgimagef1,sizeof(Ipp32f)*w,pLowGrayregion,sizeof(Ipp32f)*w,bigroi,nGrayThreshold,-1);//ǰ������С��nGrayThreshold��Ϊ1�������Ķ�Ϊ0��
		ippiThreshold_GTVal_32f_C1IR(pLowGrayregion,sizeof(Ipp32f)*w,bigroi,-1,1);
		ippiThreshold_LTVal_32f_C1IR(pLowGrayregion,sizeof(Ipp32f)*w,bigroi,1,0);

		
		ippiDivC_32f_C1IR(MAX_GRAY,imgdiff,sizeof(Ipp32f)*w,bigroi);//imgdiff=(pOrg[pos]-dMean)/MAX_GRAY
		ippiAbs_32f_C1R(imgdiff,sizeof(Ipp32f)*w,pAbsdiff,sizeof(Ipp32f)*w,bigroi);//pAbsdiff=abs(imgdiff)
		ippiMul_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,pAbsdiff,sizeof(Ipp32f)*w,bigroi);//pAbsdiff=pAbsdiff*(pOrg)^3
		ippiMul_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,pAbsdiff,sizeof(Ipp32f)*w,bigroi);
		ippiMul_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,pAbsdiff,sizeof(Ipp32f)*w,bigroi);
		float fdifT=fDifThreshold*pow((float)MAX_GRAY,3.0f);
        //С����ģ��
		ippiThreshold_GTVal_32f_C1R(pAbsdiff,sizeof(Ipp32f)*w,pLowGrayregionlowdif,sizeof(Ipp32f)*w,bigroi,fdifT-1,-1);//pLowGrayregionlowdif=(pAbsdiff>fdifT-1��Ϊ-1)
		ippiThreshold_GTVal_32f_C1IR(pLowGrayregionlowdif,sizeof(Ipp32f)*w,bigroi,-1,1);//pLowGrayregionlowdif=(pAbsdiff<fdifT��Ϊ1)
		ippiThreshold_LTVal_32f_C1IR(pLowGrayregionlowdif,sizeof(Ipp32f)*w,bigroi,1,0);//��-1���滻Ϊ0
		ippiMul_32f_C1IR(pLowGrayregion,sizeof(Ipp32f)*w,pLowGrayregionlowdif,sizeof(Ipp32f)*w,bigroi);//AND(plowdif,pLowregion)//�ͻҶ�����С�������Ϊ1��������Ϊ0��
		//�����ģ��
	 	ippiThreshold_LTVal_32f_C1R(pAbsdiff,sizeof(Ipp32f)*w,pLowGrayregionhighdif,sizeof(Ipp32f)*w,bigroi,fdifT,-1);//pLowGrayregionhighdif=(pAbsdiff<fdifT��Ϊ-1)
		ippiThreshold_GTVal_32f_C1IR(pLowGrayregionhighdif,sizeof(Ipp32f)*w,bigroi,-1,1);//pLowGrayregionhighdif=(pLowGrayregionhighdif>-1��Ϊ1)
		ippiThreshold_LTVal_32f_C1IR(pLowGrayregionhighdif,sizeof(Ipp32f)*w,bigroi,1,0);//��-1���滻Ϊ0
		ippiMul_32f_C1IR(pLowGrayregion,sizeof(Ipp32f)*w,pLowGrayregionhighdif,sizeof(Ipp32f)*w,bigroi);//AND(phighdif,pLowregion)//�ͻҶ����ڴ�������Ϊ1��������Ϊ0��
		
		//dif=dif*2;
		ippiMulC_32f_C1IR(2.0,imgdiff,sizeof(Ipp32f)*w,bigroi);
		//dif=min(dif,1.of);
		ippiMinEvery_32f_C1IR(imgone,sizeof(Ipp32f)*w,imgdiff,sizeof(Ipp32f)*w,bigroi);//dif=min(dif,1.0f);
		//dif>=0��dif<0ģ��
		ippiThreshold_LTVal_32f_C1R(imgdiff,sizeof(Ipp32f)*w,pLowdiffpo,sizeof(Ipp32f)*w,bigroi,0,-1);//pLowDiffpo=(imgdiff<0��Ϊ-1)
		ippiThreshold_GTVal_32f_C1IR(pLowdiffpo,sizeof(Ipp32f)*w,bigroi,-1,1);//pLowDiffpo=(imgdiff>=0��Ϊ1)
		ippiMulC_32f_C1R(pLowdiffpo,sizeof(Ipp32f)*w,-1,pLowdiffne,sizeof(Ipp32f)*w,bigroi);//pLowDiffne=(imgdiff<0��Ϊ1������Ϊ-1)
		ippiThreshold_LTVal_32f_C1IR(pLowdiffpo,sizeof(Ipp32f)*w,bigroi,1,0);//pLowDiffpo=(imgdiff<0��Ϊ0)
		ippiThreshold_LTVal_32f_C1IR(pLowdiffne,sizeof(Ipp32f)*w,bigroi,1,0);//pLowDiffne=(imgdiff>=0��Ϊ0)
		//**************************************************************************************************************************************************************
		
		//�����dif>=0ģ��
		//pHighDiffpo=pLowdiffpo(and)pLowGrayregionhighdif
		ippiMul_32f_C1R(pLowGrayregionhighdif,sizeof(Ipp32f)*w,pLowdiffpo,sizeof(Ipp32f)*w,pHighdiffpo,sizeof(Ipp32f)*w,bigroi);
		//�����dif<0ģ��
		//pHighDiffne=pLowdiffne(and)pLowGrayregionhighdif
		ippiMul_32f_C1R(pLowGrayregionhighdif,sizeof(Ipp32f)*w,pLowdiffne,sizeof(Ipp32f)*w,pHighdiffne,sizeof(Ipp32f)*w,bigroi);


		//С����dif>=0ģ��
		//pLowdiffpo=pLowdiffpo(and)pLowGrayregionlowdif
		ippiMul_32f_C1IR(pLowGrayregionlowdif,sizeof(Ipp32f)*w,pLowdiffpo,sizeof(Ipp32f)*w,bigroi);
		//С����dif<0ģ��
		//pLowdiffne=pLowdiffne(and)pLowGrayregionlowdif
		ippiMul_32f_C1IR(pLowGrayregionlowdif,sizeof(Ipp32f)*w,pLowdiffne,sizeof(Ipp32f)*w,bigroi);

		//**************************************************************************************************************************************************
		//�ͻҶ�������
		//����������
		//rescos=cos(PI/2*dif)
		ippiMulC_32f_C1IR(PI/2,imgdiff,sizeof(Ipp32f)*w,bigroi);//imgdiff=PI*imgdiff;
		for(int i=0;i<h*w;i++)
		{
			rescos[i]=cos(imgdiff[i]);
		}
		//imgdiff1=(pOrg[pos]-dMean)*fEdgeDegree*detail;
		ippiMulC_32f_C1IR(detail*fEdgeDegree,imgdiff1,sizeof(Ipp32f)*w,bigroi);//imgdiff1*=detail*fEdgeDegree;


		//С����dif>=0����
		
		expIndex=3*detail*fDetailDegree*MAX_GRAY*0.001;
		ippiMulC_32f_C1R(rescos,sizeof(Ipp32f)*w,expIndex,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=cos(pi/2*dif)*expIndex*MAX_GRAY*0.001;
		ippiAdd_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.001
		ippiMulC_32f_C1R(imgdiff1,sizeof(Ipp32f)*w,4,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=(pOrg[pos]-dMean)*fEdgeDegree*detail*4;
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.001+(pOrg[pos]-dMean)*fEdgeDegree*detail*4;

		ippiMul_32f_C1IR(temp1,sizeof(Ipp32f)*w,pLowdiffpo,sizeof(Ipp32f)*w,bigroi);//����С����dif>=0ģ�壬pLowdiffpo�����0�������Ҷ�ֵ


		//С����dif<0����
		expIndex=4*detail*fDetailDegree*MAX_GRAY*0.001;
		ippiMulC_32f_C1R(rescos,sizeof(Ipp32f)*w,expIndex,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=cos(pi/2*dif)*expIndex*MAX_GRAY*0.001;
		ippiSub_32f_C1R(temp2,sizeof(Ipp32f)*w,orgimagef1,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=org-temp2;
		ippiMulC_32f_C1R(imgdiff1,sizeof(Ipp32f)*w,8,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=(pOrg[pos]-dMean)*fEdgeDegree*detail*8;
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.001+(pOrg[pos]-dMean)*fEdgeDegree*detail*4;

		ippiMul_32f_C1IR(temp1,sizeof(Ipp32f)*w,pLowdiffne,sizeof(Ipp32f)*w,bigroi);//����С����dif<0ģ�壬pLowdiffne�����0�������Ҷ�ֵ

		//�����dif>=0����
		expIndex=3*detail*fDetailDegree*MAX_GRAY*0.01;
		ippiMulC_32f_C1R(rescos,sizeof(Ipp32f)*w,expIndex,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=cos(pi/2*dif)*expIndex*MAX_GRAY*0.01;
		ippiAdd_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.01
		ippiMulC_32f_C1R(imgdiff1,sizeof(Ipp32f)*w,4,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=(pOrg[pos]-dMean)*fEdgeDegree*detail*4;
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.01+(pOrg[pos]-dMean)*fEdgeDegree*detail*4;

		ippiMul_32f_C1IR(temp1,sizeof(Ipp32f)*w,pHighdiffpo,sizeof(Ipp32f)*w,bigroi);//���Դ����dif>=0ģ�壬pHighdiffpo�����0�������Ҷ�ֵ


		//�����dif<0����
		expIndex=4*detail*fDetailDegree*MAX_GRAY*0.01;
		ippiMulC_32f_C1R(rescos,sizeof(Ipp32f)*w,expIndex,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=cos(pi/2*dif)*expIndex*MAX_GRAY*0.01;
		ippiSub_32f_C1R(temp2,sizeof(Ipp32f)*w,orgimagef1,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=org-temp2;
		ippiMulC_32f_C1R(imgdiff1,sizeof(Ipp32f)*w,8,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=(pOrg[pos]-dMean)*fEdgeDegree*detail*8;
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.01+(pOrg[pos]-dMean)*fEdgeDegree*detail*4;

		ippiMul_32f_C1IR(temp1,sizeof(Ipp32f)*w,pHighdiffne,sizeof(Ipp32f)*w,bigroi);//���Դ����dif<0ģ�壬pHighdiffne�����0�������Ҷ�ֵ


		//�ϳ�
		ippiAdd_32f_C1R(pHighdiffne,sizeof(Ipp32f)*w,pHighdiffpo,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=pHighdiffpo+pHighdiffne���ͻҶȴ���촦��
		ippiAdd_32f_C1R(pLowdiffne,sizeof(Ipp32f)*w,pLowdiffpo,sizeof(Ipp32f)*w,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=pLowdiffpo+pLowdiffne���ͻҶ�С���촦��
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2���ͻҶ�������
		ippiAdd_32f_C1R(pHighGrayregion,sizeof(Ipp32f)*w,pBackground,sizeof(Ipp32f)*w,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=pBackground+pHighGrayregion���߻Ҷȴ���+����
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2�������Ҷ�������
#ifdef ISWORD
		ippiConvert_32f16u_C1R(temp1, sizeof(Ipp32f)*w, pRes, sizeof(WORDTYPE)*w, bigroi, ippRndZero );
#else
		Ipp32u* res=(Ipp32u*)pRes;
		ippiMulC_32f_C1IR(4096,temp1,sizeof(Ipp32f)*w,bigroi); //����12λ����
    	ippiConvert_32f32u_C1RSfs(temp1,sizeof(Ipp32f)*w,res,sizeof(WORDTYPE)*w,bigroi,ippRndZero,0);
#endif
		delete[] pBackground;
		delete[] pHighGrayregion;
		delete[] pLowGrayregion;
		delete[] pLowGrayregionlowdif;
		delete[] pLowGrayregionhighdif;
		delete[] pAbsdiff;		
		delete[] pLowdiffpo;		
		delete[] pLowdiffne;
		delete[] pHighdiffpo;
		delete[] pHighdiffne;
		delete[] imgone;
		delete[] rescos;
		delete[] imgdiff1;
		delete[] temp1;
		delete[] temp2;

	}
	else
	{
		ippiMulC_32f_C1IR(detail,imgdiff,sizeof(Ipp32f)*w,bigroi);
		ippiAdd_32f_C1IR(imgdiff,sizeof(Ipp32f)*w,orgimagef1,sizeof(Ipp32f)*w,bigroi);
#ifdef ISWORD
		ippiConvert_32f16u_C1R(orgimagef1, sizeof(Ipp32f)*w, pRes, sizeof(WORDTYPE)*w, bigroi, ippRndZero );
#else
		Ipp32u* res=(Ipp32u*)pRes;
		ippiMulC_32f_C1IR(4096,orgimagef1,sizeof(WORDTYPE)*w,bigroi); //����12λ����
		ippiConvert_32f32u_C1RSfs(orgimagef1,sizeof(Ipp32f)*w,res,sizeof(WORDTYPE)*w,bigroi,ippRndZero,0);
#endif
		
	}
	delete[] orgimage;
	delete[] orgimagef1;
	delete[] imagem;
	delete[] imgdiff;
	return TRUE;
}