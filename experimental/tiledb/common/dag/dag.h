/**
 * @file   dag.h
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
 * This file declares the Dag class as well as its associated classes.
 */

#ifndef TILEDB_DAG_H
#define TILEDB_DAG_H

#include <cstddef>
#include <mutex>
#include <optional>

#include "tiledb/common/thread_pool.h"

namespace tiledb::common {

/*
 * Forward declarations
 */
template <class Block>
class EdgeQueue;

template <class Block>
class Source;

template <class Block>
class Sink;

/*
 * To be defined. First test is to hook up a raw source and a raw sink with an
 * edge.
 */
class Node;

/**
 * A fixed size block, an untyped carrier for data to be interpreted by its
 * users.
 *
 * Intended to be allocated from a pool with a bitmap allocation strategy.
 * Implemented internally as a span.
 *
 * Implements standard library interface for random access container.
 */
class DataBlock {
 public:
  DataBlock();  // fixed size configured as private constexpr

  class DataBlockIterator;
  class DataBlockConstIterator;

  size_t size() const;
  using iterator = DataBlockIterator;
  using const_iterator = DataBlockConstIterator;
  const_iterator cbegin() const;
  iterator begin();
  const_iterator cend() const;
  iterator end();
};

/**
 * A data flow source, used by both edges and nodes.
 *
 * Source objects have two states: empty and ready.
 */
template <class Block>
class Source {
  /**
   * @inv If an item is present, `try_send` will succeed.
   */
  std::optional<Block> item_;

  /**
   * The correspondent Sink, if any
   */
  Sink<Block>* correspondent_;

 public:
  /**
   * Notification function to be called by a correspondent Sink to signal that
   * it is ready to send data. If `try_send()` is called immediately, it should
   * ordinarily succeed.
   *
   * At the point of construction it should be as if
   * ready_to_receive(false) was called in the constructor body.
   */
  void ready_to_receive(bool);

  /**
   * Send a block to a correspondent sink.  Called by the sink.
   *
   * @post If return is true, `item_` is empty.
   */
  bool try_send(Block);

  /**
   * Assign a correspondent for this Source.
   */
  void bind(Sink<Block>*);

  /**
   * Remove the current correspondent, if any.
   */
  void unbind();
};

/**
 * Assign sink as correspondent to source and vice versa.
 */
template <class Block>
inline void bind(Source<Block>& src, Sink<Block>& snk) {
  src.bind(&snk);
  snk.bind(&src);
}

/**
 * Remove the correspondent relationship between a source and sink
 *
 * @pre `src` and `snk` are in a correspondent relationship.
 */
template <class Block>
inline void unbind(Source<Block>& src, Sink<Block>& snk) {
  src.unbind();
  snk.unbind();
};

/**
 * An edge in a task graph.
 *
 * Contains a queue of blocks of size 3, that is, at any time it has between 0
 * and 3 blocks in it.  Three blocks in the queue allows one to be written to on
 * one side of the edge, read from on the other side of the edge, with one ready
 * to be read.
 *
 * Edges implement a demand-pull pattern for synchronization.
 */
template <class Block>
class Edge : public Source<Block>, public Sink<Block> {
  EdgeQueue<Block*> queue_;

 public:
  Edge(Source<Block>& from, Sink<Block>& to);
};

/**
 * A data flow sink, used by both edges and nodes.
 *
 * Sink objects have two states: full and ready.
 */
template <class Block>
class Sink {
  friend class Source<Block>;

  /**
   * @inv If an item is present, `try_receive` will succeed.
   */
  std::optional<Block> item_;

  /**
   * The correspondent Source, if any
   */
  Source<Block>* correspondent_;

  /**
   * Mutex shared by a correspondent pair. It's defined in only the Sink
   * arbitrarily.
   */
  std::mutex m_;

 public:
  /**
   * Notification function to be called by a correspondent Source to signal that
   * it is ready to send data. If `try_send()` is called immediately, it should
   * ordinarily succeed.
   *
   * At the point of construction it should be as if
   * ready_to_send(false) was called in the constructor body.
   *
   * @pre This Sink object is registered as alive with the Scheduler.
   */
  void ready_to_send();

  /**
   * Receive a block from a correspondent source. Called by the source.
   */
  Block try_receive();

  /**
   * Assign a correspondent for this Sink.
   */
  void bind(Source<Block>*);

  /**
   * Remove the current correspondent, if any.
   */
  void unbind();
};

/**
 * Scheduler for the graph.
 *
 * The scheduler owns a thread pool. It is also an active object; at least one
 * thread in its pool is dedicated to its own operation.
 */
template <class Block>
class Scheduler {
  ThreadPool tp_;

 public:
  void notify_alive(Source<Block>*);
  void notify_quiescent(Source<Block>*);
  void notify_alive(Sink<Block>*);
  void notify_quiescent(Sink<Block>*);
  // Possibly needed
  // wakeup(Source *);
  // wakeup(Sink *);
};

/*
 * States for objects containing Source or Sink member variables.
 *
 * The design goal of these states is to limit the total number of std::thread
 * objects that simultaneously exist. Instead of a worker thread blocking
 * because correspondent source is empty or because a correspondent sink is
 * full, the worker can simply return. Tasks may become dormant without any
 * thread that runs them needing to block.
 *
 * States:
 *   Quiescent: initial and final state. No correspondent sources or sinks
 *   Dormant: Some correspondent exists, but no thread is currently active
 *   Active: Some correspondent exists, and some thread is currently active
 *
 * An element is alive if it is either dormant or active, that is, some
 * correspondent exists, regardless of thread state.
 *
 * Invariant: an element is registered with the scheduler as alive if and only
 * if the element is alive. Invariant: each element is registered with the
 * scheduler as either alive or quiescent.
 */

}  // namespace tiledb::common

#endif  // TILEDB_DAG_H
