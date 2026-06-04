#include "core/rng.h"
#include "wiz_test.h"

int main() {
    using namespace wiz::core;
    Rng r(42);
    for (int i = 0; i < 1000; ++i) {
        int v = r.range(1, 6);
        WIZ_CHECK(v >= 1 && v <= 6);
    }
    Rng a(123), b(123);
    for (int i = 0; i < 10; ++i) {
        WIZ_CHECK_EQ(a.next_u32(), b.next_u32());
    }
    int total = 0;
    Rng dd(7);
    for (int i = 0; i < 10000; ++i) total += dd.dice(1, 8);
    // 1d8 average ~ 4.5 → 45000
    WIZ_CHECK(total > 42000 && total < 48000);
    return 0;
}
