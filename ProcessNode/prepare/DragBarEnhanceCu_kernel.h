#pragma once

#include "ExportFunction.h"
#include "DragBarEnhanceCu.h"

__global__ void BuLiCUDA(WORDTYPE* Tab1,float k,float db);
__global__ void HiCUDA(const WORDTYPE* pOrg, WORDTYPE* pRes,int h, int w,WORDTYPE* temp1,UINT unMovebyte);
__global__ void DeCUDA(const WORDTYPE* pOrg, WORDTYPE* pRes,float detail,int h, int w, DetailPara depara,UINT unMovebyte);

