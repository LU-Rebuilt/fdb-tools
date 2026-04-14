#include "netdevil/database/fdb/fdb_reader.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: fdb_converter <cdclient.fdb> <output.sqlite>\n";
        return 1;
    }

    const char* input_path = argv[1];
    const char* output_path = argv[2];

    // Read input file
    std::ifstream file(input_path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error: cannot open " << input_path << "\n";
        return 1;
    }

    auto size = file.tellg();
    file.seekg(0);
    std::vector<uint8_t> data(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(data.data()), size);

    try {
        std::cout << "Parsing FDB schema (" << data.size() << " bytes)...\n";
        auto tables = lu::assets::fdb_parse(data);
        std::cout << "Found " << tables.size() << " tables:\n";

        for (const auto& table : tables) {
            std::cout << "  " << table.name << ": " << table.columns.size() << " columns\n";
        }

        std::cout << "Stream-converting to SQLite: " << output_path << "...\n";
        lu::assets::fdb_to_sqlite_direct(data, output_path);
        std::cout << "Done.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
