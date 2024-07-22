#include "DataProductsRNTuple.h"
#include "ROOT/RNTuple.hxx"
#include "IOPool/Common/interface/getWrapperBasePtr.h"

using namespace edm::input;
using namespace ROOT::Experimental;

namespace {
  std::string fixName(std::string_view iName) {
    if (not iName.empty() and iName.back() == '.') {
      iName.remove_suffix(1);
    }
    return std::string(iName);
  }
}  // namespace

DataProductsRNTuple::DataProductsRNTuple(TFile* iFile, std::string const& iName, std::string const& iAux)
    : reader_(RNTupleReader::Open(iFile->Get<RNTuple>(iName.c_str()))) {
  auxDesc_ = reader_->GetDescriptor().FindFieldId(iAux);
}

bool DataProductsRNTuple::setupToReadProductIfAvailable(BranchDescription const& iBranch) {
  auto fixedName = fixName(iBranch.branchName());
  auto desc = reader_->GetDescriptor().FindFieldId(fixedName);
  if (desc == ROOT::Experimental::kInvalidDescriptorId) {
    return false;
  }
  infos_.emplace(iBranch.branchID().id(), ProductInfo(iBranch.wrappedName(), desc));
  return true;
}

TClass const* DataProductsRNTuple::WrapperFactory::wrapperBase() {
  static TClass const* const s_base = TClass::GetClass("edm::WrapperBase");
  return s_base;
}

DataProductsRNTuple::WrapperFactory::WrapperFactory(std::string const& iTypeName)
    : wrapperClass_(TClass::GetClass(iTypeName.c_str())) {
  offsetToWrapperBase_ = wrapperClass_->GetBaseClassOffset(wrapperBase());
}

void DataProductsRNTuple::WrapperFactory::Deleter::operator()(void* iPtr) const { class_->Destructor(iPtr); }

std::unique_ptr<void, DataProductsRNTuple::WrapperFactory::Deleter> DataProductsRNTuple::WrapperFactory::newWrapper()
    const {
  return std::unique_ptr<void, Deleter>(wrapperClass_->New(), Deleter(wrapperClass_));
}

std::shared_ptr<edm::WrapperBase> DataProductsRNTuple::WrapperFactory::toWrapperBase(
    std::unique_ptr<void, Deleter> iProduct) const {
  return getWrapperBasePtr(iProduct.release(), offsetToWrapperBase_);
}

std::shared_ptr<edm::WrapperBase> DataProductsRNTuple::dataProduct(edm::BranchID const& iBranch, int iEntry) {
  auto const& info = infos_.find(iBranch.id());
  assert(info != infos_.end());

  auto product = info->second.factory_.newWrapper();
  auto view = reader_->GetView<void>(info->second.descriptor_, std::shared_ptr<void>());
  view.BindRawPtr(product.get());

  view(iEntry);

  return info->second.factory_.toWrapperBase(std::move(product));
}
