#ifndef LCDetectorConstruction_h
#define LCDetectorConstruction_h 1

#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4FieldManager.hh"
#include "G4UniformMagField.hh"

class G4VPhysicalVolume;

class LCDetectorConstruction : public G4VUserDetectorConstruction{
    public:
        LCDetectorConstruction(){};
        virtual ~LCDetectorConstruction(){};

        virtual G4VPhysicalVolume* Construct();
        virtual void ConstructSDandField();

        void DefineMaterials();
        G4VPhysicalVolume* DefineVolumes();
    private:
        G4LogicalVolume* fPadLV = nullptr;
};

#endif
