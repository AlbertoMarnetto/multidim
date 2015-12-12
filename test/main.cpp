#if 1

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#else

int main(int argc, char* argv[]) {
    try {
        stringstream ss;
        {
            // Standard array, no .size()
            int u[2][3][4];
            auto cg = ch::computeContainerGeometry(u);
            std::copy(begin(cg.bounds), end(cg.bounds), std::ostream_iterator<size_t>(ss, ", "));
            ss << "; " << std::boolalpha << cg.isJagged << "\n";
        }

        cout << ss.str();
        cout << "\n\n" << debugLog.str() << "\n";
        return 0;
    } catch (const exception& e) {
        cout << e.what() << "\n***\n" << debugLog.str() << "\n";
        return 1;
    }
}

#endif

