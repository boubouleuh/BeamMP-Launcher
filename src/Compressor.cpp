/*
 Copyright (C) 2024 BeamMP Ltd., BeamMP team and contributors.
 Licensed under AGPL-3.0 (or later), see <https://www.gnu.org/licenses/>.
 SPDX-License-Identifier: AGPL-3.0-or-later
*/


#include "Logger.h"
#include <span>
#include <vector>
#include <zconf.h>
#include <zstd.h>
#ifdef __linux__
#include <cstring>
#endif

std::vector<char> Comp(std::span<const char> input) {
    size_t max_size = ZSTD_compressBound(input.size());
    std::vector<char> output(max_size);

    size_t compressed_size = ZSTD_compress(
        output.data(), output.size(),
        input.data(), input.size(),
        3
    );

    if (ZSTD_isError(compressed_size)) {
        error("zstd compress() failed: " + std::string(ZSTD_getErrorName(compressed_size)));
        throw std::runtime_error("zstd compress() failed");
    }

    debug("zstd compressed " + std::to_string(input.size()) + " B to " + std::to_string(compressed_size) + " B");
    output.resize(compressed_size);
    return output;
}

std::vector<char> DeComp(std::span<const char> input) {
    unsigned long long decompressed_size = ZSTD_getFrameContentSize(input.data(), input.size());

    if (decompressed_size == ZSTD_CONTENTSIZE_ERROR) {
        throw std::runtime_error("zstd decompression failed: not a valid zstd frame");
    }
    if (decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        throw std::runtime_error("zstd decompression failed: original size unknown");
    }
    if (decompressed_size > 30 * 1024 * 1024) {
        throw std::runtime_error("decompressed packet size of 30 MB exceeded");
    }

    std::vector<char> output(decompressed_size);

    size_t result = ZSTD_decompress(
        output.data(), output.size(),
        input.data(), input.size());

    if (ZSTD_isError(result)) {
        error("zstd decompress() failed: " + std::string(ZSTD_getErrorName(result)));
        throw std::runtime_error("zstd decompress() failed");
    }

    return output;
}
