/**
 * @file   experimental/tiledb/common/dag/graph/taskgraph.h
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
 * This file declares the Dag class as well as its associated classes.
 */

#ifndef TILEDB_DAG_GRAPH_TASKGRAPH_H
#define TILEDB_DAG_GRAPH_TASKGRAPH_H

#include <memory>
#include <vector>

#include "experimental/tiledb/common/dag/edge/edge.h"
#include "experimental/tiledb/common/dag/execution/duffs.h"
#include "experimental/tiledb/common/dag/execution/task.h"
#include "experimental/tiledb/common/dag/execution/task_state_machine.h"
#include "experimental/tiledb/common/dag/execution/task_traits.h"
#include "experimental/tiledb/common/dag/nodes/node_traits.h"
#include "experimental/tiledb/common/dag/nodes/segmented_nodes.h"
#include "experimental/tiledb/common/dag/utility/print_types.h"

namespace tiledb::common {

template <class Scheduler>
class TaskGraph {
  using node_base_type = node_base;
  using node_type = node_t<node_base_type>;
  using node_handle_type = node_handle_t<node_base_type>;
  using task_type = task_t<Task<node_type>>;
  using task_handle_type = task_handle_t<Task<node_type>>;
  using scheduler_type = Scheduler;
  using edge_type = GraphEdge;
  using edge_handle_type = std::shared_ptr<edge_type>;

 public:
  /**
   * Default constructor.
   */
  TaskGraph() = default;

  /**
   * Default move constructor.
   */
  TaskGraph(TaskGraph&&) noexcept = default;

  /**
   * Default copy constructor.
   */
  TaskGraph(const TaskGraph&) = delete;

  /**
   * Default move assignment operator.
   */
  TaskGraph& operator=(TaskGraph&&) noexcept = default;

  /**
   * Default copy assignment operator.
   */
  TaskGraph& operator=(const TaskGraph&) = delete;

  /**
   * Default destructor.
   */
  ~TaskGraph() = default;

  /**
   * Create a producer node and add it to the graph.
   *
   * @param f The function to store in the node.
   *
   * @tparam Function The type of function to be stored by the node.  The
   * function must take as input an `std::stop_source` and return an item to be
   * processed by the next node in the graph. The function calls
   * `std::stop_source::request_stop()` to signal that the function will not
   * produce any more items.
   *
   * @todo: With CTAD we don't need separate kinds of node functions.
   */
  template <class Function>
  auto initial_node(Function&& f) {
    using T = std::invoke_result_t<Function, std::stop_source&>;
    auto tmp = producer_node<DuffsMover3, T>(std::move(f));
    nodes_.emplace_back(tmp);
    return tmp;
  }

  /**
   * Create a function node and add it to the graph.
   *
   * @param f The function to store in the node.
   *
   * @tparam Function The type of function to be held by the node.
   * The function must take an item as input
   * and return an item as output.
   */
  template<class R, class T>
  auto transform_node(std::function<R(T)>&& f) {
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    auto tmp = function_node<DuffsMover3, U, DuffsMover3, R>(std::move(f));
    nodes_.emplace_back(tmp);
    return tmp;
  }

  template <class Function>
  auto transform_node(Function&& f) {
    return transform_node(std::function{std::forward<Function>(f)});
  }

  /**
   * Create a multi-input, multi-output function node and add it to the graph.
   *
   * @param f The function to store in the node.
   *
   * @tparam Function The type of function to be held by the node.
   * The function must take an item as input
   * and return an item as output.
   */
  template <class Function>
  auto mimo_node(Function&& f) {
  }

  /**
   * Create a terminal node and add it to the graph.
   *
   * @param f The function to store in the node.
   *
   * @tparam Function The type of function to be held by the node.
   * The function must take an item as input
   * and return void.
   */
  template<class T>
  auto terminal_node(std::function<void(T)>&& f) {
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    auto tmp = consumer_node<DuffsMover3, U>(f);
    nodes_.emplace_back(tmp);
    return tmp;
  }

  template<class Func>
  auto terminal_node(Func&& f) {
    return terminal_node(std::function{std::forward<Func>(f)});
  }

  /**
   * Connect node `from` to node `to` with an edge.
   * An `Edge` connecting the `Source` of `from` to the `Sink` of `to` will be
   * created and added to the graph. A predecessor successor relationship will
   * be created between the `from` and `to` nodes as well as between the tasks
   * created from the `from` and `to` nodes.
   *
   * @param from A node supplying data to `to`.
   * @param to A node receiving data from `from`.
   */

  // producer_node<ThrowCatchMover2, unsigned long>
  // function_node<ThrowCatchMover2, unsigned long>
  // producer_node_impl<ThrowCatchMover2, unsigned long>
  // function_node_impl<ThrowCatchMover2, unsigned long>
  //
  template <class From, class To>
  void make_edge(From& from, To& to) {
    connect(from, to);
    edges_.emplace_back(std::make_shared<GraphEdge>(Edge(*from, *to)));
  }

  /**
   * Add an already created node to the graph.
   *
   * @param node The node to add to the graph.
   */
  void add_node(const node_handle_type& node) {
  }

  /**
   * Add an already created node to the graph.
   *
   * @param node The node to add to the graph.
   */
  void add_node(node_type&& node) {
  }

  /**
   * Begin execution of the graph.
   */
  void schedule(Scheduler& scheduler) {
    sched = &scheduler;
    for (auto& node : nodes_) {
      sched->submit(std::move(node));
    }
  }

  /**
   * Wait for the graph to complete its execution.  This function will block
   * until the graph has completed execution.
   */
  void sync_wait() {
    sched->sync_wait_all();
  }

 private:
  Scheduler *sched;
  std::vector<node_handle_type> nodes_;
  std::vector<task_handle_type> tasks_;
  std::vector<task_handle_type> roots_;
  std::vector<task_handle_type> leaves_;
  std::vector<edge_handle_type> edges_;
};


/*
 * @todo Note -- the typing of the functions require *values* because
 * of some details about optional items.  But we need to be able to use
 * functions taking things by reference.
 *
 * @todo Probably need to use std::decay et al to get the correct types for CTAD
 */

/**
 * @ brief Add an initial node to a graph.
 * @tparam Function
 * @param f
 */
template <class Graph, class Function>
auto initial_node(Graph& graph, Function&& f) {
  return graph.initial_node(std::forward<Function>(f));
}

/**
 * @ brief Add a function node to a graph.
 * @tparam Function
 * @param f
 */
template <class Graph, class Function>
auto transform_node(Graph& graph, Function&& f) {
  return graph.transform_node(std::forward<Function>(f));
}

/**
 * @ brief Add a multi-input, multi-output node to a graph.
 * @tparam Function
 * @param f
 */
template <class Graph, class Function>
auto mimo_node(Graph& graph, Function&& f) {
  return graph.mimo_node(std::forward<Function>(f));
}

/**
 * @ brief Add a terminal node to a graph.
 * @tparam Function
 * @param f
 */
template <class Graph, class Function>
auto terminal_node(Graph& graph, Function&& f) {
  return graph.terminal_node(std::forward<Function>(f));
}

template <class Graph, class From, class To>
void make_edge(Graph& graph, From& from, To& to) {
  graph.make_edge(from, to);
}

template <class Graph, class Schedule>
void schedule(Graph& graph, Schedule& sched) {
  graph.schedule(sched);
}

template <class Graph>
void sync_wait(Graph& graph) {
  graph.sync_wait();
}

}  // namespace tiledb::common

#endif  //  TILEDB_DAG_GRAPH_TASKGRAPH_H
