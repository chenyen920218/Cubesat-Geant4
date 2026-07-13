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
/// \file B2aDetectorConstruction.hh
/// \brief Definition of the B2aDetectorConstruction class

#ifndef B2aDetectorConstruction_h
#define B2aDetectorConstruction_h 1

#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "tls.hh"
#include "G4Material.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
//class G4Material;
class G4UserLimits;
class G4GlobalMagFieldMessenger;

class B2aDetectorMessenger;

/// Detector construction class to define materials, geometry
/// and global uniform magnetic field.

class B2aDetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    B2aDetectorConstruction();
    virtual ~B2aDetectorConstruction();

  public:
    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField();

    G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }
    // Set methods
    // void SetShellMaterial (G4String );
    void SetMaxStep (G4double );
    void SetCheckOverlaps(G4bool );
    G4String GetShellMaterial() const { return fShellMaterial->GetName();}

  private:
    // methods
    void DefineMaterials();
    G4VPhysicalVolume* DefineVolumes();

    G4Material* Al6061;
    G4Material* TitanAlloy;
    G4Material* Scell;
    G4Material* coverGlass;
    G4Material*        fShellMaterial;

    G4UserLimits* fStepLimit;            // pointer to user step limits

    B2aDetectorMessenger*  fMessenger;   // messenger

    static G4ThreadLocal G4GlobalMagFieldMessenger*  fMagFieldMessenger;
                                         // magnetic field messenger

    G4bool  fCheckOverlaps; // option to activate checking of volumes overlaps
  protected:
    G4LogicalVolume* fScoringVolume;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
