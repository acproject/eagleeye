#include "stdafx.h"
//#include "cuda_runtime.h"

#include "DragBarEnhanceCu_kernel.h"
#include "DragBarEnhanceCu.h"

extern UINT g_unMovebyte;

/************************************************************************/
/* Init CUDA                                                            */
/************************************************************************/
#if __DEVICE_EMULATION__

BOOL InitCUDA(void){return true;}

#else

BOOL InitCUDA(void)
{
	int count = 0;
	int i = 0;

	cudaGetDeviceCount(&count);
	if(count == 0) {
		fprintf(stderr, "There is no device.\n");
		return FALSE;
	}

	for(i = 0; i < count; i++) {
		cudaDeviceProp prop;
		if(cudaGetDeviceProperties(&prop, i) == cudaSuccess) {
			if(prop.major >= 1 && prop.major != 9999) {                //找到可用的显卡即停止搜索。 如果有多个显卡，多个程序，可以协调分配
				break;
			}
		}
	}

	if(i == count) {
		fprintf(stderr, "There is no device supporting CUDA.\n");   // 如果没有可支持cuda的显卡，则应调用cpu算法
		return FALSE;
	}
	cudaSetDevice(i);  

	printf("CUDA initialized.\n");

	return TRUE;
}

#endif


bool LigConProcCuda(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect)
{
	//初始化数据
    float light=fBarPos[0],contrast=fBarPos[1];
	float k,nx,ny,db,nx1,ny1;
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
	int newlight=pBarTab[(int)((1-fabs(light))*nTabLen)];  //刻度变换

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

	//采用GPU加速亮度和对比度
	//建表
	WORDTYPE *LiconTable = new WORDTYPE[MAX_GRAY+1];
	memset(LiconTable,0,sizeof(WORDTYPE) * (MAX_GRAY+1));
	WORDTYPE * TabResult1;
	cudaMalloc((void**) &TabResult1, sizeof(WORDTYPE) * (MAX_GRAY+1));

	BuLiCUDA <<<(MAX_GRAY+1)/64,64>>>(TabResult1,k,db);//并开计算
	cudaThreadSynchronize();//同步，等待所有进程运行完成
	cudaMemcpy(LiconTable, TabResult1, sizeof(WORDTYPE) * (MAX_GRAY+1), cudaMemcpyDeviceToHost);

	//查表
	WORDTYPE * pDeviceOrignal;
	WORDTYPE * pDeviceResult;
	WORDTYPE * temp1;
	cudaMalloc((void**) &temp1, sizeof(WORDTYPE) * (MAX_GRAY+1));
	cudaMalloc((void**) &pDeviceOrignal, sizeof(WORDTYPE) * h*w);
	cudaMalloc((void**) &pDeviceResult, sizeof(WORDTYPE) * h*w);
	cudaMemcpy(temp1, LiconTable, sizeof(WORDTYPE) * (MAX_GRAY+1), cudaMemcpyHostToDevice);
	cudaMemcpy(pDeviceOrignal, pOrg, sizeof(WORDTYPE) * h*w, cudaMemcpyHostToDevice);

	dim3 dg((w+15)/16,(h+15)/16),dbb(16,16);
	HiCUDA <<<dg,dbb>>>(pDeviceOrignal,pDeviceResult,h,w,temp1,g_unMovebyte);//并开计算
	cudaThreadSynchronize();//同步，等待所有进程运行完成
	cudaMemcpy(pRes, pDeviceResult, sizeof(WORDTYPE) * h*w, cudaMemcpyDeviceToHost);

	cudaFree(pDeviceResult);
	cudaFree(pDeviceOrignal);
	cudaFree(temp1);
	cudaFree(TabResult1);
	delete[] pBarTab;
	delete[] LiconTable;
	return TRUE;
}

bool DetailProcCuda(const WORDTYPE* pOrg, WORDTYPE* pRes, int w,int h, float* fBarPos,bool bIsDrag,bool bIsROI,RECT* mROIRect,DetailPara depara)
{
	//方法二：用局部增强的方法，选（2*r-1）*（2*r-1）邻域的范围的均值作为衡量是否进行边缘增强的因子
    float detail=fBarPos[0];
	//采用GPU加速细节增强
	WORDTYPE * pDeviceOrignal;
	WORDTYPE * pDeviceResult;
	cudaMalloc((void**) &pDeviceOrignal, sizeof(WORDTYPE) * h*w);
	cudaMalloc((void**) &pDeviceResult, sizeof(WORDTYPE) * h*w);
	cudaMemcpy(pDeviceOrignal, pOrg, sizeof(WORDTYPE) * h*w, cudaMemcpyHostToDevice);

	dim3 dg((w+15)/16,(h+15)/16),db(16,16);
	DeCUDA <<<dg,db>>>(pDeviceOrignal,pDeviceResult,detail,h,w,depara,g_unMovebyte);//并开计算
	cudaThreadSynchronize();//同步，等待所有进程运行完成
	cudaMemcpy(pRes, pDeviceResult, sizeof(WORDTYPE) * h*w, cudaMemcpyDeviceToHost);

	cudaFree(pDeviceResult);
	cudaFree(pDeviceOrignal);

	return TRUE;

}