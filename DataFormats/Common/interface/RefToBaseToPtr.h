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

  /*
   * Do not use this outside an ioread rule in classes_def.xml
   */
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
