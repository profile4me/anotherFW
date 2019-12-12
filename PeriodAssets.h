#ifndef __PeriodAssets__
#define __PeriodAssets__

#include <string>

#include <TNamed.h>
#include <TMath.h>
#include <TSpectrum.h>
#include <TH1.h>
#include <TF1.h>
#include <TCanvas.h>
#include <THStack.h>
#include <TGraph.h>

const int NCELLS = 208;
const int WIDTH_MIN = 0;
const int WIDTH_MAX = 2500;
const int EMPTY_CELLS[] = {65, 66, 77, 78};

struct PeriodAssets : public TNamed {
	static float SIGMA;
	static float THRES;
	static const void* mock;
	static void* init();
	static bool isEmpty(int cid);

	TH1* width[NCELLS];
	TH1* width_smoothedWoBg[NCELLS];
	TGraph* z1_z2;

	PeriodAssets() {
		for (int cid=0; cid<NCELLS; cid++) width[cid] = width_smoothedWoBg[cid] = NULL;
		z1_z2 = NULL;
	}
	PeriodAssets(const char* name) : TNamed(name, "") {
		for (int cid=0; cid<NCELLS; cid++) {
			width[cid] = new TH1F(Form("width_cell%d", cid), Form("#%d",cid), 200, WIDTH_MIN, WIDTH_MAX);
			width_smoothedWoBg[cid] = new TH1F(Form("mock_cell%d", cid), "", 1, 0, 1);
		}
		z1_z2 = new TGraph;
	}

	void fill(int cid, float w) {
		if (cid<NCELLS) width[cid]->Fill(w);
	}
	void fit(int cid) {
		if (isEmpty(cid)) return;
		//smoothing and bg substraction
		TH1* bg = TSpectrum::StaticBackground(width[cid], 15);
		delete width_smoothedWoBg[cid];
		width_smoothedWoBg[cid] = TSpectrum::StaticBackground(width[cid], 1);
		width_smoothedWoBg[cid]->SetName(Form("width_smoothedWoBg_cell%d",cid));
		width_smoothedWoBg[cid]->SetLineColor(kGreen);
		width_smoothedWoBg[cid]->Add(bg, -1);
		//searching peaks
		TSpectrum sp(8);
		int npeaks = sp.Search(width_smoothedWoBg[cid], SIGMA, "", THRES);
		float* posX = sp.GetPositionX();
		float* posY = sp.GetPositionY();
		int* index = new int[npeaks];
		TMath::Sort(npeaks, posX, index, false);
		//fit
		std::string formula("gaus(0)");
		for (int p=1; p<npeaks; p++) formula += Form("+gaus(%d)", p*3);
		TF1* fit = new TF1(Form("fit_cell%d", cid), formula.data(), WIDTH_MIN, WIDTH_MAX);
		for (int p=0; p<npeaks; p++) {
			fit->SetParameter(p*3 , 	posY[index[p]]);
			fit->SetParameter(p*3+1, 	posX[index[p]]);
			fit->SetParameter(p*3+2, 	50);
		}
		width_smoothedWoBg[cid]->Fit(fit, "qS");
		float z1 = fit->GetParameter(7);
		float z2 = fit->GetParameter(10);
		if (cid==33 && npeaks==2) {
			z1 = fit->GetParameter(1);
			z2 = fit->GetParameter(4);
		}
		printf("CELL%d: z1 = %.2f   z2 = %.2f\n", cid, z1, z2);	
		z1_z2->SetPoint(z1_z2->GetN(), z1, z2);
	}
	void fit() {
		delete z1_z2;
		z1_z2 = new TGraph;
		for (int cid=0; cid<NCELLS; cid++) fit(cid);
	}
	void print(int cid) {
		TCanvas c;
		width_smoothedWoBg[cid]->Draw();
		c.SaveAs(Form("cells/fit_%d.png",cid));
	}
	void print() {
		THStack* st = new THStack;
		TCanvas c;
		c.Print("cells.pdf[");
		for (int cid=0; cid<NCELLS; cid++) {
			if (isEmpty(cid)) continue;
			st->Add(width_smoothedWoBg[cid]);
			if (st->GetNhists()==16) {
				st->Draw("PADS");
				c.Update();
				c.Print("cells.pdf");

				c.Clear();
				delete st;
				st = new THStack;
			}
		}
		if (st->GetNhists()) {
			st->Draw("PADS");
			c.Update();
			c.Print("cells.pdf");
		}
		z1_z2->SetMarkerStyle(kFullCircle);
		z1_z2->Draw("AP");
		c.Print("cells.pdf)");
	}
	ClassDef(PeriodAssets,1);
};

#endif