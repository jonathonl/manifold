#pragma once

#ifndef MANIFOLD_HPACK_HPP
#define MANIFOLD_HPACK_HPP

#include <string>
#include <array>
#include <queue>
#include <deque>
#include <list>
#include <unordered_map>


namespace manifold
{
  namespace hpack
  {
    enum class cacheability
    {
      yes = 1,
      no,
      never
    };

    struct header_field
    {
      std::string name;
      std::string value;
      cacheability cache;
      header_field(std::string&& n, std::string&& v, cacheability cache_field = cacheability::yes)
        : name(std::move(n)), value(std::move(v)), cache(cache_field) {}
      header_field(const std::string& n, const std::string& v, cacheability cache_field = cacheability::yes)
        : name(n), value(v), cache(cache_field) {}
    };

    extern const std::array<std::pair<std::string,std::string>, 61> static_table;
    extern const std::unordered_multimap<std::string, std::size_t> static_table_reverse_lookup_map;

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
      std::size_t max_dynamic_table_size_;
      std::size_t current_dynamic_table_size_;
      std::deque<std::pair<std::string,std::string>> dynamic_table_;

      const std::pair<std::string,std::string>& at(std::size_t index) const
      {
        if (!index)
        {
          throw std::out_of_range("Table index cannot be zero.");
        }
        else
        {
          --index;
          if (index < static_table.size())
            return static_table[index];
          else if (index < this->dynamic_table_.size())
            return this->dynamic_table_[index];
          else
            throw std::out_of_range("Table index out of range.");
        }
      }

      std::size_t dynamic_table_size() const { return this->current_dynamic_table_size_; }
      std::size_t header_list_length() const { return static_table.size() + this->dynamic_table_.size(); }
      void table_evict();
      void table_insert(const std::pair<std::string,std::string>& entry);
      void table_insert(std::pair<std::string,std::string>&& entry);
    public:
      context(std::size_t max_table_size)
        : max_dynamic_table_size_(max_table_size), current_dynamic_table_size_(0) {}
      virtual ~context() {}
    };
    //================================================================//

    //================================================================//
    class encoder : public context
    {
    private:
      struct find_result { std::size_t name_index = 0; std::size_t name_and_value_index = 0; };
      std::queue<std::size_t> table_size_updates_;
      //std::multimap<std::pair<std::string,std::string>, std::size_t> dynamic_table_reverse_lookup_map_;

      static void encode_integer(prefix_mask prfx_mask, std::uint64_t input, std::string& output);
      static void huffman_encode(std::string::const_iterator begin, std::string::const_iterator end, std::string& output);
      find_result find(const header_field& header_to_find);
    public:
      encoder(std::size_t max_table_size)
        : context(max_table_size) {}
      void encode(const std::list<header_field>& headers, std::string& output);
      void add_table_size_update(std::size_t value) { this->table_size_updates_.push(value); }
    };
    //================================================================//

    //================================================================//
    class decoder : public context
    {
    private:
      static std::uint64_t decode_integer(prefix_mask prfx_mask, std::string::const_iterator& itr);
      static bool decode_string_literal(std::string::const_iterator& itr, std::string& output);
      bool decode_nvp(std::size_t table_index, cacheability cache_header, std::string::const_iterator& itr, std::list<header_field>& headers);
      static void huffman_decode(std::string::const_iterator begin, std::string::const_iterator end, std::string& output) {}
    public:
      decoder(std::size_t max_table_size)
        : context(max_table_size) {}
      bool decode(std::string::const_iterator beg, std::string::const_iterator end, std::list<header_field>& headers);
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HPACK_HPP
