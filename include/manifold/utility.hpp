// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#ifndef MANIFOLD_UTILITY_HPP
#define MANIFOLD_UTILITY_HPP

#include <string>
#include <array>
#include <sstream>
#include <iomanip>

namespace manifold
{
  namespace detail
  {
    bool path_exists(const std::string& input_path);

    bool is_regular_file(const std::string& input_path);

    bool is_directory(const std::string& input_path);

    std::string basename(const std::string& input_path);

    std::string basename_sans_extension(const std::string& input_path);

    std::string extension(const std::string& input_path);

    std::string directory(const std::string& input_path);

    std::string content_type_from_extension(const std::string& extension);

    template<typename Rng>
    static std::array<std::uint8_t, 16> gen_uuid(Rng& rng)
    {
      static_assert(sizeof(typename Rng::result_type) == 8, "gen_uuid requires a 64 bit PRNG");

      // xxxxxxxx-xxxx-4xxx-{8,9,A,B}xxx-xxxxxxxxxxxx
      // https://www.cryptosys.net/pki/uuid-rfc4122.html

      std::array<std::uint8_t, 16> ret;

      std::uint64_t r1 = rng();
      std::uint64_t r2 = rng();

      std::memcpy(ret.data(), &r1, 8);
      std::memcpy(ret.data() + 8, &r2, 8);

      ret[6] = static_cast<std::uint8_t>(ret[6] & 0x0F) | static_cast<std::uint8_t>(0x40);
      ret[8] = static_cast<std::uint8_t>(ret[8] & 0x3F) | static_cast<std::uint8_t>(0x80);

      return ret;
    }

    template<typename Rng>
    std::string gen_uuid_str(Rng& rng)
    {
      std::array<std::uint8_t, 16> tmp = gen_uuid(rng);
      std::stringstream ret;
      ret << std::hex << std::setfill('0');

      std::size_t i = 0;
      for ( ; i < 16; ++i)
      {
        if (i == 4 || i == 6 || i == 8 || i == 10)
          ret << "-";
        ret << std::setw(2) << (unsigned)tmp[i];
      }

      return ret.str();
    }

  }
}

#endif //MANIFOLD_UTILITY_HPP