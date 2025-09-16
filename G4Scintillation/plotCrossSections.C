void plotCrossSections()
{
  // Open the ROOT file
  TFile *file = TFile::Open("myhistos.root");
  if (!file || file->IsZombie()) {
    std::cerr << "Error opening file myhistos.root" << std::endl;
    return;
  }

  // Get the ntuple
  TNtuple *ntuple = (TNtuple*)file->Get("CrossSection");
  if (!ntuple) {
    std::cerr << "Ntuple CrossSection not found in file" << std::endl;
    return;
  }

  // Prepare vectors to hold data
  std::vector<double> energy;
  std::vector<double> photo;
  std::vector<double> compton;
  std::vector<double> rayleigh;
  std::vector<double> pairProd;

  float e, p, c, r, pa;

  // Loop over all entries
  Long64_t nEntries = ntuple->GetEntries();
  for (Long64_t i = 0; i < nEntries; ++i) {
    ntuple->GetEntry(i);
    // The ntuple columns order is: Energy_MeV, Photo, Compton, Pair
    // You need to add Rayleigh manually if it's stored; otherwise zero
    e = ntuple->GetLeaf("Energy_MeV")->GetValue();
    p = ntuple->GetLeaf("Photo")->GetValue();
    c = ntuple->GetLeaf("Compton")->GetValue();
    pa = ntuple->GetLeaf("Pair")->GetValue();

    // If you stored Rayleigh cross section separately, add this line accordingly.
    // Assuming you didn't, set zero or add a column in your ntuple.
    r = 0.0;

    energy.push_back(e);
    photo.push_back(p);
    compton.push_back(c);
    rayleigh.push_back(r);
    pairProd.push_back(pa);
  }

  // Convert vectors to arrays for TGraph
  int n = energy.size();
  double *x = &energy[0];

  // Create arrays for each cross section
  double *yPhoto = &photo[0];
  double *yCompton = &compton[0];
  double *yRayleigh = &rayleigh[0];
  double *yPair = &pairProd[0];

  // Create TGraphs for each
  TGraph *gPhoto = new TGraph(n, x, yPhoto);
  TGraph *gCompton = new TGraph(n, x, yCompton);
  TGraph *gRayleigh = new TGraph(n, x, yRayleigh);
  TGraph *gPair = new TGraph(n, x, yPair);

  // Setup graph styles
  gPhoto->SetMarkerStyle(20);
  gPhoto->SetMarkerColor(kRed);
  gPhoto->SetTitle("Photoelectric Cross Section");

  gCompton->SetMarkerStyle(21);
  gCompton->SetMarkerColor(kBlue);
  gCompton->SetTitle("Compton Cross Section");

  gRayleigh->SetMarkerStyle(22);
  gRayleigh->SetMarkerColor(kGreen+2);
  gRayleigh->SetTitle("Rayleigh Cross Section");

  gPair->SetMarkerStyle(23);
  gPair->SetMarkerColor(kMagenta);
  gPair->SetTitle("Pair Production Cross Section");

  // Create a canvas
  TCanvas *c1 = new TCanvas("c1", "Cross Sections", 800, 600);
  c1->SetLogx();
  c1->SetLogy();

  // Draw first graph, setting axis labels and limits
  gPhoto->SetTitle("Cross Sections vs Energy");
  gPhoto->GetXaxis()->SetTitle("Energy (MeV)");
  gPhoto->GetYaxis()->SetTitle("Cross Section (cm^{-1})");
  gPhoto->Draw("AP");  // A = axis, P = points

  // Draw the other graphs on the same canvas
  gCompton->Draw("P SAME");
  gRayleigh->Draw("P SAME");
  gPair->Draw("P SAME");

  // Add legend
  TLegend *legend = new TLegend(0.6,0.7,0.88,0.88);
  legend->AddEntry(gPhoto, "Photoelectric", "p");
  legend->AddEntry(gCompton, "Compton", "p");
  legend->AddEntry(gRayleigh, "Rayleigh", "p");
  legend->AddEntry(gPair, "Pair Production", "p");
  legend->Draw();

  // Update canvas to show everything
  c1->Update();
}
