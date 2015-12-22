﻿// This ﬁle is encoded in UTF-8.

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <iterator>  // std::begin

#include "catch.hpp"

#include "../FlatView.h"
#include "../BoxedView.h"

namespace md = multidim;
using std::vector;
using std::list;
using std::string;
using std::begin;

TEST_CASE( "ContainerHelpers", "[multidim]" ) {
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


#define TEST_SCALAR_TYPE(x, y) (std::is_same<md::IteratorScalarType<md::NoCustomScalars, decltype(begin(x))>::type, y>::value)
#define TEST_SCALAR_TYPE_CUSTOM(x, y) (std::is_same<md::IteratorScalarType<md::StringsAsScalars, decltype(begin(x))>::type, y>::value)
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
            CHECK((it3 - it1) == 1);
            CHECK((it1 - it3) == -1);

            it1 = fv.begin();
            auto it0 = it1 - 1;
            it2 = fv.end();
            it3 = it2 - 1;
            CHECK(it0 != it1);
            CHECK(it0 != it2);
            CHECK(it2 != it3);

            decltype(fv)::const_iterator converted = fv.begin();
        }
        {

            // The same but on a nested container
            // (to test the other specialization of FlatViewIterator)

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
            CHECK((it3 - it1) == 1);
            CHECK((it1 - it3) == -1);

            it1 = fv.begin();
            auto it0 = it1 - 1;
            it2 = fv.end();
            it3 = it2 - 1;
            CHECK(it0 != it1);
            CHECK(it0 != it2);
            CHECK(it2 != it3);

            decltype(fv)::const_iterator converted = fv.begin();
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
            // Alteration (including proxied "container")
            vector<vector<int>> uriahFuller = {{},{1,2,3,},{4},{},{},{5,6}};
            vector<vector<string>> table = {{"Aa!", "Bb"}, {"C", ""}};
            vector<vector<string>> table2 = {{"Aa!", "Bb"}, {"C", ""}};
            vector<vector<bool>> vecVecBool = {{true, false,}, {true, true,}};

            auto fv1 = md::makeFlatView(uriahFuller);
            auto fv2 = md::makeFlatView(table);
            auto fv3 = md::makeFlatView<md::StringsAsScalars>(table2);
            auto fv4 = md::makeFlatView(vecVecBool);
            fv1[2] += fv1[2];
            fv2[2] += fv2[2];
            fv3[2] += fv3[2];
            fv4[2] = !fv4[2];

            CHECK((threeWayCopy(fv1)) == (vector<int> {1,2,3+3,4,5,6,6,5,4,3+3,2,1,1,2,3+3,4,5,6}));
            CHECK((threeWayCopy(fv2))  == (vector<char> {'A','a','!'+'!','B','b','C','C','b','B','!'+'!','a','A','A','a','!'+'!','B','b','C',}));
            CHECK((threeWayCopy(fv3))  == (vector<string> {"Aa!", "Bb", "CC", "", "", "CC", "Bb", "Aa!", "Aa!", "Bb", "CC", "", }));
            CHECK((threeWayCopy(fv4))  == (vector<bool> {true, false, false, true, true, false, false, true, true, false, false, true, }));
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
    SECTION("BoxedView") {
        {
            // Iterator operators
            const vector<int> simple = {1,2,3,4,5,6};

            auto fv = md::makeBoxedView(simple, 0);
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
            CHECK((it3 - it1) == 1);
            CHECK((it1 - it3) == -1);

            it1 = fv.begin();
            auto it0 = it1 - 1;
            it2 = fv.end();
            it3 = it2 - 1;
            CHECK(it0 != it1);
            CHECK(it0 != it2);
            CHECK(it2 != it3);

            decltype(fv)::const_iterator converted = fv.begin();
        }
    }
}



