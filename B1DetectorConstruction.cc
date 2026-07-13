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
// $Id: B1DetectorConstruction.cc 94307 2015-11-11 13:42:46Z gcosmo $
//
/// \file B1DetectorConstruction.cc
/// \brief Implementation of the B1DetectorConstruction class

#include "B1DetectorConstruction.hh"

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


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolume(0)
{ }
B1DetectorConstruction::~B1DetectorConstruction()
{ }

G4VPhysicalVolume* B1DetectorConstruction::Construct()
{
  // Define materials
  DefineMaterials();

  // Define volumes
  return DefineVolumes();
}//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void B1DetectorConstruction::DefineMaterials()
{
    // Material definition
    
    G4NistManager* nistManager = G4NistManager::Instance();
    
    // Air defined using NIST Manager
    nistManager->FindOrBuildMaterial("G4_AIR");

    // Lead defined using NIST Manager
    fShellMaterial  = nistManager->FindOrBuildMaterial("G4_KEVLAR");

    // Elements
    G4Element* elementMg = new G4Element("Magnesium", "Mg", 12., 24.3050*g/mole);
    G4Element* elementAl = new G4Element("Aluminium", "Al", 13., 26.9815*g/mole);
    G4Element* elementSi = new G4Element("Silicon",   "Si", 14., 28.0855*g/mole);
    G4Element* elementFe = new G4Element("Iron",      "Fe", 26., 55.8450*g/mole);
    G4Element* elementO  = new G4Element("Oxygen",    "O" , 8.,  15.9994*g/mole);
    G4Element* elN = nistManager->FindOrBuildElement("N");
    G4Element* elC = nistManager->FindOrBuildElement("C");
    G4Element* elH = nistManager->FindOrBuildElement("H");
    G4Element* elTi = nistManager->FindOrBuildElement("Ti");
    G4Element* elV = nistManager->FindOrBuildElement("V");

    // Titanium alloy Grade 5 R56400
    TitanAlloy = new G4Material("TitanAlloy", 4.43*g/cm3, 8);
    TitanAlloy->AddElement(elTi, 89.405*perCent);
    TitanAlloy->AddElement(elC, 0.08*perCent);
    TitanAlloy->AddElement(elN, 0.05*perCent);
    TitanAlloy->AddElement(elementO, 0.2*perCent);
    TitanAlloy->AddElement(elementFe, 0.25*perCent);
    TitanAlloy->AddElement(elH, 0.015*perCent);
    TitanAlloy->AddElement(elV, 4.*perCent);
    TitanAlloy->AddElement(elementAl, 6.*perCent);

    // Aluminium alloy 6061-T6
    Al6061 = new G4Material("Al6061", 2.70*g/cm3, 4);
    Al6061->AddElement(elementAl, 0.980);
    Al6061->AddElement(elementMg, 0.010);
    Al6061->AddElement(elementSi, 0.006);
    Al6061->AddElement(elementFe, 0.004);

    // Solar Cells
    Scell = new G4Material("Scell",   7.82*g/cm3, 1);
    Scell->AddElement(elementSi, 1.00);

    // Cover glass
    coverGlass = new G4Material("Glass", 2.5*g/cm3, 2);
    coverGlass->AddElement(elementSi, 1);
    coverGlass->AddElement(elementO, 2);
    // Print materials
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B1DetectorConstruction::DefineVolumes()
{
    G4NistManager* nist = G4NistManager::Instance();
    // Define colour attributes
    G4VisAttributes* cyan_vat = new G4VisAttributes(G4Colour::Cyan());
    G4VisAttributes* lblue_vat = new G4VisAttributes(G4Colour(0.,0.,0.75));
    G4VisAttributes* green_vat = new G4VisAttributes(G4Colour::Green());
    G4VisAttributes* sol_red_vat = new G4VisAttributes(G4Colour(1.,0.,0.,0.1));
    sol_red_vat->SetForceSolid(true);
    G4VisAttributes* sol_green_vat = new G4VisAttributes(G4Colour::Green());
    sol_green_vat->SetForceSolid(true);
  // Envelope parameters
  G4double sat_height = 30.*cm;      // CubeSat height
  G4double sat_width = 20.*cm;       // CubeSat width
  G4double sat_length = 10.*cm;      // CubeSat length
  G4double coverGlass_thi = 0.1*mm;  // Cover glass thickness
  G4double cell_thi = 0.15*mm;       // Solar cell thickness
  G4double pcb_thi = 1.6*mm;         // PCB thickness



  G4bool checkOverlaps = true;

  // World
    G4double worldLength = 100.0*cm;
    G4double a, z;
    G4String name;
    G4double density      = universe_mean_density;
    G4double pressure     = 1.e-19*pascal;
    G4double temperature  = 0.1*kelvin;
    G4Material* vacuum = new G4Material(name = "Galactic", z=1., a=1.01*g/mole, density,
                                        kStateGas, temperature, pressure);
    G4Box* solidWorld
      = new G4Box("world",                                    //its name
                  worldLength/2,worldLength/2,worldLength/2); //its size
    G4LogicalVolume* logicWorld
      = new G4LogicalVolume(
                   solidWorld,   //its solid
                   vacuum,      //its material
                   "World"); //its name

    //  Must place the World Physical volume unrotated at (0,0,0).
    //
    G4VPhysicalVolume* physWorld
      = new G4PVPlacement(
                   0,               // no rotation
                   G4ThreeVector(), // at (0,0,0)
                   logicWorld,         // its logical volume
                   "World",         // its name
                   0,               // its mother  volume
                   false,           // no boolean operations
                   0,               // copy number
                   checkOverlaps); // checking overlaps
  // CubeSat components
  G4Box* satOuter = new G4Box("Sat_outer", 0.5 * sat_length, 0.5 * sat_width, 0.5 * sat_height);
  G4Box* pcbOuter = new G4Box("PCB_outer", 0.5 * (sat_length + pcb_thi), 0.5 * (sat_width + pcb_thi), 0.5 * (sat_height + pcb_thi));
  G4Box* cellOuter = new G4Box("Cell_outer", 0.5 * (sat_length + pcb_thi + cell_thi), 0.5 * (sat_width + pcb_thi + cell_thi), 0.5 * (sat_height + pcb_thi + cell_thi));
  G4Box* glassOuter = new G4Box("Glass_outer", 0.5 * (sat_length + pcb_thi + cell_thi + coverGlass_thi), 0.5 * (sat_width + pcb_thi + cell_thi + coverGlass_thi), 0.5 * (sat_height + pcb_thi + cell_thi + coverGlass_thi));

  G4SubtractionSolid* pcbS = new G4SubtractionSolid("PCB", pcbOuter, satOuter);
  G4LogicalVolume* pcbLV = new G4LogicalVolume(pcbS, Al6061, "PCB");
  new G4PVPlacement(0, G4ThreeVector(), pcbLV, "PCB", logicWorld, false, 0, checkOverlaps);
    pcbLV->SetVisAttributes(cyan_vat);
  
  G4SubtractionSolid* cellS = new G4SubtractionSolid("Cell", cellOuter, pcbOuter);
  G4LogicalVolume* cellLV = new G4LogicalVolume(cellS, Scell, "Cell");
  new G4PVPlacement(0, G4ThreeVector(), cellLV, "Cell", logicWorld, false, 0, checkOverlaps);
    cellLV->SetVisAttributes(green_vat);
  
  G4SubtractionSolid* glassS = new G4SubtractionSolid("Glass", glassOuter, cellOuter);
  G4LogicalVolume* glassLV = new G4LogicalVolume(glassS, coverGlass, "Glass");
  new G4PVPlacement(0, G4ThreeVector(), glassLV, "Glass", logicWorld, false, 0, checkOverlaps);
  glassLV->SetVisAttributes(lblue_vat);
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

    elecBoardLV->SetVisAttributes(sol_green_vat);

    fScoringVolume = elecBoardLV;
    //
    G4RotationMatrix* rotationMatrix1 = new G4RotationMatrix();
    rotationMatrix1->rotateX(90.*deg);
    G4double elecBox_width = 96.*mm;
    G4double elecBox_height = 3.*cm;
    G4double elecBox_thi = 2.*mm;
    G4Transform3D transform4 = G4Transform3D(*rotationMatrix1, G4ThreeVector(0., -83., 100.*mm));
    G4Box* elecBoxOuter = new G4Box("Elec_box_outer", 0.5*elecBox_width, 0.5*elecBox_width,
                                0.5*elecBox_height);
    G4Box* elecBoxInner = new G4Box("Elec_box_innter", 0.5*elecBox_width-elecBox_thi,
                                0.5*elecBox_width-elecBox_thi,0.5*elecBox_height-elecBox_thi);
    G4SubtractionSolid* elecBoxS = new G4SubtractionSolid("Elec_box",
                                                          elecBoxOuter, elecBoxInner);
    // G4LogicalVolume* elecBoxLV = new G4LogicalVolume(elecBoxS, Al6061, "Elec_box");
    G4LogicalVolume* elecBoxLV = new G4LogicalVolume(elecBoxS, TitanAlloy, "Elec_box");
    new G4PVPlacement(0, G4ThreeVector(0.,-55.,-37.*mm), elecBoxLV, "Elec_box0",
                      logicWorld, false, 0, checkOverlaps);
    new G4PVPlacement(transform4, elecBoxLV, "Elec_box1", logicWorld, false, 1, checkOverlaps);
    elecBoxLV->SetVisAttributes(sol_red_vat);
    fScoringVolume = elecBoxLV;
    //
    G4double startracker_x = 44.*mm;
    G4double startracker_y = 50.*mm;
    G4double startracker_z = 95.*mm;
    G4Box* startracker = new G4Box("startracker", 0.5*startracker_x, 0.5*startracker_y, 0.5*startracker_z);
    G4LogicalVolume* startrackerLV = new G4LogicalVolume(startracker, Scell, "startracker");
    new G4PVPlacement(0, G4ThreeVector(-2., 30., -68.*mm), startrackerLV, "startracker0", logicWorld, false, 0, checkOverlaps);
    startrackerLV->SetVisAttributes(sol_green_vat);
    fScoringVolume = startrackerLV;
    // Cylinder

    G4double cylinder_innerRadius = 0. * mm;
    G4double cylinder_outerRadius = 35. * mm;
    G4double cylinder_halfHeight = 48. * mm;
    G4double cylinder_startAngle = 0. * deg;
    G4double cylinder_spanningAngle = 360.0 * deg;

    G4Tubs* cylinderSolid = new G4Tubs("Cylinder", cylinder_innerRadius, cylinder_outerRadius,
      cylinder_halfHeight, cylinder_startAngle, cylinder_spanningAngle);

    G4LogicalVolume* cylinderLV = new G4LogicalVolume(cylinderSolid, Al6061, "CylinderLV");

    new G4PVPlacement(0, G4ThreeVector(0., -55., -100. * mm), cylinderLV, "CylinderPhysical", logicWorld, false, 0, checkOverlaps);

    cylinderLV->SetVisAttributes(sol_green_vat);

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

    solar1LV->SetVisAttributes(sol_green_vat);
    solar2LV->SetVisAttributes(sol_green_vat);

    fScoringVolume = solar1LV; // You can set this to one of the solar panels

    return physWorld;
    }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
