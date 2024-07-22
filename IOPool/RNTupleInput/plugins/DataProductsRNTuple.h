#if !defined(DataProductsRNTuple_h)
#define DataProductsRNTuple_h

#include "DataFormats/Provenance/interface/BranchDescription.h"
#include "DataFormats/Common/interface/WrapperBase.h"
#include "FWCore/Utilities/interface/InputType.h"
#include "ROOT/RNTupleReader.hxx"
#include "TFile.h"
#include "TClass.h"
#include <string>
#include <memory>

namespace edm::input {
  class DataProductsRNTuple {
  public:
    DataProductsRNTuple(TFile*, std::string const& iName, std::string const& iAux);

    bool setupToReadProductIfAvailable(BranchDescription const&);

    std::shared_ptr<edm::WrapperBase> dataProduct(edm::BranchID const&, int iEntry);

    template <typename T>
    ROOT::Experimental::RNTupleView<T, true> auxView(std::shared_ptr<T> oStorage) {
      return reader_->GetView(auxDesc_, std::move(oStorage));
    }

    ROOT::Experimental::NTupleSize_t numberOfEntries() const { return reader_->GetNEntries(); }

    ROOT::Experimental::DescriptorId_t findDescriptorID(std::string const& iFieldName) {
      return reader_->GetDescriptor().FindFieldId(iFieldName);
    }

    template <typename T>
    ROOT::Experimental::RNTupleView<T, true> viewFor(ROOT::Experimental::DescriptorId_t iID,
                                                     std::shared_ptr<T> oStorage) {
      return reader_->GetView(iID, std::move(oStorage));
    }

  private:
    struct WrapperFactory {
      struct Deleter {
        Deleter(TClass* iClass) : class_(iClass) {}
        void operator()(void*) const;
        TClass* class_;
      };

      WrapperFactory(std::string const& iTypeName);

      std::unique_ptr<void, Deleter> newWrapper() const;
      std::shared_ptr<edm::WrapperBase> toWrapperBase(std::unique_ptr<void, Deleter>) const;
      TClass* wrapperClass_;
      Int_t offsetToWrapperBase_;

      static TClass const* wrapperBase();
    };

    struct ProductInfo {
      ProductInfo(std::string const& iTypeName, ROOT::Experimental::DescriptorId_t iDesc)
          : factory_(iTypeName), descriptor_(iDesc) {}
      WrapperFactory factory_;
      ROOT::Experimental::DescriptorId_t descriptor_;
    };

    std::unique_ptr<ROOT::Experimental::RNTupleReader> reader_;
    std::unordered_map<edm::BranchID::value_type, ProductInfo> infos_;
    ROOT::Experimental::DescriptorId_t auxDesc_;
  };
}  // namespace edm::input

#endif
