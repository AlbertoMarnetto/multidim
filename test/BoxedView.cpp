// This ﬁle is encoded in UTF-8.

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <iterator>  // std::begin

#include "catch.hpp"

#include "../BoxedView.h"

namespace md = multidim;
using std::vector;
using std::list;
using std::string;
using std::begin;


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

TEST_CASE( "BoxedView", "[multidim]" ) {
    SECTION("Iteration 1") {
        // Iterator operators
        const vector<int> simple = {1,2,3,4,5,6};

        auto bv = md::makeBoxedView(simple, 0);
        auto it1 = bv.begin();

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

        it1 = bv.begin();
        auto it0 = it1 - 1;
        it2 = bv.end();
        it3 = it2 - 1;
        CHECK(it0 != it1);
        CHECK(it0 != it2);
        CHECK(it2 != it3);

        decltype(bv)::const_iterator converted = bv.begin();
    }
    SECTION("Iteration 2") {

        const vector<vector<int>> uriahFuller = {{},{1,2,3,},{4},{},{},{5,6}};

        auto bv = md::makeBoxedView(uriahFuller, 0);
        auto it1 = bv[1];

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

        it1 = bv[1];
        auto it0 = it1 - 1;
        it2 = bv[1]+2;
        it3 = it2 - 1;
        CHECK(it0 != it1);
        CHECK(it0 != it2);
        CHECK(it2 != it3);
    }
    SECTION("Iteration 3") {
        int test1[2][3] = {{1,2,3,},{4,5,6},};
        for (auto it1 = std::begin(test1[0]); it1 < std::end(test1[0]); ++it1) std::cout << *it1;
    }
}
