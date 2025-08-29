#pragma once
#ifndef TIMEDSET_HPP
#define TIMEDSET_HPP

#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include <utility>
#include <ranges>

namespace Containers {
    template<typename TIndex, typename TData>
    class IndexedTimedSet {
    private:
        //lookup done by index
        std::unordered_map<TIndex, std::pair<TData, std::chrono::steady_clock::time_point>> map;

    public:

        std::pair<TData, std::chrono::steady_clock::time_point>& operator[](const TIndex& index) {
            return this->map[index];
        } 

        //non modifying
        const std::pair<TData, std::chrono::steady_clock::time_point>& operator[](const TIndex& index) const {
            return this->map.at(index);
        } 

        void update(const TIndex& index, TData data) {
            this->map[index] = {data, std::chrono::steady_clock::now()};
        }

        void remove(const TIndex& index) {
            this->map.erase(index);
        }

        void remove(const std::chrono::steady_clock::duration& maxDuration) {
            auto now = std::chrono::steady_clock::now();
            for (auto it = this->map.begin(); it != this->map.end();) {
                if (now - it->second.second > maxDuration) {
                    it = map.erase(it);
                } else {
                    ++it;
                }
            }
        }

        std::size_t size() const {
            return this->map.size();
        }

        std::vector<TData> data() const {
            auto view = this->map | std::views::values | std::views::transform([](auto& p){ return p.first; });
            return std::vector<TData>(view.begin(), view.end());
        }
    };
}

#endif