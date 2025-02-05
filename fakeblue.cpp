#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#if __BMI2__
#include <immintrin.h>
#endif

using u64 = uint64_t;
using u32 = uint32_t;

struct pcg {
    u64 state, inc;

    u32 operator()() {
        u64 oldstate = state;
        state = oldstate * 6364136223846793005ull + inc;
        u32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
        u32 rot = oldstate >> 59u;
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    pcg(u64 seed = time(0), u64 sequence = 12345) : state(0), inc(sequence | 1) {
        operator()();
        state += seed;
        operator()();
    }

};

struct blue {
    std::string field;
    u32 depth, density;

    template <typename T = pcg>
    blue(u32 depth, u32 density, T rng = {}) : depth(depth), density(density) {
        u32 side = 1 << depth;
        u32 npoints = 1 << density;
        field.resize(side / 8 * side);

        std::cerr << "size: " << side << 'x' << side << '\n';
        std::cerr << "noise: " << npoints << " points\n";

        u32 shift = depth * 2 - density;
        u32 mask = ~((-1u) << shift);

        auto demorton = [](u32 x) {
            #if __BMI2__
            return std::pair{_pext_u32(x, 0x55555555), _pext_u32(x, 0xaaaaaaaa)};
            #endif

            auto pext = [](u32 x) {
                x = (x | (x >> 1)) & 0x33333333;
                x = (x | (x >> 2)) & 0x0f0f0f0f;
                x = (x | (x >> 4)) & 0x00ff00ff;
                x = (x | (x >> 8)) & 0x0000ffff;
                return x;
            };
            return std::pair{pext(x & 0x55555555), pext((x & 0xaaaaaaaa) >> 1)};
        };

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        for (u32 i = 0; i < npoints; i++) {
            // construct a random point with a set leading sequence
            auto point = (rng() & mask) | (i << shift);
            // interpret it as a point on a morton curve
            auto [x, y] = demorton(point);
            auto idx = x * side + y;
            field[idx / 8] |= 1 << (idx % 8);
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        std::cerr << "generated in "
                  << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin) << '\n';
    }

    void topbm(std::ostream &out = std::cout) {
        u32 side = 1 << depth;

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        out << "P4 " << side << ' ' << side << '\n' << field;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cerr << "dumped in "
                  << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin) << '\n';
    }
};

int main(int argc, char **argv) {
    auto depth = argc > 1 ? atoi(argv[1]) : 10;
    auto density = argc > 2 ? atoi(argv[2]) : 16;

    if (depth <= 0 || depth > 16 || density <= 0 || density > depth * 2) {
        std::cerr << "invalid parameters.  requirements:\n";
        std::cerr << "0 < depth <= 16\n";
        std::cerr << "0 < density <= depth*2\n";
        return 1;
    }

    auto file = argc > 3 ? argv[3] : "blue.pbm";
    std::cerr << "output: " << file << '\n';
    std::ofstream out(file);
    blue(depth, density).topbm(out);
}
