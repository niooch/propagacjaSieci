// main.cpp
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <random>
#include <thread>
#include <vector>
#include <string>

constexpr int MEDIUM_LENGTH = 80;
constexpr int MAX_ATTEMPTS   = 32;

enum class NodeState {
    IDLE,
    TRANSMITTING,
    BACKOFF,
    JAMMING,
    SUCCESS
};

struct Node {
    char      name;
    int       position;
    int       transmission_tick;
    NodeState state    = NodeState::IDLE;
    int       backoff  = 0;
    int       attempts = 0;
    int       jam_start = 0;

    Node(char n, int pos, int tx)
      : name(n), position(pos), transmission_tick(tx) {}

    std::string to_string() const {
        return std::string(1, name) + "(x=" + std::to_string(position) + ")";
    }
};

struct Signal {
    int  pos;
    int  direction;
    char source;
    int  created;
};

typedef std::optional<std::pair<char,int>> Cell;

std::vector<Signal> propagate_signal(
    std::vector<Cell>& medium,
    const std::vector<Signal>& signals,
    int tick
) {
    std::vector<Signal> new_signals;
    for (auto& sig : signals) {
        int new_pos = sig.pos + sig.direction;
        if (0 <= new_pos && new_pos < MEDIUM_LENGTH) {
            auto& cell = medium[new_pos];
            if (!cell.has_value()) {
                cell = std::make_pair(sig.source, 0);
            } else if (cell->first != sig.source) {
                cell = std::make_pair('x', 0);
            }
            new_signals.push_back({ new_pos, sig.direction, sig.source, sig.created });
        }
    }
    return new_signals;
}

bool is_medium_idle(const std::vector<Cell>& medium, int pos) {
    return !medium[pos].has_value();
}

void run_simulation(std::vector<Node>& nodes) {
    std::vector<Cell> medium(MEDIUM_LENGTH);
    std::vector<Signal> signals;
    std::vector<std::string> log;
    int success = 0;
    int tick = 0;

    while (success < nodes.size()) {
        tick++;
        std::string line(MEDIUM_LENGTH, '_');

        signals = propagate_signal(medium, signals, tick);

        for (auto& node : nodes) {
            if (tick == node.transmission_tick && node.state == NodeState::IDLE) {
                if (is_medium_idle(medium, node.position)) {
                    medium[node.position] = std::make_pair(node.name, 0);
                    signals.push_back({node.position, -1, node.name, tick});
                    signals.push_back({node.position,  1, node.name, tick});
                    node.state = NodeState::TRANSMITTING;
                } else {
                    node.transmission_tick++;
                }
            } else if (node.state == NodeState::TRANSMITTING) {
                auto& cell = medium[node.position];
                if (cell.has_value() && cell->first != node.name) {
                    node.state = NodeState::JAMMING;
                    if (node.attempts < MAX_ATTEMPTS) {
                        std::mt19937 rng(std::random_device{}());
                        int factor = 1 << node.attempts;
                        std::uniform_int_distribution<int> dist(0, factor);
                        node.backoff = 2 * MEDIUM_LENGTH * dist(rng);
                        node.transmission_tick = tick + 2*MEDIUM_LENGTH + 1 + node.backoff;
                    }
                    node.attempts++;
                    node.jam_start = tick;
                    medium[node.position] = std::make_pair('x', 0);
                } else {
                    if (tick >= node.transmission_tick + 2*MEDIUM_LENGTH) {
                        node.state = NodeState::SUCCESS;
                        success++;
                        medium[node.position] = std::make_pair(node.name, 0);
                    } else {
                        medium[node.position] = std::make_pair(node.name, 0);
                        signals.push_back({node.position, -1, node.name, tick});
                        signals.push_back({node.position,  1, node.name, tick});
                    }
                }
            } else if (node.state == NodeState::JAMMING) {
                if (tick >= node.jam_start + 2*MEDIUM_LENGTH) {
                    node.state = NodeState::IDLE;
                } else {
                    medium[node.position] = std::make_pair('x', 0);
                    signals.push_back({node.position, -1, 'x', tick});
                    signals.push_back({node.position,  1, 'x', tick});
                }
            }
        }

        for (int i = 0; i < MEDIUM_LENGTH; ++i) {
            if (medium[i].has_value()) line[i] = medium[i]->first;
        }
        log.push_back(line);

        std::fill(medium.begin(), medium.end(), std::nullopt);
    }

    std::ofstream fout("output.txt");
    for (auto& l : log) fout << l << "\n";
    fout.close();

    for (auto& l : log) {
        std::cout << l << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// New main: all nodes start transmitting at tick = 0
int main() {
    const int NODE_COUNT = 8;  // fixed number, or adjust as desired
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> pos_dist(0, MEDIUM_LENGTH - 1);

    std::vector<Node> nodes;
    for (int i = 0; i < NODE_COUNT; ++i) {
        char name = char('a' + i);
        int pos = pos_dist(rng);
        nodes.emplace_back(name, pos, 0);  // transmission_tick = 0 for all
    }

    run_simulation(nodes);
    return 0;
}
