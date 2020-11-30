// -*- C++ -*-
//
// Package:     DetectorDescription/DDCMS
// Class  :     simpleSpecParParser
//
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Christopher Jones
//         Created:  Mon, 30 Nov 2020 15:57:48 GMT
//

// system include files
#include <cstdlib>
#include <algorithm>
#include <cassert>

// user include files
#include "DetectorDescription/DDCMS/interface/simpleSpecParParser.h"

using namespace cms::geometry;

namespace {
  Units parseUnits(std::string_view iUnit) {
    using namespace std::literals::string_view_literals;
    static constexpr std::array<std::pair<std::string_view, Units>, 11> names = {{{"*m"sv, Units::m},
                                                                                  {"*cm"sv, Units::cm},
                                                                                  {"*mm"sv, Units::mm},
                                                                                  {"*mum"sv, Units::mum},
                                                                                  {"*fm"sv, Units::fm},
                                                                                  {"/cm"sv, Units::inv_cm},
                                                                                  {"*ns"sv, Units::ns},
                                                                                  {"*nanosecond"sv, Units::ns},
                                                                                  {"*ms"sv, Units::ms},
                                                                                  {"*deg"sv, Units::deg},
                                                                                  {"*rad"sv, Units::rad}}};

    auto itFound =
        std::find_if(names.begin(), names.end(), [&iUnit](auto const& iElement) { return iElement.first == iUnit; });
    if (itFound != names.end()) {
      return itFound->second;
    }
    return Units::unknown;
  }

  double convertLength(ValueWithUnit const& iValue, Units iConvertTo) {
    static constexpr std::array<double, 5> scale = {{1.0, 100, 1000., 1.e6, 1.e15}};
    constexpr int kStart = 1;
    static_assert(Units::m == Units{kStart});
    static_assert(Units::cm == Units{kStart + 1});
    static_assert(Units::mm == Units{kStart + 2});
    static_assert(Units::mum == Units{kStart + 3});
    static_assert(Units::fm == Units{kStart + 4});
    if (iValue.unit_ == iConvertTo) {
      return iValue.value_;
    }
    return iValue.value_ * scale[static_cast<int>(iConvertTo) - kStart] /
           scale[static_cast<int>(iValue.unit_) - kStart];
  }

  double convertInvLength(ValueWithUnit const& iValue, Units iConvertTo) {
    //We will just swap the convert and unit and do a length conversion
    ValueWithUnit temp{iValue.value_, iConvertTo};
    //There is only 1 inverse length right now
    return convertLength(temp, Units::cm);
  }

  double convertTime(ValueWithUnit const& iValue, Units iConvertTo) {
    static constexpr std::array<double, 2> scale = {{1000.0, 1.0}};
    constexpr int kStart = static_cast<int>(Units::ns);
    static_assert(Units::ns == Units{kStart});
    static_assert(Units::ms == Units{kStart + 1});
    if (iValue.unit_ == iConvertTo) {
      return iValue.value_;
    }
    return iValue.value_ * scale[static_cast<int>(iConvertTo) - kStart] /
           scale[static_cast<int>(iValue.unit_) - kStart];
  }

  double convertAngle(ValueWithUnit const& iValue, Units iConvertTo) {
    constexpr double PI = 3.141592653589793238463;
    static constexpr std::array<double, 2> scale = {{360, 2 * PI}};
    constexpr int kStart = static_cast<int>(Units::deg);
    static_assert(Units::deg == Units{kStart});
    static_assert(Units::rad == Units{kStart + 1});
    if (iValue.unit_ == iConvertTo) {
      return iValue.value_;
    }
    return iValue.value_ * scale[static_cast<int>(iConvertTo) - kStart] /
           scale[static_cast<int>(iValue.unit_) - kStart];
  }

}  // namespace

std::optional<ValueWithUnit> cms::geometry::simpleSpecParParser(const char* iStringValue) {
  std::string_view stringValue = iStringValue;
  if (stringValue.empty()) {
    return std::nullopt;
  }

  auto const end = stringValue.data() + stringValue.size();
  char* parsedTill;
  double v = std::strtod(stringValue.data(), &parsedTill);

  if (stringValue.data() == parsedTill) {
    //Is this a boolean?
    if (stringValue == "true" or stringValue == "True") {
      return std::optional(ValueWithUnit{1.0, Units::unit_less});
    }
    if (stringValue == "false" or stringValue == "False") {
      return std::optional(ValueWithUnit{0.0, Units::unit_less});
    }
    return std::nullopt;
  }

  if (end == parsedTill) {
    //No units given
    return std::optional(ValueWithUnit{v, Units::unit_less});
  }

  auto unit = parseUnits({parsedTill, static_cast<std::string_view::size_type>(end - parsedTill)});
  if (unit == Units::unknown) {
    return std::nullopt;
  }
  return std::optional(ValueWithUnit{v, unit});
}

DefaultUnits::DefaultUnits(Units distance, Units time, Units angle) : distance_{distance}, time_{time}, angle_{angle} {
  assert((static_cast<int>(Units::m) <= static_cast<int>(distance)) and
         (static_cast<int>(distance) <= static_cast<int>(Units::fm)));
  assert(time == Units::ns or time == Units::ms);
  assert(angle == Units::deg or angle == Units::rad);
}

double cms::geometry::convertToDefault(ValueWithUnit const& iValue, DefaultUnits const& iDefaults) {
  switch (iValue.unit_) {
    case Units::unit_less:
      return iValue.value_;
    case Units::m: {
      return convertLength(iValue, iDefaults.distance_);
    }
    case Units::cm: {
      return convertLength(iValue, iDefaults.distance_);
    }

    case Units::mm: {
      return convertLength(iValue, iDefaults.distance_);
    }

    case Units::mum: {
      return convertLength(iValue, iDefaults.distance_);
    }

    case Units::fm: {
      return convertLength(iValue, iDefaults.distance_);
    }

    case Units::inv_cm: {
      return convertInvLength(iValue, iDefaults.distance_);
    }

    case Units::ns: {
      return convertTime(iValue, iDefaults.time_);
    }
    case Units::ms: {
      return convertTime(iValue, iDefaults.time_);
    }
    case Units::deg: {
      return convertAngle(iValue, iDefaults.angle_);
    }
    case Units::rad: {
      return convertAngle(iValue, iDefaults.angle_);
    }
    case Units::unknown:
      return iValue.value_;
  }
  return iValue.value_;
}
