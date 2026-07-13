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
/// \file B2aDetectorConstruction.cc
/// \brief Implementation of the B2aDetectorConstruction class

#include "B2aDetectorConstruction.hh"
#include "B2aDetectorMessenger.hh"
#include "B2TrackerSD.hh"

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

G4ThreadLocal
G4GlobalMagFieldMessenger* B2aDetectorConstruction::fMagFieldMessenger = 0;

B2aDetectorConstruction::B2aDetectorConstruction()
:G4VUserDetectorConstruction(),
 fShellMaterial(NULL),
 fStepLimit(NULL),
 fCheckOverlaps(true)
{
  fMessenger = new B2aDetectorMessenger(this);

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B2aDetectorConstruction::~B2aDetectorConstruction()
{
  delete fStepLimit;
  delete fMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B2aDetectorConstruction::Construct()
{
  // Define materials
  DefineMaterials();

  // Define volumes
  return DefineVolumes();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B2aDetectorConstruction::DefineMaterials()
{
  // Material definition

  G4NistManager* nistManager = G4NistManager::Instance();

  // Air defined using NIST Manager
  //nistManager->FindOrBuildMaterial("G4_AIR");

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

G4VPhysicalVolume* B2aDetectorConstruction::DefineVolumes()
{
  // Define colour attributes
  G4VisAttributes* cyan_vat = new G4VisAttributes(G4Colour::Cyan());
  G4VisAttributes* lblue_vat = new G4VisAttributes(G4Colour(0.,0.,0.75));
  G4VisAttributes* green_vat = new G4VisAttributes(G4Colour::Green());
  G4VisAttributes* sol_red_vat = new G4VisAttributes(G4Colour(1.,0.,0.,0.7));
  sol_red_vat->SetForceSolid(true);
  G4VisAttributes* sol_green_vat = new G4VisAttributes(G4Colour::Green());
  sol_green_vat->SetForceSolid(true);

  // G4double thickness = 5.0*mm;
  // G4double height = 30.0*cm;
  // G4double width = 10.0*cm;
  // G4double smallWidth = (width-2*thickness);

  G4double worldLength = 100.0*cm;
  G4double a, z;
  G4String name;
  G4double density      = universe_mean_density;
  G4double pressure     = 1.e-19*pascal;
  G4double temperature  = 0.1*kelvin;
  //G4double pressure     = 1.e-18*pascal;
  //G4double temperature  = 2.73*kelvin;

  G4Material* vacuum = new G4Material(name = "Galactic", z=1., a=1.01*g/mole, density,
                                      kStateGas, temperature, pressure);

  G4Box* worldS
    = new G4Box("world",                                    //its name
                worldLength/2,worldLength/2,worldLength/2); //its size
  G4LogicalVolume* worldLV
    = new G4LogicalVolume(
                 worldS,   //its solid
                 vacuum,      //its material
                 "World"); //its name

  //  Must place the World Physical volume unrotated at (0,0,0).
  //
  G4VPhysicalVolume* worldPV
    = new G4PVPlacement(
                 0,               // no rotation
                 G4ThreeVector(), // at (0,0,0)
                 worldLV,         // its logical volume
                 "World",         // its name
                 0,               // its mother  volume
                 false,           // no boolean operations
                 0,               // copy number
                 fCheckOverlaps); // checking overlaps

  // Define CubeSat structure and solar panel parameter, construct using boolean operation

  G4double sat_height = 30.*cm;      // CubeSat height
  G4double sat_width = 20.*cm;       // CubeSat width
  G4double sat_length = 10.*cm;      // CubeSat length
  G4double coverGlass_thi = 0.1*mm;  // Cover glass thickness
  G4double cell_thi = 0.15*mm;       // Solar cell thickness
  G4double pcb_thi = 1.6*mm;         // PCB thickness

  G4Box* satOuter = new G4Box("Sat_outer", 0.5*sat_length, 0.5*sat_width, 0.5*sat_height);
  G4Box* pcbOuter = new G4Box("PCB_outer", 0.5*sat_length+pcb_thi, 0.5*sat_width+pcb_thi,
                              0.5*sat_height+pcb_thi);
  G4Box* cellOuter = new G4Box("Cell_outer", 0.5*sat_length+pcb_thi+cell_thi,
                               0.5*sat_width+pcb_thi+cell_thi, 0.5*sat_height+pcb_thi+cell_thi);
  G4Box* glassOuter = new G4Box("Glass_outer", 0.5*sat_length+pcb_thi+cell_thi+coverGlass_thi,
                                0.5*sat_width+pcb_thi+cell_thi+coverGlass_thi,
                                0.5*sat_height+pcb_thi+cell_thi+coverGlass_thi);

  G4SubtractionSolid* pcbS = new G4SubtractionSolid("PCB", pcbOuter, satOuter);
  G4LogicalVolume* pcbLV = new G4LogicalVolume(pcbS, Al6061, "PCB");
  new G4PVPlacement(0, G4ThreeVector(), pcbLV, "PCB", worldLV, false,
                    0, fCheckOverlaps);
  pcbLV->SetVisAttributes(cyan_vat);
  G4SubtractionSolid* cellS = new G4SubtractionSolid("Cell", cellOuter, pcbOuter);
  G4LogicalVolume* cellLV = new G4LogicalVolume(cellS, Scell, "Cell");
  new G4PVPlacement(0, G4ThreeVector(), cellLV, "Cell",  worldLV, false,
                    0, fCheckOverlaps);
  cellLV->SetVisAttributes(green_vat);
  G4SubtractionSolid* glassS = new G4SubtractionSolid("Glass", glassOuter, cellOuter);
  G4LogicalVolume* glassLV = new G4LogicalVolume(glassS, coverGlass, "Glass");
  new G4PVPlacement(0, G4ThreeVector(), glassLV, "Glass",  worldLV, false,
                    0, fCheckOverlaps);
  glassLV->SetVisAttributes(lblue_vat);

  // Electronics box and PCB
    G4double elecBoard_thi = 2.*mm;
    G4double elecBoard_size = 96.*mm;

    // 創建旋轉矩陣，將物體旋轉90度
    G4RotationMatrix* rotationMatrix = new G4RotationMatrix();
    rotationMatrix->rotateZ(90.*deg);

    // 創建一個新的 G4Transform3D，將位移和旋轉結合在一起
    G4Transform3D transform1 = G4Transform3D(*rotationMatrix, G4ThreeVector(40., 41., 100.*mm));
    G4Transform3D transform2 = G4Transform3D(*rotationMatrix, G4ThreeVector(-25., 41., 100.*mm));
    G4Transform3D transform3 = G4Transform3D(*rotationMatrix, G4ThreeVector(-40., 41., 100.*mm));
    G4Transform3D transform5 = G4Transform3D(*rotationMatrix, G4ThreeVector(40., 41., -68.*mm));
    G4Box* elecBoardS = new G4Box("Elec_board", 0.5*elecBoard_size, 0.5*elecBoard_thi, 0.5*elecBoard_size);
    G4LogicalVolume* elecBoardLV = new G4LogicalVolume(elecBoardS, Scell, "Elec_board");

    // 在xz平面創建三個 "Elec_board"
    new G4PVPlacement(0, G4ThreeVector(0.,-47.,100.*mm), elecBoardLV, "Elec_board0", worldLV, false, 0, fCheckOverlaps);
    new G4PVPlacement(0, G4ThreeVector(0.,-56.,100.*mm), elecBoardLV, "Elec_board1", worldLV, false, 1, fCheckOverlaps);
    new G4PVPlacement(0, G4ThreeVector(0.,-65.,100.*mm), elecBoardLV, "Elec_board2", worldLV, false, 2, fCheckOverlaps);

    // 在xy平面創建三個旋轉後的 "Elec_board"
    new G4PVPlacement(transform1, elecBoardLV, "Elec_board3", worldLV, false, 3, fCheckOverlaps);
    new G4PVPlacement(transform2, elecBoardLV, "Elec_board4", worldLV, false, 4, fCheckOverlaps);
    new G4PVPlacement(transform3, elecBoardLV, "Elec_board5", worldLV, false, 5, fCheckOverlaps);
    new G4PVPlacement(transform5, elecBoardLV, "Elec_board6", worldLV, false, 6, fCheckOverlaps);
    
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
                    worldLV, false, 0, fCheckOverlaps);
  new G4PVPlacement(transform4, elecBoxLV, "Elec_box1", worldLV, false, 1, fCheckOverlaps);
  elecBoxLV->SetVisAttributes(sol_red_vat);
  fScoringVolume = elecBoxLV;
  //
  G4double startracker_x = 44.*mm;
  G4double startracker_y = 50.*mm;
  G4double startracker_z = 95.*mm;
  G4Box* startracker = new G4Box("startracker", 0.5*startracker_x, 0.5*startracker_y, 0.5*startracker_z);
  G4LogicalVolume* startrackerLV = new G4LogicalVolume(startracker, Scell, "startracker");
  new G4PVPlacement(0, G4ThreeVector(-2., 30., -68.*mm), startrackerLV, "startracker0", worldLV, false, 0, fCheckOverlaps);
  startrackerLV->SetVisAttributes(sol_green_vat);
    
    // 定義圓柱體的參數
    G4double cylinder_innerRadius = 0.*mm;
    G4double cylinder_outerRadius = 35.*mm;
    G4double cylinder_halfHeight = 48.*mm;
    G4double cylinder_startAngle = 0.*deg;
    G4double cylinder_spanningAngle = 360.0 * deg; // 完整圓柱

    // 創建圓柱體
    G4Tubs* cylinderSolid = new G4Tubs("Cylinder", cylinder_innerRadius, cylinder_outerRadius,
                                       cylinder_halfHeight, cylinder_startAngle, cylinder_spanningAngle);

    // 創建圓柱體的邏輯卷
    G4LogicalVolume* cylinderLV = new G4LogicalVolume(cylinderSolid, Al6061, "CylinderLV");

    // 放置圓柱體
    new G4PVPlacement(0, G4ThreeVector(0.,-55.,-100.*mm), cylinderLV, "CylinderPhysical", worldLV, false, 0, fCheckOverlaps);

    // 設置可視化屬性
    cylinderLV->SetVisAttributes(sol_green_vat);
    fScoringVolume = cylinderLV;
    //solar panels
    G4double solar_thi = 2.*mm;
    G4double solar1_length = 700.*mm;
    G4double solar1_width = 200.*mm;
    G4double solar2_length = 100.*mm;
    G4double solar2_width = 300.*mm;
    G4Transform3D transform6 = G4Transform3D(*rotationMatrix1, G4ThreeVector(0., 0., 151.*mm));
    G4Box* solar1S = new G4Box("solar", 0.5*solar1_length, 0.5*solar_thi, 0.5*solar1_width);
    G4LogicalVolume* solar1LV = new G4LogicalVolume(solar1S, Scell, "solar1");
    G4Transform3D transform7 = G4Transform3D(*rotationMatrix1, G4ThreeVector(0., 250., 151.*mm));
    G4Box* solar2S = new G4Box("solar", 0.5*solar2_length, 0.5*solar_thi, 0.5*solar2_width);
    G4LogicalVolume* solar2LV = new G4LogicalVolume(solar2S, Scell, "solar2");
    G4Transform3D transform8 = G4Transform3D(*rotationMatrix1, G4ThreeVector(0., -250., 151.*mm));
    // 在xy平面創建三個旋轉後的 "Elec_board"
    new G4PVPlacement(transform6, solar1LV, "solar1", worldLV, false, 1, fCheckOverlaps);
    new G4PVPlacement(transform7, solar2LV, "solar2", worldLV, false, 2, fCheckOverlaps);
    new G4PVPlacement(transform8, solar2LV, "solar3", worldLV, false, 3, fCheckOverlaps);
    solar1LV->SetVisAttributes(sol_green_vat);
    fScoringVolume = solar1LV;
    solar2LV->SetVisAttributes(sol_green_vat);
    fScoringVolume = solar2LV;
  // CubeSat

  // G4Box* shellTopBot = new G4Box("Shell_solid", smallWidth/2, smallWidth/2, thickness/2);
  // fLogicTop = new G4LogicalVolume(shellTopBot, fShellMaterial, "Shell_LV",0,0,0);
  // new G4PVPlacement(0,
  //                    G4ThreeVector(0,0,(height-thickness)/2),
  //                    fLogicTop,
  //                    "Shell_PV",
  //                    worldLV,
  //                    false,
  //                    0,
  //                    fCheckOverlaps);
  // fLogicBot = new G4LogicalVolume(shellTopBot, fShellMaterial, "Shell_LV",0,0,0);
  // new G4PVPlacement(0,
  //                    G4ThreeVector(0,0,-(height-thickness)/2),
  //                    fLogicTop,
  //                    "Shell_PV",
  //                    worldLV,
  //                    false,
  //                    0,
  //                    fCheckOverlaps);
  // G4Box* shellFrontBack = new G4Box("Shell_solid",thickness/2,width/2,height/2);
  // fLogicFront = new G4LogicalVolume(shellFrontBack, fShellMaterial, "Shell_LV",0,0,0);
  // new G4PVPlacement(0,
  //                   G4ThreeVector((width-thickness)/2,0,0),
  //                   fLogicFront,
  //                   "Shell_PV",
  //                   worldLV,
  //                   false,
  //                   0,
  //                   fCheckOverlaps);
  // fLogicBack = new G4LogicalVolume(shellFrontBack, fShellMaterial, "Shell_LV",0,0,0);
  // new G4PVPlacement(0,
  //                   G4ThreeVector(-(width-thickness)/2,0,0),
  //                   fLogicBack,
  //                   "Shell_PV",
  //                   worldLV,
  //                   false,
  //                   0,
  //                   fCheckOverlaps);

  // G4Box* shellLeftRight = new G4Box("Shell_solid",smallWidth/2,thickness/2,height/2);

  // fLogicLeft = new G4LogicalVolume(shellLeftRight, fShellMaterial, "Shell_LV",0,0,0);
  // new G4PVPlacement(0,
  //                   G4ThreeVector(0,(width-thickness)/2,0),
  //                   fLogicLeft,
  //                   "Shell_PV",
  //                   worldLV,
  //                   false,
  //                   0,
  //                   fCheckOverlaps);
  // fLogicRight = new G4LogicalVolume(shellLeftRight, fShellMaterial, "Shell_LV",0,0,0);
  // new G4PVPlacement(0,
  //                   G4ThreeVector(0,-(width-thickness)/2,0),
  //                   fLogicRight,
  //                   "Shell_PV",
  //                   worldLV,
  //                   false,
  //                   0,
  //                   fCheckOverlaps);
  // G4Orb* doseSite = new G4Orb("Dose_site", 20.0*mm);

  // G4LogicalVolume* logicDoseSite = new G4LogicalVolume(doseSite, Scell, "Dose_Site_LV", 0, 0, 0);
  // new G4PVPlacement(0,
  //                   G4ThreeVector(0, 0, 0),
  //                   logicDoseSite,
  //                   "Dose_Site_PV",
  //                   worldLV,
  //                   false,
  //                   0,
  //                   fCheckOverlaps);
  // fScoringVolume = logicDoseSite;
  // Example of User Limits
  //
  // Below is an example of how to set tracking constraints in a given
  // logical volume
  //
  // Sets a max step length in the tracker region, with G4StepLimiter

  // G4double maxStep = 0.5*thickness;
  G4double maxStep = 0.05*mm;
  fStepLimit = new G4UserLimits(maxStep);

  /// Set additional contraints on the track, with G4UserSpecialCuts
  ///
  /// G4double maxLength = 2*trackerLength, maxTime = 0.1*ns, minEkin = 10*MeV;
  /// trackerLV->SetUserLimits(new G4UserLimits(maxStep,
  ///                                           maxLength,
  ///                                           maxTime,
  ///                                           minEkin));

  // Always return the physical world

  return worldPV;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B2aDetectorConstruction::ConstructSDandField()
{
  // Sensitive detectors

  G4String trackerChamberSDname = "B2/TrackerChamberSD";
  B2TrackerSD* aTrackerSD = new B2TrackerSD(trackerChamberSDname,
                                            "TrackerHitsCollection");
  G4SDManager::GetSDMpointer()->AddNewDetector(aTrackerSD);
  // Setting aTrackerSD to all logical volumes with the same name
  // of "Chamber_LV".
  // SetSensitiveDetector("Shell_LV", aTrackerSD, true);

  // Create global magnetic field messenger.
  // Uniform magnetic field is then created automatically if
  // the field value is not zero.
  G4ThreeVector fieldValue = G4ThreeVector();
  fMagFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
  fMagFieldMessenger->SetVerboseLevel(1);

  // Register the field messenger for deleting
  G4AutoDelete::Register(fMagFieldMessenger);
  }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void B2aDetectorConstruction::SetShellMaterial(G4String materialName)
// {
//   G4NistManager* nistManager = G4NistManager::Instance();

//   G4Material* pttoMaterial =
//               nistManager->FindOrBuildMaterial(materialName);

//   if (fShellMaterial != pttoMaterial) {
//      if ( pttoMaterial ) {
//         fShellMaterial = pttoMaterial;
//         if (fLogicBot)     fLogicBot->SetMaterial(fShellMaterial);
//         if (fLogicTop)     fLogicTop->SetMaterial(fShellMaterial);
//         if (fLogicFront)   fLogicFront->SetMaterial(fShellMaterial);
//         if (fLogicBack)    fLogicBack->SetMaterial(fShellMaterial);
//         if (fLogicLeft)    fLogicLeft->SetMaterial(fShellMaterial);
//         if (fLogicRight)   fLogicRight->SetMaterial(fShellMaterial);
//         G4cout
//           << G4endl
//           << "----> The target is made of " << materialName << G4endl;
//      } else {
//         G4cout
//           << G4endl
//           << "-->  WARNING from SetTargetMaterial : "
//           << materialName << " not found" << G4endl;
//      }
//   }
// }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B2aDetectorConstruction::SetMaxStep(G4double maxStep)
{
  if ((fStepLimit)&&(maxStep>0.)) fStepLimit->SetMaxAllowedStep(maxStep);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B2aDetectorConstruction::SetCheckOverlaps(G4bool checkOverlaps)
{
  fCheckOverlaps = checkOverlaps;
}
