/*
 Copyright (C) 2024 BeamMP Ltd., BeamMP team and contributors.
 Licensed under AGPL-3.0 (or later), see <https://www.gnu.org/licenses/>.
 SPDX-License-Identifier: AGPL-3.0-or-later
*/


#include "Logger.h"
#include <span>
#include <vector>
#include <zconf.h>
#include <lz4.h>
#ifdef __linux__
#include <cstring>
#endif

std::vector<char> Comp(std::span<const char> input) {
    auto max_size = LZ4_compressBound(input.size());
    std::vector<char> output(max_size);

    int res = LZ4_compress_default(
        input.data(),
        output.data(),
        static_cast<int>(input.size()),
        static_cast<int>(max_size));

    if (res <= 0) {
        throw std::runtime_error("lz4 compress() failed");
    }

    debug("lz4 compressed " + std::to_string(input.size()) + " B to " + std::to_string(res) + " B");

    output.resize(res);
    return output;
}


std::vector<char> DeComp(std::span<const char> input) {
    std::vector<char> output_buffer(std::min<size_t>(input.size() * 5, 15 * 1024 * 1024));

    while (true) {
        int decompressed_size_ret = LZ4_decompress_safe(
            input.data(),
            output_buffer.data(),
            static_cast<int>(input.size()),
            static_cast<int>(output_buffer.size()));

        if (decompressed_size_ret < 0) {
            if (output_buffer.size() > 30 * 1024 * 1024) {
                throw std::runtime_error("LZ4 decompression exceeded 30 MB limit");
            }
            output_buffer.resize(output_buffer.size() * 2);
            continue;
        }

        output_buffer.resize(decompressed_size_ret);
        return output_buffer;
    }
}
