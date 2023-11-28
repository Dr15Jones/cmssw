#ifndef DataFormats_Common_RefToBaseToPtr_ioread_h
#define DataFormats_Common_RefToBaseToPtr_ioread_h

/*----------------------------------------------------------------------
  
Ref: A function template for conversion from RefToBase to Ptr for use
in ROOT ioread rules for schema migration

----------------------------------------------------------------------*/

#include "DataFormats/Common/interface/RefToBase.h"
#include "DataFormats/Common/interface/Ptr.h"

namespace edm {
  template <typename T>
  Ptr<T> refToBaseToPtr_ioread(RefToBase<T> const& ref) {
    if (ref.isNull()) {
      return Ptr<T>();
    }
    if (ref.isTransient()) {
      // This is a logic error, any ref being deserialized
      // by construction would be persistent
      return Ptr<T>();
    } else {
      EDProductGetter const* getter = ref.productGetter();
      if (getter) {
        return Ptr<T>(ref.id(), ref.key(), getter);
      }
    }
    // If this is called in an iorule outside the framework, we cannot call
    // ref.get(), but since outside the framework we can never fetch the ref,
    // the Ptr will only be useful if accessed later from inside the framework.
    // We can fill with a nullptr for now
    return Ptr<T>(ref.id(), static_cast<const T*>(nullptr), ref.key());
  }
}  // namespace edm
#endif
