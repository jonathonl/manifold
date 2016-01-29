#pragma once

#ifndef MANIFOLD_HTTP_ERROR_CATEGORY_HPP
#define MANIFOLD_HTTP_ERROR_CATEGORY_HPP

#include <string>
#include <system_error>

//#define _MSC_VER 1800
#ifdef _MSC_VER
#if (_MSC_VER < 1900)
#define MANIFOLD_DISABLE_HTTP2
#endif
#endif

namespace manifold
{
  namespace http
  {
    //**********************************************************************//
    // custom error conditions enum type:
    enum class errc : std::uint32_t
    {
      no_error            = 0x0,
      protocol_error      = 0x1,
      internal_error      = 0x2,
      flow_control_error  = 0x3,
      settings_timeout    = 0x4,
      stream_closed       = 0x5,
      frame_size_error    = 0x6,
      refused_stream      = 0x7,
      cancel              = 0x8,
      compression_error   = 0x9,
      connect_error       = 0xa,
      enhance_your_calm   = 0xb,
      inadequate_security = 0xc,
      http_1_1_required   = 0xd
    };
    //**********************************************************************//

    errc int_to_errc(std::uint32_t error_code);
  }
}

namespace std
{
  template<> struct is_error_code_enum<manifold::http::errc> : public true_type {};
  //template<> struct is_error_condition_enum<manifold::http::errc> : public true_type {};
}

namespace manifold
{
  namespace http
  {
    //**********************************************************************//
    // custom category:
    class error_category_impl : public std::error_category
    {
    public:
      error_category_impl();
      ~error_category_impl();
      //----------------------------------------------------------------------//
      const char* name() const noexcept;
      //std::error_condition default_error_condition (int ev) const noexcept;
      //bool equivalent (const std::error_code& code, int condition) const noexcept;
      std::string message(int ev) const;
      //----------------------------------------------------------------------//
    };
    //**********************************************************************//
    const error_category_impl error_category_object;
    //const error_category_t& error_category();

    std::error_code make_error_code(manifold::http::errc e);
  }
}


  //error_condition make_error_condition(BridgeDB::BTree::errc e);

#endif //MANIFOLD_HTTP_ERROR_CATEGORY_HPP