// -*- C++ -*-
//
// Package:     FWCore/Framework
// Class  :     edm::stream::EDProducerBase
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Fri, 02 Aug 2013 23:49:57 GMT
//

// system include files

// user include files
#include "FWCore/Framework/interface/stream/EDSummaryProducer.h"

using namespace edm::stream;
//
// constants, enums and typedefs
//

//
// static data member definitions
//
static const std::string kBaseType("EDSummaryProducer");

namespace edm {
  namespace stream {
    const std::string&
    edsummaryproducer_basetype() {
      return kBaseType;
    }    
  }
}
