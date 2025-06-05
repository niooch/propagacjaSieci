#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>

// HDLC flag sequence: 0x7E = 01111110
const std::vector<bool> FLAG = {0,1,1,1,1,1,1,0};

// CRC-16-CCITT (poly 0x1021, init 0xFFFF)
uint16_t crc16_ccitt(const std::vector<bool>& bits) {
    uint16_t crc = 0xFFFF;
    for (bool bit : bits) {
        bool msb = (crc >> 15) & 1;
        crc <<= 1;
        if (bit ^ msb) crc ^= 0x1021;
    }
    // finalize with 16 zero bits
    for (int i = 0; i < 16; ++i) {
        bool msb = (crc >> 15) & 1;
        crc <<= 1;
        if (msb) crc ^= 0x1021;
    }
    return crc;
}

// Insert a 0 after every sequence of five consecutive 1s
std::vector<bool> bit_stuff(const std::vector<bool>& in) {
    std::vector<bool> out;
    int ones = 0;
    for (bool b : in) {
        out.push_back(b);
        if (b) {
            if (++ones == 5) {
                out.push_back(false);
                ones = 0;
            }
        } else {
            ones = 0;
        }
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

// Write a bit vector as '0'/'1' chars to a file
void write_bitfile(const std::string& path, const std::vector<bool>& bits) {
    std::ofstream fout(path);
    for (bool b : bits) fout << (b ? '1' : '0');
}

int main() {
    auto raw = read_bitfile("stream.txt");
    if (raw.empty()) {
        std::cerr << "Input stream is empty.\n";
        return 1;
    }

    std::vector<bool> out_bits;
    int frames = 0;

    // Process in 1000-bit chunks
    for (size_t offset = 0; offset < raw.size(); offset += 80) {
        size_t len = std::min<size_t>(80, raw.size() - offset);
        std::vector<bool> chunk(raw.begin() + offset, raw.begin() + offset + len);

        // Compute CRC on raw chunk
        uint16_t crc = crc16_ccitt(chunk);
        // Append CRC bits MSB-first
        for (int i = 15; i >= 0; --i) {
            chunk.push_back((crc >> i) & 1);
        }

        // Bit-stuff the chunk+CRC
        auto stuffed = bit_stuff(chunk);

        // Surround with flags
        out_bits.insert(out_bits.end(), FLAG.begin(), FLAG.end());
        out_bits.insert(out_bits.end(), stuffed.begin(), stuffed.end());
        out_bits.insert(out_bits.end(), FLAG.begin(), FLAG.end());

        ++frames;
    }

    write_bitfile("codedStream.txt", out_bits);
    std::cout << "Encoded " << frames << " frames.\n";
    return 0;
}
