/**
 * @file unit_ports.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2022 TileDB, Inc.
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
 * Tests the ports classes, `Source` and `Sink`.  We use some pseudo-nodes
 * for the testing.
 */

#include "unit_ports.h"
#include "experimental/tiledb/common/dag/ports/ports.h"
#include "pseudo_nodes.h"

using namespace tiledb::common;

TEST_CASE("Ports: Test bind", "[ports]") {
  Source<int> left;
  Sink<int> right;
  bind(left, right);
}

TEST_CASE("Ports: Test proto producer_node", "[ports]") {
  auto gen = generator<size_t>(10UL);
  auto pn = producer_node<size_t>(std::move(gen));
}

TEST_CASE("Ports: Test proto consumer_node", "[ports]") {
  std::vector<size_t> v;
  auto con = consumer<std::back_insert_iterator<std::vector<size_t>>>(
      std::back_insert_iterator<std::vector<size_t>>(v));
  auto cn = consumer_node<size_t>(std::move(con));
}

TEST_CASE(
    "Ports: Test connect proto consumer_node and proto producer_node",
    "[ports]") {
  std::vector<size_t> v;
  auto gen = generator<size_t>(10UL);
  auto con = consumer<std::back_insert_iterator<std::vector<size_t>>>(
      std::back_insert_iterator<std::vector<size_t>>(v));

  auto pn = producer_node<size_t>(std::move(gen));
  auto cn = consumer_node<size_t>(std::move(con));

  bind(pn, cn);
}

