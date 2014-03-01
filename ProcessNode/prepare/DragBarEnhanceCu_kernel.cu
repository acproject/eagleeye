#include "stdafx.h"
//#include "cuda_runtime.h"

#include "DragBarEnhanceCu_kernel.h"

__global__ void BuLiCUDA(WORDTYPE* Tab1,float k,float db)
{
	int nx=blockIdx.x * blockDim.x + threadIdx.x;

	if(nx<(MAX_GRAY+1))
	{
		float pix;
		pix=nx*k+db;  
		if(pix<0) Tab1[nx]=0;
		else if(pix>MAX_GRAY) Tab1[nx]=MAX_GRAY;
		else Tab1[nx]=(WORDTYPE)pix;
	}

}


__global__ void HiCUDA(const WORDTYPE* pOrg, WORDTYPE* pRes,int h, int w,WORDTYPE* temp1,UINT unMovebyte)
{
	int nx=blockIdx.x * blockDim.x + threadIdx.x;
	int ny=blockIdx.y * blockDim.y + threadIdx.y;
	int nindex = ny * w + nx;

	if(nx<w && ny<h)
		pRes[nindex] = temp1[pOrg[nindex]>>unMovebyte]<<unMovebyte;

}


__global__ void DeCUDA(const WORDTYPE* pOrg, WORDTYPE* pRes,float detail,int h, int w, DetailPara depara,UINT unMovebyte)
{
	int nx=blockIdx.x * blockDim.x + threadIdx.x;
	int ny=blockIdx.y * blockDim.y + threadIdx.y;
	int nindex = ny * w + nx;

	WORDTYPE wBackgroundThreshold = depara.wBackgroundThreshold;
	int nRadius = depara.nRadius;
	float fDetailDegree = depara.fDetailDegree;
	WORDTYPE nGrayThreshold = depara.nGrayThreshold;
	float fDifThreshold = depara.fDifThreshold;
	float fEdgeDegree = depara.fEdgeDegree;

	if(nx<w && ny<h)
	{
		double dTempDouble,dMean,sum;
		int r,s;
		float expIndex;
		float dif;
		float curGray;

		if(nx<3 || ny<3 || nx>w-4 || ny>h-4)//边缘像素
		{
			pRes[nindex] = pOrg[nindex]>>unMovebyte;
		}
		else
		{
			if((pOrg[nindex]>>unMovebyte) < wBackgroundThreshold>>unMovebyte)
			{
    		 	for(r = -nRadius; r<=nRadius; r++)
		    	{
				    for(s = -nRadius;s<=nRadius; s++)
				    {
				    	sum += (pOrg[nindex + r*w + s]>>unMovebyte);
				    }
			    }
			    dMean = (WORDTYPE)(sum/((2*nRadius+1)*(2*nRadius+1)));
		    	dTempDouble = (pOrg[nindex]>>unMovebyte)-dMean;

			    if(detail>0)
				{
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

					if(((pOrg[nindex]>>unMovebyte)-dMean)>=0)
						expIndex=3*detail*fDetailDegree;//亮细节增强程度
					else
						expIndex=4*detail*fDetailDegree;//暗细节增强程度

					if((pOrg[nindex]>>unMovebyte)>nGrayThreshold)//灰度阈值，灰度高于一定的阈值，则不进行细节增强，只稍微进行边缘增强
					{
						dTempDouble=(pOrg[nindex]>>unMovebyte)+((pOrg[nindex]>>unMovebyte)-dMean)*3; 
					}
					else
					{
						curGray = (float)(pOrg[nindex]>>unMovebyte)/MAX_GRAY;
						dif	= curGray - (float)dMean/MAX_GRAY;
					    if (fabs(dif)*curGray*curGray*curGray < fDifThreshold)  //0.002)  //0.0005)	
						{
							dif	*= 2;          //cos曲线x方向压缩比例，默认取1，就是不进行压缩
							dif = min(dif,1.0f);
							if (dif>=0)
							{	
								dTempDouble = (pOrg[nindex]>>unMovebyte) + expIndex* cos(3.14159/2*dif) * MAX_GRAY * 0.001 + ((pOrg[nindex]>>unMovebyte)-dMean)*4*fEdgeDegree*detail;                //亮边缘增强程度							
							}
							else
							{
								dTempDouble = (pOrg[nindex]>>unMovebyte) - expIndex* cos(3.14159/2*dif) * MAX_GRAY * 0.001 + ((pOrg[nindex]>>unMovebyte)-dMean)*8*fEdgeDegree*detail;                //暗边缘增强程度
							}
						}
						else
						{
							dif	*= 2;          //cos曲线x方向压缩比例，默认取1，就是不进行压缩
							dif = min(dif,1.0f);
							if (dif>=0)
							{
								dTempDouble =(pOrg[nindex]>>unMovebyte) +  expIndex * cos(3.14159/2*dif) * MAX_GRAY * 0.01 + ((pOrg[nindex]>>unMovebyte)-dMean)*4*fEdgeDegree*detail; 							
							}
							else
							{
								dTempDouble= (pOrg[nindex]>>unMovebyte) - expIndex* cos(3.14159/2*dif) * MAX_GRAY * 0.01 + ((pOrg[nindex]>>unMovebyte)-dMean)*8*fEdgeDegree*detail; 	
							}
						}		
					}
				}
				else
				{
					dTempDouble=(pOrg[nindex]>>unMovebyte)+dTempDouble*detail;
				}
			}
			else 
			{
				dTempDouble=(pOrg[nindex]>>unMovebyte);

			}

			if(dTempDouble<0) dTempDouble = 0;
			if(dTempDouble>MAX_GRAY) dTempDouble = MAX_GRAY;
			pRes[nindex] = ((WORDTYPE)dTempDouble)<<unMovebyte;
		}
	}

}
