#include "PeriodAssets.h"
#include <cstdio>
#include <TPRegexp.h>
#include <TObjArray.h>
#include <TObjString.h>

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

TString PeriodAssets::getDSTs(int d, int n) {
	TPRegexp re("be19([0-9]{3})");
	char buffer[100];
	int counter=0;
	TString res;
	FILE* dst_list = fopen("dst.list", "r");
	while (EOF != fscanf(dst_list, "%s", buffer)) {
		TObjArray* matches = re.MatchS(buffer);
		if (matches->GetEntries()) {
			int day = ((TObjString*)matches->At(1))->String().Atoi();
			if (day==d) {
				res += Form("%s,", buffer);
				if (++counter==n) {
					fclose(dst_list);
					return res;
				}
			}
		}
	}
	fclose(dst_list);
	printf("WARNING!!! Not enoght dst files!!\n");
	return res;
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


