#pragma once

#ifndef MANIFOLD_HTTP_STREAM_DEPENDENCY_TREE_HPP
#define MANIFOLD_HTTP_STREAM_DEPENDENCY_TREE_HPP

#include <vector>

namespace manifold
{
  namespace http
  {

    //================================================================//
    class stream_dependency_tree
    {
    private:
      //----------------------------------------------------------------//
      std::vector<stream_dependency_tree> children_;
      stream* stream_;
      //----------------------------------------------------------------//
    public:
      //----------------------------------------------------------------//
      stream_dependency_tree(connection::stream* stream_ptr);
      stream_dependency_tree(connection::stream* stream_ptr, const std::vector<stream_dependency_tree>& children);
      ~stream_dependency_tree() {}
      //----------------------------------------------------------------//


      //----------------------------------------------------------------//
      uint8_t weight() const;
      std::uint32_t stream_id() const;
      const std::vector<stream_dependency_tree>& children() const;
      //----------------------------------------------------------------//
    };
    //================================================================//
  }
}

#endif // MANIFOLD_HTTP_STREAM_DEPENDENCY_TREE_HPP