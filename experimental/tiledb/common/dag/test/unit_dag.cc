/**
 * @file tiledb/common/thread_pool/test/unit_thread_pool.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2018-2021 TileDB, Inc.
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
 * Tests the `Dag` class.
 */

#include "unit_dag.h"
#include "experimental/tiledb/common/dag/dag.h"
#include "proto.h"

using namespace tiledb::common;

TEST_CASE("Dag: Test bind", "[dag]") {
  Source<int> left;
  Sink<int> right;
  bind(left, right);
}

TEST_CASE("Dag: Test proto producer_node", "[dag]") {
  auto gen = generator<size_t>(10UL);
  auto pn = producer_node<size_t>(std::move(gen));
}

TEST_CASE("Dag: Test proto consumer_node", "[dag]") {
  std::vector<size_t> v;
  auto con = consumer<std::back_insert_iterator<std::vector<size_t>>>(
      std::back_insert_iterator<std::vector<size_t>>(v));
  auto cn = consumer_node<size_t>(std::move(con));
}

TEST_CASE(
    "Dag: Test connect proto consumer_node and proto producer_node", "[dag]") {
  std::vector<size_t> v;
  auto gen = generator<size_t>(10UL);
  auto con = consumer<std::back_insert_iterator<std::vector<size_t>>>(
      std::back_insert_iterator<std::vector<size_t>>(v));

  auto pn = producer_node<size_t>(std::move(gen));
  auto cn = consumer_node<size_t>(std::move(con));

  bind(pn, cn);
}

void db_test_0(DataBlock& db) {
  auto a = db.begin();
  auto b = db.cbegin();
  auto c = db.end();
  auto d = db.cend();

  REQUIRE(a == b);
  REQUIRE(++a == ++b);
  REQUIRE(a++ == b++);
  REQUIRE(a == b);
  REQUIRE(++a != b);
  REQUIRE(a == ++b);
  REQUIRE(c == d);
  auto e = c + 5;
  auto f = d + 5;
  REQUIRE(c == e - 5);
  REQUIRE(d == f - 5);
  REQUIRE(e == f);
  REQUIRE(e - 5 == f - 5);
  auto g = a + 1;
  REQUIRE(g > a);
  REQUIRE(g >= a);
  REQUIRE(a < g);
  REQUIRE(a <= g);
}

void db_test_1(const DataBlock& db) {
  auto a = db.begin();
  auto b = db.cbegin();
  auto c = db.end();
  auto d = db.cend();

  REQUIRE(a == b);
  REQUIRE(++a == ++b);
  REQUIRE(a++ == b++);
  REQUIRE(a == b);
  REQUIRE(++a != b);
  REQUIRE(a == ++b);
  REQUIRE(c == d);
  auto e = c + 5;
  auto f = d + 5;
  REQUIRE(c == e - 5);
  REQUIRE(d == f - 5);
  REQUIRE(e == f);
  REQUIRE(e - 5 == f - 5);
  auto g = a + 1;
  REQUIRE(g > a);
  REQUIRE(g >= a);
  REQUIRE(a < g);
  REQUIRE(a <= g);
}

TEST_CASE("Dag: Test create DataBlock", "[dag]") {
  auto db = DataBlock();
  db_test_0(db);
  db_test_1(db);
}
