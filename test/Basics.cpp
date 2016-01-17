// This ﬁle is encoded in UTF-8.

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <iterator>  // std::begin

#include "catch.hpp"

#include "../Basics.h"

namespace md = multidim;
using std::vector;
using std::list;
using std::string;
using std::begin;

TEST_CASE( "Basics", "[multidim]" ) {
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
    std::vector<bool> vecBool = {true, false, true};
    std::vector<std::vector<bool>> vecVecBool = {{true, false, true}, {true, false, true}};

    SECTION("Basic templates") {
        CHECK(md::IsRange<decltype(scalar)>::value == false);
        CHECK(md::IsRange<decltype(vectorInt)>::value == true);
        CHECK(md::IsRange<decltype(cArray)>::value == true);
        CHECK(md::IsRange<decltype(refCArray)>::value == true);
        CHECK(md::IsRange<decltype(tricky)>::value == true);
        CHECK(md::IsRange<decltype(table)>::value == true);
        CHECK(md::IsRange<decltype(riddled)>::value == true);
        CHECK(md::IsRange<decltype(vecBool)>::value == true);
        CHECK(md::IsRange<decltype(vecVecBool)>::value == true);

        CHECK((md::IsScalar<md::NoCustomScalars, decltype(scalar)>::value) == true);
        CHECK((md::IsScalar<md::NoCustomScalars, decltype(vectorInt)>::value) == false);
        CHECK((md::IsScalar<md::NoCustomScalars, decltype(cArray)>::value) == false);
        CHECK((md::IsScalar<md::NoCustomScalars, decltype(refCArray)>::value) == false);
        CHECK((md::IsScalar<md::NoCustomScalars, decltype(tricky)>::value) == false);
        CHECK((md::IsScalar<md::NoCustomScalars, decltype(table)>::value) == false);
        CHECK((md::IsScalar<md::NoCustomScalars, decltype(table.front().front())>::value) == false);
        CHECK((md::IsScalar<md::StringsAsScalars, decltype(table.front().front())>::value) == true);
        CHECK((md::IsScalar<md::NoCustomScalars, decltype(riddled)>::value) == false);
        CHECK((md::IsScalar<md::NoCustomScalars, decltype(vecBool)>::value) == false);
        CHECK((md::IsScalar<md::NoCustomScalars, decltype(vecVecBool)>::value) == false);


#define TEST_SCALAR_TYPE(x, y) (std::is_same<decltype(md::scalarType(x)), y>::value)
#define TEST_SCALAR_TYPE_CUSTOM(x, y) (std::is_same<decltype(md::scalarType<md::StringsAsScalars>(x)), y>::value)
//#define TEST_SCALAR_TYPE_CUSTOM(x, y) (std::is_same<md::IteratorScalarType<md::StringsAsScalars, decltype(begin(x))>::type, y>::value)
        CHECK(TEST_SCALAR_TYPE(vectorInt, int));
        CHECK(TEST_SCALAR_TYPE(cArray, int));
        CHECK(TEST_SCALAR_TYPE(refCArray, int));
        CHECK(TEST_SCALAR_TYPE(tricky, int));
        CHECK(TEST_SCALAR_TYPE(table, char));
        CHECK(TEST_SCALAR_TYPE_CUSTOM(table, std::string));
        CHECK(TEST_SCALAR_TYPE(riddled, float));
        CHECK(TEST_SCALAR_TYPE(vecBool, bool));
        CHECK(TEST_SCALAR_TYPE(vecVecBool, bool));

        CHECK(md::dimensionality(scalar) == 0);
        CHECK(md::dimensionality(vectorInt) == 1);
        CHECK(md::dimensionality(cArray) == 2);
        CHECK(md::dimensionality(refCArray) == 2);
        CHECK(md::dimensionality(tricky) == 4);
        CHECK(md::dimensionality(table) == 3);
        CHECK(md::dimensionality<md::StringsAsScalars>(table) == 2);
        CHECK(md::dimensionality(riddled) == 3);
        CHECK(md::dimensionality(begin(riddled), begin(riddled)+2) == 3);
        CHECK(md::dimensionality(vecBool) == 1);
        CHECK(md::dimensionality(vecVecBool) == 2);
    }

    SECTION( "bounds") {
        vector<vector<int>> jaggedLastDimension = {{1,2,3},{4,5}};
        vector<vector<vector<int>>> jaggedInternally = {
            {{1,2,3},{4,5,6}},
            {{1,2,3},{4,5}},
        };

        CHECK(md::bounds(scalar) == (vector<size_t> {}));
        CHECK(md::bounds(vectorInt) == (vector<size_t> {8}));
        CHECK(md::bounds(begin(vectorInt), begin(vectorInt)+2) == (vector<size_t> {2}));
        CHECK(md::bounds(cArray) == (vector<size_t> {2, 3}));
        CHECK(md::bounds(refCArray) == (vector<size_t> {2, 3}));
        CHECK(md::bounds(tricky) == (vector<size_t> {0, 0, 0, 0})); // (*)
        CHECK(md::bounds(table) == (vector<size_t> {2, 2, 3}));
        CHECK(md::bounds<md::StringsAsScalars>(table) == (vector<size_t> {2, 2}));
        CHECK(md::bounds(riddled) == (vector<size_t> {5, 5, 2}));
        CHECK(md::bounds(begin(riddled), begin(riddled)+2) == (vector<size_t> {2, 5, 2}));
        CHECK(md::bounds(vecBool) == (vector<size_t> {3}));
        CHECK(md::bounds(vecVecBool) == (vector<size_t> {2, 3}));
        CHECK(md::bounds(jaggedLastDimension) == (vector<size_t> {2, 3}));
        CHECK(md::bounds(jaggedInternally) == (vector<size_t> {2, 2, 3}));

        CHECK(md::scalarSize(scalar) == 1);
        CHECK(md::scalarSize(vectorInt) == 8);
        CHECK(md::scalarSize(begin(vectorInt), begin(vectorInt)+2) == 2);
        CHECK(md::scalarSize(cArray) == 6);
        CHECK(md::scalarSize(refCArray) == 6);
        CHECK(md::scalarSize(tricky) == 0);
        CHECK(md::scalarSize(table) == 6);
        CHECK(md::scalarSize<md::StringsAsScalars>(table) == 4);
        CHECK(md::scalarSize(riddled) == 7);
        CHECK(md::scalarSize(begin(riddled), begin(riddled)+2) == 3);
        CHECK(md::scalarSize(vecBool) == 3);
        CHECK(md::scalarSize(vecVecBool) == 6);
        CHECK(md::scalarSize(jaggedLastDimension) == 5);
        CHECK(md::scalarSize(jaggedInternally) == 11);

        // (*) the last index should maybe be 5, but it's a moot point as the object cannot be filled,
        // see http://stackoverflow.com/questions/11044304/can-i-push-an-array-of-int-to-a-c-vector
    }
}



