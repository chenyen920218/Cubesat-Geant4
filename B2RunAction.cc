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
/// \file B2RunAction.cc
/// \brief Implementation of the B2RunAction class

#include "B2RunAction.hh"
#include "B2PrimaryGeneratorAction.hh"
#include "B2aDetectorConstruction.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4AccumulableManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4GeneralParticleSource.hh"
#include "G4GenericMessenger.hh"

#include "Randomize.hh"
#include <time.h>

#include <fstream>
#include <iomanip>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B2RunAction::B2RunAction()
  : G4UserRunAction(),
    fMessenger(nullptr),
    fNormFactor(1./cm2),
    fArea(20.*cm2)
{
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  analysisManager->CreateH1("0", "Edep in first", 100, 0, 2000*MeV);
  analysisManager->CreateH1("1", "Edep in second", 100, 0, 2000*MeV);
  analysisManager->CreateH1("2", "Edep in third", 100, 0, 2000*MeV);
  // set printing event number per each 100 events
  //G4RunManager::GetRunManager()->SetPrintProgress(1000);
  const G4double milligray = 1.e-3*gray;
  const G4double microgray = 1.e-6*gray;
  const G4double nanogray  = 1.e-9*gray;
  const G4double picogray  = 1.e-12*gray;
  const G4double percm2 = 1./cm2;

  new G4UnitDefinition("milligray", "milliGy" , "Dose", milligray);
  new G4UnitDefinition("microgray", "microGy" , "Dose", microgray);
  new G4UnitDefinition("nanogray" , "nanoGy"  , "Dose", nanogray);
  new G4UnitDefinition("picogray" , "picoGy"  , "Dose", picogray);
  new G4UnitDefinition("percm2"  , "/cm2"   , "Fluence", percm2);

  // Register accumulable to the accumulable manager
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  for (G4int i = 0; i<3; i++) {
    accumulableManager->CreateAccumulable<G4double>("Edep" + std::to_string(i), 0.);
    accumulableManager->CreateAccumulable<G4double>("Edep2" + std::to_string(i), 0.);
  }
  // accumulableManager->RegisterAccumulable(fEdep);
  // accumulableManager->RegisterAccumulable(fEdep2);

  filename = G4String("CubeSat");
  DefineCommand();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B2RunAction::~B2RunAction()
{
  delete fMessenger;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B2RunAction::BeginOfRunAction(const G4Run*)
{
  auto analysisManager = G4AnalysisManager::Instance();

  G4String histoFileName = "CubeSat.root";
  analysisManager->OpenFile(histoFileName);


  //inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
  time_t systime = time(NULL);
  fseeds[0] = (long) systime;
  fseeds[1] = (long) (systime*G4UniformRand());
  CLHEP::HepRandom::setTheSeeds(fseeds);
  //CLHEP::HepRandom::showEngineStatus();

  // reset accumulables to their initial values
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Reset();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B2RunAction::EndOfRunAction(const G4Run* run)
{
  auto analysisManager = G4AnalysisManager::Instance();

  analysisManager->Write();
  analysisManager->CloseFile();

  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;

  // Merge accumulables
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Merge();

  // Compute dose = total energy deposit in a run and its variance
  //
  // G4double edep  = fEdep[0].GetValue();
  // G4double edep2 = fEdep2[0].GetValue();
  G4double edep[3];
  G4double edep2[3];
  G4double dose[3];
  G4double rmsDose[3];
  G4double rms[3];
  G4double rR[3];
  const B2aDetectorConstruction* detectorConstruction
    = static_cast<const B2aDetectorConstruction*>
    (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  G4double mass = detectorConstruction->GetScoringVolume()->GetMass();
  for (G4int i = 0; i<3; i++) {
    edep[i] = accumulableManager->GetAccumulable<G4double>
      ("Edep"+std::to_string(i))->GetValue();
    edep2[i] = accumulableManager->GetAccumulable<G4double>
      ("Edep2"+std::to_string(i))->GetValue();

    rms[i] = edep2[i] - edep[i]*edep[i]/nofEvents;
    if (rms[i] > 0.) rms[i] = std::sqrt(rms[i]); else rms[i] = 0.;
    rR[i] = rms[i]?rms[i]/edep[i]:0;
    dose[i] = edep[i]/mass;
    rmsDose[i] = rms[i]/mass;
  }


  // Run conditions
  //  note: There is no primary generator action object for "master"
  //        run manager for multi-threaded mode.
  // const B2PrimaryGeneratorAction* generatorAction
  //  = static_cast<const B2PrimaryGeneratorAction*>
  //    (G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
  // G4String runCondition;
  // if (generatorAction)
  // {
  //   const G4ParticleGun* particleGun = generatorAction->GetParticleGun();
  //   runCondition += particleGun->GetParticleDefinition()->GetParticleName();
  //   runCondition += " of ";
  //   G4double particleEnergy = particleGun->GetParticleEnergy();
  //   runCondition += G4BestUnit(particleEnergy,"Energy");
  // }

  // Print
  //
  if (IsMaster()) {
    G4cout
     << G4endl
     << "--------------------End of Global Run-----------------------";
  }
  else {
    G4cout
     << G4endl
     << "--------------------End of Local Run------------------------";
  }

  G4cout
     << G4endl
     << " The run consists of " << nofEvents << " " //<< runCondition
     << G4endl
     << " Cumulated dose per run, in scoring volume : " << G4endl
     << G4BestUnit(dose[0],"Dose") << " rms = " << G4BestUnit(rmsDose[0],"Dose")
     << " Relative error = "  << rR[0]*100 << "%" << G4endl
     << G4BestUnit(dose[1],"Dose") << " rms = " << G4BestUnit(rmsDose[1],"Dose")
     << " Relative error = "  << rR[1]*100 << "%" << G4endl
     << G4BestUnit(dose[2],"Dose") << " rms = " << G4BestUnit(rmsDose[2],"Dose")
     << " Relative error = "  << rR[2]*100 << "%" << G4endl
     << G4endl
     << "------------------------------------------------------------"
     << G4endl
     << G4endl;
  const B2PrimaryGeneratorAction* generatorAction
    = static_cast<const B2PrimaryGeneratorAction*>
    (G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());

  if (generatorAction) {
    B2RunAction* masterRunAction
      = (B2RunAction*)
      G4MTRunManager::GetMasterRunManager()->GetUserRunAction();
    const G4GeneralParticleSource* GPS = generatorAction->GetGPS();
    generatorAction->GetGPS();
    G4double radius = GPS->GetCurrentSource()->GetPosDist()
      ->GetRadius();
    masterRunAction->SetArea(radius?4*CLHEP::pi*radius*radius:fArea);
    masterRunAction->SetParticleName(GPS->GetParticleDefinition()->GetParticleName());
  }
  //G4UImanager::GetUIpointer()->ApplyCommand("/CubeSat/run/area 3");
  // G4UImanager::GetUIpointer()->ApplyCommand("/B2/area 4");
  if (IsMaster()) {
    std::ofstream outfile("./output/" + filename + ".out", std::ios::app);
    G4cout << "Write output to file: output/" << filename << ".out" << G4endl << G4endl
           << "------------------------------------------------------------"
           << G4endl << G4endl;
    outfile << "--------------------------------------------------------" << G4endl
            << fParticleName << G4endl
            << G4BestUnit(fNormFactor,"Fluence") << '\t' << G4BestUnit(fArea,"Surface") << G4endl
            << "Seed:\t" << *fseeds << "\t" << *(fseeds+1) << G4endl;
    for (G4int i = 0; i<3; i++) {
      outfile
        << "Dose:\t" << G4BestUnit(dose[i], "Dose") << G4endl
        << "rms:\t" << G4BestUnit(rmsDose[i],"Dose") << G4endl
        << "Relative error:\t" << rR[i]*100 << "%" << G4endl
        << "Normalistion:\t" << G4BestUnit(dose[i]/nofEvents*fNormFactor*fArea/4.,"Dose") << G4endl;
    }
    outfile
      << "--------------------------------------------------------"
      << G4endl << G4endl;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B2RunAction::AddEdep(G4double edep, G4int i)
{
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  *accumulableManager->GetAccumulable<G4double>("Edep"+ std::to_string(i)) += edep;
  *accumulableManager->GetAccumulable<G4double>("Edep2" + std::to_string(i)) += edep*edep;
  // fEdep  += edep;
  // fEdep2 += edep*edep;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B2RunAction::DefineCommand()
{
  fMessenger = new G4GenericMessenger(this, "/CubeSat/run/", "Run control");
  // auto& filenameCmd
  fMessenger->DeclareProperty("filename", filename);
  auto& fAreaCmd
    = fMessenger->DeclarePropertyWithUnit("area", "cm2", fArea);
  fAreaCmd.SetParameterName("area", true);
  fAreaCmd.SetRange("area>0.");
  fAreaCmd.SetDefaultValue("20.");
  auto& fNormFactorCmd
    = fMessenger->DeclarePropertyWithUnit("normFactor", "percm2", fNormFactor);
  fNormFactorCmd.SetParameterName("normFactor", true);
  fNormFactorCmd.SetRange("normFactor>=0");
  fNormFactorCmd.SetDefaultValue("1.");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
