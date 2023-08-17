#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define atomic_xchg(ptr, i) ({                       \
    __atomic_exchange_n(ptr, (i), __ATOMIC_SEQ_CST); \
})

#define atomic_add(ptr, n) \
    (__atomic_fetch_add(ptr, n, __ATOMIC_SEQ_CST))

#define x(i) (i)
#define zero_r x(0)
#define ra_r x(1)

#define mask_1  0b1
#define mask_3  0b111
#define mask_5  0b11111
#define mask_7  0b1111111
#define mask_12 0b111111111111

#define GEN_I(imm, rs1, fn3, rd, op)      \
    (                                     \
        (((imm) & mask_12) << 20) |       \
        (((rs1) & mask_5)  << 15) |       \
        (((fn3) & mask_3)  << 12) |       \
        (((rd)  & mask_5)  << 7)  |       \
        ((op)   & mask_7)                 \
    )
#define gen_jalr(rd, offset, rs1)    GEN_I(offset, rs1, 0b000, rd, 0b1100111)
#define gen_ret                      gen_jalr(zero_r, 0UL, ra_r)

typedef void (*Fn)();
__attribute__((section(".myrodata"))) const unsigned int fn[] = {gen_ret};

#define bool int
#define true 1
#define false 0

bool isPrime(int x) {
    for (int i = 2; i * i <= x; ++i) {
        if (x % i == 0) {
            return false;
        }
    }
    return true;
}

int countPrimes(int n) {
    int ans = 0;
    for (int i = 2; i < n; ++i) {
        ans += isPrime(i);
    }
    return ans;
}

typedef int32_t target_int;
typedef uint32_t target_uint;
typedef int32_t target_long;
typedef uint32_t target_ulong;
typedef uint16_t target_hulong;
typedef uint32_t target_uintptr;
#define TARGET_LONG_BITS (sizeof(target_ulong) << 3)

void wmulu(target_ulong x, target_ulong y, target_ulong *hi, target_ulong *lo)
{
    const uint32_t half_long_bits = TARGET_LONG_BITS >> 1;
    const target_ulong x0 = (target_hulong)x, x1 = x >> half_long_bits;
    const target_ulong y0 = (target_hulong)y, y1 = y >> half_long_bits;
    const target_ulong p11 = x1 * y1, p01 = x0 * y1;
    const target_ulong p10 = x1 * y0, p00 = x0 * y0;
    const target_ulong c = (target_ulong) ((p00 >> half_long_bits) + (target_hulong)p10 + (target_hulong)p01) >> half_long_bits;

    *lo = p00 + (p10 << half_long_bits) + (p01 << half_long_bits);
    *hi = p11 + (p10 >> half_long_bits) + (p01 >> half_long_bits) + c;
}

void wmul(target_long x, target_long y, target_long *hi, target_long *lo)
{
    wmulu((target_ulong)x, (target_ulong)y, (target_ulong *)hi, (target_ulong *)lo);
    if (x < 0LL) {
        *hi -= y;
    }
    if (y < 0LL) {
        *hi -= x;
    }
}

void wmulsu(target_long x, target_ulong y, target_long *hi, target_long *lo)
{
    wmulu((target_ulong)x, (target_ulong)y, (target_ulong *)hi, (target_ulong *)lo);
    if (x < 0LL) {
        *hi -= y;
    }
}

int foo()
{
    target_ulong x, y, hi, lo;

    int n = countPrimes(200000);
    printf("hello, rvvk, 0x%d!\n", n);
    puts("");

    x = 0x8fff0000;
    y = 0xffffff00;
    printf("      x: 0x%08x\n      y: 0x%08x\n", x, y);

    wmulu(x, y, &hi, &lo);
    printf("  wmulu: 0x%08x%08x\n", hi, lo);
    printf("compare: 0x%016lx\n", (uint64_t)((uint32_t)x) * (uint64_t)((uint32_t)y));

    wmul((target_long)x, (target_long)y, (target_long *)&hi, (target_long *)&lo);
    printf("   wmul: 0x%08x%08x\n", hi, lo);
    printf("compare: 0x%016lx\n", (int64_t)((int32_t)x) * (int64_t)((int32_t)y));

    wmulsu((target_long)x, (target_ulong)y, (target_long *)&hi, (target_long *)&lo);
    printf(" wmulsu: 0x%08x%08x\n", hi, lo);
    printf("compare: 0x%016lx\n", (int64_t)((int32_t)x) * (uint64_t)((uint32_t)y));
    puts("");

    x = 0x9fff1234;
    y = 0xf78ef3ab;
    printf("      x: 0x%08x\n      y: 0x%08x\n", x, y);

    wmulu(x, y, &hi, &lo);
    printf("  wmulu: 0x%08x%08x\n", hi, lo);
    printf("compare: 0x%016lx\n", (uint64_t)((uint32_t)x) * (uint64_t)((uint32_t)y));

    wmul((target_long)x, (target_long)y, (target_long *)&hi, (target_long *)&lo);
    printf("   wmul: 0x%08x%08x\n", hi, lo);
    printf("compare: 0x%016lx\n", (int64_t)((int32_t)x) * (int64_t)((int32_t)y));

    wmulsu((target_long)x, (target_ulong)y, (target_long *)&hi, (target_long *)&lo);
    printf(" wmulsu: 0x%08x%08x\n", hi, lo);
    printf("compare: 0x%016lx\n", (int64_t)((int32_t)x) * (uint64_t)((uint32_t)y));
    puts("");

    uint64_t xx = 0xffffeeeeddddcccc;
    uint64_t yy = 0x1111222283334444;
    printf("      xx: 0x%016lx\n      yy: 0x%016lx\n", xx, yy);
    yy = atomic_xchg((int32_t *) &xx, yy);
    puts("atomic swap:");
    printf("      xx: 0x%016lx\n      yy: 0x%016lx\n", xx, yy);
    puts("");

    xx = 0xffffeeeeddddcccc;
    yy = 0x1111222283334444;
    printf("      xx: 0x%016lx\n      yy: 0x%016lx\n", xx, yy);
    yy = atomic_add((int32_t *) &xx, yy);
    puts("atomic add:");
    printf("      xx: 0x%016lx\n      yy: 0x%016lx\n", xx, yy);
    puts("");

    puts("end");
    return 0;
}

int main(int argc, char *argv[])
{
    // exit(0);
    // ((Fn)fn)();
    return foo();
}
