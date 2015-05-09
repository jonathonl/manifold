#pragma once

#ifndef MANIFOLD_HPACK_HPP
#define MANIFOLD_HPACK_HPP

#include <string>
#include <array>
#include <vector>
#include <list>


namespace manifold
{
  namespace hpack
  {
    extern const std::array<std::pair<std::string,std::string>, 61> static_table;

    enum class prefix_mask : std::uint8_t
    {
      one_bit =    0x1,
      two_bit =    0x3,
      three_bit =  0x7,
      four_bit =   0xF,
      five_bit =  0x1F,
      six_bit =   0x3F,
      seven_bit = 0x7F,
      eight_bit = 0xFF
    };

    //================================================================//
    class context
    {
    protected:
      std::uint32_t max_dynamic_table_size_;
      std::vector<std::pair<std::string,std::string>> dynamic_table_;
    public:
      context()
        : max_dynamic_table_size_(std::numeric_limits<std::uint32_t>::max()) {}
      virtual ~context() {}
    };
    //================================================================//

    //================================================================//
    class encoder : public context
    {
    private:
      static void encode_integer(prefix_mask prfx_mask, std::uint64_t input, std::string& output);
    public:
      void encode(const std::list<std::pair<std::string,std::string>>& headers, std::string& output);
    };
    //================================================================//

    //================================================================//
    class decoder : public context
    {
    private:
      static std::uint64_t decode_integer(prefix_mask prfx_mask, const std::string& input, std::size_t& pos);
    public:
      bool decode(const std::string& input, std::list<std::pair<std::string,std::string>>& headers);
    };
    //================================================================//
  };
}

#endif // MANIFOLD_HPACK_HPP
