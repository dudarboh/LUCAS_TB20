#include "LCDetectorConstruction.hh"
#include "LCSD.hh"

#include "G4TransportationManager.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4PVReplica.hh"

#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4Element.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4SystemOfUnits.hh"

G4VPhysicalVolume* LCDetectorConstruction::Construct(){
    DefineMaterials();
    return DefineVolumes();
}


void LCDetectorConstruction::DefineMaterials(){
    auto nistManager = G4NistManager::Instance();

    nistManager->FindOrBuildMaterial("G4_AIR");
    nistManager->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    nistManager->FindOrBuildMaterial("G4_Si");
    nistManager->FindOrBuildMaterial("G4_Al");

    //Absorber
    auto w = nistManager->FindOrBuildMaterial("G4_W");
    auto ni = nistManager->FindOrBuildMaterial("G4_Ni");
    auto cu = nistManager->FindOrBuildMaterial("G4_Cu");
    auto absorber = new G4Material("Absorber", 18.0*g/cm3, 3);
    absorber->AddMaterial(w, 0.95);
    absorber->AddMaterial(ni, 0.025);
    absorber->AddMaterial(cu, 0.025);

    //Sensor support
    auto c = nistManager->FindOrBuildMaterial("G4_C");
    auto glue = new G4Material("Glue", 1.3*g/cm3, 3);
    glue->AddElement( new G4Element("Hydrogen", "H", 1., 1.01*g/mole), 0.1310 );
    glue->AddElement( new G4Element("Carbon", "C", 6., 12.01*g/mole), 0.5357 );
    glue->AddElement( new G4Element("Oxygen", "O", 8., 16.00*g/mole), 0.3333 );
    auto carbonFiber = new G4Material("CarbonFiber", 1.6*g/cm3, 2);
    carbonFiber->AddMaterial(c, 0.5);
    carbonFiber->AddMaterial(glue, 0.5);

    // Fanout
    auto kapton = nistManager->FindOrBuildMaterial("G4_KAPTON");
    double weightGlue = glue->GetDensity() * (0.06 / 0.15);
    double weightKapton = kapton->GetDensity() * (0.06 / 0.15);
    double weightCu = cu->GetDensity() * (0.03 / 0.15);
    double fanoutDensity = weightGlue + weightKapton + weightCu;
    auto fanout = new G4Material("Fanout", fanoutDensity, 3);
    fanout->AddMaterial(glue, weightGlue/fanoutDensity);
    fanout->AddMaterial(kapton , weightKapton/fanoutDensity);
    fanout->AddMaterial(cu, weightCu/fanoutDensity);
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

G4VPhysicalVolume* LCDetectorConstruction::DefineVolumes(){
    int nLayers = 16;
    double layerThickness = 4.5*mm;
    
    auto WorldS = new G4Box("World", 1200.*mm, 1200.*mm, 10000.*mm);
    auto WorldLV = new G4LogicalVolume(WorldS, G4Material::GetMaterial("G4_AIR"), "World");
    auto WorldPV = new G4PVPlacement(0, G4ThreeVector(), WorldLV, "World", 0, false, 0, true);

    // Trigger scintilators
    // auto ScS = new G4Box("Trigger", 15.*mm, 15.*mm, 2.064*mm);
    // auto ScLV = new G4LogicalVolume(ScS, G4Material::GetMaterial("G4_PLASTIC_SC_VINYLTOLUENE"), "Trigger");
    // I didn't find where they were during the TB2020...
    // new G4PVPlacement(0, G4ThreeVector(0., 0., (450.+2.064)*mm), ScLV, "Trigger1", WorldLV, false, 0, 1);
    // new G4PVPlacement(0, G4ThreeVector(0., 0., (3607.+2.064)*mm), ScLV, "Trigger2", WorldLV, false, 0, 1);

    // ALPIDE telescope
    auto alpideS = new G4Box("Tracker", 15.0*mm, 7.5*mm, 0.025*mm);
    auto alpideLV = new G4LogicalVolume(alpideS, G4Material::GetMaterial("G4_Si"), "Tracker");
    new G4PVPlacement(0, G4ThreeVector(0., 0., 950*mm), alpideLV, "Tracker", WorldLV, false, 0, true);
    new G4PVPlacement(0, G4ThreeVector(0., 0., 1120*mm), alpideLV, "Tracker", WorldLV, false, 1, true);
    new G4PVPlacement(0, G4ThreeVector(0., 0., 3290*mm), alpideLV, "Tracker", WorldLV, false, 2, true);
    new G4PVPlacement(0, G4ThreeVector(0., 0., 3380*mm), alpideLV, "Tracker", WorldLV, false, 3, true);
    new G4PVPlacement(0, G4ThreeVector(0., 0., 3470*mm), alpideLV, "Tracker", WorldLV, false, 4, true);

    // Move calorimeter so beam hits around pad 26 with 56.6/43.3 left/right sectors as in FLAME data
    double xPosLumical = 16.1;
    double yPosLumical = 12.4*mm;
    auto lumicalS = new G4Box("Calorimeter", 100.0*mm, 100.0*mm, nLayers*layerThickness/2.);
    auto lumicalLV = new G4LogicalVolume(lumicalS, G4Material::GetMaterial("G4_AIR"), "Calorimeter");
    new G4PVPlacement(0, G4ThreeVector(xPosLumical, yPosLumical, 4070*mm), lumicalLV, "Calorimeter", WorldLV, false, 0, true);
    
    auto layerS = new G4Box("Layer", 100.0*mm, 100.0*mm, layerThickness/2.);
    auto layerLV = new G4LogicalVolume(layerS, G4Material::GetMaterial("G4_AIR"), "Layer");
    new G4PVReplica("Layer", layerLV, lumicalLV, kZAxis, nLayers, layerThickness);

    auto absorberS = new G4Box("Absorber", 100.0*mm, 100.0*mm, 3.5/2.*mm);
    auto absorberLV = new G4LogicalVolume(absorberS, G4Material::GetMaterial("Absorber"), "Absorber");
    new G4PVPlacement(0, G4ThreeVector(0., 0., 0.4*mm), absorberLV, "Absorber", layerLV, false, 0, true);

    auto supportS = new G4Box("Support", 100.0*mm, 100.0*mm, 0.4*mm);
    auto supportLV = new G4LogicalVolume(supportS, G4Material::GetMaterial("CarbonFiber"), "Support");
    new G4PVPlacement(0, G4ThreeVector(0., 0., -1.75*mm), supportLV, "Support", layerLV, false, 0, true);

    G4double centerAtOrigin = -(80. + 0.5 * (195.2 - 80.))*mm;
    auto fanoutS = new G4Tubs("Fanout", 80.*mm, 195.2*mm, 0.075*mm, 75.*deg, 30.*deg);
    auto fanoutLV = new G4LogicalVolume(fanoutS, G4Material::GetMaterial("Fanout"), "Fanout");
    new G4PVPlacement(0, G4ThreeVector(0., centerAtOrigin, -0.245), fanoutLV, "Fanout", supportLV, false, 0, true);
    new G4PVPlacement(0, G4ThreeVector(0., centerAtOrigin, 0.245), fanoutLV, "Fanout", supportLV, false, 1, true);

    auto wiresS = new G4Tubs("Wires", 80.*mm, 195.2*mm, 0.01*mm, 75.*deg, 30.*deg);
    auto wiresLV = new G4LogicalVolume(wiresS, G4Material::GetMaterial("G4_Al"), "Wires");
    new G4PVPlacement(0, G4ThreeVector(0., centerAtOrigin, -0.17), wiresLV, "Wires", supportLV, false, 0, true);
    new G4PVPlacement(0, G4ThreeVector(0., centerAtOrigin, 0.17), wiresLV, "Wires", supportLV, false, 1, true);

    auto sensorS = new G4Tubs("Sensor", 80.*mm, 195.2*mm, 0.16*mm, 75.*deg, 30.*deg);
    auto sensorLV = new G4LogicalVolume(sensorS, G4Material::GetMaterial("G4_AIR"), "Sensor");
    new G4PVPlacement(0, G4ThreeVector(0., centerAtOrigin, 0.), sensorLV, "Sensor", supportLV, false, 0, true);

    auto sectorS = new G4Tubs("Sector", 80.*mm, 195.2*mm, 0.16*mm, -3.75*deg, 7.5*deg);
    auto sectorLV = new G4LogicalVolume(sectorS, G4Material::GetMaterial("G4_AIR"), "Sensor");
    new G4PVReplica("Sector", sectorLV, sensorLV, kPhi, 4, 7.5*deg, 75.*deg);

    auto padS = new G4Tubs("Pad", 0.*mm, 1.8*mm, 0.16*mm, -3.75*deg, 7.5*deg);
    fPadLV = new G4LogicalVolume(padS, G4Material::GetMaterial("G4_Si"), "Pad");
    new G4PVReplica("Pad", fPadLV, sectorLV, kRho, 64, 1.8*mm, 80.*mm);

    WorldLV->SetVisAttributes(G4Color(0.79, 1.00, 0.90, 0.1));
    alpideLV->SetVisAttributes(G4Color(0.847, 0.835, 0.047));
    lumicalLV->SetVisAttributes(G4VisAttributes::Invisible);
    layerLV->SetVisAttributes(G4VisAttributes::Invisible);
    absorberLV->SetVisAttributes(G4Color(0.23, 0.48, 0.34));
    supportLV->SetVisAttributes(G4VisAttributes::Invisible);
    fanoutLV->SetVisAttributes(G4VisAttributes::Invisible);
    wiresLV->SetVisAttributes(G4VisAttributes::Invisible);
    sensorLV->SetVisAttributes(G4VisAttributes::Invisible);
    sectorLV->SetVisAttributes(G4VisAttributes::Invisible);
    fPadLV->SetVisAttributes(G4Color(0., 0., 1., 0.5));

    return WorldPV;
}

void LCDetectorConstruction::ConstructSDandField(){
    G4SDManager::GetSDMpointer()->SetVerboseLevel(1);

    auto lumicalSD = new LCSD("LumicalSD", "LumicalHCName");
    G4SDManager::GetSDMpointer()->AddNewDetector(lumicalSD);
    fPadLV->SetSensitiveDetector(lumicalSD);
}
