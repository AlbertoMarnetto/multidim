#define RUN_TESTS 1

#if RUN_TESTS

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#else  // run sample program

#include <iostream>
#include <vector>
#include <type_traits> // std::is_same

#include "../multidim.hpp"

using std::cout;
using std::vector;

int main() {
    // For this example, a vector will be used,
    // but any standard-conforming container allowing mutation
    // could be exployed in alternative

    auto test = vector<vector<int>>{{},{1,2,3,},{4},{}};

    // dimensionality
    cout << multidim::dimensionality(test) << "\n";
    // 2, since it's a container of containers

    // bounds
    auto testBounds = multidim::bounds(test);
    cout << "[" << testBounds[0] << ", " << testBounds[1] << "]\n";
    // [4, 3]: `test` holds 4 childs (sub-vectors),
    // each of whom holds a maximum of 3 childs  (ints)

    // scalarSize
    cout << multidim::scalarSize(test) << "\n";
    // 4, since a total of 4 ints are stored

    // scalarType
    cout << std::is_same<decltype(multidim::scalarType(test)), int>::value << "\n";
    // 1 (i.e. "true"), as the leaf elements of `test` are ints

    // makeFlatView
    auto flatView = multidim::makeFlatView(test);
    // flatView now behaves as a container holding {1,2,3,4,}
    // One can read ...
    for (int value : flatView) cout << value << ",\n";
    // ... and write the values as if they were stored in a linear array
    flatView[3] = 42;
    // Still, one is actually accessing the original container
    cout << test[2][0] << "\n"; // 42

    // Restore the original state of `test`
    test = vector<vector<int>>{{},{1,2,3,},{4},{}};

    // makeBoxedView
    // To make a BoxedView, one has to provide
    // * a default element to be used in case of out-of-bounds access
    //   (99 in this example)
    // * the list of bounds of the view. In alternative, "{}" will
    //   use the result of multidim::bounds

    auto boxedView = multidim::makeBoxedView(test, 99, {});
    // boxedView now behaves as a int[4][3] initialized as
    // {{99,99,99}, {1,2,3}, {4,99,99}, {99,99,99}}

    // Again, one can read inside...
    cout << boxedView[2][0] << "\n"; // 4
    // ...and outside the physical limits
    cout << boxedView[2][1] << "\n"; // 99
    // The same goes for assigment
    boxedView[2][0] = 42;
    cout << test[2][0] << "\n"; // 42
    // Modifications outside the limits of the undelying container
    // are allowed and simply ignored
    boxedView[2][1] = 42; // has no effect
}

#endif

