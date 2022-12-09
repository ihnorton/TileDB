/**
 * @file experimental/tiledb/common/dag/graph/test/unit_taskgraph.cc
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
 */

#include "unit_taskgraph.h"
#include "experimental/tiledb/common/dag/execution/duffs.h"
#include "experimental/tiledb/common/dag/graph/taskgraph.h"
#include "experimental/tiledb/common/dag/nodes/segmented_nodes.h"

using namespace tiledb::common;

TEST_CASE("TaskGraph: Trivial test", "[taskgraph]") {
  CHECK(true);
}

TEST_CASE("TaskGraph: Default construction", "[taskgraph]") {
  auto graph = TaskGraph<DuffsScheduler<node>>();
}

TEST_CASE("TaskGraph: Default construction + initial node", "[taskgraph]") {
  auto graph = TaskGraph<DuffsScheduler<node>>();

  initial_node(graph, [](std::stop_source stop) { return 0UL; });
}

void bar(const size_t) {
}

TEST_CASE("TaskGraph: Default construction + terminal node", "[taskgraph]") {
  auto graph = TaskGraph<DuffsScheduler<node>>();

  auto w = terminal_node(graph, bar);
  // auto foo = [](size_t){};
  // auto u = terminal_node(graph, foo);
  // auto v = terminal_node(graph, [](size_t) {});
}

/**
 * Some dummy functions and classes to test node constructors
 * with.
 */
size_t dummy_source(std::stop_source&) {
  return size_t{};
}

size_t dummy_function(size_t) {
  return size_t{};
}

void dummy_sink(size_t) {
}

class dummy_source_class {
 public:
  size_t operator()(std::stop_source&) {
    return size_t{};
  }
};

class dummy_function_class {
 public:
  size_t operator()(size_t) {
    return size_t{};
  }
};

class dummy_sink_class {
 public:
  void operator()(size_t) {
  }
};

size_t dummy_bind_source(std::stop_source, double) {
  return size_t{};
}

size_t dummy_bind_function(double, float, size_t) {
  return size_t{};
}

void dummy_bind_sink(size_t, float, int) {
}

/**
 * Some dummy function template and class templates to test node constructors
 * with.
 */
template <class Block = size_t>
size_t dummy_source_t(std::stop_source&) {
  return Block{};
}

template <class InBlock = size_t, class OutBlock = InBlock>
OutBlock dummy_function_t(InBlock) {
  return OutBlock{};
}

template <class Block = size_t>
void dummy_sink_t(const Block&) {
}

template <class Block = size_t>
class dummy_source_class_t {
 public:
  Block operator()() {
    return Block{};
  }
};

template <class InBlock = size_t, class OutBlock = InBlock>
class dummy_function_class_t {
 public:
  OutBlock operator()(InBlock) {
    return OutBlock{};
  }
};

template <class Block = size_t>
class dummy_sink_class_t {
 public:
  void operator()(Block) {
  }
};

template <class Block = size_t>
Block dummy_bind_source_t(std::stop_source, double) {
  return Block{};
}

template <class InBlock = size_t, class OutBlock = InBlock>
OutBlock dummy_bind_function_t(double, float, InBlock) {
  return OutBlock{};
}

template <class Block = size_t>
void dummy_bind_sink_t(Block, float, const int) {
}

TEST_CASE("TaskGraph: Initial and terminal node construction with various function types", "[taskgraph]") {
  auto graph = TaskGraph<DuffsScheduler<node>>();

  SECTION("function") {
    auto u = initial_node(graph, dummy_source);
    auto w = terminal_node(graph, dummy_sink);
  }

  SECTION("lambda") {
    auto dummy_source_lambda = [](std::stop_source&) { return 0UL; };
    auto dummy_sink_lambda = [](size_t) {};
    auto u = initial_node(graph, dummy_source_lambda);
    auto w = terminal_node(graph, dummy_sink_lambda);
  }

  SECTION("inline lambda") {
    auto u = initial_node(graph, [](std::stop_source&) { return 0UL; });
    auto w = terminal_node(graph, [](size_t) {});

    auto x = initial_node(graph, [](std::stop_source) { return 0UL; });
    auto z = terminal_node(graph, [](const size_t) {});
  }

  SECTION("function object") {
    auto a = dummy_source_class();
    auto b = dummy_sink_class();
    auto u = initial_node(graph, a);
    auto w = terminal_node(graph, b);
  }

  SECTION("inline function object") {
    auto u = initial_node(graph, dummy_source_class());
    auto w = terminal_node(graph, dummy_sink_class());
  }

  // Bind just ain't gonna work
#if 0
  SECTION("bind") {
    double x = 0.01;
    float y = -0.001;
    int z = 8675309;

    auto a = std::bind(dummy_bind_source, std::placeholders::_1, x);
    auto d = std::bind(dummy_bind_sink, std::placeholders::_1, y, z);

    auto u = initial_node(graph, a);
    auto w = terminal_node(graph, [](size_t x) { dummy_bind_sink(x, 0.0, 0); });
    auto ww = graph.terminal_node(d);
  }

  SECTION("inline bind") {
    double x = 0.01;
    float y = -0.001;
    int z = 8675309;

    auto u = initial_node ( graph, std::bind(dummy_bind_source, std::placeholders::_1, x) );
    auto w = terminal_node (graph, std::bind(dummy_bind_sink, y, std::placeholders::_1, z));
  }
#endif
}



TEST_CASE("TaskGraph: Initial, terminal, and transform node construction with various function types", "[taskgraph]") {
  auto graph = TaskGraph<DuffsScheduler<node>>();

  SECTION("function") {
    auto u = initial_node(graph, dummy_source);
    auto v = transform_node(graph, dummy_function);
    auto w = terminal_node(graph, dummy_sink);
  }

  SECTION("lambda") {
    auto dummy_source_lambda = [](std::stop_source&) { return 0UL; };
    auto dummy_function_lambda = [](size_t) { return 0UL; };
    auto dummy_sink_lambda = [](size_t) {};
    auto u = initial_node(graph, dummy_source_lambda);
    auto v = transform_node(graph, [](size_t) { return 0UL; });
    auto w = terminal_node(graph, dummy_sink_lambda);
  }

  SECTION("inline lambda") {
    auto u = initial_node(graph, [](std::stop_source&) { return 0UL; });
    auto v = transform_node(graph, [](size_t) { return 0UL; });
    auto w = terminal_node(graph, [](size_t) {});

    auto x = initial_node(graph, [](std::stop_source) { return 0UL; });
    auto y = transform_node(graph, [](size_t) { return 0UL; });
    auto z = terminal_node(graph, [](const size_t) {});
  }

  SECTION("function object") {
    auto a = dummy_source_class();
    auto b = dummy_function_class();
    auto c = dummy_sink_class();
    auto u = initial_node(graph, a);
    auto v = transform_node(graph, b);
    auto w = terminal_node(graph, c);
  }

  SECTION("inline function object") {
    auto u = initial_node(graph, dummy_source_class());
    auto v = transform_node(graph, dummy_function_class());
    auto w = terminal_node(graph, dummy_sink_class());
  }
}


TEST_CASE("TaskGraph: Task graph construction + edges", "[taskgraph]") {
  auto graph = TaskGraph<DuffsScheduler<node>>();

  SECTION("function") {
    auto u = initial_node(graph, dummy_source);
    auto v = transform_node(graph, dummy_function);
    auto w = terminal_node(graph, dummy_sink);

    make_edge(graph, u, v);
    make_edge(graph, v, w);
  }

  SECTION("lambda") {
    auto dummy_source_lambda = [](std::stop_source&) { return 0UL; };
    auto dummy_function_lambda = [](size_t) { return 0UL; };
    auto dummy_sink_lambda = [](size_t) {};
    auto u = initial_node(graph, dummy_source_lambda);
    auto v = transform_node(graph, [](size_t) { return 0UL; });
    auto w = terminal_node(graph, dummy_sink_lambda);
    make_edge(graph, u, v);
    make_edge(graph, v, w);
  }

  SECTION("inline lambda") {
    auto u = initial_node(graph, [](std::stop_source&) { return 0UL; });
    auto v = transform_node(graph, [](size_t) { return 0UL; });
    auto w = terminal_node(graph, [](size_t) {});

    auto x = initial_node(graph, [](std::stop_source) { return 0UL; });
    auto y = transform_node(graph, [](size_t) { return 0UL; });
    auto z = terminal_node(graph, [](const size_t) {});

    make_edge(graph, u, v);
    make_edge(graph, v, w);
    make_edge(graph, x, y);
    make_edge(graph, y, z);
  }

  SECTION("function object") {
    auto a = dummy_source_class();
    auto b = dummy_function_class();
    auto c = dummy_sink_class();
    auto u = initial_node(graph, a);
    auto v = transform_node(graph, b);
    auto w = terminal_node(graph, c);

    make_edge(graph, u, v);
    make_edge(graph, v, w);
  }

  SECTION("inline function object") {
    auto u = initial_node(graph, dummy_source_class());
    auto v = transform_node(graph, dummy_function_class());
    auto w = terminal_node(graph, dummy_sink_class());

    make_edge(graph, u, v);
    make_edge(graph, v, w);
  }
}



TEST_CASE("TaskGraph: Schedule", "[taskgraph]") {
  auto graph = TaskGraph<DuffsScheduler<node>>();

  auto num_threads = GENERATE(1, 2, 3, 4, 5, 8, 17);
  auto sched = DuffsScheduler<node>(num_threads);

  auto u = initial_node(graph, [](std::stop_source stop) { stop.request_stop(); return 0UL; });
  auto v = transform_node(graph, [](size_t) { return 0UL; });
  auto w = terminal_node(graph, [](size_t) {});

  make_edge(graph, u, v);
  make_edge(graph, v, w);

  schedule(graph, sched);
  sync_wait(graph);

}