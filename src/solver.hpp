#pragma once
#include <type_traits>
#include <concepts>
#include <queue>
#include <unordered_map>

enum class AStarType {
    // the element doesn't exist, will be treated the same as filled
    NONE,

    EMPTY,
    FILLED,
};

using CoordType = int64_t;
using Position = std::pair<CoordType, CoordType>;

template<class T>
concept SingleGettable = requires (T const& a, CoordType i, CoordType j) {
    { a.get(i, j) } -> std::same_as<AStarType>;
};
template<class T>
concept DoubleGettable = requires (T const& a, int64_t i, int64_t j) {
    { a.get(i, j).get_type() } -> std::same_as<AStarType>;
};
template<class T>
concept CoordinatesAvailable = requires (T const& a) {
    { a.start_x() } -> std::convertible_to<CoordType>;
    { a.start_y() } -> std::convertible_to<CoordType>;
    { a.end_x() } -> std::convertible_to<CoordType>;
    { a.end_y() } -> std::convertible_to<CoordType>;
};

template<class T>
concept Gettable = SingleGettable<T> || DoubleGettable<T>;


class AStarNode final {
public:
    using FValueType = uint64_t;
    using GValueType = uint64_t;

    AStarNode() : position{0, 0}, predecessor{-1, -1}, f_value{0}, g_value{0} {
    }
    AStarNode(CoordType x, CoordType y) : position{x, y}, predecessor{-1, -1}, f_value{0}, g_value{0} {
    }
    AStarNode(CoordType x, CoordType y, CoordType pred_x, CoordType pred_y, FValueType f_val, GValueType g_val) : position{x, y}, predecessor{pred_x, pred_y}, f_value{f_val}, g_value{g_val} {
    }
    AStarNode(Position const& pos) : position{pos}, predecessor{-1, -1}, f_value{0}, g_value{0} {
    }
    AStarNode(AStarNode const& o) : position{o.position}, predecessor{o.predecessor}, f_value{o.f_value}, g_value{o.g_value} {
    }
    AStarNode(AStarNode&& o) : position{o.position}, predecessor{o.predecessor}, f_value{o.f_value}, g_value{o.g_value} {
    }
    AStarNode& operator=(AStarNode const& o) {
        this->position = o.position;
        this->predecessor = o.predecessor;
        this->f_value = o.f_value;
        this->g_value = o.g_value;
        return *this;
    }
    AStarNode& operator=(AStarNode&& o) {
        this->position = o.position;
        this->predecessor = o.predecessor;
        this->f_value = o.f_value;
        this->g_value = o.g_value;
        return *this;
    }
    ~AStarNode() {
    }

    Position position;
    Position predecessor;
    FValueType f_value;
    GValueType g_value;
};



template<typename T, bool allow_diagonal_movement = false>
    requires Gettable<T> &&
    CoordinatesAvailable<T>
class AStarSolver final {
private:
    // custom priority class required for the priority queue
    struct AStarNodePrioritySorter final {
        bool operator()(AStarNode const& l, AStarNode const& r) const {
            return l.f_value > r.f_value;
        }
    };

    // custom pair hashing class required for the unordered_map
    struct HashPair final {
        template<class TFirst, class TSecond>
            size_t operator()(const std::pair<TFirst, TSecond>& p) const noexcept {
                uintmax_t hash = std::hash<TFirst>{}(p.first);
                hash <<= sizeof(uintmax_t) * 4;
                hash ^= std::hash<TSecond>{}(p.second);
                return std::hash<uintmax_t>{}(hash);
            }
    };

    // this is required as the priority_queue does not allow access to the underlying iterators
    // so this is a way to get the container of the priority_queue
    template <class _T, class _S, class _C>
    static _S& Container(std::priority_queue<_T, _S, _C>& q) {
        struct HackedQueue : private std::priority_queue<_T, _S, _C> {
            static _S& Container(std::priority_queue<_T, _S, _C>& q) {
                return q.*&HackedQueue::c;
            }
        };
        return HackedQueue::Container(q);
    }

    using AreaType = T;
    using QueueType = std::priority_queue<AStarNode, std::vector<AStarNode>, AStarNodePrioritySorter>;
    using MapType = std::unordered_map<Position, AStarNode, HashPair>;

    [[nodiscard]] AStarType get_at(CoordType i, CoordType j) const noexcept {
        if constexpr (DoubleGettable<T>) {
            return this->area.get(i, j).get_type();
        }
        else {
            return this->area.get(i, j);
        }
    }

    [[nodiscard]] bool check_start_and_end_are_valid() const noexcept {
        auto const sx = area.start_x();
        auto const sy = area.start_y();
        auto const ex = area.end_x();
        auto const ey = area.end_y();
        return this->get_at(sx, sy) == AStarType::EMPTY && this->get_at(ex, ey) == AStarType::EMPTY;
    }

    AreaType const& area;
    QueueType open_list;
    MapType closed_list;

    auto predicted_cost(CoordType x, CoordType y) const {
        auto const ex = this->area.end_x();
        auto const ey = this->area.end_y();
        return std::llabs(ex - x) + std::llabs(ey - y);
    }

    void add_node(CoordType x, CoordType y, CoordType prev_x, CoordType prev_y, AStarNode::GValueType prev_g_val) noexcept {
        if(this->closed_list.contains({x, y})) {
            return;
        }
        std::vector<AStarNode>& open_list_container = Container(open_list);
        // cost is always 1, even for diagonal moves
        static constexpr AStarNode::GValueType COST = 1;
        auto tentative_g = prev_g_val + COST; 
        auto successor = std::find_if(open_list_container.begin(), open_list_container.end(), [x,y](AStarNode const& n) {
            return n.position.first == x && n.position.second == y;
        });
        if(successor != open_list_container.end() && tentative_g >= successor->g_value) {
            return;
        }
        auto const successor_f_val = tentative_g + static_cast<AStarNode::FValueType>(predicted_cost(x, y));
        if (successor != open_list_container.end()) {
            successor->f_value = successor_f_val;
            successor->g_value = tentative_g;
            successor->predecessor = {prev_x, prev_y};
        }
        else {
            open_list.emplace(x, y, prev_x, prev_y, successor_f_val, tentative_g);
        }
    }
    void expand_node(AStarNode const& node) noexcept {
        {
            auto left = this->get_at(node.position.first - 1, node.position.second);
            if(left == AStarType::EMPTY) {
                this->add_node(node.position.first - 1, node.position.second, node.position.first, node.position.second, node.g_value);
            }
            auto right = this->get_at(node.position.first + 1, node.position.second);
            if(right == AStarType::EMPTY) {
                this->add_node(node.position.first + 1, node.position.second, node.position.first, node.position.second, node.g_value);
            }
            auto top = this->get_at(node.position.first, node.position.second - 1);
            if(top == AStarType::EMPTY) {
                this->add_node(node.position.first, node.position.second - 1, node.position.first, node.position.second, node.g_value);
            }
            auto bottom = this->get_at(node.position.first, node.position.second + 1);
            if(bottom == AStarType::EMPTY) {
                this->add_node(node.position.first, node.position.second + 1, node.position.first, node.position.second, node.g_value);
            }
        }
        if constexpr (allow_diagonal_movement) {
            auto lt = this->get_at(node.position.first - 1, node.position.second - 1);
            if(lt == AStarType::EMPTY) {
                this->add_node(node.position.first - 1, node.position.second - 1, node.position.first, node.position.second, node.g_value);
            }
            auto rt = this->get_at(node.position.first + 1, node.position.second - 1);
            if(rt == AStarType::EMPTY) {
                this->add_node(node.position.first + 1, node.position.second - 1, node.position.first, node.position.second, node.g_value);
            }
            auto lb = this->get_at(node.position.first - 1, node.position.second + 1);
            if(lb == AStarType::EMPTY) {
                this->add_node(node.position.first - 1, node.position.second + 1, node.position.first, node.position.second, node.g_value);
            }
            auto rb = this->get_at(node.position.first + 1, node.position.second + 1);
            if(rb == AStarType::EMPTY) {
                this->add_node(node.position.first + 1, node.position.second + 1, node.position.first, node.position.second, node.g_value);
            }
        }


    }



public:
    AStarSolver(AreaType const& t) noexcept 
        : area(t) {
    }
    ~AStarSolver() noexcept {
    }
    AStarSolver(AStarSolver const& o) {
        this->area = o.area;
        this->open_list = o.open_list;
        this->closed_list = o.closed_list;
    }
    AStarSolver(AStarSolver&& o) {
        this->area = std::move(o.area);
        this->open_list = std::move(o.open_list);
        this->closed_list = std::move(o.closed_list);
    }

    AStarSolver& operator=(AStarSolver const& o) {
        this->area = o.area;
        this->open_list = o.open_list;
        this->closed_list = o.closed_list;
        return *this;
    }
    AStarSolver& operator=(AStarSolver&& o) {
        this->area = std::move(o.area);
        this->open_list = std::move(o.open_list);
        this->closed_list = std::move(o.closed_list);
        return *this;
    }

    [[nodiscard]] auto solve() noexcept -> std::vector<Position> {
        if(!check_start_and_end_are_valid()) {
            return {};
        }
        const auto sx = area.start_x();
        const auto sy = area.start_y();
        const auto ex = area.end_x();
        const auto ey = area.end_y();

        this->open_list.emplace(sx, sy);

        while(!this->open_list.empty()) {
            AStarNode cur_node = this->open_list.top();
            this->open_list.pop();
            closed_list[cur_node.position] = cur_node;

            if(cur_node.position.first == ex && cur_node.position.second == ey) {
                auto path = std::vector<Position>{};
                auto* build_node = &this->closed_list[{ex, ey}];
                while(build_node->predecessor.first != -1 && build_node->predecessor.second != -1) {
                    path.emplace_back(build_node->position.first, build_node->position.second);
                    build_node = &this->closed_list[build_node->predecessor];
                }
                // add the start node to the path
                path.push_back({sx, sy});

                std::reverse(path.begin(), path.end());
                return path;
            }

            this->expand_node(cur_node);
        }
        // no path found
        return {};
    }

};




