#pragma once
#include "solver.hpp"
#include <stdexcept>
#include <iostream>

// Start is:  O
// End is:    X
// Empty is: ' '
// Filled is: #
class Area {
public:
    Area(std::string const& map) {
        size_t cur_width = 0;
        this->width = 0;
        CoordType cx = 0;
        CoordType cy = 0;
        for(auto c : map) {
            if(c == '\n') {
                this->width = cur_width;
                cur_width = 0;
                cx = 0;
                ++cy;
                continue;
            }
            if(c == '#') {
                this->types.push_back(AStarType::FILLED);
            }
            else if(c == ' ') {
                this->types.push_back(AStarType::EMPTY);
            }
            else if(c == 'O') {
                this->types.push_back(AStarType::EMPTY);
                this->sx = cx;
                this->sy = cy;
            }
            else if(c == 'X') {
                this->types.push_back(AStarType::EMPTY);
                this->ex = cx;
                this->ey = cy;
            }
            else {
                throw std::invalid_argument("invalid input data");
            }
            ++cur_width;
            ++cx;
        }
        
    }
    ~Area() {
    }

    [[nodiscard]] AStarType get(CoordType x, CoordType y) const noexcept {
        size_t const idx = static_cast<size_t>(y) * this->width + static_cast<size_t>(x);
        if(idx < this->types.size() && static_cast<size_t>(x) < this->width) {
            return this->types.at(idx);
        }
        return AStarType::NONE;
    }
    [[nodiscard]] CoordType start_x() const noexcept {
        return this->sx;
    }
    [[nodiscard]] CoordType start_y() const noexcept {
        return this->sy;
    }
    [[nodiscard]] CoordType end_x() const noexcept {
        return this->ex;
    }
    [[nodiscard]] CoordType end_y() const noexcept {
        return this->ey;
    }

    // The path is shown by W characters
    void DrawAreaWithPath(std::ostream& os, std::vector<Position> const& path) const noexcept {
        size_t const height = this->types.size() / this->width;
        for(size_t y = 0; y < height; ++y) {
            for(size_t x = 0; x < this->width; ++x) {
                if(this->types[static_cast<size_t>(y) * this->width + static_cast<size_t>(x)] == AStarType::EMPTY) {
                    const size_t asx = static_cast<size_t>(this->sx);
                    const size_t asy = static_cast<size_t>(this->sy);

                    const size_t aex = static_cast<size_t>(this->ex);
                    const size_t aey = static_cast<size_t>(this->ey);

                    if(asx == x && asy == y) {
                        os << 'O';
                    }
                    else if(aex == x && aey == y) {
                        os << 'X';
                    }
                    else if(std::find_if(path.begin(), path.end(), [x,y](Position const& p) {
                                return static_cast<size_t>(p.first) == x && static_cast<size_t>(p.second) == y;
                                }) != path.end()){
                        os << 'W';
                    }
                    else {
                        os << ' ';
                    }
                }
                else {
                    os << '#';
                }
            }
            os << '\n';
        }
    }

    friend std::ostream& operator<<(std::ostream& os, Area const& area) noexcept {
        size_t const height = area.types.size() / area.width;
        for(size_t y = 0; y < height; ++y) {
            for(size_t x = 0; x < area.width; ++x) {
                if(area.types[static_cast<size_t>(y) * area.width + static_cast<size_t>(x)] == AStarType::EMPTY) {
                    const size_t asx = static_cast<size_t>(area.sx);
                    const size_t asy = static_cast<size_t>(area.sy);

                    const size_t aex = static_cast<size_t>(area.ex);
                    const size_t aey = static_cast<size_t>(area.ey);

                    if(asx == x && asy == y) {
                        os << 'O';
                    }
                    else if(aex == x && aey == y) {
                        os << 'X';
                    }
                    else {
                        os << ' ';
                    }
                }
                else {
                    os << '#';
                }
            }
            os << '\n';
        }
        return os;
    }

private:
    std::vector<AStarType> types;
    size_t width;
    CoordType sx;
    CoordType sy;
    CoordType ex;
    CoordType ey;

};
