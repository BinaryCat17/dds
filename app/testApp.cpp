#include "dds/dds.hpp"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    DdsInstance instance = ddsCreateInstance();

    ddsReadColumn(instance, "users", "gaben", [](float const* arr, DdsCount size) {
         ddsWriteColumn(instance, "users", "hackman", [](float* arrm, DdsCount sizem) {

         });
    });
}