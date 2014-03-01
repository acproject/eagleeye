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

		if(nx<3 || ny<3 || nx>w-4 || ny>h-4)//��Ե����
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
						expIndex=3*detail*fDetailDegree;//��ϸ����ǿ�̶�
					else
						expIndex=4*detail*fDetailDegree;//��ϸ����ǿ�̶�

					if((pOrg[nindex]>>unMovebyte)>nGrayThreshold)//�Ҷ���ֵ���Ҷȸ���һ������ֵ���򲻽���ϸ����ǿ��ֻ��΢���б�Ե��ǿ
					{
						dTempDouble=(pOrg[nindex]>>unMovebyte)+((pOrg[nindex]>>unMovebyte)-dMean)*3; 
					}
					else
					{
						curGray = (float)(pOrg[nindex]>>unMovebyte)/MAX_GRAY;
						dif	= curGray - (float)dMean/MAX_GRAY;
					    if (fabs(dif)*curGray*curGray*curGray < fDifThreshold)  //0.002)  //0.0005)	
						{
							dif	*= 2;          //cos����x����ѹ��������Ĭ��ȡ1�����ǲ�����ѹ��
							dif = min(dif,1.0f);
							if (dif>=0)
							{	
								dTempDouble = (pOrg[nindex]>>unMovebyte) + expIndex* cos(3.14159/2*dif) * MAX_GRAY * 0.001 + ((pOrg[nindex]>>unMovebyte)-dMean)*4*fEdgeDegree*detail;                //����Ե��ǿ�̶�							
							}
							else
							{
								dTempDouble = (pOrg[nindex]>>unMovebyte) - expIndex* cos(3.14159/2*dif) * MAX_GRAY * 0.001 + ((pOrg[nindex]>>unMovebyte)-dMean)*8*fEdgeDegree*detail;                //����Ե��ǿ�̶�
							}
						}
						else
						{
							dif	*= 2;          //cos����x����ѹ��������Ĭ��ȡ1�����ǲ�����ѹ��
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
