#include <cppunit/extensions/HelperMacros.h>

#include "DetectorDescription/DDCMS/interface/simpleSpecParParser.h"

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"

class test_simpleSpecParParser : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(test_simpleSpecParParser);
  CPPUNIT_TEST(checkParsing);
  CPPUNIT_TEST(checkConversion);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override{};
  void tearDown() override {}
  void checkParsing();
  void checkConversion();

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(test_simpleSpecParParser);

void test_simpleSpecParParser::checkParsing() {
  using namespace cms::geometry;
  {
    auto v = simpleSpecParParser("");
    CPPUNIT_ASSERT(not v.has_value());
  }

  {
    auto v = simpleSpecParParser("[cantdo]");
    CPPUNIT_ASSERT(not v.has_value());
  }

  {
    auto v = simpleSpecParParser("1cm");
    CPPUNIT_ASSERT(not v.has_value());
  }

  {
    auto v = simpleSpecParParser("1*bad");
    CPPUNIT_ASSERT(not v.has_value());
  }

  {
    auto v = simpleSpecParParser("true");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("True");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("false");
    CPPUNIT_ASSERT(v->value_ == 0.0);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("False");
    CPPUNIT_ASSERT(v->value_ == 0.0);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("1");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("-1");
    CPPUNIT_ASSERT(v->value_ == -1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("3.4");
    CPPUNIT_ASSERT(v->value_ == 3.4);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("-3.4");
    CPPUNIT_ASSERT(v->value_ == -3.4);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("3.4e2");
    CPPUNIT_ASSERT(v->value_ == 3.4e2);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("-3.4e2");
    CPPUNIT_ASSERT(v->value_ == -3.4e2);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("3.4e-2");
    CPPUNIT_ASSERT(v->value_ == 3.4e-2);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("-3.4e-2");
    CPPUNIT_ASSERT(v->value_ == -3.4e-2);
    CPPUNIT_ASSERT(v->unit_ == Units::unit_less);
  }

  {
    auto v = simpleSpecParParser("1*m");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::m);
  }

  {
    auto v = simpleSpecParParser("1*cm");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::cm);
  }

  {
    auto v = simpleSpecParParser("1*mm");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::mm);
  }

  {
    auto v = simpleSpecParParser("1*mum");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::mum);
  }

  {
    auto v = simpleSpecParParser("1*fm");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::fm);
  }

  {
    auto v = simpleSpecParParser("1/cm");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::inv_cm);
  }

  {
    auto v = simpleSpecParParser("1*ms");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::ms);
  }

  {
    auto v = simpleSpecParParser("1*nanosecond");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::ns);
  }

  {
    auto v = simpleSpecParParser("1*ns");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::ns);
  }

  {
    auto v = simpleSpecParParser("1*deg");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::deg);
  }

  {
    auto v = simpleSpecParParser("1*rad");
    CPPUNIT_ASSERT(v->value_ == 1.0);
    CPPUNIT_ASSERT(v->unit_ == Units::rad);
  }
}

void test_simpleSpecParParser::checkConversion() {
  using namespace cms::geometry;

  {
    ValueWithUnit v{1.0, Units::m};
    DefaultUnits d{Units::m, Units::ms, Units::deg};

    auto c = convertToDefault(v, d);
    CPPUNIT_ASSERT(c == 1.0);
  }

  {
    ValueWithUnit v{1.0, Units::cm};
    DefaultUnits d{Units::m, Units::ms, Units::deg};

    auto c = convertToDefault(v, d);
    CPPUNIT_ASSERT(c == 0.01);
  }

  {
    ValueWithUnit v{1.0, Units::mm};
    DefaultUnits d{Units::m, Units::ms, Units::deg};

    auto c = convertToDefault(v, d);
    CPPUNIT_ASSERT(c == 0.001);
  }

  {
    ValueWithUnit v{1.0, Units::mum};
    DefaultUnits d{Units::m, Units::ms, Units::deg};

    auto c = convertToDefault(v, d);
    CPPUNIT_ASSERT(c == 1e-6);
  }
  {
    ValueWithUnit v{1.0, Units::fm};
    DefaultUnits d{Units::m, Units::ms, Units::deg};

    auto c = convertToDefault(v, d);
    CPPUNIT_ASSERT(c == 1e-15);
  }

  {
    ValueWithUnit v{1.0, Units::ms};
    DefaultUnits d{Units::m, Units::ms, Units::deg};

    auto c = convertToDefault(v, d);
    CPPUNIT_ASSERT(c == 1.0);
  }

  {
    ValueWithUnit v{1.0, Units::inv_cm};
    DefaultUnits d{Units::cm, Units::ms, Units::deg};

    auto c = convertToDefault(v, d);
    CPPUNIT_ASSERT(c == 1.0);
  }
  {
    ValueWithUnit v{1.0, Units::inv_cm};
    DefaultUnits d{Units::mm, Units::ms, Units::deg};

    auto c = convertToDefault(v, d);
    CPPUNIT_ASSERT(c == 0.1);
  }

  {
    ValueWithUnit v{1.0, Units::ns};
    DefaultUnits d{Units::m, Units::ms, Units::deg};

    auto c = convertToDefault(v, d);
    CPPUNIT_ASSERT(c == 0.001);
  }
}
