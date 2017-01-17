#include <Riostream.h>
#include <TComplex.h>
#include <TF1.h>
#include <TH1D.h>
#include <TLorentzVector.h>
#include <TMath.h>
#include <TParticlePDG.h>
#include <TProfile.h>
#include <TRandom.h>
#include <TRandom3.h>
#include <TStopwatch.h>
#include "Cumulants.h"

CPart::CPart(const TParticle &p) : fPt(p.Pt()), fEta(p.Eta()), fPhi(TVector2::Phi_0_2pi(p.Phi()))
{
  TParticlePDG *pdg = p.GetPDG(1);
  fCharge = pdg->Charge();
}

Cumulants::Cumulants(const char *name, Int_t mbins, Int_t minM) : 
  TNamed(name,""), fCumMBins(mbins), fMinM(minM), fEtaMin(-1), fEtaMax(1), fPtMin(0.3), fPtMax(3), fDoEtaGap(0),
  fDoCharge(1), fDoQC(0), fDoQC44(0), fMaxNL4(50), fEGminNL4(0), fDoQC43(0), fMaxNL3(100), fEGminNL3(0), 
  fDoQC4withEG(1), fQC4el(-0.5), fQC4eu(0.5), fDoDebug(0), fDoPrint(0) 
{
  fList = new TList;
  fList->SetName(name);
  fParts.resize(9999);
  fHists[0] = new TH1D("fCumMnocut",";mult",fCumMBins,0,fCumMBins);
  fList->Add(fHists[0]);
  fHists[1] = new TH1D("fCumMall",";mult",fCumMBins,0,fCumMBins);
  fList->Add(fHists[1]);
  fHists[2] = new TH1D("fCumMin",";mult",fCumMBins,0,fCumMBins);
  fList->Add(fHists[2]);
  fHists[3] = new TH1D("fCumM",";mult",fCumMBins,0,fCumMBins);
  fList->Add(fHists[3]);
}

void Cumulants::EnableEG()
{
  // Enable eta gap measurements
  Double_t etacuts[] = {0.1,0.2,0.3,0.4,0.5,0.6,0.8,1.0,1.2,1.4,1.6,1.8,2.0,2.5};
  const Int_t N = 14;
  fEGCuts.resize(N);
  fEGC2.resize(N);
  fEGC3.resize(N);
  fEGS2.resize(N);
  fEGS3.resize(N);
  fEGCounts.resize(N);
  Int_t hindex=100;
  for (Int_t i=0;i<N;++i) {
    fEGCuts[i]=etacuts[i];
    fHists[hindex] = new TProfile(Form("fEtaGapC2%02d",Int_t(etacuts[i]*10)),Form(";M;#LTcos(2#phi_{1}-2#phi_{2})#GT (|#eta|>%.1f)",etacuts[i]),fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex]);
    ++hindex;
    fHists[hindex] = new TProfile(Form("fEtaGapC3%02d",Int_t(etacuts[i]*10)),Form(";M;#LTcos(3#phi_{1}-3#phi_{2})#GT (|#eta|>%.1f)",etacuts[i]),fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex]);
    ++hindex;
    fHists[hindex] = new TProfile(Form("fEtaGapS2%02d",Int_t(etacuts[i]*10)),Form(";M;#LTsin(2#phi_{1}-2#phi_{2})#GT (|#eta|>%.1f)",etacuts[i]),fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex]);
    ++hindex;
    fHists[hindex] = new TProfile(Form("fEtaGapS3%02d",Int_t(etacuts[i]*10)),Form(";M;#LTsin(3#phi_{1}-3#phi_{2})#GT (|#eta|>%.1f)",etacuts[i]),fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex]);
    ++hindex;
  }
  fDoEtaGap = 1;
}

void Cumulants::EnableQC()
{
  // Enable QC measurements
  fDoQC = 1;
  Int_t hindex=200;
  fHists[hindex] = new TProfile("fCumQ2r",";M;q2r",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCumQ2i",";M;q2i",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCumQ3r",";M;q3r",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCumQ3i",";M;q3i",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCumQ4r",";M;q4r",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCumQ4i",";M;q4i",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCumQ6r",";M;q6r",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCumQ6i",";M;q6i",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  hindex=230;
  fHists[hindex] = new TProfile("fCumQC2",";M;qc2",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCumQC4",";M;qc4",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCumQC6",";M;qc6",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCum3QC2",";M;qc2",fCumMBins,0,fCumMBins);
  fList->Add(fHists[hindex++]);
  fHists[hindex] = new TProfile("fCum3QC4",";M;qc4",fCumMBins,0,fCumMBins);
  if (fDoCharge) {
    hindex=260;
    fHists[hindex] = new TH1D("fCumMpp",";mult (pos)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TH1D("fCumMnn",";mult (neg)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCumQC2pp",";M;qc2 (pos)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCum3QC2pp",";M;qc2 (pos)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCumQC4pp",";M;qc4 (pos)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCum3QC4pp",";M;qc4 (pos)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCumQC2nn",";M;qc2 (neg)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCumQC4nn",";M;qc4 (neg)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCum3QC2nn",";M;qc2 (neg)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCum3QC4nn",";M;qc4 (neg)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    hindex=280;
    fHists[hindex] = new TProfile("fCumQC2ss",";M;qc2 (ss)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCumQC4ss",";M;qc4 (ss)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCum3QC2ss",";M;qc2 (ss)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
    fHists[hindex] = new TProfile("fCum3QC4ss",";M;qc4 (ss)",fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex++]);
  }
}

void Cumulants::EnableQC4with4NL(Int_t mn, Double_t etamin)
{
  // Enable QC measurements with 4NL
  fDoQC44   = 1;
  fMaxNL4   = mn;
  fEGminNL4 = etamin;
  Int_t hindex=300;
  for (Int_t i=0;i<6;++i) {
    Double_t etagap=(Double_t)i/10;
    fHists[hindex] = new TProfile(Form("fQC4NL42%d",i),Form(";M;QC4 (|#eta|>%.1f)",etagap),fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex]);
    ++hindex;
  }
}

void Cumulants::EnableQC4with3NL(Int_t mn, Double_t etamin)
{
  // Enable QC measurements with 3NL
  fDoQC43   = 1;
  fMaxNL3   = mn;
  fEGminNL3 = etamin;
  Int_t hindex=350;
  for (Int_t i=0;i<6;++i) {
    Double_t etagap=(Double_t)i/10;
    fHists[hindex] = new TProfile(Form("fQC3NL42%d",i),Form(";M;QC4 (|#eta|>%.1f)",etagap),fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex]);
    ++hindex;
  }
}

void Cumulants::EnableQC4withEG(Double_t etal, Double_t etau)
{
  // Enable QC measurements with 3NL
#if 0
  fDoQC43   = 1;
  fMaxNL3   = mn;
  fEGminNL3 = etamin;
  Int_t hindex=350;
  for (Int_t i=0;i<6;++i) {
    Double_t etagap=(Double_t)i/10;
    fHists[hindex] = new TProfile(Form("fQC3NL42%d",i),Form(";M;QC4 (|#eta|>%.1f)",etagap),fCumMBins,0,fCumMBins);
    fList->Add(fHists[hindex]);
    ++hindex;
  }
#endif
}

void Cumulants::RunAll()
{
  // Run all enabled methods
  if (fM<fMinM)
    return;
  fHists[3]->Fill(fM);

  TStopwatch w;
  if (fDoEtaGap) {
    w.Start();
    RunEG();
    w.Stop();
  }
  if (fDoQC) {
    w.Start();
    RunQC();
    w.Stop();
  }
  if (fDoQC44) {
    w.Start();
    RunQC4with4NL();
    w.Stop();
  }
  if (fDoQC43) {
    w.Start();
    RunQC4with3NL();
    w.Stop();
  }
  if (fDoQC4withEG) {
    w.Start();
    RunQC4withEG();
    w.Stop();
  }
}

void Cumulants::RunEG()
{
  // Run eta gap two particle method
  const UInt_t N=fEGCuts.size();
  fEGC2.assign(N,0);
  fEGC3.assign(N,0);
  fEGS2.assign(N,0);
  fEGS3.assign(N,0);
  fEGCounts.assign(N,0);
  for (Int_t i=0; i<fM; ++i) {
    CPart &p1 = fParts.at(i);
    for (Int_t j=i+1; j<fM; ++j) {
      CPart &p2 = fParts.at(j);
      Double_t dphi=p1.Phi()-p2.Phi();
      Double_t deta=TMath::Abs(p1.Eta()-p2.Eta());
      Double_t c2 = TMath::Cos(2*dphi); 
      Double_t s2 = TMath::Sin(2*dphi); 
      Double_t c3 = TMath::Cos(3*dphi); 
      Double_t s3 = TMath::Sin(3*dphi); 
      for (UInt_t k=0;k<N;++k) {
	if (deta>fEGCuts.at(k)) {
 	  fEGC2[k] += c2;
	  fEGC3[k] += c3;
	  fEGS2[k] += s2;
	  fEGS3[k] += s3;
	  fEGCounts[k] += 1;
	} else 
	  break;
      }
    }
  }
  Int_t hind=100;
  for (UInt_t i=0;i<N;++i) {
    if (fEGCounts[i]<=0)
      continue;
    fEGC2[i]/=fEGCounts[i];
    fHists[hind++]->Fill(fM,fEGC2[i]);
    fEGC3[i]/=fEGCounts[i];
    fHists[hind++]->Fill(fM,fEGC3[i]);
    fEGS2[i]/=fEGCounts[i];
    fHists[hind++]->Fill(fM,fEGS2[i]);
    fEGS3[i]/=fEGCounts[i];
    fHists[hind++]->Fill(fM,fEGS3[i]);
  }
}

void Cumulants::RunQC()
{
  // Run QC method
  fQC[2]=0;fQC[3]=0;fQC[4]=0;fQC[5]=0;fQC[6]=0;
  for (Int_t i=0; i<fM; ++i) {
    CPart &p = fParts.at(i);
    Double_t phi=p.Phi();
    for (Int_t k=2; k<7; ++k) {
      fQC[k] += TComplex(TMath::Cos(k*phi),TMath::Sin(k*phi));
    }
  }

  Double_t Q22   = fQC[2].Rho2();
  Double_t Q32   = fQC[3].Rho2();
  Double_t Q42   = fQC[4].Rho2();
  Double_t Q52   = fQC[5].Rho2();
  Double_t Q62   = fQC[6].Rho2();
  Double_t Q32re = (fQC[6]*TComplex::Power(TComplex::Conjugate(fQC[3]),2)).Re();
  Double_t Q42re = TComplex(fQC[4]*TComplex::Power(TComplex::Conjugate(fQC[2]),2)).Re();
  Double_t Q6are = TComplex(fQC[4]*fQC[2]*TComplex::Power(TComplex::Conjugate(fQC[2]),3)).Re();
  Double_t Q6bre = TComplex(fQC[6]*TComplex::Power(TComplex::Conjugate(fQC[2]),3)).Re();
  Double_t Q6cre = TComplex(fQC[6]*TComplex::Conjugate(fQC[4])*TComplex::Conjugate(fQC[2])).Re();

  Int_t M  = fM;
  Int_t M2 = M*(M-1);
  Int_t M4 = M*(M-1)*(M-2)*(M-3);
  if (M>1) {
    Int_t hindex=200;
    fHists[hindex++]->Fill(M,fQC[2].Re()/M);
    fHists[hindex++]->Fill(M,fQC[2].Im()/M);
    fHists[hindex++]->Fill(M,fQC[3].Re()/M);
    fHists[hindex++]->Fill(M,fQC[3].Im()/M);
    fHists[hindex++]->Fill(M,fQC[4].Re()/M);
    fHists[hindex++]->Fill(M,fQC[4].Im()/M);
    fHists[hindex++]->Fill(M,fQC[6].Re()/M);
    fHists[hindex++]->Fill(M,fQC[6].Im()/M);
    hindex=230;
    fHists[hindex++]->Fill(M,(Q22-M)/M2);
    if (M>3) {
      Double_t qc42tmp = (Q22*Q22+Q42-2*Q42re-4*(M-2)*Q22+2*M*(M-3));
      fHists[hindex]->Fill(M,qc42tmp/M4);
    }
    hindex++;
    if (M>5) {
      Double_t qc6tmp = Q22*Q22*Q22 + 9*Q42*Q22 - 6*Q6are 
	              + 4*Q6bre - 12*Q6cre 
                      + 18*(M-4)*Q42re + 4*Q62
                      - 9*(M-4)*Q22*Q22 - 9*(M-4)*Q42
                      + 18*(M-2)*(M-5)*Q22
                      - 6*M*(M-4)*(M-5);
      fHists[hindex]->Fill(M,qc6tmp/M/(M-1)/(M-2)/(M-3)/(M-4)/(M-5));
    }
    hindex++;
    fHists[hindex++]->Fill(M,(Q32-M)/M2);
    if (M>3) {
      Double_t qc43tmp = (Q32*Q32+Q62-2*Q32re-4*(M-2)*Q32+2*M*(M-3));
      fHists[hindex]->Fill(M,qc43tmp/M4);
    }
  }

  if (!fDoCharge)
    return;

  Int_t Mpp=0,Mnn=0;
  TComplex qcpp[7],qcnn[7]; 
  qcpp[2]=0;qcpp[3]=0;qcpp[4]=0,qcpp[5]=0,qcpp[6]=0;
  qcnn[2]=0;qcnn[3]=0;qcnn[4]=0,qcnn[5]=0,qcnn[6]=0;
  for (Int_t i=0; i<fM; ++i) {
    CPart &p = fParts.at(i);
    Double_t phi=p.Phi();
    Short_t c=p.Charge();
    if (c==0)
      continue;
    if (c>0) {
      ++Mpp;
      for (Int_t k=2; k<5; ++k)
	qcpp[k] += TComplex(TMath::Cos(k*phi),TMath::Sin(k*phi));
    } else {
      ++Mnn;
      for (Int_t k=2; k<5; ++k)
	qcnn[k] += TComplex(TMath::Cos(k*phi),TMath::Sin(k*phi));
    }
  }

  Double_t Q22pp   = qcpp[2].Rho2();
  Double_t Q32pp   = qcpp[3].Rho2();
  Double_t Q42pp   = qcpp[4].Rho2();
  Double_t Q62pp   = qcpp[6].Rho2();
  Double_t Q32ppre = (qcpp[6]*TComplex::Power(TComplex::Conjugate(qcpp[3]),2)).Re();
  Double_t Q42ppre = (qcpp[4]*TComplex::Power(TComplex::Conjugate(qcpp[2]),2)).Re();
  Double_t Q22nn   = qcnn[2].Rho2();
  Double_t Q32nn   = qcnn[3].Rho2();
  Double_t Q42nn   = qcnn[4].Rho2();
  Double_t Q62nn   = qcnn[6].Rho2();
  Double_t Q32nnre = (qcnn[6]*TComplex::Power(TComplex::Conjugate(qcnn[3]),2)).Re();
  Double_t Q42nnre = (qcnn[4]*TComplex::Power(TComplex::Conjugate(qcnn[2]),2)).Re();

  Int_t hindex=260;
  const Int_t sind=280;
  fHists[260]->Fill(Mpp);
  fHists[261]->Fill(Mnn);
  Int_t Mpp2 = Mpp*(Mpp-1);
  Int_t Mpp4 = Mpp*(Mpp-1)*(Mpp-2)*(Mpp-3);
  Int_t Mnn2 = Mnn*(Mnn-1);
  Int_t Mnn4 = Mnn*(Mnn-1)*(Mnn-2)*(Mnn-3);
  if (Mpp>1) {
    fHists[262]->Fill(Mpp,(Q22pp-Mpp)/Mpp2);
    fHists[280]->Fill(M,(Q22pp-Mpp)/Mpp2);
    fHists[263]->Fill(Mpp,(Q32pp-Mpp)/Mpp2);
    fHists[282]->Fill(M,(Q32pp-Mpp)/Mpp2);
    if (Mpp>3) {
      Double_t qc4tmp = (Q22pp*Q22pp+Q42pp-2*Q42ppre-4*(Mpp-2)*Q22pp+2*Mpp*(Mpp-3));
      fHists[264]->Fill(Mpp,qc4tmp/Mpp4);
      fHists[281]->Fill(M,qc4tmp/Mpp4);
      Double_t qc43tmp = (Q32pp*Q32pp+Q62pp-2*Q32ppre-4*(Mpp-2)*Q32pp+2*Mpp*(Mpp-3));
      fHists[265]->Fill(Mpp,qc43tmp/Mpp4);
      fHists[283]->Fill(M,qc43tmp/Mpp4);
    }
  }
  if (Mnn>1) {
    fHists[266]->Fill(Mnn,(Q22nn-Mnn)/Mnn2);
    fHists[280]->Fill(M,(Q22nn-Mnn)/Mnn2);
    fHists[267]->Fill(Mnn,(Q32nn-Mnn)/Mpp2);
    fHists[282]->Fill(M,(Q32pp-Mpp)/Mpp2);
    if (Mnn>3) {
      Double_t qc4tmp = (Q22nn*Q22nn+Q42nn-2*Q42nnre-4*(Mnn-2)*Q22nn+2*Mnn*(Mnn-3));
      fHists[268]->Fill(Mnn,qc4tmp/Mnn4);
      fHists[281]->Fill(M,qc4tmp/Mnn4);
      Double_t qc43tmp = (Q32nn*Q32nn+Q62nn-2*Q32nnre-4*(Mnn-2)*Q32nn+2*Mnn*(Mnn-3));
      fHists[269]->Fill(Mnn,qc43tmp/Mnn4);
      fHists[283]->Fill(M,qc43tmp/Mnn4);
    }
  }
}

void Cumulants::RunQC4with4NL()
{
  // Run QC method with 4 nested loops
  if (fM>fMaxNL4)
    return;

  Double_t er[6] = {0};
  Double_t nc[6] = {0};

  for (Int_t i1=0; i1<fM; ++i1) {
    CPart &p1 = fParts.at(i1);
    Double_t eta1=p1.Eta();
    Double_t phi1=p1.Phi();
    for (Int_t i2=0; i2<fM; ++i2) {
      if (i2==i1)
	continue;
      CPart &p2 = fParts.at(i2);
      Double_t eta2=p2.Eta();
      Double_t eta12=TMath::Abs(eta1-eta2);
      if (eta12<fEGminNL4)
	continue;
      Double_t phi2=p2.Phi();
      for (Int_t i3=0; i3<fM; ++i3) {
	if (i3==i2)
	  continue;
	if (i3==i1)
	  continue;
	CPart &p3 = fParts.at(i3);
	Double_t eta3=p3.Eta();
	Double_t eta13=TMath::Abs(eta1-eta3);
	if (eta13<fEGminNL4)
	  continue;
	Double_t eta23=TMath::Abs(eta2-eta3);
	if (eta23<fEGminNL4)
	  continue;
	Double_t phi3=p3.Phi();
	for (Int_t i4=0; i4<fM; ++i4) {
	  if (i4==i3)
	    continue;
	  if (i4==i2)
	    continue;
	  if (i4==i1)
	    continue;
	  CPart &p4 = fParts.at(i4);
	  Double_t eta4=p4.Eta();
	  Double_t phi4=p4.Phi();
	  Double_t eta14=TMath::Abs(eta1-eta4);
	  Double_t eta24=TMath::Abs(eta2-eta4);
	  Double_t eta34=TMath::Abs(eta3-eta4);
	  if (eta14<fEGminNL4)
	    continue;
	  if (eta24<fEGminNL4)
	    continue;
	  if (eta23<fEGminNL4)
	    continue;
	  Double_t arg=(phi1+phi2-phi3-phi4);
	  Double_t val = TMath::Cos(2*arg);;
	  for (Int_t eg=0;eg<6;++eg) {
	    Double_t etagap=Double_t(eg)/10;
	    if ((eta12>etagap)&&(eta13>etagap)&&(eta14>etagap)&&
		(eta23>etagap)&&(eta24>etagap)&&(eta34>etagap)) {
	      er[eg] += val;
	      nc[eg] += 1;
	    }
	  }
	}
      }
    }
  }

  Int_t hindex=300;
  for (Int_t eg=0;eg<6;++eg) {
    Int_t n=nc[eg];
    if (n<=0)
      continue;
    Double_t val = er[eg]/n;
    fHists[hindex+eg]->Fill(fM,val);
  }
}

void Cumulants::RunQC4with3NL()
{
  // Run QC method with 3 nested loops (from Dhevan's thesis with 7.19 corrected)
  if (fM>fMaxNL3)
    return;

  TComplex nq2[6],nq3[6],nq4[6];
  Int_t np[6]={0}, ns[6]={0};
  for (Int_t i1=0; i1<fM; ++i1) {
    CPart &p1 = fParts.at(i1);
    Double_t eta1=p1.Eta();
    Double_t phi1=p1.Phi();
    for (Int_t i2=0; i2<fM; ++i2) {
      if (i1==i2)
	continue;
      CPart &p2 = fParts.at(i2);
      Double_t eta2=p2.Eta();
      Double_t eta12=TMath::Abs(eta1-eta2);
      if (eta12<fEGminNL3)
	continue;
      Double_t phi2=p2.Phi();
      Double_t phi12=phi1-phi2;
      TComplex v2(TMath::Cos(2*phi12),TMath::Sin(2*phi12));
      TComplex v4(1+TMath::Cos(4*phi12),TMath::Sin(4*phi12));
      for (Int_t eg=0;eg<6;++eg) {
	Double_t etagap=Double_t(eg)/10;
	if (eta12>etagap) {
	  nq2[eg] += v2;
	  nq4[eg] += v4;
	  np[eg]  += 1;
	  ns[eg]  += 2;
	}
      }
      for (Int_t i3=0; i3<fM; ++i3) {
	if (i1==i3)
	  continue;
	if (i2==i3)
	  continue;
	CPart &p3 = fParts.at(i3);
	Double_t eta3=p3.Eta();
	Double_t phi3=p3.Phi();
	Double_t eta13=TMath::Abs(eta1-eta3);
	if (eta13<fEGminNL3)
	  continue;
	Double_t eta23=TMath::Abs(eta2-eta3);
	if (eta23<fEGminNL3)
	  continue;
	Double_t phi13=phi1-phi3;
	Double_t phi23=phi2-phi3;
	Double_t t1=2*TMath::Cos(2*(phi12+phi13))+2*TMath::Cos(2*(phi12-phi13));
	Double_t t2=2*TMath::Sin(2*(phi12+phi13))+2*TMath::Sin(2*(phi12-phi13));
	TComplex val(t1,t2);
	for (Int_t eg=0;eg<6;++eg) {
	  Double_t etagap=Double_t(eg)/10;
	  if ((eta13>etagap)&&(eta23>etagap)) {
	    nq3[eg] += val;
	    ns[eg]  += 4;
	  }
	}
      }
    }
  }

  Int_t hindex=350;
  for (Int_t eg=0;eg<6;++eg) {
    Int_t n=np[eg]*np[eg]-ns[eg];
    if (n<=0)
      continue;
    Double_t val=TComplex(nq2[eg]*nq2[eg]-nq4[eg]-nq3[eg]).Re()/n;
    fHists[hindex+eg]->Fill(fM,val);
  }
}

void Cumulants::RunQC4withEG()
{
}

void Cumulants::SetTracks(TObjArray &trks, Bool_t doKinCuts)
{
  // Set tracks from array, and do kinematic cuts if required.
  fM    = 0;
  Int_t Mall = 0;
  Int_t Mnoc = 0;

  const Int_t en = trks.GetEntries();
  for (Int_t i=0;i<en;++i) {
    CPart part;
    AliVParticle *pav = dynamic_cast<AliVParticle*>(trks.At(i));
    if (pav) {
      part = CPart(*pav);
    } else {
      TParticle *par = dynamic_cast<TParticle*>(trks.At(i));
      if (par) {
	part = CPart(*par);
      } else {
	::Error("Cumulants::SetTracks","Type of particle not known!");
	continue;
      }
    }

    ++Mnoc;
    if (doKinCuts) {
      if (part.Eta()<fEtaMin)
	continue;
      if (part.Eta()>fEtaMax)
	continue;
      ++Mall;
      if (part.Pt()<fPtMin)
	continue;
      if (part.Pt()>fPtMax)
	continue;
    }
    fParts[fM]=part;
    ++fM;
  }
  fHists[0]->Fill(Mnoc);
  fHists[1]->Fill(Mall);
  fHists[2]->Fill(fM);
}



#if 0
    if (M<70) {
      cout << "\n------> Starting for M = " << M << endl;
      TStopwatch st;
      st.Start();
      for (Int_t i1=0; i1<M; ++i1) {
	TParticle *track1 = static_cast<TParticle*>(trs.At(i1));
	Double_t eta1=track1->Eta();
	Double_t phi1=track1->Phi();
	for (Int_t i2=0; i2<M; ++i2) {
	  if (i2==i1)
	    continue;
	  TParticle *track2 = static_cast<TParticle*>(trs.At(i2));
	  Double_t eta2=track2->Eta();
	  Double_t phi2=track2->Phi();
	  Double_t eta12=TMath::Abs(eta1-eta2);
	  for (Int_t i3=0; i3<M; ++i3) {
	    if (i3==i2)
	      continue;
	    if (i3==i1)
	      continue;
	    TParticle *track3 = static_cast<TParticle*>(trs.At(i3));
	    Double_t eta3=track3->Eta();
	    Double_t phi3=track3->Phi();
	    Double_t eta13=TMath::Abs(eta1-eta3);
	    Double_t eta23=TMath::Abs(eta2-eta3);
	    for (Int_t i4=0; i4<M; ++i4) {
	      if (i4==i3)
		continue;
	      if (i4==i2)
		continue;
	      if (i4==i1)
		continue;
	      TParticle *track4 = static_cast<TParticle*>(trs.At(i4));
	      Double_t eta4=track4->Eta();
	      Double_t phi4=track4->Phi();
	      Double_t eta14=TMath::Abs(eta1-eta4);
	      Double_t eta24=TMath::Abs(eta2-eta4);
	      Double_t eta34=TMath::Abs(eta3-eta4);
	      Double_t arg=(phi1+phi2-phi3-phi4);
	      er += TMath::Cos(2*arg);
	      ei += TMath::Sin(2*arg);
	      ++nn;
	      if ((eta12>0.1)&&(eta13>0.1)&&(eta14>0.1)&&
		  (eta23>0.1)&&(eta24>0.1)&&(eta34>0.1)) {
		er1 += TMath::Cos(2*arg);
		ei1 += TMath::Sin(2*arg);
		++nn1;
	      }
	      if ((eta12>0.2)&&(eta13>0.2)&&(eta14>0.2)&&
		  (eta23>0.2)&&(eta24>0.2)&&(eta34>0.2)) {
		er2 += TMath::Cos(2*arg);
		ei2 += TMath::Sin(2*arg);
		++nn2;
	      }
	    }
	  }
	}
      }
      if (nn>0) {
	std::cout << "nn " << nn << endl;
	er/=nn;
	ei/=nn;
	fHists[160]->Fill(M,er);
	fHists[163]->Fill(M,ei);
      }
      if (nn1>0){
	er1/=nn1;
	ei1/=nn1;
	fHists[161]->Fill(M,er1);
	fHists[164]->Fill(M,ei1);
      }
      if (nn2>0) {
	er2/=nn2;
	ei2/=nn2;
	fHists[162]->Fill(M,er2);
	fHists[165]->Fill(M,ei2);
      }
      st.Stop();
      st.Print();
    }
#endif
