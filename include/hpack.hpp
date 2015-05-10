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
      one_bit    = 0x1,
      two_bit    = 0x3,
      three_bit  = 0x7,
      four_bit   = 0xF,
      five_bit  = 0x1F,
      six_bit   = 0x3F,
      seven_bit = 0x7F,
      eight_bit = 0xFF
    };

    //================================================================//
    class context
    {
    protected:
      std::uint32_t max_dynamic_table_size_;
      std::vector<std::pair<std::string,std::string>> dynamic_table_;

      const std::pair<std::string,std::string>& at(std::size_t index) const
      {
        if (index < static_table.size())
          return static_table[index];
        else if (index < this->dynamic_table_.size())
          return this->dynamic_table_[index];
        else
          throw std::out_of_range("Table index out of range.");
      }

      std::size_t combined_table_size() const { return static_table.size() + this->dynamic_table_.size(); }
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
      static void huffman_encode(std::string::const_iterator begin, std::string::const_iterator end, std::string& output);
    public:
      void encode(const std::list<std::pair<std::string,std::string>>& headers, std::string& output);
      void set_table_max(std::uint32_t value);
    };
    //================================================================//

    //================================================================//
    class decoder : public context
    {
    private:
      static std::uint64_t decode_integer(prefix_mask prfx_mask, std::string::const_iterator& itr);
      bool decode_nvp(std::size_t table_index, std::string::const_iterator& itr, std::list<std::pair<std::string,std::string>>& headers);
      static void huffman_decode(std::string::const_iterator begin, std::string::const_iterator end, std::string& output) {}
      static bool decode_string_literal(std::string::const_iterator& itr, std::string& output);
    public:
      bool decode(std::string::const_iterator beg, std::string::const_iterator end, std::list<std::pair<std::string,std::string>>& headers);
    };
    //================================================================//
  };
}

#endif // MANIFOLD_HPACK_HPP
