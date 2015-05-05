#pragma once

#ifndef MANIFOLD_HPACK_CONTEXT_HPP
#define MANIFOLD_HPACK_CONTEXT_HPP

#include <string>
#include <array>
#include <vector>


namespace manifold
{
  namespace hpack
  {
    extern const std::array<std::pair<std::string,std::string>, 61> static_table;

    //================================================================//
    class context
    {
    protected:
      std::vector<std::pair<std::string,std::string>> dynamic_table_;
    public:
      context() {}
      virtual ~context() {}
    };
    //================================================================//

    //================================================================//
    class encoder : public context
    {
    };
    //================================================================//

    //================================================================//
    class decoder : public context
    {
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HPACK_CONTEXT_HPP
