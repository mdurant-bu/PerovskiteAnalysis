//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file optical/LXe/src/LXeRunAction.cc
/// \brief Implementation of the LXeRunAction class
//
//
#include "LXeRunAction.hh"

#include "LXeDetectorConstruction.hh"
#include "LXeHistoManager.hh"
#include "LXeRun.hh"

#include "G4EmCalculator.hh"
#include "G4Gamma.hh"
#include "G4NistManager.hh"
#include "G4RunManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeRunAction::LXeRunAction()
{
  // Book predefined histograms
  fHistoManager = new LXeHistoManager();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LXeRunAction::~LXeRunAction()
{
  delete fHistoManager;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4Run* LXeRunAction::GenerateRun()
{
  fRun = new LXeRun();
  return fRun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeRunAction::BeginOfRunAction(const G4Run*)
{
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetFileName("myhistos");
  if (analysisManager->IsActive()) {
    analysisManager->OpenFile("myhistos");
  }

  // Create ntuple for cross section
  analysisManager->CreateNtuple("CrossSection", "E, Photo, Compton, Pair");
  analysisManager->CreateNtupleDColumn("Energy_MeV");
  analysisManager->CreateNtupleDColumn("Photo");
  analysisManager->CreateNtupleDColumn("Compton");
  analysisManager->CreateNtupleDColumn("Rayleigh");
  analysisManager->CreateNtupleDColumn("Pair");
  analysisManager->FinishNtuple();

  auto detConstruction = static_cast<const LXeDetectorConstruction*>(
    G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  // Get the perovskite material from the detector construction
  G4Material* Material = detConstruction->GetPerMaterial();
  //G4Material* Material = detConstruction->GetLeadMaterial();
  //G4Material* Material = detConstruction->GetPVTMaterial();

  G4EmCalculator emCal;

  G4ParticleDefinition* particle = G4Gamma::GammaDefinition();

  const int nPoints = 100;

  G4double a; 
  G4double z;  
  G4double density;
  G4double units;

  std::vector<double> energies;
  std::vector<double> crossSections;


  double E_min = 0.01 * CLHEP::keV;
  double E_max = 100000 * CLHEP::MeV;
  G4cout << "Energy (MeV)\tPhotoelectric\tCompton\tPair production" << G4endl;
  for (int i = 0; i < nPoints; ++i) {
    double E = E_min * std::pow(E_max / E_min, double(i) / (nPoints - 1));  // log spacing

    // get total cross section per atom (photoelectric + Compton + pair)
    double sigma_photo = emCal.ComputeCrossSectionPerVolume(E, particle, "phot",  Material);
    double sigma_compt = emCal.ComputeCrossSectionPerVolume(E, particle, "compt", Material);
    double sigma_pair  = emCal.ComputeCrossSectionPerVolume(E, particle, "conv",  Material);
    double sigma_rayl  = emCal.ComputeCrossSectionPerVolume(E, particle, "Rayl",  Material);

    //double sigma_photo = emCal.ComputeCrossSectionPerAtom(E, particle, "phot",  z = 82, a = 207.2 * CLHEP::g / CLHEP::mole);
    //double sigma_compt = emCal.ComputeCrossSectionPerAtom(E, particle, "compt", z = 82, a = 207.2 * CLHEP::g / CLHEP::mole);
    //double sigma_pair  = emCal.ComputeCrossSectionPerAtom(E, particle, "conv",  z = 82, a = 207.2 * CLHEP::g / CLHEP::mole);
    //double sigma_rayl  = emCal.ComputeCrossSectionPerAtom(E, particle, "Rayl",  z = 82, a = 207.2 * CLHEP::g / CLHEP::mole);

    double sigma_total = sigma_photo + sigma_compt + sigma_pair + sigma_rayl;

    units = CLHEP::cm; // cm^-1 or barn = 1e-28 m^2
    analysisManager->FillNtupleDColumn(0, E / CLHEP::MeV);
    analysisManager->FillNtupleDColumn(1, sigma_photo / units);
    analysisManager->FillNtupleDColumn(2, sigma_compt / units);
    analysisManager->FillNtupleDColumn(3, sigma_rayl  / units);
    analysisManager->FillNtupleDColumn(4, sigma_pair  / units);
    analysisManager->AddNtupleRow();

    /*G4cout << E / CLHEP::MeV << " MeV \t"
           << sigma_photo / units << " cm^-1 \t"
           << sigma_rayl  / units << " cm^-1 \t"
           << sigma_compt / units << " cm^-1 \t"
           << sigma_pair  / units << " cm^-1" << G4endl;*/
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void LXeRunAction::EndOfRunAction(const G4Run*)
{
  if (isMaster) fRun->EndOfRun();

  // save histograms
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  if (analysisManager->IsActive()) {
    analysisManager->Write();
    analysisManager->CloseFile();
  }
}