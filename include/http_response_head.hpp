#pragma once

#ifndef IPSUITE_HTTP_RESPONSE_HEAD_HPP
#define IPSUITE_HTTP_RESPONSE_HEAD_HPP

#include "http_message_head.hpp"

namespace IPSuite
{
  namespace HTTP
  {
    //================================================================//
    enum class StatusCode : unsigned short
    {
      // Informational
      Continue = 100,
      SwitchingProtocols = 101,

      // Successful
      Ok                            = 200,
      Created                       = 201,
      NonAuthoritativeInformation   = 202,
      NoContent                     = 203,
      ResetContent                  = 204,
      PartialContent                = 205,

      // Redirection
      MultipleChoices               = 300,
      MovedPermanently              = 301,
      Found                         = 302,
      SeeOther                      = 303,
      NotModified                   = 304,
      UseProxy                      = 305,
      // 306 unused
      TemporaryRedirect             = 307,

      // Client Error
      BadRequest                    = 400,
      Unauthorize                   = 401,
      PaymentRequired               = 402,
      Forbidden                     = 403,
      NotFound                      = 404,
      MethodNotAllowed              = 405,
      NotAcceptable                 = 406,
      ProxyAuthenticationRequired   = 407,
      RequestTimeout                = 408,
      Conflict                      = 409,
      Gone                          = 410,
      LengthRequired                = 411,
      PreconditionFailed            = 412,
      RequestEntityTooLarge         = 413,
      RequestURITooLong             = 414,
      UnsupportedMediaType          = 415,
      RequestedRangeNotSatisfiable  = 416,
      ExpectationFailed             = 417,

      // Server Error
      InternalServerError           = 500,
      NotImplemented                = 501,
      BadGateway                    = 502,
      ServiceUnavailable            = 503,
      GatewayTimeout                = 504,
      HTTPVersionNotSupported       = 505
    };
    //================================================================//

    //================================================================//
    class ResponseHead : public MessageHead
    {
    private:
      //----------------------------------------------------------------//
      unsigned short statusCode_;
      std::string reasonPhrase_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      ResponseHead();
      ~ResponseHead();
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      unsigned short statusCode() const;
      void statusCode(unsigned short value);
      void statusCode(StatusCode value);
      const std::string& reasonPhrase() const;
      void reasonPhrase(const std::string& value);
      //----------------------------------------------------------------//

      //----------------------------------------------------------------//
      void startLine(const std::string& value);
      std::string startLine() const;
      //----------------------------------------------------------------//
    };
    //================================================================//
  };
}

#endif // IPSUITE_HTTP_RESPONSE_HEAD_HPP