//-------------------------------------------------
//
/** \class L1MuGMTLFMergeRankPtQLUT
 *
 *   LFMergeRankPtQ look-up table
 *          
 *   this class was automatically generated by 
 *     L1MuGMTLUT::MakeSubClass()  
*/
//
//   Author :
//   H. Sakulin            HEPHY Vienna
//
//   Migrated to CMSSW:
//   I. Mikulec
//
//--------------------------------------------------
#ifndef L1TriggerGlobalMuonTrigger_L1MuGMTLFMergeRankPtQLUT_h
#define L1TriggerGlobalMuonTrigger_L1MuGMTLFMergeRankPtQLUT_h

//---------------
// C++ Headers --
//---------------

//----------------------
// Base Class Headers --
//----------------------
#include "L1Trigger/GlobalMuonTrigger/src/L1MuGMTLUT.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//              ---------------------
//              -- Class Interface --
//              ---------------------

class L1MuGMTLFMergeRankPtQLUT : public L1MuGMTLUT {
public:
  enum { DT, BRPC, CSC, FRPC };

  /// constuctor using function-lookup
  L1MuGMTLFMergeRankPtQLUT() : L1MuGMTLUT("LFMergeRankPtQ", "DT BRPC CSC FRPC", "q(3) pt(5)", "rank_ptq(2)", 8, true) {
    InitParameters();
  };

  /// destructor
  ~L1MuGMTLFMergeRankPtQLUT() override {}

  /// specific lookup function for rank_ptq
  unsigned SpecificLookup_rank_ptq(int idx, unsigned q, unsigned pt) const {
    std::vector<unsigned> addr(2);
    addr[0] = q;
    addr[1] = pt;
    return Lookup(idx, addr)[0];
  };

  /// specific lookup function for entire output field
  unsigned SpecificLookup(int idx, unsigned q, unsigned pt) const {
    std::vector<unsigned> addr(2);
    addr[0] = q;
    addr[1] = pt;
    return LookupPacked(idx, addr);
  };

  /// access to lookup function with packed input and output

  unsigned LookupFunctionPacked(int idx, unsigned address) const override {
    std::vector<unsigned> addr = u2vec(address, m_Inputs);
    return TheLookupFunction(idx, addr[0], addr[1]);
  };

private:
  /// Initialize scales, configuration parameters, alignment constants, ...
  void InitParameters();

  /// The lookup function - here the functionality of the LUT is implemented
  unsigned TheLookupFunction(int idx, unsigned q, unsigned pt) const;

  /// Private data members (LUT parameters);
};
#endif
