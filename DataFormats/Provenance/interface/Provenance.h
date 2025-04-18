#ifndef DataFormats_Provenance_Provenance_h
#define DataFormats_Provenance_Provenance_h

/*----------------------------------------------------------------------

Provenance: The full description of a product and how it came into
existence.

----------------------------------------------------------------------*/

#include "DataFormats/Provenance/interface/ParameterSetID.h"
#include "DataFormats/Provenance/interface/Parentage.h"
#include "DataFormats/Provenance/interface/StableProvenance.h"

#include <memory>

#include <iosfwd>
/*
  Provenance

  definitions:
  Product: The EDProduct to which a provenance object is associated

  Creator: The EDProducer that made the product.

  Parents: The EDProducts used as input by the creator.
*/

namespace edm {
  class MergeableRunProductMetadataBase;
  class ProductProvenance;
  class ProductProvenanceLookup;

  class Provenance {
  public:
    Provenance();

    Provenance(std::shared_ptr<ProductDescription const> const& p, ProductID const& pid);

    Provenance(StableProvenance const&);

    StableProvenance const& stable() const { return stableProvenance_; }
    StableProvenance& stable() { return stableProvenance_; }

    ProductDescription const& productDescription() const { return stable().productDescription(); }
    std::shared_ptr<ProductDescription const> const& constProductDescriptionPtr() const {
      return stable().constProductDescriptionPtr();
    }

    ProductProvenance const* productProvenance() const;
    bool knownImproperlyMerged() const;
    BranchID const& branchID() const { return stable().branchID(); }
    std::string const& branchName() const { return stable().branchName(); }
    std::string const& className() const { return stable().className(); }
    std::string const& moduleLabel() const { return stable().moduleLabel(); }
    std::string const& processName() const { return stable().processName(); }
    std::string const& productInstanceName() const { return stable().productInstanceName(); }
    std::string const& friendlyClassName() const { return stable().friendlyClassName(); }
    ProductProvenanceLookup const* store() const { return store_; }
    std::set<std::string> const& branchAliases() const { return stable().branchAliases(); }

    // Usually branchID() and originalBranchID() return exactly the same result.
    // The return values can differ only in cases where an EDAlias is involved.
    // For example, if you "get" a product and then get the Provenance object
    // available through the Handle, you will find that branchID() and originalBranchID()
    // will return different values if and only if an EDAlias was used to specify
    // the desired product and in a previous process the EDAlias was kept and
    // the original branch name was dropped. In that case, branchID() returns
    // the BranchID of the EDAlias and originalBranchID() returns the BranchID
    // of the branch name that was dropped. One reason the original BranchID can
    // be useful is that Parentage information is stored using the original BranchIDs.
    BranchID const& originalBranchID() const { return stable().originalBranchID(); }

    void write(std::ostream& os) const;

    void setStore(ProductProvenanceLookup const* store) { store_ = store; }

    ProductID const& productID() const { return stable().productID(); }

    void setProductID(ProductID const& pid) { stable().setProductID(pid); }

    void setMergeableRunProductMetadata(MergeableRunProductMetadataBase const* mrpm) {
      mergeableRunProductMetadata_ = mrpm;
    }

    void setProductDescription(std::shared_ptr<ProductDescription const> const& p) {
      stable().setProductDescription(p);
    }

    void swap(Provenance&);

  private:
    StableProvenance stableProvenance_;
    ProductProvenanceLookup const* store_;
    MergeableRunProductMetadataBase const* mergeableRunProductMetadata_;
  };

  inline std::ostream& operator<<(std::ostream& os, Provenance const& p) {
    p.write(os);
    return os;
  }

  bool operator==(Provenance const& a, Provenance const& b);

}  // namespace edm
#endif
