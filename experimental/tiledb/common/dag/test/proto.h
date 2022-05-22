/**
 * @file experimental/tiledb/common/dag/test/proto.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2021 TileDB, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * This file defines some elementary node types for testing
 */

#ifndef TILEDB_PROTO_H
#define TILEDB_PROTO_H

#include <atomic>
#include "../dag.h"

namespace tiledb::common {
/*
 * Prototype source node.  Generates N integers
 */
template <class Block = size_t>
class producer_node : public Source<Block> {
  using Base = Source<Block>;

  std::atomic<size_t> i_{0};
  size_t N_{0};

 public:
  /**
   * Constructor
   * @tparam N The number of integers to generate
   */
  producer_node(size_t N)
      : N_{N} {
  }

  /**
   * Generate an output.
   */
  void run() {
    Base::item_ = i_++;
  }
};

/**
 * A proto consumer node.  Puts received data onto on output iterator
 */
template <class Iterator, class Block = size_t>
class output_node : public Sink<Block> {
  using Base = Sink<Block>;
  Iterator iter_;

 public:
  /**
   * Constructor
   * @param out An output iterator for
   */
  output_node(Iterator iter)
      : iter_{iter} {
  }

  /**
   * Coroutine to receive an input, case where Input is not void
   */
  void run() {
    *iter_++ = Base::item_;
  }
};  // namespace tiledb::common
}  // namespace tiledb::common
#endif  // TILEDB_PROTO_H
