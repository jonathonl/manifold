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

    //================================================================//
    // xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    static const std::uint64_t y[4] = {0x8000000000000000, 0x9000000000000000, 0xa000000000000000, 0xb000000000000000};

    template<typename Rng>
    std::array<std::uint64_t, 2> gen_uuid(Rng& rng)
    {
      std::array<std::uint64_t, 2> ret;

      std::uint32_t r32 = (std::uint32_t) rng();

      std::uint64_t r64_1 = rng();
      r64_1 = r64_1 << 32;

      std::uint64_t r64_2 = rng();
      r64_2 = r64_2 << 32;

      ret[0] = (0xFFFFFFFFFFFF0FFF & (r64_1 | rng())) | 0x4000;
      ret[1] = ((0x0FFFFFFF00000000 & (r64_2 | rng())) | r32) | y[0x03 & r32]; // Should be using a separate rand call to choose index, but this is faster.

      return ret;
    }

    template<typename Rng>
    std::string gen_uuid_str(Rng& rng)
    {
      std::array<std::uint64_t, 2> tmp = gen_uuid(rng);
      std::stringstream ret;
      ret << std::hex << std::setfill('0');
      ret << std::setw(8) << (0xFFFFFFFF & (tmp[0] >> 32));
      ret << "-";
      ret << std::setw(4) << (0xFFFF & (tmp[0] >> 16));
      ret << "-";
      ret << std::setw(4) << (0xFFFF & tmp[0]);
      ret << "-";
      ret << std::setw(4) << (0xFFFF & tmp[1] >> 48);
      ret << "-";
      ret << std::setw(12) << (0xFFFFFFFFFFFF & tmp[1]);
      return ret.str();
    }

  }
}

#endif //MANIFOLD_UTILITY_HPP