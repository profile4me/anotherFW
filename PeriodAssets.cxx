#include "PeriodAssets.h"
#include <cstdio>

const void* PeriodAssets::mock = PeriodAssets::init(); 
float PeriodAssets::SIGMA = 2;
float PeriodAssets::THRES = 0.05;

ClassImp(PeriodAssets);

using namespace std;

bool PeriodAssets::isEmpty(int cid) {
	for (int i=0; i<4; i++) 
		if (EMPTY_CELLS[i]==cid) return true;
	return false;
}


void* PeriodAssets::init() {
	FILE* params = fopen("params.txt", "r");
	fscanf(params, "%*s %f", &SIGMA);
	fscanf(params, "%*s %f", &THRES);
	printf("-------------PARAMS---------------\n");
	printf("\t: %.1f\n", SIGMA);
	printf("\t: %.4f\n", THRES);
	printf("----------------------------------\n");
	fclose(params);
}


