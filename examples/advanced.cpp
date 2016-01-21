#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <type_traits> // std::is_same

#include "../multidim.hpp"

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


