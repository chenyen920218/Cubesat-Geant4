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
/// \file B1/src/DetectorConstruction.cc
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4NistManager.hh"
#include "G4SDManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Orb.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4AutoDelete.hh"
#include "G4PhysicalConstants.hh"

#include "G4GeometryTolerance.hh"
#include "G4GeometryManager.hh"

#include "G4UserLimits.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4SystemOfUnits.hh"

namespace B1 {

// ... (other code)

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  // Envelope parameters
  G4double sat_height = 30.*cm;      // CubeSat height
  G4double sat_width = 20.*cm;       // CubeSat width
  G4double sat_length = 10.*cm;      // CubeSat length
  G4double coverGlass_thi = 0.1*mm;  // Cover glass thickness
  G4double cell_thi = 0.15*mm;       // Solar cell thickness
  G4double pcb_thi = 1.6*mm;         // PCB thickness

  G4Material* Al6061 = nist->FindOrBuildMaterial("G4_Al");
  G4Material* Scell = nist->FindOrBuildMaterial("G4_Si");
  G4Material* coverGlass = nist->FindOrBuildMaterial("G4_GLASS_PLATE");
  G4Material* TitanAlloy = nist->FindOrBuildMaterial("G4_TITANIUM_DIOXIDE");

  G4bool checkOverlaps = true;

  // World
  G4double world_sizeXY = 1.2 * sat_width; // Adjust the size to fit the CubeSat
  G4double world_sizeZ = 1.2 * sat_height; // Adjust the size to fit the CubeSat
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");

  auto solidWorld = new G4Box("World", 0.5 * world_sizeXY, 0.5 * world_sizeXY, 0.5 * world_sizeZ);
  auto logicWorld = new G4LogicalVolume(solidWorld, world_mat, "World");
  auto physWorld = new G4PVPlacement(nullptr, G4ThreeVector(), logicWorld, "World", nullptr, false, 0, checkOverlaps);

  // CubeSat components
  G4Box* satOuter = new G4Box("Sat_outer", 0.5 * sat_length, 0.5 * sat_width, 0.5 * sat_height);
  G4Box* pcbOuter = new G4Box("PCB_outer", 0.5 * (sat_length + pcb_thi), 0.5 * (sat_width + pcb_thi), 0.5 * (sat_height + pcb_thi));
  G4Box* cellOuter = new G4Box("Cell_outer", 0.5 * (sat_length + pcb_thi + cell_thi), 0.5 * (sat_width + pcb_thi + cell_thi), 0.5 * (sat_height + pcb_thi + cell_thi));
  G4Box* glassOuter = new G4Box("Glass_outer", 0.5 * (sat_length + pcb_thi + cell_thi + coverGlass_thi), 0.5 * (sat_width + pcb_thi + cell_thi + coverGlass_thi), 0.5 * (sat_height + pcb_thi + cell_thi + coverGlass_thi));

  G4SubtractionSolid* pcbS = new G4SubtractionSolid("PCB", pcbOuter, satOuter);
  G4LogicalVolume* pcbLV = new G4LogicalVolume(pcbS, Al6061, "PCB");
  new G4PVPlacement(0, G4ThreeVector(), pcbLV, "PCB", logicWorld, false, 0, checkOverlaps);
  pcbLV->SetVisAttributes(G4VisAttributes(G4Colour::Cyan()));
  
  G4SubtractionSolid* cellS = new G4SubtractionSolid("Cell", cellOuter, pcbOuter);
  G4LogicalVolume* cellLV = new G4LogicalVolume(cellS, Scell, "Cell");
  new G4PVPlacement(0, G4ThreeVector(), cellLV, "Cell", logicWorld, false, 0, checkOverlaps);
  cellLV->SetVisAttributes(G4VisAttributes(G4Colour::Green()));
  
  G4SubtractionSolid* glassS = new G4SubtractionSolid("Glass", glassOuter, cellOuter);
  G4LogicalVolume* glassLV = new G4LogicalVolume(glassS, coverGlass, "Glass");
  new G4PVPlacement(0, G4ThreeVector(), glassLV, "Glass", logicWorld, false, 0, checkOverlaps);
  glassLV->SetVisAttributes(G4VisAttributes(G4Colour::LightBlue()));
    // ... (previous code)

    // Electronics box and PCB
    G4double elecBoard_thi = 2. * mm;
    G4double elecBoard_size = 96. * mm;

    G4RotationMatrix* rotationMatrix = new G4RotationMatrix();
    rotationMatrix->rotateZ(90. * deg);

    G4Transform3D transform1 = G4Transform3D(*rotationMatrix, G4ThreeVector(40., 41., 100. * mm));
    G4Transform3D transform2 = G4Transform3D(*rotationMatrix, G4ThreeVector(-25., 41., 100. * mm));
    G4Transform3D transform3 = G4Transform3D(*rotationMatrix, G4ThreeVector(-40., 41., 100. * mm));
    G4Transform3D transform5 = G4Transform3D(*rotationMatrix, G4ThreeVector(40., 41., -68. * mm));

    G4Box* elecBoardS = new G4Box("Elec_board", 0.5 * elecBoard_size, 0.5 * elecBoard_thi, 0.5 * elecBoard_size);
    G4LogicalVolume* elecBoardLV = new G4LogicalVolume(elecBoardS, Scell, "Elec_board");

    new G4PVPlacement(0, G4ThreeVector(0., -47., 100. * mm), elecBoardLV, "Elec_board0", logicWorld, false, 0, checkOverlaps);
    new G4PVPlacement(0, G4ThreeVector(0., -56., 100. * mm), elecBoardLV, "Elec_board1", logicWorld, false, 1, checkOverlaps);
    new G4PVPlacement(0, G4ThreeVector(0., -65., 100. * mm), elecBoardLV, "Elec_board2", logicWorld, false, 2, checkOverlaps);
    new G4PVPlacement(transform1, elecBoardLV, "Elec_board3", logicWorld, false, 3, checkOverlaps);
    new G4PVPlacement(transform2, elecBoardLV, "Elec_board4", logicWorld, false, 4, checkOverlaps);
    new G4PVPlacement(transform3, elecBoardLV, "Elec_board5", logicWorld, false, 5, checkOverlaps);
    new G4PVPlacement(transform5, elecBoardLV, "Elec_board6", logicWorld, false, 6, checkOverlaps);

    elecBoardLV->SetVisAttributes(G4VisAttributes(G4Colour::Green()));

    fScoringVolume = elecBoardLV;

    // Cylinder
    G4RotationMatrix* rotationMatrix1 = new G4RotationMatrix();
    rotationMatrix1->rotateX(90. * deg);

    G4double cylinder_innerRadius = 0. * mm;
    G4double cylinder_outerRadius = 35. * mm;
    G4double cylinder_halfHeight = 48. * mm;
    G4double cylinder_startAngle = 0. * deg;
    G4double cylinder_spanningAngle = 360.0 * deg;

    G4Tubs* cylinderSolid = new G4Tubs("Cylinder", cylinder_innerRadius, cylinder_outerRadius,
      cylinder_halfHeight, cylinder_startAngle, cylinder_spanningAngle);

    G4LogicalVolume* cylinderLV = new G4LogicalVolume(cylinderSolid, Al6061, "CylinderLV");

    new G4PVPlacement(0, G4ThreeVector(0., -55., -100. * mm), cylinderLV, "CylinderPhysical", logicWorld, false, 0, checkOverlaps);

    cylinderLV->SetVisAttributes(G4VisAttributes(G4Colour::Green()));

    fScoringVolume = cylinderLV;

    // Solar Panels
    G4double solar_thi = 2. * mm;
    G4double solar1_length = 700. * mm;
    G4double solar1_width = 200. * mm;
    G4double solar2_length = 100. * mm;
    G4double solar2_width = 300. * mm;

    G4Transform3D transform6 = G4Transform3D(*rotationMatrix1, G4ThreeVector(0., 0., 151. * mm));
    G4Box* solar1S = new G4Box("solar", 0.5 * solar1_length, 0.5 * solar_thi, 0.5 * solar1_width);
    G4LogicalVolume* solar1LV = new G4LogicalVolume(solar1S, Scell, "solar1");

    G4Transform3D transform7 = G4Transform3D(*rotationMatrix1, G4ThreeVector(0., 250., 151. * mm));
    G4Box* solar2S = new G4Box("solar", 0.5 * solar2_length, 0.5 * solar_thi, 0.5 * solar2_width);
    G4LogicalVolume* solar2LV = new G4LogicalVolume(solar2S, Scell, "solar2");

    G4Transform3D transform8 = G4Transform3D(*rotationMatrix1, G4ThreeVector(0., -250., 151. * mm));
    new G4PVPlacement(transform6, solar1LV, "solar1", logicWorld, false, 1, checkOverlaps);
    new G4PVPlacement(transform7, solar2LV, "solar2", logicWorld, false, 2, checkOverlaps);
    new G4PVPlacement(transform8, solar2LV, "solar3", logicWorld, false, 3, checkOverlaps);

    solar1LV->SetVisAttributes(G4VisAttributes(G4Colour::Green()));
    solar2LV->SetVisAttributes(G4VisAttributes(G4Colour::Green()));

    fScoringVolume = solar1LV; // You can set this to one of the solar panels

    return physWorld;
    }

    // ... (rest of your code)

    } // namespace B1
