#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>

// CRC-16-CCITT (polynomial 0x1021, initial 0xFFFF) :contentReference[oaicite:2]{index=2}
uint16_t crc16_ccitt(const std::vector<bool>& bits) {
    uint16_t crc = 0xFFFF;
    for (bool bit : bits) {
        bool msb = (crc >> 15) & 1;
        crc <<= 1;
        if (bit ^ msb) crc ^= 0x1021;
        crc &= 0xFFFF;
    }
    // Append 16 zero bits for final remainder check :contentReference[oaicite:3]{index=3}
    for (int i = 0; i < 16; ++i) {
        bool msb = (crc >> 15) & 1;
        crc <<= 1;
        if (msb) crc ^= 0x1021;
        crc &= 0xFFFF;
    }
    return crc;
}

// Stuff bits: insert a 0 after five consecutive 1s :contentReference[oaicite:4]{index=4}
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

// Destuff bits: remove any 0 following five consecutive 1s :contentReference[oaicite:5]{index=5}
std::vector<bool> bit_destuff(const std::vector<bool>& in) {
    std::vector<bool> out;
    int ones = 0;
    for (size_t i = 0; i < in.size(); ++i) {
        bool b = in[i];
        if (b) {
            if (++ones == 5) {
                // skip the next bit if zero
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

// Read bits from a text file of '0'/'1' characters
std::vector<bool> read_bitfile(const std::string& path) {
    std::ifstream fin(path);
    std::vector<bool> bits;
    char c;
    while (fin >> c) {
        if (c=='0' || c=='1') bits.push_back(c=='1');
    }
    return bits;
}

// Write bits as '0'/'1'
void write_bitfile(const std::string& path, const std::vector<bool>& bits) {
    std::ofstream fout(path);
    for (bool b : bits) fout << (b?'1':'0');
}

int main() {
    // 1. Read raw stream
    auto raw = read_bitfile("random");

    // 2. Stuff bits
    auto stuffed = bit_stuff(raw);

    // 3. Compute CRC and append its 16 bits MSB-first
    uint16_t crc = crc16_ccitt(raw);
    for (int i = 15; i >= 0; --i)
        stuffed.push_back((crc >> i) & 1);

    write_bitfile("codedStream.txt", stuffed);

    // 4. Decode: verify CRC and destuff
    //    Separate data+crc
    std::vector<bool> recv_data(stuffed.begin(), stuffed.end()-16);
    std::vector<bool> recv_crc_bits(stuffed.end()-16, stuffed.end());
    uint16_t recv_crc = 0;
    for (bool b : recv_crc_bits) recv_crc = (recv_crc<<1)|(b?1:0);

    // Verify
    if (crc16_ccitt(bit_destuff(recv_data)) != recv_crc) {
        std::cerr<<"CRC mismatch!"<<std::endl;
        return 1;
    }

    // Destuff
    auto decoded = bit_destuff(recv_data);
    write_bitfile("decodedStream.txt", decoded);
    return 0;
}
