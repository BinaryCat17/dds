#include <iostream>
#include <cista/serialization.h>
#include <cista/containers.h>
#include <cista/mmap.h>

int main() {
    namespace data = cista::offset;
    constexpr auto const MODE = cista::mode::WITH_VERSION | cista::mode::WITH_INTEGRITY;

    struct pos {
        int x, y;
    };

    using vec = data::vector<pos>;

    {
        vec positions{{1, 2}, {3, 4}, {5, 6}, {7, 8}};
        cista::buf mmap{cista::mmap{"data"}};
        cista::serialize<MODE>(mmap, positions);
    }

    auto b = cista::mmap("data", cista::mmap::protection::MODIFY);
    auto positions = cista::deserialize<vec, MODE>(b);
    positions->push_back({5, 5});

    for(auto pos : *positions) {
        std::cout << pos.x << " " << pos.y << std::endl;
    }
}
