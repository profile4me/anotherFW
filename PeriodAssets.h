#ifndef __PeriodAssets__
#define __PeriodAssets__

#include <string>

#include <TNamed.h>
#include <TMath.h>
#include <TSpectrum.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TCanvas.h>
#include <THStack.h>
#include <TGraph.h>

const int NCELLS = 208;
const int WIDTH_MIN = 0;
const int WIDTH_MAX = 2500;
const int TIME_MIN = 1100;
const int TIME_MAX = 1400;
const int EMPTY_CELLS[] = {65, 66, 77, 78};

struct PeriodAssets : public TNamed {
	static float SIGMA;
	static float THRES;
	static const void* mock;
	static void* init();
	static bool isEmpty(int cid);
	static TString getDSTs(int d, int n);

	int day;
	TH1* width[NCELLS];
	TH1* width_smoothedWoBg[NCELLS];
	TH2* width_time[NCELLS];
	TGraph* z1_z2;
	float z1[NCELLS];
	float z2[NCELLS];

	PeriodAssets() {
		for (int cid=0; cid<NCELLS; cid++) width[cid] = width_smoothedWoBg[cid] = NULL;
		for (int cid=0; cid<NCELLS; cid++) width_time[cid] = NULL;
		z1_z2 = NULL;
	}
	PeriodAssets(int _day) {
		day = _day;
		SetName(Form("day%d", day));
		for (int cid=0; cid<NCELLS; cid++) {
			width[cid] = new TH1F(Form("width_cell%d", cid), Form("#%d",cid), 200, WIDTH_MIN, WIDTH_MAX);
			width_smoothedWoBg[cid] = new TH1F(Form("mock_cell%d", cid), "", 1, 0, 1);
			width_time[cid] = new TH2F(Form("widthTime_cell%d", cid), Form("#%d",cid), 200, WIDTH_MIN, WIDTH_MAX, 200, TIME_MIN, TIME_MAX);
		}
		z1_z2 = new TGraph;
	}

	void fill(int cid, float w, float t) {
		if (cid<NCELLS) {
			width[cid]->Fill(w);
			width_time[cid]->Fill(w, t);
		}
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
		z1[cid] = fit->GetParameter(7);
		z2[cid] = fit->GetParameter(10);
		if (cid==33 && npeaks==2) {
			z1[cid] = fit->GetParameter(1);
			z2[cid] = fit->GetParameter(4);
		}
		printf("CELL%d: z1 = %.2f   z2 = %.2f\n", cid, z1[cid], z2[cid]);	
		z1_z2->SetPoint(z1_z2->GetN(), z1[cid], z2[cid]);
	}
	void fit() {
		delete z1_z2;
		z1_z2 = new TGraph;
		for (int cid=0; cid<NCELLS; cid++) fit(cid);
	}
	void print(int cid) {
		TCanvas c;
		width_smoothedWoBg[cid]->Draw();
		c.SaveAs(Form("output_assets/fit_%d.png",cid));
	}
	void printWidth() {
		THStack* st = new THStack;
		TCanvas c;
		std::string pdf_name = Form("output_assets/cells_day%d.pdf", day);
		c.Print(Form("%s[", pdf_name.data()));
		for (int cid=0; cid<NCELLS; cid++) {
			if (isEmpty(cid)) continue;
			st->Add(width_smoothedWoBg[cid]);
			if (st->GetNhists()==16) {
				st->Draw("PADS");
				c.Update();
				c.Print(pdf_name.data());

				c.Clear();
				delete st;
				st = new THStack;
			}
		}
		if (st->GetNhists()) {
			st->Draw("PADS");
			c.Update();
			c.Print(pdf_name.data());
		}
		z1_z2->SetMarkerStyle(kFullCircle);
		z1_z2->Draw("AP");
		c.Print(Form("%s)", pdf_name.data()));
	}
	void printWidthTime() {
		THStack* st = new THStack;
		TCanvas c;
		std::string pdf_name = Form("output_assets/cells_widthTime_day%d.pdf", day);
		c.Print(Form("%s[", pdf_name.data()));
		for (int cid=0; cid<NCELLS; cid++) {
			if (isEmpty(cid)) continue;
			st->Add(width_time[cid]);
			if (st->GetNhists()==16) {
				st->Draw("PADS");
				c.Update();
				c.Print(pdf_name.data());

				c.Clear();
				delete st;
				st = new THStack;
			}
		}
		if (st->GetNhists()) {
			st->Draw("PADS");
			c.Update();
			c.Print(pdf_name.data());
		}
		z1_z2->SetMarkerStyle(kFullCircle);
		z1_z2->Draw("AP");
		c.Print(Form("%s)", pdf_name.data()));
	}
	ClassDef(PeriodAssets,1);
};

#endif