#ifndef DataFormats_Common_RefToBaseToPtr_h
#define DataFormats_Common_RefToBaseToPtr_h

/*----------------------------------------------------------------------
  
Ref: A function template for conversion from RefToBase to Ptr

----------------------------------------------------------------------*/

#include "DataFormats/Common/interface/RefToBase.h"
#include "DataFormats/Common/interface/Ptr.h"

namespace edm {
  template <typename T>
  Ptr<T> refToBaseToPtr(RefToBase<T> const& ref) {
    if (ref.isNull()) {
      return Ptr<T>();
    }
    if (ref.isTransient()) {
      return Ptr<T>(ref.get(), ref.key());
    } else {
      //Another thread could change this value so get only once
      EDProductGetter const* getter = ref.productGetter();
      if (getter) {
        return Ptr<T>(ref.id(), ref.key(), getter);
      }
    }
    return Ptr<T>(ref.id(), ref.get(), ref.key());
  }
}  // namespace edm
#endif
