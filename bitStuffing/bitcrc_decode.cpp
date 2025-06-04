#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>

const std::vector<bool> FLAG = {0,1,1,1,1,1,1,0};

// CRC-16-CCITT (poly 0x1021, init 0xFFFF)
uint16_t crc16_ccitt(const std::vector<bool>& bits) {
    uint16_t crc = 0xFFFF;
    for (bool bit : bits) {
        bool msb = (crc >> 15) & 1;
        crc <<= 1;
        if (bit ^ msb) crc ^= 0x1021;
    }
    for (int i = 0; i < 16; ++i) {
        bool msb = (crc >> 15) & 1;
        crc <<= 1;
        if (msb) crc ^= 0x1021;
    }
    return crc;
}

// Remove any 0 following five consecutive 1s
std::vector<bool> bit_destuff(const std::vector<bool>& in) {
    std::vector<bool> out;
    int ones = 0;
    for (size_t i = 0; i < in.size(); ++i) {
        bool b = in[i];
        if (b) {
            if (++ones == 5) {
                // skip the next bit if it's a stuffed '0'
                if (i + 1 < in.size() && in[i+1] == false) ++i;
                ones = 0;
            }
        } else {
            ones = 0;
        }
        out.push_back(b);
    }
    return out;
}

// Read '0'/'1' chars from a file into a bit vector
std::vector<bool> read_bitfile(const std::string& path) {
    std::ifstream fin(path);
    std::vector<bool> bits;
    char c;
    while (fin >> c) {
        if (c=='0' || c=='1') bits.push_back(c=='1');
    }
    return bits;
}

// Write '0'/'1' chars to a file
void write_bitfile(const std::string& path, const std::vector<bool>& bits) {
    std::ofstream fout(path);
    for (bool b : bits) fout << (b ? '1' : '0');
}

bool match_flag(const std::vector<bool>& v, size_t pos) {
    if (pos + FLAG.size() > v.size()) return false;
    for (size_t i = 0; i < FLAG.size(); ++i)
        if (v[pos+i] != FLAG[i]) return false;
    return true;
}

int main() {
    auto coded = read_bitfile("codedStream.txt");
    std::vector<bool> output_data;
    int frames = 0;

    size_t i = 0;
    while (i + FLAG.size() <= coded.size()) {
        // find opening flag
        if (!match_flag(coded, i)) { ++i; continue; }
        size_t start = i + FLAG.size();
        // find closing flag
        size_t j = start;
        while (j + FLAG.size() <= coded.size() && !match_flag(coded, j)) {
            ++j;
        }
        if (j + FLAG.size() > coded.size()) break;  // no closing flag

        // extract stuffed payload
        std::vector<bool> stuffed(coded.begin() + start, coded.begin() + j);
        // destuff entire payload
        auto deframed = bit_destuff(stuffed);

        // must be at least 16 bits for CRC
        if (deframed.size() < 16) {
            std::cerr << "Frame " << frames << " too short.\n";
            break;
        }

        // split data vs CRC
        std::vector<bool> data(deframed.begin(), deframed.end() - 16);
        uint16_t recv_crc = 0;
        for (size_t k = deframed.size() - 16; k < deframed.size(); ++k)
            recv_crc = (recv_crc << 1) | (deframed[k] ? 1 : 0);

        // verify CRC
        uint16_t calc_crc = crc16_ccitt(data);
        if (calc_crc != recv_crc) {
            std::cerr << "CRC mismatch in frame " << frames << "\n";
        } else {
            // append recovered data
            output_data.insert(output_data.end(), data.begin(), data.end());
            ++frames;
        }

        // advance past closing flag
        i = j + FLAG.size();
    }

    write_bitfile("decodedStream.txt", output_data);
    std::cout << "Decoded " << frames << " frames.\n";
    return 0;
}
