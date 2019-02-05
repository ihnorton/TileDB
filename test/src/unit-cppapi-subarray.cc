/**
 * @file   unit-cppapi-subarray.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 TileDB Inc.
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
 * Tests the C++ API for subarray related functions.
 */

#include "catch.hpp"
#include "tiledb/sm/cpp_api/tiledb"
#include "tiledb/sm/misc/utils.h"

using namespace tiledb;

TEST_CASE("C++ API: Test subarray", "[cppapi], [sparse], [subarray]") {
  const std::string array_name = "cpp_unit_array";
  Context ctx;
  VFS vfs(ctx);

  if (vfs.is_dir(array_name))
    vfs.remove_dir(array_name);

  // Create
  Domain domain(ctx);
  domain.add_dimension(Dimension::create<int>(ctx, "rows", {{0, 3}}, 4))
      .add_dimension(Dimension::create<int>(ctx, "cols", {{0, 3}}, 4));
  ArraySchema schema(ctx, TILEDB_SPARSE);
  schema.set_domain(domain).set_order({{TILEDB_ROW_MAJOR, TILEDB_ROW_MAJOR}});
  schema.add_attribute(Attribute::create<int>(ctx, "a"));
  Array::create(array_name, schema);

  // Write
  std::vector<int> data_w = {1, 2, 3, 4};
  std::vector<int> coords_w = {0, 0, 1, 1, 2, 2, 3, 3};
  Array array_w(ctx, array_name, TILEDB_WRITE);
  Query query_w(ctx, array_w);
  query_w.set_coordinates(coords_w)
      .set_layout(TILEDB_UNORDERED)
      .set_buffer("a", data_w);
  query_w.submit();
  query_w.finalize();
  array_w.close();

  SECTION("- Read single cell") {
    Array array(ctx, array_name, TILEDB_READ);
    Query query(ctx, array);
    Subarray subarray(ctx, array, TILEDB_UNORDERED);
    int range[] = {0, 0};
    subarray.add_range(0, range);
    subarray.add_range(1, range);

    auto est_size = subarray.est_result_size("a");
    REQUIRE(est_size == 1);

    std::vector<int> data(est_size);
    query.set_subarray(subarray)
        .set_layout(TILEDB_ROW_MAJOR)
        .set_buffer("a", data);
    query.submit();
    REQUIRE(query.result_buffer_elements()["a"].second == 1);
    REQUIRE(data[0] == 1);
  }

  SECTION("- Read single range") {
    Array array(ctx, array_name, TILEDB_READ);
    Query query(ctx, array);
    Subarray subarray(ctx, array, TILEDB_UNORDERED);
    int range[] = {1, 2};
    subarray.add_range(0, range);
    subarray.add_range(1, range);

    auto est_size = subarray.est_result_size("a");
    REQUIRE(est_size == 4);

    std::vector<int> data(est_size);
    query.set_subarray(subarray)
        .set_layout(TILEDB_ROW_MAJOR)
        .set_buffer("a", data);
    query.submit();
    REQUIRE(query.result_buffer_elements()["a"].second == 2);
    REQUIRE(data[0] == 2);
    REQUIRE(data[1] == 3);
  }

  SECTION("- Read two cells") {
    Array array(ctx, array_name, TILEDB_READ);
    Query query(ctx, array);
    Subarray subarray(ctx, array, TILEDB_UNORDERED);
    int range0[] = {0, 0}, range1[] = {2, 2};
    subarray.add_range(0, range0);
    subarray.add_range(1, range0);
    subarray.add_range(0, range1);
    subarray.add_range(1, range1);

    auto est_size = subarray.est_result_size("a");
    REQUIRE(est_size == 4);

    std::vector<int> data(est_size);
    query.set_subarray(subarray)
        .set_layout(TILEDB_UNORDERED)
        .set_buffer("a", data);
    query.submit();
    REQUIRE(query.result_buffer_elements()["a"].second == 2);
    REQUIRE(data[0] == 1);
    REQUIRE(data[1] == 3);
  }

  SECTION("- Read two regions") {
    Array array(ctx, array_name, TILEDB_READ);
    Query query(ctx, array);
    Subarray subarray(ctx, array, TILEDB_UNORDERED);
    int range0[] = {0, 1}, range1[] = {2, 3};
    subarray.add_range(0, range0);
    subarray.add_range(1, range0);
    subarray.add_range(0, range1);
    subarray.add_range(1, range1);

    auto est_size = subarray.est_result_size("a");
    std::vector<int> data(est_size);
    query.set_subarray(subarray)
        .set_layout(TILEDB_UNORDERED)
        .set_buffer("a", data);
    query.submit();
    REQUIRE(query.result_buffer_elements()["a"].second == 4);
    REQUIRE(data[0] == 1);
    REQUIRE(data[1] == 2);
    REQUIRE(data[2] == 3);
    REQUIRE(data[3] == 4);
  }

  if (vfs.is_dir(array_name))
    vfs.remove_dir(array_name);
}

TEST_CASE(
    "C++ API: Test subarray (incomplete)",
    "[cppapi], [sparse], [cppapi-subarray], [incomplete]") {
  const std::string array_name = "cpp_unit_array";
  Context ctx;
  VFS vfs(ctx);

  if (vfs.is_dir(array_name))
    vfs.remove_dir(array_name);

  // Create
  Domain domain(ctx);
  domain.add_dimension(Dimension::create<int>(ctx, "rows", {{0, 100}}, 101))
      .add_dimension(
          Dimension::create<int>(ctx, "cols", {{0, 100000}}, 100001));
  ArraySchema schema(ctx, TILEDB_SPARSE);
  schema.set_domain(domain)
      .set_order({{TILEDB_COL_MAJOR, TILEDB_COL_MAJOR}})
      .set_capacity(10000);
  schema.add_attribute(Attribute::create<char>(ctx, "a"));
  Array::create(array_name, schema);

  // Write
  std::vector<char> data_w = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
  std::vector<int> coords_w = {0, 12277, 0, 12771, 0, 13374, 0, 13395, 0, 13413,
                               0, 13451, 0, 13519, 0, 13544, 0, 13689, 0, 17479,
                               0, 17486, 1, 12277, 1, 12771, 1, 13389};
  Array array_w(ctx, array_name, TILEDB_WRITE);
  Query query_w(ctx, array_w);
  query_w.set_coordinates(coords_w)
      .set_layout(TILEDB_UNORDERED)
      .set_buffer("a", data_w);
  query_w.submit();
  query_w.finalize();
  array_w.close();

  // Open array for reading
  Array array(ctx, array_name, TILEDB_READ);
  Query query(ctx, array);
  query.set_layout(TILEDB_GLOBAL_ORDER);

  // Set up subarray for read
  int row_range[] = {0, 1};
  int col_range0[] = {12277, 13499};
  int col_range1[] = {13500, 17486};
  Subarray subarray(ctx, array, TILEDB_UNORDERED);
  subarray.add_range(0, row_range);
  subarray.add_range(1, col_range0);
  subarray.add_range(1, col_range1);
  query.set_subarray(subarray);

  // Allocate buffers large enough to hold 2 cells at a time.
  std::vector<char> data(2, '\0');
  std::vector<int> coords(4);
  query.set_coordinates(coords).set_buffer("a", data);

  // Submit query
  auto st = query.submit();
  REQUIRE(st == Query::Status::INCOMPLETE);
  auto result_elts = query.result_buffer_elements();
  auto result_num = result_elts[TILEDB_COORDS].second / 2;
  REQUIRE(result_num == 2);
  REQUIRE(data[0] == 'a');
  REQUIRE(data[1] == 'l');

  // Resubmit
  st = query.submit();
  REQUIRE(st == Query::Status::INCOMPLETE);
  result_elts = query.result_buffer_elements();
  result_num = result_elts[TILEDB_COORDS].second / 2;
  REQUIRE(result_num == 2);
  REQUIRE(data[0] == 'b');
  REQUIRE(data[1] == 'm');

  // Resubmit
  st = query.submit();
  REQUIRE(st == Query::Status::INCOMPLETE);
  result_elts = query.result_buffer_elements();
  result_num = result_elts[TILEDB_COORDS].second / 2;
  REQUIRE(result_num == 1);
  REQUIRE(data[0] == 'c');

  // Resubmit
  st = query.submit();
  REQUIRE(st == Query::Status::INCOMPLETE);
  result_elts = query.result_buffer_elements();
  result_num = result_elts[TILEDB_COORDS].second / 2;
  REQUIRE(result_num == 2);
  REQUIRE(data[0] == 'n');
  REQUIRE(data[1] == 'd');

  // Resubmit
  st = query.submit();
  REQUIRE(st == Query::Status::INCOMPLETE);
  result_elts = query.result_buffer_elements();
  result_num = result_elts[TILEDB_COORDS].second / 2;
  REQUIRE(result_num == 1);
  REQUIRE(data[0] == 'e');

  // Resubmit
  st = query.submit();
  REQUIRE(st == Query::Status::INCOMPLETE);
  result_elts = query.result_buffer_elements();
  result_num = result_elts[TILEDB_COORDS].second / 2;
  REQUIRE(result_num == 1);
  REQUIRE(data[0] == 'f');

  // Resubmit
  st = query.submit();
  REQUIRE(st == Query::Status::INCOMPLETE);
  result_elts = query.result_buffer_elements();
  result_num = result_elts[TILEDB_COORDS].second / 2;
  REQUIRE(result_num == 2);
  REQUIRE(data[0] == 'g');
  REQUIRE(data[1] == 'h');

  // Resubmit
  st = query.submit();
  REQUIRE(st == Query::Status::INCOMPLETE);
  result_elts = query.result_buffer_elements();
  result_num = result_elts[TILEDB_COORDS].second / 2;
  REQUIRE(result_num == 1);
  REQUIRE(data[0] == 'i');

  // Resubmit
  st = query.submit();
  REQUIRE(st == Query::Status::COMPLETE);
  result_elts = query.result_buffer_elements();
  result_num = result_elts[TILEDB_COORDS].second / 2;
  REQUIRE(result_num == 2);
  REQUIRE(data[0] == 'j');
  REQUIRE(data[1] == 'k');

  if (vfs.is_dir(array_name))
    vfs.remove_dir(array_name);
}