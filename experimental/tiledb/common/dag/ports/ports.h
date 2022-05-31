/**
 * @file   ports.h
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
 * This file declares the Source and Sink ports for dag.
 *
 *
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

#ifndef TILEDB_PORTS_H
#define TILEDB_PORTS_H

namespace tiledb::comon {

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
  std::optional<Block> item_{};

  /**
   * The correspondent Sink, if any
   */
  Sink<Block>* correspondent_{nullptr};

 public:
  /**
   * Notification function to be called by a correspondent Sink to signal that
   * it is ready to receive data. If `try_send()` is called immediately, it
   * should ordinarily succeed.
   *
   * At the point of construction it should be as if
   * ready_to_receive(false) was called in the constructor body.
   */
  void ready_to_receive(bool);

  /**
   * Send the item_ to a correspondent sink.  Called by the sink.
   *
   * Call is non-blocking and will return false if there is no item available to
   * return. Otherwise, `block` will be swapped with `item_`.
   *
   * @param block Reference to item to receive data in the sink.
   * @post If copied to the sink, `item_` will be empty.
   */
  bool try_get(std::optional<Block>& block);

  /**
   * Assign a correspondent for this Source.
   */
  void bind(Sink<Block>& predecessor) {
    if (correspondent_ == nullptr) {
      correspondent_ = &predecessor;
    } else {
      throw std::runtime_error(
          "Attempting to bind to already bound correspondent");
    }
  }

  /**
   * Remove the current correspondent, if any.
   */
  void unbind() {
    correspondent_ = nullptr;
  }
};

/**
 * A data flow sink, used by both edges and nodes.
 *
 * Sink objects have two states: full and ready.
 */
template <class Block>
class Sink {
  friend class Source<Block>;
  template <class Bl>
  friend void bind(Source<Bl>& src, Sink<Bl>& snk);

  /**
   * @inv If an item is present, `try_receive` will succeed.
   */
  std::optional<Block> item_;

  /**
   * The correspondent Source, if any
   */
  Source<Block>* correspondent_{nullptr};

  /**
   * Mutex shared by a correspondent pair. It's defined in only the Sink
   * arbitrarily.  Protects transfer of data item from Source to the Sink.
   */
  std::mutex mutex_;

 public:
  /**
   * Notification function to be called by a correspondent Source to signal that
   * it is ready to send data. If `try_put()` is called immediately, it should
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
   *
   * If `item_` is empty when `try_receive` is called, it will be swapped with
   * the item being sent and true will be returned.  Otherwise, false will be
   * returned.
   *
   * @param block The item to be sent.  Normally this will be the source's
   * `item_`.
   * @post If return value is true, item_ will contain `block`.
   */
  bool try_put(std::optional<Block>& block);

  /**
   * Assign a correspondent for this Sink.
   */
  void bind(Source<Block>& successor) {
    if (correspondent_ == nullptr) {
      correspondent_ = &successor;
    } else {
      throw std::runtime_error(
          "Attempting to bind to already bound correspondent");
    }
  }

  /**
   * Remove the current correspondent, if any.
   */
  void unbind() {
    correspondent_ = nullptr;
  }
};

/**
 * Assign sink as correspondent to source and vice versa.
 */
template <class Block>
inline void bind(Source<Block>& src, Sink<Block>& snk) {
  std::scoped_lock(snk.mutex_);
  src.bind(snk);
  snk.bind(src);
}

/**
 * Remove the correspondent relationship between a source and sink
 *
 * @pre `src` and `snk` are in a correspondent relationship.
 */
template <class Block>
inline void unbind(Source<Block>& src, Sink<Block>& snk) {
  std::scoped_lock(snk.mutex_);
  src.unbind();
  snk.unbind();
};

}  // namespace tiledb::comon
#endif TILEDB_PORTS_H
