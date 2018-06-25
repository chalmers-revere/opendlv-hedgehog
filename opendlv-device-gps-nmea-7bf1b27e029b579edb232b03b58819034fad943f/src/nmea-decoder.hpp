/*
 * Copyright (C) 2018  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NMEA_DECODER
#define NMEA_DECODER

#include "nmea-decoder-constants.hpp"

#include <chrono>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

class NMEADecoder {
   private:
    NMEADecoder(const NMEADecoder &) = delete;
    NMEADecoder(NMEADecoder &&)      = delete;
    NMEADecoder &operator=(const NMEADecoder &) = delete;
    NMEADecoder &operator=(NMEADecoder &&) = delete;

   public:
    NMEADecoder(std::function<void(const double &latitude, const double &longitude, const std::chrono::system_clock::time_point &tp)> delegateLatitudeLongitude,
                std::function<void(const float &heading, const std::chrono::system_clock::time_point &tp)> delegateHeading) noexcept;
    ~NMEADecoder() = default;

   public:
    void decode(const std::string &data, std::chrono::system_clock::time_point &&tp) noexcept;

   private:
    std::function<void(const double &latitude, const double &longitude, const std::chrono::system_clock::time_point &tp)> m_delegateLatitudeLongitude{};
    std::function<void(const float &heading, const std::chrono::system_clock::time_point &tp)> m_delegateHeading{};

    bool m_foundHeader{false};
    bool m_foundCRLF{false};
    bool m_buffering{false};
    uint32_t m_toRemove{0};
    std::stringstream m_buffer{};
    uint32_t m_arrayWriteIndex{0};
    std::array<char, static_cast<uint32_t>(NMEADecoderConstants::NMEA_BUFFER)> m_bufferForNextNMEAMessage{};
    NMEADecoderConstants m_nextNMEAMessage{NMEADecoderConstants::UNKNOWN};
};

#endif

