# The `multidim` library

`multidim` is a header-only C++11 library, helping you to deal with nested containers of arbitrary dimensionality. Here are the functions it offers:

  - `dimensionality`: returns the nesting level of a container (e.g. a container of containers of containers has a dimensionality of 3).
  - `bounds`: returns the bounds of a nested container, i.e. its maximum sizes in all its subdimensions
  - `scalarSize`: returns the number of "leaf" elements of a nested container
  - `scalarType`: its return type (to be used with `decltype`) is the type of the "leaf" elements of a nested container
  - `makeFlatView`: returns a `FlatView` of the container, i.e. a class allowing to iterate through the leaf elements as if they were contained in a 1D range
  - `makeBoxedView`: returns a `BoxedView` of the container, i.e. a class allowing to access it as a C array with custom bounds, allowing square-bracket access. The view also eliminates jaggedness by returning a default element in case of out-of-bounds access.

All the functions can also be called with ranges (iterator pairs). Moreover, their behavior can be customized by indicating containers classes which should be considered as "leaf" elements (e.g. `string`s).

### Sample 1: basic concepts

```c++
#include <iostream>
#include <vector>
#include <type_traits> // std::is_same

#include "multidim.hpp"

using std::cout;
using std::vector;

int main() {
    // For this example, a vector will be used,
    // but any standard-conforming container allowing mutation
    // could be exployed in alternative

    auto test = vector<vector<int>>{{},{1,2,3,},{4},{}};

    // dimensionality
    cout << multidim::dimensionality(test) << "\n";
    // output: 2, since it's a container of containers

    // bounds
    auto testBounds = multidim::bounds(test);
    cout << "[" << testBounds[0] << ", " << testBounds[1] << "]\n";
    // output: [4, 3]: `test` holds 4 childs (sub-vectors),
    // each of whom holds a maximum of 3 childs  (ints)

    // scalarSize
    cout << multidim::scalarSize(test) << "\n";
    // output: 4, since a total of 4 ints are stored

    // scalarType
    cout << std::is_same<decltype(multidim::scalarType(test)), int>::value << "\n";
    // output: 1 (i.e. "true"), as the leaf elements of `test` are ints

    // makeFlatView
    auto flatView = multidim::makeFlatView(test);
    // flatView now behaves as a container holding {1,2,3,4,}
    // One can read ...
    for (int value : flatView) cout << value << ",";
    cout << "\n";
    // output: 1,2,3,4,
    // ... and write the values as if they were stored in a linear array
    flatView[3] = 42;
    // Still, one is actually accessing the original container
    cout << test[2][0] << "\n";
    // output: 42

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
    cout << boxedView[2][0] << "\n";
    // output: 4
    // ...and outside the physical limits
    cout << boxedView[2][1] << "\n";
    // output: 99
    // The same goes for assigment
    boxedView[2][0] = 42;
    cout << test[2][0] << "\n";
    // output: 42
    // Modifications outside the limits of the undelying container
    // are allowed and simply ignored
    boxedView[2][1] = 42; // has no effect
}
```

### Sample 2: advanced concepts

```c++
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <type_traits> // std::is_same

#include "multidim.hpp"

using std::cout;
using std::vector;
using std::string;

// A custom scalar policy: it must be a templated class
// offering a "constexpr bool isCustomScalar" member
template<typename T>
struct VectorStringAsScalar {
    static constexpr bool isCustomScalar =
        std::is_same<
            typename std::decay<T>::type,
            std::vector<std::string>>
        ::value;
};

int main() {
	// *** Scalar policies
    vector<vector<string>> test1 = {{"A1", "A2", "A3"}, {"B1", "B2"}};
	cout << multidim::dimensionality(test1) << "\n";
	// output: 3, since string is considered a container of chars
	// Anyway, for some use cases it would be more appropriate to treat
	// strings as if they were scalar types. This is easily done:
	cout << multidim::dimensionality<multidim::StringsAsScalars>(test1) << "\n";
	// output: 2, since string is considered a container of chars

	// Scalar policies work with all the function of multidim
	auto flatView1 = multidim::makeFlatView<multidim::StringsAsScalars>(test1);
	for (auto& value : flatView1) cout << value << ",";
	cout << "\n";
	// output: A1,A2,A3,B1,B2

	// Example of use of a custom scalar policy
	cout << multidim::dimensionality<VectorStringAsScalar>(test1) << "\n";
	// output: 1

	// *** Proxied containers
	// The multidim library works also with proxied "containers"
	// like std::vector<bool>
	vector<vector<bool>> test2 = {{true, false},{true}};
	auto flatView2 = multidim::makeFlatView(test2);
	for (int value : flatView2) cout << value << ",";
	// output: 1,0,1

	// This also means that one can create a FlatView of a BoxedView
	vector<vector<int>> test3  = {{1,2},{3}};
	auto boxedView3 = multidim::makeBoxedView(test3, 99, {3,3});
	auto flatView3 = multidim::makeFlatView(boxedView3);
	for (int value : flatView3) cout << value << ",";
	// output: 1,2,99,3,99,99,99,99,99
}
```

### Version
0.9.0


### Installation

Just `#include "multidim.hpp"` in your source file. The only prerequisite is a compiler supporting C++11.


### License

MIT
