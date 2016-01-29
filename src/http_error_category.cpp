
#include "http_error_category.hpp"


//**********************************************************************//
namespace manifold
{
  //**********************************************************************//
  namespace http
  {
    //----------------------------------------------------------------------//
    errc int_to_errc(std::uint32_t error_code)
    {
      switch (error_code)
      {
        case (std::uint32_t)errc::no_error:            return errc::no_error;
        case (std::uint32_t)errc::protocol_error:      return errc::protocol_error;
        case (std::uint32_t)errc::internal_error:      return errc::internal_error;
        case (std::uint32_t)errc::flow_control_error:  return errc::flow_control_error;
        case (std::uint32_t)errc::settings_timeout:    return errc::settings_timeout;
        case (std::uint32_t)errc::stream_closed:       return errc::stream_closed;
        case (std::uint32_t)errc::frame_size_error:    return errc::frame_size_error;
        case (std::uint32_t)errc::refused_stream:      return errc::refused_stream;
        case (std::uint32_t)errc::cancel:              return errc::cancel;
        case (std::uint32_t)errc::compression_error:   return errc::compression_error;
        case (std::uint32_t)errc::connect_error:       return errc::connect_error;
        case (std::uint32_t)errc::enhance_your_calm:   return errc::enhance_your_calm;
        case (std::uint32_t)errc::inadequate_security: return errc::inadequate_security;
        case (std::uint32_t)errc::http_1_1_required:   return errc::http_1_1_required;
        default: return errc::protocol_error;
      }
    }
    //----------------------------------------------------------------------//

    //----------------------------------------------------------------------//
    error_category_impl::error_category_impl(): std::error_category()
    {
    }
    //----------------------------------------------------------------------//

    //----------------------------------------------------------------------//
    error_category_impl::~error_category_impl()
    {
    }
    //----------------------------------------------------------------------//

    //----------------------------------------------------------------------//
    const char* error_category_impl::name() const noexcept
    {
      return "http";
    }
    //----------------------------------------------------------------------//

//    //----------------------------------------------------------------------//
//    std::error_condition error_category_t::default_error_condition (int ev) const noexcept
//    {
//      if (ev<100) return std::error_condition(errc::NMOTOR_HTTP_ERRC_SUCCESS);
//      else if ((ev>=100)&&(ev<200)) return std::error_condition(errc::NMOTOR_HTTP_ERRC_DATA_TRANSFER_TIMEOUT);
//        //else if ((ev>=200)&&(ev<300)) return std::error_condition(errc::);
//      else return std::error_condition(errc::NMOTOR_HTTP_ERRC_OTHER);
//    }
//    //----------------------------------------------------------------------//

    ////----------------------------------------------------------------------//
    //std::error_condition error_category_t::default_error_condition (int ev) const
    //{
    //  if ((ev>=200)&&(ev<300)) return std::error_condition(error_condition::success);
    //  else if ((ev>=400)&&(ev<500)) return std::error_condition(error_condition::client_error);
    //  else if ((ev>=500)&&(ev<600)) return std::error_condition(error_condition::server_error);
    //  else return std::error_condition(error_condition::other);
    //}
    ////----------------------------------------------------------------------//

//    //----------------------------------------------------------------------//
//    bool error_category_t::equivalent (const std::error_code& code, int condition) const noexcept
//    {
//      return *this==code.category() && static_cast<int>(default_error_condition(code.value()).value())==condition;
//    }
//    //----------------------------------------------------------------------//

    //----------------------------------------------------------------------//
    std::string error_category_impl::message(int ev) const
    {
      switch (ev)
      {
        case 0x0: return "No Error";
        case 0x1: return "Protocol Error";
        case 0x2: return "Internal Error";
        case 0x3: return "Flow Control Error";
        case 0x4: return "Settings Timeout";
        case 0x5: return "Stream Closed";
        case 0x6: return "Frame Size Error";
        case 0x7: return "Refused Stream";
        case 0x8: return "Cancelled";
        case 0x9: return "Compression Error";
        case 0xA: return "Connect Error";
        case 0xB: return "Slow Your Roll";
        case 0xC: return "Inadequate Security";
        case 0xD: return "HTTP/1.1 Required";
        default: return "Unknown Error";
      }
    }
    //----------------------------------------------------------------------//

    std::error_code make_error_code (manifold::http::errc e)
    {
      return std::error_code(static_cast<int>(e), manifold::http::error_category_object);
    }
  }
  //**********************************************************************//
}
//**********************************************************************//

//**********************************************************************//

//**********************************************************************//


////**********************************************************************//
//// make_error_code overload to generate custom conditions:
//std::error_condition make_error_condition (BridgeDB::BTree::errc e)
//{
//  return std::error_condition(static_cast<int>(e), manifold::http::error_category_object);
//}
////**********************************************************************//
