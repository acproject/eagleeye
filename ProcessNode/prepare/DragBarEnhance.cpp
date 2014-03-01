#include "stdafx.h"

#include "DragBarEnhance.h"
#include "DragBarEnhanceCu.h"

#include "SpecEnh/libs/ippi.h"
#include "SpecEnh/libs/ippcore.h"

//定义全局变量
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


#ifndef USE_IPP//如果不用IPP,则判断是否用CUDA,如果不用CUDA,则用CPU
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
			sprintf_s(sBuf1,"(GPU)亮度对比度=%d(ms)",g_nDuration1);
			MessageBox(NULL,sBuf1,"计时",MB_OK);
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
			sprintf_s(sBuf1,"(CPU)亮度对比度=%d(ms)",g_nDuration1);
			MessageBox(NULL,sBuf1,"计时",MB_OK);
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
			sprintf_s(sBuf2,"(GPU)细节=%d(ms)",g_nDuration2);
			MessageBox(NULL,sBuf2,"计时",MB_OK);
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
			sprintf_s(sBuf2,"(CPU)细节=%d(ms)",g_nDuration2);
			MessageBox(NULL,sBuf2,"计时",MB_OK);
#endif
		}

		break;
	default:
		break;
	}
#else   //else如果用IPP
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
		sprintf_s(sBuf1,"(IPP)亮度对比度=%d(ms)",g_nDuration1);
		MessageBox(NULL,sBuf1,"计时",MB_OK);
#endif
		break;
	case 2:
#ifdef OUTPUT_DURATION
		g_nStartTime1 = GetTickCount();
#endif
		BackValue=DetailProcIpp(pOrg, pRes, w, h, fBarPos,bIsDrag,bIsROI,mROIRect);
#ifdef OUTPUT_DURATION
		g_nDuration1 = GetTickCount()-g_nStartTime1;
		sprintf_s(sBuf1,"(IPP)细节对比度=%d(ms)",g_nDuration1);
		MessageBox(NULL,sBuf1,"计时",MB_OK);
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
	
	//begin超过了last,则返回结果
	if(begin>last)
	{
		return begin;
	}
	
	int mid=(begin+last)/2;
	//如果x与数组中的data[mid]的值相等，则返回结果
	if(x==data[mid])
	{
		return mid;
	}
	//当x<data[mid]的时候，则进行左边查找数据x，采用递归方法实现
	if(x<data[mid])
	{
		Bisearch(data,x,begin,mid-1);
	}
	//如果x>data[mid]的时候，则进行右边查找数据x,采用递归方法实现
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

	if(nDalta3>0)//说明均值靠近nMin，所以nMid要小于nMean
	{
		nMid=nMean-nDalta2*0.3;
	}
	else//说明均值靠近nMax，所以nMid要大于nMean
	{
		if(nDalta1/nMean<0.01)
			nMid=nMean-nDalta1*0.01;
		else
			nMid=nMean+nDalta1*0.1;
	}
	//根据数值计算newlight和contrast************************
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
	


	//建立刻度表，并进行查表***************************
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
	WORDTYPE *pBarTab=new WORDTYPE[nTabLen];//刻度转换表
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
//使用CPU计算实现无极调整
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
	WORDTYPE *pTab=new WORDTYPE[MAX_GRAY+1];//灰度转换表
	WORDTYPE *pBarTab=new WORDTYPE[nTabLen];//刻度转换表
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

	for(i=0;i<=MAX_GRAY;i++)//亮度增强，通过平移直线.对比度增强，通过调节直线斜率
	{
		pix=i*k+db;  
		if(pix<0) pTab[i]=0;
		else if(pix>MAX_GRAY) pTab[i]=MAX_GRAY;
		else pTab[i]=(WORDTYPE)pix;
	}

	//查表
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


	//方法4：积分的方法

 
	int x,y,sx,ex,sy,ey,pos;
	float detail=fBarPos[0];

	double dTempSum, dTempDouble,dMean;
	double * pSum =  new double [h*w]; //从图像左上角至每一点的像素值累加和
	memset(pSum,0,sizeof(double) *h*w);


	//计算以当前点为右下角的区域累加和
	pos = 0;
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			if(x>0 && y>0)
			{
				pSum[pos] = pSum[pos-w]+(pSum[pos-1]-pSum[pos-w-1])+(pOrg[pos]>>g_unMovebyte);
			}
			else if(x>0)//第一行
			{
				pSum[pos] = pSum[pos-1]+(pOrg[pos]>>g_unMovebyte);
			}
			else if(y>0)//第一列
			{
				pSum[pos] = pSum[pos-w]+(pOrg[pos]>>g_unMovebyte);
			}
			else//原点
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
						expIndex=3*detail*fDetailDegree;//亮细节增强程度
					else
						expIndex=4*detail*fDetailDegree;//暗细节增强程度
		
					float dif;
					if(OrgPos>nGrayThreshold)//灰度阈值，灰度高于一定的阈值，则不进行细节增强，只稍微进行边缘增强
					{
						dTempDouble=OrgPos+(OrgPos-dMean)*3; 
					}
					else
					{
						float curGray = (float)OrgPos/MAX_GRAY;
						dif	= curGray - (float)dMean/MAX_GRAY;
						if (fabs(dif)*curGray*curGray*curGray < fDifThreshold)  //0.002)  //0.0005)	
						{
							dif	*= 2;          //cos曲线x方向压缩比例，默认取1，就是不进行压缩
							dif = min(dif,1.0f);
							if (dif>=0)
							{	
								dTempDouble = OrgPos + expIndex* cos(PI/2*dif) * MAX_GRAY * 0.001 + (OrgPos-dMean)*4*fEdgeDegree*detail;                //亮边缘增强程度							
							}
							else
							{
								dTempDouble= OrgPos - expIndex * cos(PI/2*dif) * MAX_GRAY * 0.001 + (OrgPos-dMean)*8*fEdgeDegree*detail;                //暗边缘增强程度
							}
						}
						else
						{
							dif	*= 2;          //cos曲线x方向压缩比例，默认取1，就是不进行压缩
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
// 使用IPP库实现无极调整
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
	WORDTYPE *pTab=new WORDTYPE[MAX_GRAY+1];//灰度转换表
	WORDTYPE *pBarTab=new WORDTYPE[nTabLen];//刻度转换表
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
	ippiDivC_32f_C1IR(4096,orgimagef1,sizeof(Ipp32f)*w,bigroi);//右移12位的功能
#endif
	ippiMulC_32f_C1IR(k,orgimagef1,sizeof(Ipp32f)*w,bigroi);
	ippiAddC_32f_C1IR(db,orgimagef1,sizeof(Ipp32f)*w,bigroi);
#ifdef ISWORD
	ippiConvert_32f16u_C1R(orgimagef1,sizeof(Ipp32f)*w,pRes,sizeof(WORDTYPE)*w,bigroi,ippRndZero);
#else
	Ipp32u* res=(Ipp32u*)pRes;
	ippiThreshold_LTVal_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,bigroi,0,0);//小于0的都置为0
	ippiThreshold_GTVal_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,bigroi,MAX_GRAY,MAX_GRAY);//大于MAX_GRAY的都置为MAX_GRAY
	ippiMulC_32f_C1IR(4096,orgimagef1,sizeof(WORDTYPE)*w,bigroi); //左移12位功能
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
	ippiConvert_16u32f_C1R(pOrg,sizeof(WORD)*w,orgimagef1,sizeof(Ipp32f)*w,bigroi);//orgimagef1存放32f原始图像
#else
	Ipp32u* org=(Ipp32u*)pOrg;
	ippiConvert_32u32f_C1R(org,sizeof(WORDTYPE)*w,orgimagef1,sizeof(Ipp32f)*w,bigroi);//orgimagef1存放32f原始图像
	ippiDivC_32f_C1IR(4096,orgimagef1,sizeof(Ipp32f)*w,bigroi);//右移12位的功能

#endif
	ippiCopyReplicateBorder_32f_C1R(orgimagef1,sizeof(Ipp32f)*w,bigroi,orgimage,sizeof(Ipp32f)*(w+2*r),dstroi,r,r);//orgimage存放加过边的图像
	ippiFilterBox_32f_C1R(orgimage+r*(w+2*r)+r,sizeof(Ipp32f)*(w+2*r),imagem,sizeof(Ipp32f)*w,bigroi,mask,anchor);//imagem存放邻域均值dMean
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

			//背景模板+处理
		ippiThreshold_LTVal_32f_C1R(orgimagef1,sizeof(Ipp32f)*w,pBackground,sizeof(Ipp32f)*w,bigroi,g_wBackgroundThreshold,0);//小于背景阈值的都置为0
		ippiThreshold_GTVal_32f_C1IR(pBackground,sizeof(Ipp32f)*w,bigroi,0,1);//大于等于背景阈值的都置为1

	    ippiMul_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,pBackground,sizeof(Ipp32f)*w,bigroi);//小于背景阈值的都为0，大于等于背景阈值的都为灰度值

		//*****************************************************************************************************************************************

		//高灰度区模板+处理
		ippiThreshold_LTValGTVal_32f_C1R(orgimagef1,sizeof(Ipp32f)*w,pHighGrayregion,sizeof(Ipp32f)*w,bigroi,nGrayThreshold+1,-1,g_wBackgroundThreshold-1,-1);//前景像素值小于nGrayThreshold+1均为0，背景均为0
		ippiThreshold_GTVal_32f_C1IR(pHighGrayregion,sizeof(Ipp32f)*w,bigroi,-1,1);
		ippiThreshold_LTVal_32f_C1IR(pHighGrayregion,sizeof(Ipp32f)*w,bigroi,1,0);
		//dTempDouble=pOrg[pos]+(pOrg[pos]-dMean)*3;
		ippiMulC_32f_C1R(imgdiff,sizeof(Ipp32f)*w,3.0,temp1,sizeof(Ipp32f)*w,bigroi);
		ippiAdd_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);

		ippiMul_32f_C1IR(temp1,sizeof(Ipp32f)*w,pHighGrayregion,sizeof(Ipp32f)*w,bigroi);//pHighGrayregion中存的0的值是无效的，其他的为处理后的灰度值
		//************************************************************************************************************************************************

		//低灰度区模板
		ippiThreshold_GTVal_32f_C1R(orgimagef1,sizeof(Ipp32f)*w,pLowGrayregion,sizeof(Ipp32f)*w,bigroi,nGrayThreshold,-1);//前景像素小于nGrayThreshold的为1，其他的都为0；
		ippiThreshold_GTVal_32f_C1IR(pLowGrayregion,sizeof(Ipp32f)*w,bigroi,-1,1);
		ippiThreshold_LTVal_32f_C1IR(pLowGrayregion,sizeof(Ipp32f)*w,bigroi,1,0);

		
		ippiDivC_32f_C1IR(MAX_GRAY,imgdiff,sizeof(Ipp32f)*w,bigroi);//imgdiff=(pOrg[pos]-dMean)/MAX_GRAY
		ippiAbs_32f_C1R(imgdiff,sizeof(Ipp32f)*w,pAbsdiff,sizeof(Ipp32f)*w,bigroi);//pAbsdiff=abs(imgdiff)
		ippiMul_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,pAbsdiff,sizeof(Ipp32f)*w,bigroi);//pAbsdiff=pAbsdiff*(pOrg)^3
		ippiMul_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,pAbsdiff,sizeof(Ipp32f)*w,bigroi);
		ippiMul_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,pAbsdiff,sizeof(Ipp32f)*w,bigroi);
		float fdifT=fDifThreshold*pow((float)MAX_GRAY,3.0f);
        //小差异模板
		ippiThreshold_GTVal_32f_C1R(pAbsdiff,sizeof(Ipp32f)*w,pLowGrayregionlowdif,sizeof(Ipp32f)*w,bigroi,fdifT-1,-1);//pLowGrayregionlowdif=(pAbsdiff>fdifT-1均为-1)
		ippiThreshold_GTVal_32f_C1IR(pLowGrayregionlowdif,sizeof(Ipp32f)*w,bigroi,-1,1);//pLowGrayregionlowdif=(pAbsdiff<fdifT均为1)
		ippiThreshold_LTVal_32f_C1IR(pLowGrayregionlowdif,sizeof(Ipp32f)*w,bigroi,1,0);//将-1均替换为0
		ippiMul_32f_C1IR(pLowGrayregion,sizeof(Ipp32f)*w,pLowGrayregionlowdif,sizeof(Ipp32f)*w,bigroi);//AND(plowdif,pLowregion)//低灰度区内小差异的置为1，其他置为0；
		//大差异模板
	 	ippiThreshold_LTVal_32f_C1R(pAbsdiff,sizeof(Ipp32f)*w,pLowGrayregionhighdif,sizeof(Ipp32f)*w,bigroi,fdifT,-1);//pLowGrayregionhighdif=(pAbsdiff<fdifT均为-1)
		ippiThreshold_GTVal_32f_C1IR(pLowGrayregionhighdif,sizeof(Ipp32f)*w,bigroi,-1,1);//pLowGrayregionhighdif=(pLowGrayregionhighdif>-1均为1)
		ippiThreshold_LTVal_32f_C1IR(pLowGrayregionhighdif,sizeof(Ipp32f)*w,bigroi,1,0);//将-1均替换为0
		ippiMul_32f_C1IR(pLowGrayregion,sizeof(Ipp32f)*w,pLowGrayregionhighdif,sizeof(Ipp32f)*w,bigroi);//AND(phighdif,pLowregion)//低灰度区内大差异的置为1，其他置为0；
		
		//dif=dif*2;
		ippiMulC_32f_C1IR(2.0,imgdiff,sizeof(Ipp32f)*w,bigroi);
		//dif=min(dif,1.of);
		ippiMinEvery_32f_C1IR(imgone,sizeof(Ipp32f)*w,imgdiff,sizeof(Ipp32f)*w,bigroi);//dif=min(dif,1.0f);
		//dif>=0和dif<0模板
		ippiThreshold_LTVal_32f_C1R(imgdiff,sizeof(Ipp32f)*w,pLowdiffpo,sizeof(Ipp32f)*w,bigroi,0,-1);//pLowDiffpo=(imgdiff<0均为-1)
		ippiThreshold_GTVal_32f_C1IR(pLowdiffpo,sizeof(Ipp32f)*w,bigroi,-1,1);//pLowDiffpo=(imgdiff>=0均为1)
		ippiMulC_32f_C1R(pLowdiffpo,sizeof(Ipp32f)*w,-1,pLowdiffne,sizeof(Ipp32f)*w,bigroi);//pLowDiffne=(imgdiff<0均为1，其他为-1)
		ippiThreshold_LTVal_32f_C1IR(pLowdiffpo,sizeof(Ipp32f)*w,bigroi,1,0);//pLowDiffpo=(imgdiff<0均为0)
		ippiThreshold_LTVal_32f_C1IR(pLowdiffne,sizeof(Ipp32f)*w,bigroi,1,0);//pLowDiffne=(imgdiff>=0均为0)
		//**************************************************************************************************************************************************************
		
		//大差异dif>=0模板
		//pHighDiffpo=pLowdiffpo(and)pLowGrayregionhighdif
		ippiMul_32f_C1R(pLowGrayregionhighdif,sizeof(Ipp32f)*w,pLowdiffpo,sizeof(Ipp32f)*w,pHighdiffpo,sizeof(Ipp32f)*w,bigroi);
		//大差异dif<0模板
		//pHighDiffne=pLowdiffne(and)pLowGrayregionhighdif
		ippiMul_32f_C1R(pLowGrayregionhighdif,sizeof(Ipp32f)*w,pLowdiffne,sizeof(Ipp32f)*w,pHighdiffne,sizeof(Ipp32f)*w,bigroi);


		//小差异dif>=0模板
		//pLowdiffpo=pLowdiffpo(and)pLowGrayregionlowdif
		ippiMul_32f_C1IR(pLowGrayregionlowdif,sizeof(Ipp32f)*w,pLowdiffpo,sizeof(Ipp32f)*w,bigroi);
		//小差异dif<0模板
		//pLowdiffne=pLowdiffne(and)pLowGrayregionlowdif
		ippiMul_32f_C1IR(pLowGrayregionlowdif,sizeof(Ipp32f)*w,pLowdiffne,sizeof(Ipp32f)*w,bigroi);

		//**************************************************************************************************************************************************
		//低灰度区处理
		//公共处理部分
		//rescos=cos(PI/2*dif)
		ippiMulC_32f_C1IR(PI/2,imgdiff,sizeof(Ipp32f)*w,bigroi);//imgdiff=PI*imgdiff;
		for(int i=0;i<h*w;i++)
		{
			rescos[i]=cos(imgdiff[i]);
		}
		//imgdiff1=(pOrg[pos]-dMean)*fEdgeDegree*detail;
		ippiMulC_32f_C1IR(detail*fEdgeDegree,imgdiff1,sizeof(Ipp32f)*w,bigroi);//imgdiff1*=detail*fEdgeDegree;


		//小差异dif>=0处理
		
		expIndex=3*detail*fDetailDegree*MAX_GRAY*0.001;
		ippiMulC_32f_C1R(rescos,sizeof(Ipp32f)*w,expIndex,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=cos(pi/2*dif)*expIndex*MAX_GRAY*0.001;
		ippiAdd_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.001
		ippiMulC_32f_C1R(imgdiff1,sizeof(Ipp32f)*w,4,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=(pOrg[pos]-dMean)*fEdgeDegree*detail*4;
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.001+(pOrg[pos]-dMean)*fEdgeDegree*detail*4;

		ippiMul_32f_C1IR(temp1,sizeof(Ipp32f)*w,pLowdiffpo,sizeof(Ipp32f)*w,bigroi);//乘以小差异dif>=0模板，pLowdiffpo里存了0和正常灰度值


		//小差异dif<0处理
		expIndex=4*detail*fDetailDegree*MAX_GRAY*0.001;
		ippiMulC_32f_C1R(rescos,sizeof(Ipp32f)*w,expIndex,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=cos(pi/2*dif)*expIndex*MAX_GRAY*0.001;
		ippiSub_32f_C1R(temp2,sizeof(Ipp32f)*w,orgimagef1,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=org-temp2;
		ippiMulC_32f_C1R(imgdiff1,sizeof(Ipp32f)*w,8,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=(pOrg[pos]-dMean)*fEdgeDegree*detail*8;
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.001+(pOrg[pos]-dMean)*fEdgeDegree*detail*4;

		ippiMul_32f_C1IR(temp1,sizeof(Ipp32f)*w,pLowdiffne,sizeof(Ipp32f)*w,bigroi);//乘以小差异dif<0模板，pLowdiffne里存了0和正常灰度值

		//大差异dif>=0处理
		expIndex=3*detail*fDetailDegree*MAX_GRAY*0.01;
		ippiMulC_32f_C1R(rescos,sizeof(Ipp32f)*w,expIndex,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=cos(pi/2*dif)*expIndex*MAX_GRAY*0.01;
		ippiAdd_32f_C1IR(orgimagef1,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.01
		ippiMulC_32f_C1R(imgdiff1,sizeof(Ipp32f)*w,4,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=(pOrg[pos]-dMean)*fEdgeDegree*detail*4;
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.01+(pOrg[pos]-dMean)*fEdgeDegree*detail*4;

		ippiMul_32f_C1IR(temp1,sizeof(Ipp32f)*w,pHighdiffpo,sizeof(Ipp32f)*w,bigroi);//乘以大差异dif>=0模板，pHighdiffpo里存了0和正常灰度值


		//大差异dif<0处理
		expIndex=4*detail*fDetailDegree*MAX_GRAY*0.01;
		ippiMulC_32f_C1R(rescos,sizeof(Ipp32f)*w,expIndex,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=cos(pi/2*dif)*expIndex*MAX_GRAY*0.01;
		ippiSub_32f_C1R(temp2,sizeof(Ipp32f)*w,orgimagef1,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=org-temp2;
		ippiMulC_32f_C1R(imgdiff1,sizeof(Ipp32f)*w,8,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=(pOrg[pos]-dMean)*fEdgeDegree*detail*8;
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2=org+cos(pi/2*dif)*expIndex*MAX_GRAY*0.01+(pOrg[pos]-dMean)*fEdgeDegree*detail*4;

		ippiMul_32f_C1IR(temp1,sizeof(Ipp32f)*w,pHighdiffne,sizeof(Ipp32f)*w,bigroi);//乘以大差异dif<0模板，pHighdiffne里存了0和正常灰度值


		//合成
		ippiAdd_32f_C1R(pHighdiffne,sizeof(Ipp32f)*w,pHighdiffpo,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=pHighdiffpo+pHighdiffne即低灰度大差异处理
		ippiAdd_32f_C1R(pLowdiffne,sizeof(Ipp32f)*w,pLowdiffpo,sizeof(Ipp32f)*w,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=pLowdiffpo+pLowdiffne即低灰度小差异处理
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2即低灰度区域处理
		ippiAdd_32f_C1R(pHighGrayregion,sizeof(Ipp32f)*w,pBackground,sizeof(Ipp32f)*w,temp2,sizeof(Ipp32f)*w,bigroi);//temp2=pBackground+pHighGrayregion即高灰度处理+背景
		ippiAdd_32f_C1IR(temp2,sizeof(Ipp32f)*w,temp1,sizeof(Ipp32f)*w,bigroi);//temp1=temp1+temp2即整个灰度区域处理
#ifdef ISWORD
		ippiConvert_32f16u_C1R(temp1, sizeof(Ipp32f)*w, pRes, sizeof(WORDTYPE)*w, bigroi, ippRndZero );
#else
		Ipp32u* res=(Ipp32u*)pRes;
		ippiMulC_32f_C1IR(4096,temp1,sizeof(Ipp32f)*w,bigroi); //左移12位功能
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
		ippiMulC_32f_C1IR(4096,orgimagef1,sizeof(WORDTYPE)*w,bigroi); //左移12位功能
		ippiConvert_32f32u_C1RSfs(orgimagef1,sizeof(Ipp32f)*w,res,sizeof(WORDTYPE)*w,bigroi,ippRndZero,0);
#endif
		
	}
	delete[] orgimage;
	delete[] orgimagef1;
	delete[] imagem;
	delete[] imgdiff;
	return TRUE;
}