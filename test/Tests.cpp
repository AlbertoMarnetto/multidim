// This ﬁle is encoded in UTF-8.

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <iterator>  // std::begin

#include "catch.hpp"

#include "../FlatView.h"

namespace md = multidim;
using std::vector;
using std::list;
using std::string;
using std::begin;

TEST_CASE( "ContainerHelpers", "[multidim]" ) {
    using Metrics1 = md::ContainerMetrics<md::NoCustomScalars>;
    using Metrics2 = md::ContainerMetrics<md::StringsAsScalars>;

    int scalar;
    std::vector<int> vectorInt(8);
    int cArray[2][3] = {{1, 2, 3}, {4, 5, 6}};
    const int (&refCArray)[2][3] = cArray;
    std::set<std::vector<std::vector<int[5]>>> tricky;  // Very pathologic, as it cannot be filled
    std::vector<std::list<std::string>> table = {{"Aaa", "Bb"}, {"C", ""}};
    std::vector<std::vector<std::vector<float>>> riddled = {
        {{}, {1.223, 4.56}, {}, {}, {3.141333}},
        {},
        {{0.1, 3.4}},
        {{}},
        {{}, {-4, 42.0}},
    };

    SECTION("Basic templates") {
#define TEST_CONTAINED_TYPE(x, y) (std::is_same<typename md::ContainedType<decltype(x)>::type, y>::value \
                                && std::is_same<typename md::PointedType<decltype(begin(x))>::type, y>::value)
        CHECK(TEST_CONTAINED_TYPE(vectorInt, int&));
        CHECK(TEST_CONTAINED_TYPE(cArray, int(&)[3]));
        CHECK(TEST_CONTAINED_TYPE(refCArray, const int(&)[3]));
        CHECK(TEST_CONTAINED_TYPE(tricky, const std::vector<std::vector<int[5]>>&)); // tricky is a set, contained are constants
        CHECK(TEST_CONTAINED_TYPE(table, std::list<std::string>&));
        CHECK(TEST_CONTAINED_TYPE(riddled, std::vector<std::vector<float>>&));

#define TEST_SCALAR_TYPE(x, y) (std::is_same<Metrics1::ScalarReferenceType<decltype(x)>::type, y>::value)
#define TEST_SCALAR_TYPE_CUSTOM(x, y) (std::is_same<Metrics2::ScalarReferenceType<decltype(x)>::type, y>::value)
        CHECK(TEST_SCALAR_TYPE(scalar, int&));
        CHECK(TEST_SCALAR_TYPE(vectorInt, int&));
        CHECK(TEST_SCALAR_TYPE(cArray, int&));
        CHECK(TEST_SCALAR_TYPE(refCArray, const int&));
        CHECK(TEST_SCALAR_TYPE(tricky, const int&));
        CHECK(TEST_SCALAR_TYPE(table, char&));
        CHECK(TEST_SCALAR_TYPE_CUSTOM(table, std::string&));
        CHECK(TEST_SCALAR_TYPE(riddled, float&));

        CHECK(Metrics1::isContainer(scalar) == false);
        CHECK(Metrics1::isContainer(vectorInt) == true);
        CHECK(Metrics1::isContainer(cArray) == true);
        CHECK(Metrics1::isContainer(refCArray) == true);
        CHECK(Metrics1::isContainer(tricky) == true);
        CHECK(Metrics1::isContainer(table) == true);
        CHECK(Metrics1::isContainer(riddled) == true);

        CHECK(Metrics1::dimensionality(scalar) == 0);
        CHECK(Metrics1::dimensionality(vectorInt) == 1);
        CHECK(Metrics1::dimensionality(cArray) == 2);
        CHECK(Metrics1::dimensionality(refCArray) == 2);
        CHECK(Metrics1::dimensionality(tricky) == 4);
        CHECK(Metrics1::dimensionality(table) == 3);
        CHECK(Metrics2::dimensionality(table) == 2);
        CHECK(Metrics1::dimensionality(riddled) == 3);
    }

    SECTION( "computeContainerGeometry") {
        vector<vector<int>> jaggedLastDimension = {{1,2,3},{4,5}};
        vector<vector<vector<int>>> jaggedInternally = {
            {{1,2,3},{4,5,6}},
            {{1,2,3},{4,5}},
        };

        CHECK(Metrics1::computeContainerGeometry(scalar).numberOfScalarElements == 1);
        CHECK(Metrics1::computeContainerGeometry(vectorInt).numberOfScalarElements == 8);
        CHECK(Metrics1::computeContainerGeometry(cArray).numberOfScalarElements == 6);
        CHECK(Metrics1::computeContainerGeometry(refCArray).numberOfScalarElements == 6);
        CHECK(Metrics1::computeContainerGeometry(tricky).numberOfScalarElements == 0);
        CHECK(Metrics1::computeContainerGeometry(table).numberOfScalarElements == 6);
        CHECK(Metrics2::computeContainerGeometry(table).numberOfScalarElements == 4);
        CHECK(Metrics1::computeContainerGeometry(riddled).numberOfScalarElements == 7);
        CHECK(Metrics1::computeContainerGeometry(jaggedLastDimension).numberOfScalarElements == 5);
        CHECK(Metrics1::computeContainerGeometry(jaggedInternally).numberOfScalarElements == 11);

        CHECK(Metrics1::computeContainerGeometry(scalar).bounds == (vector<size_t> {}));
        CHECK(Metrics1::computeContainerGeometry(vectorInt).bounds == (vector<size_t> {8}));
        CHECK(Metrics1::computeContainerGeometry(cArray).bounds == (vector<size_t> {2, 3}));
        CHECK(Metrics1::computeContainerGeometry(refCArray).bounds == (vector<size_t> {2, 3}));
        CHECK(Metrics1::computeContainerGeometry(tricky).bounds == (vector<size_t> {0, 0, 0, 0})); // (*)
        CHECK(Metrics1::computeContainerGeometry(table).bounds == (vector<size_t> {2, 2, 3}));
        CHECK(Metrics2::computeContainerGeometry(table).bounds == (vector<size_t> {2, 2}));
        CHECK(Metrics1::computeContainerGeometry(riddled).bounds == (vector<size_t> {5, 5, 2}));
        CHECK(Metrics1::computeContainerGeometry(jaggedLastDimension).bounds == (vector<size_t> {2, 3}));
        CHECK(Metrics1::computeContainerGeometry(jaggedInternally).bounds == (vector<size_t> {2, 2, 3}));

        // (*) the last index should maybe be 5, but it's a moot point as the object cannot be filled,
        // see http://stackoverflow.com/questions/11044304/can-i-push-an-array-of-int-to-a-c-vector
    }
}


// **************************************************************************
// Helper functions
template <typename T>
auto threeWayCopy(const T& fv) -> vector<typename std::remove_const<typename T::value_type>::type> {
    vector<typename std::remove_const<typename T::value_type>::type> result;
    result.insert(result.end(), fv.begin(), fv.end());
    result.insert(result.end(), fv.rbegin(), fv.rend());
    result.insert(result.end(), fv.begin(), fv.end());
    return result;
}

struct NotComparable {public: int a, b; bool operator ==(const NotComparable&)const{return true;}};

// **************************************************************************

TEST_CASE( "Views", "[multidim]" ) {
    SECTION("FlatView") {
        {
            // Iterator operators
            const vector<vector<int>> uriahFuller = {{},{1,2,3,},{4},{},{},{5,6}};
            auto fv = md::makeFlatView(uriahFuller);
            auto it1 = fv.begin();

            CHECK(*(it1) == 1);
            ++it1;
            CHECK(*(it1) == 2);
            --it1;
            CHECK(*(it1) == 1);

            CHECK(it1[1] == 2);

            auto it2 = it1++;
            CHECK(*(it1) == 2);
            CHECK(*(it2) == 1);
            CHECK(it2 != it1);
            CHECK((it2 == it1) == false);

            auto it3 = it1--;
            CHECK(*(it1) == 1);
            CHECK(*(it3) == 2);

            CHECK(it3 == (it1 + 1));
            CHECK(it3 == (1 + it1));
            CHECK(it1 == (it3 - 1));
        }
        {
            // The same but on a non-nested container
            // (to test the other specialization of FlatViewIterator)
            const vector<int> simple = {1,2,3,4,5,6};

            auto fv = md::makeFlatView(simple);
            auto it1 = fv.begin();

            CHECK(*(it1) == 1);
            ++it1;
            CHECK(*(it1) == 2);
            --it1;
            CHECK(*(it1) == 1);

            CHECK(it1[1] == 2);

            auto it2 = it1++;
            CHECK(*(it1) == 2);
            CHECK(*(it2) == 1);
            CHECK(it2 != it1);
            CHECK((it2 == it1) == false);

            auto it3 = it1--;
            CHECK(*(it1) == 1);
            CHECK(*(it3) == 2);

            CHECK(it3 == (it1 + 1));
            CHECK(it3 == (1 + it1));
            CHECK(it1 == (it3 - 1));

            std::cout << sizeof(it1) << " " << sizeof(&simple[0]);

        }
        {
            // Iteration, forward and back
            const int doctorMatrix[2][3] = {{1,2,3,},{4,5,6}};
            const vector<vector<int>> uriahFuller = {{},{1,2,3,},{4},{},{},{5,6}};
            const vector<vector<string>> table = {{"Aa!", "Bb"}, {"C", ""}};

            CHECK((threeWayCopy(md::makeFlatView(doctorMatrix))) == (vector<int> {1,2,3,4,5,6,6,5,4,3,2,1,1,2,3,4,5,6}));
            CHECK((threeWayCopy(md::makeFlatView(uriahFuller))) == (vector<int> {1,2,3,4,5,6,6,5,4,3,2,1,1,2,3,4,5,6}));
            CHECK((threeWayCopy(md::makeFlatView(table))) == (vector<char> {'A','a','!','B','b','C','C','b','B','!','a','A','A','a','!','B','b','C',}));
            // Custom scalars
            CHECK((threeWayCopy(md::makeFlatView<md::StringsAsScalars>(table))) == (vector<string> {"Aa!", "Bb", "C", "", "", "C", "Bb", "Aa!", "Aa!", "Bb", "C", "", }));
            // Range version
            CHECK((threeWayCopy(md::makeFlatView(begin(uriahFuller), begin(uriahFuller)+3))) == (vector<int> {1,2,3,4,4,3,2,1,1,2,3,4,}));
        }
        {
            // Check size() and empty
            vector<vector<int>> formallyEmpty;
            vector<vector<int>> actuallyEmpty = {{}};
            vector<vector<int>> full = {{1}};

            auto fv1 = md::makeFlatView(formallyEmpty);
            auto fv2 = md::makeFlatView(actuallyEmpty);
            auto fv3 = md::makeFlatView(full);

            CHECK(size(fv1) == 0);
            CHECK(fv1.empty() == true);
            CHECK(size(fv2) == 0);  // note that instead size(actuallyEmpty) == 1!
            CHECK(fv2.empty() == true);
            CHECK(size(fv3) == 1);
            CHECK(fv3.empty() == false);
        }
        {
            // FlatView operators and auxiliary functions
            vector<vector<int>> uriahFuller1 = {{},{1,2,3,},{4},{},{},{5,6}};
            vector<vector<int>> uriahFuller2 = {{1},{2,3,},{},{},{},{4,5,6}};   // same scalars, in different locations
            vector<vector<int>> uriahFuller3 = {{},{1,2,3,},{4999},{},{},{5,6}};  // one altered element

            auto fv1 = md::makeFlatView(uriahFuller1);
            auto fv2 = md::makeFlatView(uriahFuller2);
            auto fv3 = md::makeFlatView(uriahFuller3);

            // Check if all comparisons work
            CHECK(fv1 == fv2);
            CHECK(fv1 <= fv2);
            CHECK(fv1 >= fv2);
            CHECK(fv1 != fv3);
            CHECK(fv1 < fv3);
            CHECK(fv1 <= fv3);
            CHECK(fv3 > fv1);
            CHECK(fv3 >= fv1);

            CHECK(fv1[2] == 3);

            *(fv3.begin()) = 3;
            // *(fv3.cbegin()) = 3;  <-- cannot compile
        }
        {
            // Check if a FlatView of non-comparable scalar type
            // can be instantiated
            vector<NotComparable> v1(3);
            vector<NotComparable> v2(4);

            auto fv1 = md::makeFlatView(v1);
            auto fv2 = md::makeFlatView(v2);

            CHECK(fv1.size() == 3);
            CHECK(fv2.size() == 4);
            CHECK((fv1 == fv2) == false);
            CHECK(fv1 != fv2);
            //CHECK(fv1 <= fv2);  <-- cannot compile
            //CHECK(fv1 >= fv2);  <-- cannot compile
            // CHECK(fv1 < fv2);  <-- cannot compile
            //CHECK(fv1 <= fv2);  <-- cannot compile
            //CHECK(fv1 > fv2);  <-- cannot compile
            //CHECK(fv1 >= fv2);  <-- cannot compile
        }
        {
            // Check if FlatView of a FlatView works
            const vector<vector<int>> uriahFuller = {{},{1,2,3,},{4},{},{},{5,6}};

            auto first = begin(uriahFuller);
            auto last = begin(uriahFuller) + 3;

            auto fv2 = md::makeFlatView(first, last);
            auto fv = md::makeFlatView(fv2);
            CHECK(threeWayCopy(fv) == (vector<int> {1,2,3,4,4,3,2,1,1,2,3,4,}));
        }
        {
            // Alteration
            vector<vector<int>> uriahFuller = {{},{1,2,3,},{4},{},{},{5,6}};
            vector<vector<string>> table = {{"Aa!", "Bb"}, {"C", ""}};
            vector<vector<string>> table2 = {{"Aa!", "Bb"}, {"C", ""}};

            auto fv1 = md::makeFlatView(uriahFuller);
            auto fv2 = md::makeFlatView(table);
            auto fv3 = md::makeFlatView<md::StringsAsScalars>(table2);
            fv1[2] += fv1[2];
            fv2[2] += fv2[2];
            fv3[2] += fv3[2];

            CHECK((threeWayCopy(fv1)) == (vector<int> {1,2,3+3,4,5,6,6,5,4,3+3,2,1,1,2,3+3,4,5,6}));
            CHECK((threeWayCopy(fv2))  == (vector<char> {'A','a','!'+'!','B','b','C','C','b','B','!'+'!','a','A','A','a','!'+'!','B','b','C',}));
            CHECK((threeWayCopy(fv3))  == (vector<string> {"Aa!", "Bb", "CC", "", "", "CC", "Bb", "Aa!", "Aa!", "Bb", "CC", "", }));
        }
        {
            // Standard algorithms
            vector<vector<int>> uriahFuller1 = {{},{1,2,3,},{4},{},{},{5,6}};
            vector<vector<int>> uriahFuller2 = {{},{1,2,3,},{4},{},{},{5,6}};

            auto fv1 = md::makeFlatView(uriahFuller1);
            std::copy(begin(fv1), begin(fv1)+3, begin(fv1)+3);
            CHECK((uriahFuller1 == vector<vector<int>>{{},{1,2,3,},{1},{},{},{2,3}}) == true);

            auto fv2 = md::makeFlatView(uriahFuller2);
            std::remove_if(begin(fv2), end(fv2), [](int n)->bool{return (n%2)==0;});
            CHECK((uriahFuller2[1]) == (vector<int>{1,3,5,}));
        }

    }
}



