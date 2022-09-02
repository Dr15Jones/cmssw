#ifndef DataFormats_Candidate_PolarStorage_h
#define DataFormats_Candidate_PolarStorage_h
// -*- C++ -*-
//
// Package:     DataFormats/Candidate
// Class  :     PolarStorage
//
/**\class PolarStorage PolarStorage.h "PolarStorage.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Christopher Jones
//         Created:  Thu, 01 Sep 2022 20:51:56 GMT
//

// system include files
#include "Rtypes.h"
#include "Math/Polar3D.h"
#include "DataFormats/Math/interface/LorentzVector.h"

// user include files

// forward declarations
namespace reco {
  namespace storage {

    struct PolarCoordinates {
      //required to be same layout as ROOT::Math::PtEtaPhiM4D

      Double32_t fPt;  //[0,0,9]
      Double32_t fEta;
      Double32_t fPhi;  //[-pi,pi,16]
      Double32_t fM;
    };

    struct PolarStorage {
      constexpr void static test() { static_assert(sizeof(PolarStorage) == sizeof(math::PtEtaPhiMLorentzVector)); }
      PolarCoordinates fCoordinates;
    };
  }  // namespace storage
}  // namespace reco
#endif
