#include <iostream>
#include <format>
#include "solver.hpp"
#include "ExampleArea.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    try {
        Area area(R"(#   O    ###   X  ###
#       ###       ###
##  ######   ########
#        ##   #######
#                   #
)");
        AStarSolver<Area, true> test(area);
        auto path = test.solve();
        std::cout << std::format("path_length: {}\n", path.size());
        for(auto const& p : path) {
            std::cout << std::format("path: {} {}\n", p.first, p.second);
        }
        area.DrawAreaWithPath(std::cout, path);
    }
    catch(const std::invalid_argument& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
