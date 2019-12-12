#include "PeriodAssets.h"
#include <hloop.h>
#include <hcategory.h>
#include <hwallraw.h>
#include <map>
#include <list>
#include <TKey.h>
#include <TColor.h>
#include <TMultiGraph.h>
#include <TLegend.h>

using namespace std;

const list<int> WHAT_DAYS = {62, 63, 64, 65};
const char*		COLORS[]  = {"#e6194b", "#3cb44b", "#ffe119", "#4363d8", "#f58231", "#911eb4", "#46f0f0", "#f032e6", "#bcf60c", "#fabebe", "#008080"};
const char*		ANOTHER_COLORS[] = {"#4c1313", "#ff4400", "#bfa38f", "#ffcc00", "#4c4113", "#60bf6c", "#00ccff", "#002b40", "#3d6df2", "#1a0040", "#aa00ff", "#af8fbf", "#f23d9d"};
const int 		MARKER_STYLES[] = {kOpenCircle, kOpenTriangleUp, kOpenTriangleDown, kOpenSquare, kOpenCircle, kOpenTriangleUp, kOpenTriangleDown, kOpenSquare, kOpenCircle, kOpenTriangleUp, kOpenTriangleDown, kOpenSquare, kOpenCircle, kOpenTriangleUp, kOpenTriangleDown, kOpenSquare};

map<int, PeriodAssets*> periods;

void fill(int day) {
	int fEvent = gLoop->getChain()->GetEntries();
	gLoop->addMultFiles(PeriodAssets::getDSTs(day, 2));	
	int lEvent = gLoop->getChain()->GetEntries();
	gLoop->setInput("-*,+HWallRaw");
	HCategory* wallCat = gLoop->getCategory("HWallRaw");
	HWallRaw* wallData;
	PeriodAssets* period = new PeriodAssets(day);

	for (int e=fEvent; e<lEvent; e++) {
		if (e%5000==0) printf("\tEvent: %d\n", e);
		gLoop->nextEvent(e);
		for (int h=0; h<wallCat->getEntries(); h++) {
			wallData = (HWallRaw*)wallCat->getObject(h);
			int cid = wallData->getCell();
			float w = wallData->getWidth(1);
			float t = wallData->getTime(1);
			period->fill(cid, w, t);
		}
	}

	periods[day] = period;
}

void readPeriods(TFile& iof) {
	for (TObject* key: *iof.GetListOfKeys()) {
 		TObject* obj = ((TKey*)key)->ReadObj();
 		if (obj->InheritsFrom("PeriodAssets")) {
 			PeriodAssets* p = (PeriodAssets*)obj;
 			periods[p->day] = p;
 		}
 	}
}

void fit() {
	for (auto& p: periods) p.second->fit();
	for (auto& p: periods) p.second->Write("", TObject::kOverwrite);
}

void print() {
	for (auto& p: periods) p.second->printWidth();
}
void printWidthTime() {
	for (auto& p: periods) p.second->printWidthTime();
}
void print(int day, int cid) {
	periods[day]->print(cid);
}

void setupLegend(TLegend* legend) {
	legend->SetX1(0.8);
	legend->SetY1(0.05);
	legend->SetX2(0.97);
	legend->SetY2(0.95);
	legend->SetNColumns(4);
	legend->SetMargin(0.4);
	legend->SetEntrySeparation(0);
	legend->SetColumnSeparation(0);
}

void trendMaking() {
	TGraph trends[NCELLS];
	float ref_vals[NCELLS];
	for (int cid=0; cid<NCELLS; cid++) ref_vals[cid] = periods.begin()->second->z1[cid];
	for (auto& p: periods) {
		printf("period for day: %d\n", p.first);
		for (int cid=0; cid<NCELLS; cid++) 
			if (!PeriodAssets::isEmpty(cid)) trends[cid].SetPoint(trends[cid].GetN(), p.first, p.second->z1[cid]-ref_vals[cid]);
	}
	TMultiGraph masterTrends[4];
	for (int i=0; i<4; i++) {
		for (int cid=i*52; cid<(i+1)*52; cid++) {
			if (PeriodAssets::isEmpty(cid)) continue;
			trends[cid].SetMarkerColor(TColor::GetColor(ANOTHER_COLORS[cid%13]));
			trends[cid].SetMarkerStyle(MARKER_STYLES[cid/13]);
			trends[cid].SetFillColor(0);
			trends[cid].SetLineColor(0);
			trends[cid].SetTitle(Form("#%d",cid));
			masterTrends[i].Add(&trends[cid]);
		}
	}
	TCanvas c("c", "", 900, 1200);
	c.Divide(1,4);
	for (int i=0; i<4; i++) {
		TVirtualPad* pad = c.cd(i+1);
		pad->SetMargin(0.05, 0.2, 0.04, 0.04);
		masterTrends[i].Draw("AP");
		setupLegend(pad->BuildLegend());
	}
	c.Update();
	c.SaveAs("output_assets/z1_trends.png");
}

int main(int argc, char const *argv[])
{	
	string str = argv[1];
	if (!str.compare("-fill")) {
		new HLoop(1);
		for (int d: WHAT_DAYS) fill(d);
		TFile ioF("output.root", "recreate");
		for (auto& p: periods) p.second->Write();
		ioF.Close();
	}
	if (!str.compare("-fit")) {
		TFile ioF("output.root", "update");
		readPeriods(ioF);
		fit();
		ioF.Close();
	}
	if (!str.compare("-print")) {
		TFile ioF("output.root");
		readPeriods(ioF);
		if (argc==2)  print();
		else if (argc==4) {
			int day = stoi(string(argv[2]));
			int cid = stoi(string(argv[3]));
			print(day, cid);
		}
	}
	if (!str.compare("-printTime")) {
		TFile ioF("output.root");
		readPeriods(ioF);
		printWidthTime();
	}
	if (!str.compare("-trend")) {
		TFile ioF("output.root");
		readPeriods(ioF);
		trendMaking();
	}
	return 0;
}