#pragma once

#ifndef MANIFOLD_HTTP_RESPONSE_HEAD_HPP
#define MANIFOLD_HTTP_RESPONSE_HEAD_HPP

#include "http_message_head.hpp"

namespace manifold
{
  namespace http
  {
    //================================================================//
    enum class status_code : unsigned short
    {
      // informational
      continue_status                 = 100,
      switching_protocols             = 101,

      // successful
      ok                              = 200,
      created                         = 201,
      non_authoritative_information   = 202,
      no_content                      = 203,
      reset_content                   = 204,
      partial_content                 = 205,

      // redirection
      multiple_choices                = 300,
      moved_permanently               = 301,
      found                           = 302,
      see_other                       = 303,
      not_modified                    = 304,
      use_proxy                       = 305,
      // 306 unused
      temporary_redirect              = 307,

      // client error
      bad_request                     = 400,
      unauthorize                     = 401,
      payment_required                = 402,
      forbidden                       = 403,
      notfound                        = 404,
      method_not_allowed              = 405,
      not_acceptable                  = 406,
      proxy_authentication_required   = 407,
      request_timeout                 = 408,
      conflict                        = 409,
      gone                            = 410,
      length_required                 = 411,
      precondition_failed             = 412,
      requestentity_too_large         = 413,
      request_uri_too_long            = 414,
      unsupported_media_type          = 415,
      requested_range_not_satisfiable = 416,
      expectation_failed              = 417,

      // server error
      internal_server_error           = 500,
      not_implemented                 = 501,
      bad_gateway                     = 502,
      service_unavailable             = 503,
      gateway_timeout                 = 504,
      http_version_not_supported      = 505
    };
    //================================================================//

    //================================================================//
    class response_head : public header_block
    {
    private:
    public:
      //----------------------------------------------------------------//
      response_head();
      ~response_head();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      unsigned short status_code() const;
      void status_code(unsigned short value);
      void status_code(http::status_code value);
      //----------------------------------------------------------------//
    };
    //================================================================//
  };
}

#endif // MANIFOLD_HTTP_RESPONSE_HEAD_HPP