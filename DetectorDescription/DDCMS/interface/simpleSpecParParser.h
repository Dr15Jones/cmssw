#ifndef DetectorDescription_DDCMS_simpleSpecParParser_h
#define DetectorDescription_DDCMS_simpleSpecParParser_h
// -*- C++ -*-
//
// Package:     DetectorDescription/DDCMS
// Class  :     simpleSpecParParser
//
/**\function simpleSpecParParser

 Description: Parses simple string containing a value and an optional unit

 Usage:
    If the string can not be parsed, the optional will not be set.

*/
//
// Original Author:  Christopher Jones
//         Created:  Mon, 30 Nov 2020 15:43:26 GMT
//

// system include files
#include <optional>
#include <string_view>

// user include files

// forward declarations

namespace cms {
  namespace geometry {

    enum class Units {
      unit_less,
      //distance
      m,
      cm,
      mm,
      mum,
      fm,
      //inverse distance
      inv_cm,
      //time
      ns,
      ms,
      //angle
      deg,
      rad,
      unknown
    };

    struct DefaultUnits {
      DefaultUnits(Units distance, Units time, Units angle);

      Units distance_;
      Units time_;
      Units angle_;
    };

    struct ValueWithUnit {
      double value_;
      Units unit_;
    };

    std::optional<ValueWithUnit> simpleSpecParParser(const char*);

    double convertToDefault(ValueWithUnit const&, DefaultUnits const&);
  }  // namespace geometry
}  // namespace cms
#endif
