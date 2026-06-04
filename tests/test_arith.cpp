#include "core/arith.h"
#include "wiz_test.h"

int main() {
    using namespace wiz::core;
    WIZ_CHECK_EQ(add_longs(100, 250), 350);
    WIZ_CHECK_EQ(sub_longs(1000, 999), 1);
    WIZ_CHECK_EQ(mul_long(7, 8), 56);
    WIZ_CHECK_EQ(div_long(100, 4), 25);
    WIZ_CHECK_EQ(div_long(100, 0), 0);  // protected
    WIZ_CHECK_EQ(add_longs(1'000'000'000LL, 2'000'000'000LL), 3'000'000'000LL);
    return 0;
}
