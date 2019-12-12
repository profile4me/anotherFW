#include "PeriodAssets.h"
#include <hloop.h>
#include <hcategory.h>
#include <hwallraw.h>

using namespace std;

void fill() {
	new HLoop(1);
	gLoop->addMultFiles("/lustre/nyx/hades/user/dborisen/fwcalib/dst/be1906305202304.root,/lustre/nyx/hades/user/dborisen/fwcalib/dst/be1906306150102.root");
	// gLoop->addMultFiles("/lustre/nyx/hades/user/dborisen/fwcalib/dst/be1906305202304.root");
	gLoop->setInput("-*,+HWallRaw");
	int nevents=gLoop->getChain()->GetEntries();
	HCategory* wallCat = gLoop->getCategory("HWallRaw");
	HWallRaw* wallData;
	TFile ioF("output.root", "recreate");
	PeriodAssets period("day63");

	for (int e=0; e<nevents; e++) {
		if (e%5000==0) printf("\tEvent: %d\n", e);
		gLoop->nextEvent(e);
		for (int h=0; h<wallCat->getEntries(); h++) {
			wallData = (HWallRaw*)wallCat->getObject(h);
			int cid = wallData->getCell();
			float w = wallData->getWidth(1);
			period.fill(cid, w);
		}
	}

	period.Write();
	ioF.Close();
}

void fit() {
	TFile ioF("output.root", "update");
	PeriodAssets* period = (PeriodAssets*)ioF.Get("day63");
	period->fit();
	period->Write("", TObject::kOverwrite);
	ioF.Close();
}

void print() {
	TFile ioF("output.root");
	PeriodAssets* period = (PeriodAssets*)ioF.Get("day63");
	period->print();
}
void print(int cid) {
	TFile ioF("output.root");
	PeriodAssets* period = (PeriodAssets*)ioF.Get("day63");
	period->print(cid);
}

// int main(TString s)
int main(int argc, char const *argv[])
{	
	// if (!s.CompareTo("-fill")) fill();
	// if (!s.CompareTo("-fit")) fit();
	string str = argv[1];
	if (!str.compare("-fill")) fill();
	if (!str.compare("-fit")) fit();
	if (!str.compare("-print")) {
		if (argc==2)  print();
		else {
			str = argv[2];
			int cid = stoi(str);
			print(cid);
		}
	}

	return 0;
}