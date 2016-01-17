// This ﬁle is encoded in UTF-8.

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <iterator>  // std::begin

#include "catch.hpp"

#include "../multidim.hpp"

namespace md = multidim;
using std::vector;
using std::list;
using std::string;
using std::begin;
using std::end;


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

        auto bv = md::makeBoxedView(simple, 0, {});
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

        CHECK(std::distance(it1, it2) == 6);

        decltype(bv)::const_iterator converted = bv.begin();
    }
    SECTION("Iteration 2") {
        const vector<vector<int>> uriahFuller = {{},{1,2,3,},{4},{},{},{5,6}};

        auto bv = md::makeBoxedView(uriahFuller, 0, {});
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

        CHECK(std::distance(it1, it2) == 2);
    }
    SECTION("Physical and apparent limits 1") {   // apparent > physical
        const vector<vector<int>> uriahFuller = {{},{1,2,3,},{4},{},{},{5,6}};

        auto bv = md::makeBoxedView(uriahFuller, 42, {});  // bv appears as a int[6][3]
        auto it = bv[2];

        CHECK(*(it) == 4);
        ++it; // it is now bv[2][1], points to a defaulted value
        CHECK(*(it) == 42);
        ++it; // it is now bv[2][2], points to a defaulted value
        CHECK(*(it) == 42);
        ++it; // it is now bv[2][3], must throw since is out of the view bounds
        CHECK_THROWS(*it);
    }
    SECTION("Physical and apparent limits 2") {   // apparent < physical
        const vector<vector<int>> test = {{1,2,3},{4,5,6}};

        auto bv = md::makeBoxedView(test, 42, {1,2});
        auto it = bv[0];

        CHECK(*(it) == 1);
        ++it; // it is now bv[0][1]
        CHECK(*(it) == 2);
        ++it;
        // it is now bv[0][2], points to a value which physically exists
        // but is outside the view limits
        CHECK_THROWS(*it);

        // Also bv[1] is outside the limits
        CHECK_THROWS(bv[1]);
    }
    SECTION("Mutation") {
        vector<vector<int>> before = {{},{1,2,3,},{4},{},{},{5,6}};
        vector<vector<int>> after = {{},{1,2,55,},{4},{},{},{5,6}};

        auto bv = md::makeBoxedView(before, 42, {});

        bv[1][2] = 55;
        CHECK(before == after);

        bv[0][1] = 66; // must be quietly ignored
        CHECK(before == after);
    }
    SECTION("Partial dereference") {
        vector<vector<vector<int>>> empty = {{{1}}};
        auto bv = md::makeBoxedView(empty, 0, {2,3,4});

        CHECK(std::distance(begin(bv), end(bv)) == 2);
        CHECK(std::distance(begin(bv[0]), end(bv[0])) == 3);
        CHECK(std::distance(begin(bv[0][0]), end(bv[0][0])) == 4);

    }
}
