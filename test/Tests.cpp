// This ﬁle is encoded in UTF-8.

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

    SECTION("Testing basic templates") {
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
template <typename Scalar, template<typename> class IsCustomScalar, typename Container>
auto threeWayFlatten(const Container& container) -> vector<Scalar> {
    vector<Scalar> result;
    auto fv = md::makeFlatView<IsCustomScalar>(container);
    result.insert(result.end(), fv.begin(), fv.end());
    result.insert(result.end(), fv.rbegin(), fv.rend());
    result.insert(result.end(), fv.begin(), fv.end());
    return result;
}

template <template<typename> class IsCustomScalar, typename Container>
auto alteredCopy(const Container& original) -> Container {
    Container result = original;
    auto fv = md::makeFlatView<IsCustomScalar>(result);
    auto it = fv.begin();
    ++++it;
    *it = (*it) + (*it);
    return result;
}
// **************************************************************************

TEST_CASE( "Views", "[multidim]" ) {

    const int doctorMatrix[2][3] = {{1,2,3,},{4,5,6}};
    const vector<vector<int>> uriahFuller = {{},{1,2,3,},{4},{},{},{5,6}};
    const vector<vector<string>> table = {{"Aa!", "Bb"}, {"C", ""}};

    SECTION("FlatView") {
        {
            // Visitation
            CHECK((threeWayFlatten<int, md::NoCustomScalars>(doctorMatrix)) == (vector<int> {1,2,3,4,5,6,6,5,4,3,2,1,1,2,3,4,5,6}));
            CHECK((threeWayFlatten<int, md::NoCustomScalars>(uriahFuller)) == (vector<int> {1,2,3,4,5,6,6,5,4,3,2,1,1,2,3,4,5,6}));
            CHECK((threeWayFlatten<char, md::NoCustomScalars>(table)) == (vector<char> {'A','a','!','B','b','C','C','b','B','!','a','A','A','a','!','B','b','C',}));
            CHECK((threeWayFlatten<string, md::StringsAsScalars>(table)) == (vector<string> {"Aa!", "Bb", "C", "", "", "C", "Bb", "Aa!", "Aa!", "Bb", "C", "", }));
        }
        {
            // Visitation, range version
            vector<int> flattened;
            auto first = begin(uriahFuller);
            auto last = begin(uriahFuller) + 3;

            auto fv = md::makeFlatView<md::NoCustomScalars>(first, last);
            flattened.insert(flattened.end(), fv.begin(), fv.end());
            flattened.insert(flattened.end(), fv.rbegin(), fv.rend());
            flattened.insert(flattened.end(), fv.begin(), fv.end());
            CHECK(flattened == (vector<int> {1,2,3,4,4,3,2,1,1,2,3,4,}));
        }
        {
            // Alteration
            CHECK((threeWayFlatten<int, md::NoCustomScalars>(alteredCopy<md::NoCustomScalars>(uriahFuller)))
                   == (vector<int> {1,2,3+3,4,5,6,6,5,4,3+3,2,1,1,2,3+3,4,5,6}));
            CHECK((threeWayFlatten<char, md::NoCustomScalars>(alteredCopy<md::NoCustomScalars>(table)))
                   == (vector<char> {'A','a','!'+'!','B','b','C','C','b','B','!'+'!','a','A','A','a','!'+'!','B','b','C',}));
            CHECK((threeWayFlatten<string, md::StringsAsScalars>(alteredCopy<md::StringsAsScalars>(table)))
                   == (vector<string> {"Aa!", "Bb", "CC", "", "", "CC", "Bb", "Aa!", "Aa!", "Bb", "CC", "", }));
        }

    }
}



