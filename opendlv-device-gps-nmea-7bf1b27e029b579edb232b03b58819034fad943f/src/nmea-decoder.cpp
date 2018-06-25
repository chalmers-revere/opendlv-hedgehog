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

#include "cluon-complete.hpp"
#include "nmea-decoder.hpp"

#include <cmath>
#include <cstring>
#include <array>
#include <sstream>
#include <string>

NMEADecoder::NMEADecoder(std::function<void(const double &latitude, const double &longitude, const std::chrono::system_clock::time_point &tp)> delegateLatitudeLongitude,
                         std::function<void(const float &heading, const std::chrono::system_clock::time_point &tp)> delegateHeading) noexcept
    : m_delegateLatitudeLongitude(std::move(delegateLatitudeLongitude))
    , m_delegateHeading(std::move(delegateHeading)) {}

void NMEADecoder::decode(const std::string &data, std::chrono::system_clock::time_point &&tp) noexcept {
    const std::chrono::system_clock::time_point timestamp{tp};

    // Add data to the end...
    m_buffer.seekp(0, std::ios_base::end);
    m_buffer.write(data.c_str(), data.size());

    // ...but always read from the beginning in case of no header was found yet.
    uint32_t startOfNextToken{0};
    if (!m_foundHeader) {
        m_buffer.seekg(startOfNextToken + m_toRemove, std::ios_base::beg);
    }

    while ((static_cast<uint32_t>(m_buffer.tellg()) + m_toRemove + static_cast<uint32_t>(NMEADecoderConstants::HEADER_SIZE)) < m_buffer.tellp()) {
        // Wait for more data if put pointer is smaller than expected buffer fill level.
        if (     m_buffering
            && ( (static_cast<uint32_t>(m_buffer.tellg()) + m_toRemove) <= static_cast<uint32_t>(m_buffer.tellp()) ) ) {
            const uint32_t MAX_TO_READ{static_cast<uint32_t>(m_buffer.tellp()) - (static_cast<uint32_t>(m_buffer.tellg()) + m_toRemove)};
            const uint32_t TO_READ = (MAX_TO_READ < static_cast<uint32_t>(NMEADecoderConstants::CHUNK_SIZE) ? MAX_TO_READ : static_cast<uint32_t>(NMEADecoderConstants::CHUNK_SIZE));

            const uint32_t BEFORE_READ = static_cast<uint32_t>(m_buffer.tellg());
            m_buffer.get(m_bufferForNextNMEAMessage.data() + m_arrayWriteIndex, TO_READ, '\r');
            const uint32_t AFTER_READ = static_cast<uint32_t>(m_buffer.tellg());
            m_arrayWriteIndex += AFTER_READ - BEFORE_READ;

            // Check last character if from sequence \r\n.
            m_foundCRLF = (0x0D == m_buffer.peek());
        }

        // Enough data available to decode the requested NMEA payload.
        if (m_foundCRLF) {
            // Consume CR/LF.
            m_buffer.get();
            m_buffer.get();

            std::string nmeaMessage(m_bufferForNextNMEAMessage.data(), m_arrayWriteIndex);
            auto fields = stringtoolbox::split(nmeaMessage, ',');

            if (NMEADecoderConstants::GGA == m_nextNMEAMessage) {
                if (5 < fields.size()) {
                    double latitude = std::stod(fields[1]) / 100.0;
                    double longitude = std::stod(fields[3]) / 100.0;

                    latitude = static_cast<int32_t>(latitude) + (latitude - static_cast<int32_t>(latitude)) * 100.0 / 60.0;
                    longitude = static_cast<int32_t>(longitude) + (longitude - static_cast<int32_t>(longitude)) * 100.0 / 60.0;

                    latitude *= ("S" == fields[2] ? -1.0 : 1.0);
                    longitude *= ("W" == fields[4] ? -1.0 : 1.0);

                    if (nullptr != m_delegateLatitudeLongitude) {
                        m_delegateLatitudeLongitude(latitude, longitude, timestamp);
                    }
                }
            }
            else if (NMEADecoderConstants::RMC == m_nextNMEAMessage) {
                if (8 < fields.size()) {
                    double latitude = std::stod(fields[2]) / 100.0;
                    double longitude = std::stod(fields[4]) / 100.0;

                    latitude = static_cast<int32_t>(latitude) + (latitude - static_cast<int32_t>(latitude)) * 100.0 / 60.0;
                    longitude = static_cast<int32_t>(longitude) + (longitude - static_cast<int32_t>(longitude)) * 100.0 / 60.0;

                    latitude *= ("S" == fields[3] ? -1.0 : 1.0);
                    longitude *= ("W" == fields[5] ? -1.0 : 1.0);
                    if (nullptr != m_delegateLatitudeLongitude) {
                        m_delegateLatitudeLongitude(latitude, longitude, timestamp);
                    }

                    const float heading = static_cast<float>(std::stod(fields[7]) / 180.0 * M_PI);
                    if (nullptr != m_delegateHeading) {
                        m_delegateHeading(heading, timestamp);
                    }
                }
            }

            // Maintain internal buffer status.
            m_nextNMEAMessage = NMEADecoderConstants::UNKNOWN;
            m_foundHeader = m_buffering = m_foundCRLF = false;
            m_arrayWriteIndex = m_toRemove = 0;
            startOfNextToken = static_cast<uint32_t>(m_buffer.tellg());
        }

        // Try decoding $??XYZ header.
        if (     !m_foundHeader
            && (   (static_cast<uint32_t>(m_buffer.tellp())
                 - (static_cast<uint32_t>(m_buffer.tellg()) + m_toRemove))
                    >= static_cast<uint32_t>(NMEADecoderConstants::HEADER_SIZE)
               )
           ) {
            // Go to where we need to read from.
            m_buffer.seekg(startOfNextToken + m_toRemove, std::ios::beg);

            std::array<char, static_cast<uint32_t>(NMEADecoderConstants::HEADER_SIZE)> header;
            m_buffer.read(header.data(), header.size());
            if ( ('G' == header[3]) &&
                 ('G' == header[4]) &&
                 ('A' == header[5]) ) {
                m_nextNMEAMessage = NMEADecoderConstants::GGA;
                m_foundHeader = true;
                m_buffering = true;
                m_foundCRLF = false;
                m_arrayWriteIndex = 0;
            }
            else if ( ('R' == header[3]) &&
                      ('M' == header[4]) &&
                      ('C' == header[5]) ) {
                m_nextNMEAMessage = NMEADecoderConstants::RMC;
                m_foundHeader = true;
                m_buffering = true;
                m_foundCRLF = false;
                m_arrayWriteIndex = 0;
            }
            else {
                // Nothing known found; skip this byte and try again.
                m_toRemove++;
            }
        }
    }

    // Buffer fully consumed; reset.
    if (m_buffer.tellg() == m_buffer.tellp()) {
        m_buffer.clear();
        m_buffer.str("");
        m_nextNMEAMessage = NMEADecoderConstants::UNKNOWN;
        m_foundHeader = m_buffering = m_foundCRLF = false;
        m_arrayWriteIndex = m_toRemove = 0;
    }

    // Discard unused data from buffer but avoid copying data too often.
    if ( (m_buffer.tellg() > 0) && (m_toRemove > static_cast<uint32_t>(NMEADecoderConstants::MAX_BUFFER)) ) {
        const std::string s{m_buffer.str().substr(m_buffer.tellg())};
        m_buffer.clear();
        m_buffer.str(s);
        m_buffer.seekp(0, std::ios::end);
        m_buffer.seekg(0, std::ios::beg);
        m_arrayWriteIndex = m_toRemove = 0;
    }
}

