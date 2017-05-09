#ifndef FWCore_Framework_stream_EDSummaryProducer_h
#define FWCore_Framework_stream_EDSummaryProducer_h
// -*- C++ -*-
//
// Package:     FWCore/Framework
// Class  :     EDSummaryProducer
// 
/**\class edm::stream::EDSummaryProducer EDSummaryProducer.h "FWCore/Framework/interface/stream/EDSummaryProducer.h"

 Description: Base class for stream based EDSummaryProducers

 Usage:
    An EDSummaryProducer accumulates data from the Event and then puts a
 data product into the Run and/or LuminosityBlock.

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 01 Aug 2013 21:41:42 GMT
//

// system include files

// user include files
#include "FWCore/Framework/interface/stream/EDProducer.h"
// forward declarations
namespace edm {
  namespace stream {
    const std::string& edsummaryproducer_basetype();
    
    template< typename... T>
    class EDSummaryProducer : public EDProducer<T...>
    {
      
    public:
      EDSummaryProducer() = default;
      //virtual ~EDSummaryProducer();
      
      // ---------- const member functions ---------------------
      
      // ---------- static member functions --------------------
      static const std::string& baseType() {
        return edsummaryproducer_basetype();
      }
      
      // ---------- member functions ---------------------------
      virtual void accumulate(edm::Event const& iEvent,
                              edm::EventSetup const& iES) = 0;
      
    private:
      void produce(edm::Event& iEvent, edm::EventSetup const& iES) override final {
        accumulate(iEvent,iES);
      }
      EDSummaryProducer(const EDSummaryProducer&) = delete; // stop default
      
      const EDSummaryProducer& operator=(const EDSummaryProducer&) = delete; // stop default
      
      // ---------- member data --------------------------------
      
    };
    
  }
}


#endif
