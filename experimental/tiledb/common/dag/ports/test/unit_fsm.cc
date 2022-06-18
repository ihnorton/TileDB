/**
 * @file unit_fsm.cc
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
 * Tests the ports finite state machine.
 */

#include "unit_fsm.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>
#include "experimental/tiledb/common/dag/ports/fsm.h"

using namespace tiledb::common;

TEST_CASE("Port FSM: Construct", "[fsm]") {
  [[maybe_unused]] auto a = PortStateMachine{};

  CHECK(a.state() == PortState::empty_full);
}

TEST_CASE("Port FSM: Start up", "[fsm]") {
  [[maybe_unused]] auto a = PortStateMachine{};

  CHECK(a.state() == PortState::empty_full);

  SECTION("start source") {
    a.event(PortEvent::source_fill);
    CHECK(a.state() == PortState::full_full);
  }

  SECTION("start sink") {
    a.event(PortEvent::sink_drain);
    CHECK(a.state() == PortState::empty_empty);
  }
}

TEST_CASE("Port FSM: Basic manual sequence", "[fsm]") {
  [[maybe_unused]] auto a = PortStateMachine{};

  CHECK(a.state() == PortState::empty_full);

  a.event(PortEvent::source_fill);
  CHECK(str(a.state()) == "full_full");
  a.event(PortEvent::sink_drain);
  CHECK(str(a.state()) == "full_empty");
  a.event(PortEvent::sink_swap);
  CHECK(str(a.state()) == "empty_full");

  a.event(PortEvent::sink_drain);
  CHECK(str(a.state()) == "empty_empty");

  CHECK(str(a.state()) == "empty_empty");
  a.event(PortEvent::source_fill);
  CHECK(str(a.state()) == "full_empty");
  a.event(PortEvent::source_swap);
  CHECK(a.state() == PortState::empty_full);

  a.event(PortEvent::sink_drain);
  CHECK(a.state() == PortState::empty_empty);

  a.event(PortEvent::source_fill);
  CHECK(str(a.state()) == "full_empty");
  a.event(PortEvent::source_swap);
  CHECK(str(a.state()) == "empty_full");
  a.event(PortEvent::source_fill);
  CHECK(str(a.state()) == "full_full");
  a.event(PortEvent::sink_drain);
  CHECK(str(a.state()) == "full_empty");
  a.event(PortEvent::source_swap);
  CHECK(str(a.state()) == "empty_full");

  a.event(PortEvent::sink_drain);
  CHECK(a.state() == PortState::empty_empty);

  a.event(PortEvent::source_fill);
  CHECK(str(a.state()) == "full_empty");
  a.event(PortEvent::source_swap);
  CHECK(str(a.state()) == "empty_full");
  a.event(PortEvent::source_fill);
  CHECK(str(a.state()) == "full_full");
  a.event(PortEvent::sink_drain);
  CHECK(str(a.state()) == "full_empty");
  a.event(PortEvent::sink_swap);
  CHECK(str(a.state()) == "empty_full");

  a.event(PortEvent::sink_drain);
  CHECK(a.state() == PortState::empty_empty);

  a.event(PortEvent::source_fill);
  CHECK(str(a.state()) == "full_empty");
  a.event(PortEvent::sink_swap);
  CHECK(str(a.state()) == "empty_full");
  a.event(PortEvent::source_fill);
  CHECK(str(a.state()) == "full_full");
  a.event(PortEvent::sink_drain);
  CHECK(str(a.state()) == "full_empty");
  a.event(PortEvent::source_swap);
  CHECK(str(a.state()) == "empty_full");

  a.event(PortEvent::sink_drain);
  CHECK(a.state() == PortState::empty_empty);

  a.event(PortEvent::source_fill);
  CHECK(str(a.state()) == "full_empty");
  a.event(PortEvent::sink_swap);
  CHECK(str(a.state()) == "empty_full");
  a.event(PortEvent::source_fill);
  CHECK(str(a.state()) == "full_full");
  a.event(PortEvent::sink_drain);
  CHECK(str(a.state()) == "full_empty");
  a.event(PortEvent::sink_swap);
  CHECK(str(a.state()) == "empty_full");

  a.event(PortEvent::sink_drain);
  CHECK(a.state() == PortState::empty_empty);
}

std::string is_src_empty(PortState st) {
  if (str(st) == "empty_start" || str(st) == "empty_empty" ||
      str(st) == "empty_full" || str(st) == "empty_ready") {
    return {};
  }
  return str(st);
}

std::string is_src_full(PortState st) {
  if (str(st) == "full_start" || str(st) == "full_empty" ||
      str(st) == "full_full" || str(st) == "full_ready") {
    return {};
  }
  return str(st);
}

std::string is_src_ready(PortState st) {
  if (str(st) == "ready_start" || str(st) == "ready_empty" ||
      str(st) == "ready_full" || str(st) == "ready_ready") {
    return {};
  }
  return str(st);
}

std::string is_src_swappable(PortState st) {
  if (str(st) == "ready_start" || str(st) == "ready_empty" ||
      str(st) == "ready_full" || str(st) == "ready_ready") {
    return {};
  }
  return str(st);
}

std::string is_snk_empty(PortState st) {
  if (str(st) == "start_empty" || str(st) == "empty_empty" ||
      str(st) == "full_empty" || str(st) == "ready_empty") {
    return {};
  }
  return str(st);
}

std::string is_snk_full(PortState st) {
  if (str(st) == "start_full" || str(st) == "empty_full" ||
      str(st) == "full_full" || str(st) == "ready_full") {
    return {};
  }
  return str(st);
}

std::string is_snk_ready(PortState st) {
  if (str(st) == "start_ready" || str(st) == "empty_ready" ||
      str(st) == "full_ready" || str(st) == "ready_ready") {
    return {};
  }
  return str(st);
}

size_t random_ms(size_t max = 7) {
  thread_local static uint64_t generator_seed =
      std::hash<std::thread::id>()(std::this_thread::get_id());
  thread_local static std::mt19937_64 generator(generator_seed);
  std::uniform_int_distribution<size_t> distribution(0, max);
  return distribution(generator);
}

TEST_CASE("Port FSM: Asynchronous source and sink", "[fsm]") {
  constexpr bool debug = false;

  [[maybe_unused]] auto a = PortStateMachine{};

  CHECK(str(a.state()) == "empty_full");

  std::mutex mutex_;
  std::condition_variable source_cv, sink_cv;

  size_t rounds = 33;
  if (debug)
    rounds = 3;

  int source_item{0};
  int sink_item{0};

  /**
   * Function to emulate an asynchronous source node
   *
   * The basic steps/states are:
   *   start -> state: empty_any
   *   while (true)
   *     invoke producer function
   *     submit -> state: full_any
   *     wait
   *       sink could swap -> state = empty_full
   *       sink could drain -> full_empty or ready_empty
   *     if (full_empty || ready_empty)
   *       swap -> empty_full
   */
  auto source_node = [&]() {
    size_t n = rounds;

    // Event loop for source
    while (n--) {
      std::unique_lock lock(mutex_);

      {
        if (debug)
          std::cout << "source filling " << str(a.state()) << std::endl;

        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(random_ms(19)));
        lock.lock();
      }

      CHECK(source_item == 0);
      a.event(PortEvent::source_fill, debug ? "source" : "");

      source_item = 1;
      CHECK(is_src_full(a.state()) == "");
      sink_cv.notify_one();
      source_cv.wait(lock);

      if (a.state() == PortState::full_empty) {
        if (debug)
          std::cout << "source swapping " << str(a.state()) << std::endl;

        a.event(PortEvent::source_swap, debug ? "source" : "");
        CHECK(source_item == 1);
        CHECK(sink_item == 0);
        std::swap(source_item, sink_item);
        sink_cv.notify_one();

        CHECK(is_src_empty(a.state()) == "");
        CHECK(is_snk_full(a.state()) == "");
      } else {
        if (debug)

          std::cout << "source try_swap: "
                    << port_state_strings[static_cast<int>(a.state())]
                    << std::endl;

        CHECK(is_src_empty(a.state()) == "");
        CHECK(source_item == 0);
      }
    }
  };

  /**
   * Function to emulate an asynchronous sink node
   *
   * The basic steps/states are:
   *   start -> state: any_full
   *   drain -> state: any_empty
   *   while (true)
   *     wait
   *       source could swap -> state = empty_full
   *       source could fill -> full_empty or full_ready
   *     if (full_empty || full_ready)
   *       swap -> empty_full
   *     invoke consumer function
   *     drain -> state: any_empty
   */
  auto sink_node = [&]() {
    size_t n = rounds;

    {
      std::unique_lock lock(mutex_);
      a.event(PortEvent::sink_drain, debug ? "sink" : "");
      CHECK(is_snk_empty(a.state()) == "");
      source_cv.notify_one();
    }

    // Event loop for sink
    while (n--) {
      std::unique_lock lock(mutex_);
      sink_cv.wait(lock);

      if (debug)
        std::cout << "sink coming out of wait  " << str(a.state()) << std::endl;

      if (a.state() == PortState::full_empty) {
        CHECK(source_item == 1);
        CHECK(sink_item == 0);
        a.event(PortEvent::sink_swap, debug ? "sink" : "");
        std::swap(source_item, sink_item);

        CHECK(is_src_empty(a.state()) == "");
        CHECK(is_snk_full(a.state()) == "");

        source_cv.notify_one();
      } else {
        if (debug)
          std::cout << "sink try swap " << str(a.state()) << std::endl;

        CHECK(is_snk_full(a.state()) == "");
        CHECK(sink_item == 1);
      }

      {
        if (debug)
          std::cout << "sink retrieving " << str(a.state()) << std::endl;

        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(random_ms(19)));
        lock.lock();
      }

      a.event(PortEvent::sink_drain, debug ? "sink" : "");
      CHECK(is_snk_empty(a.state()) == "");
      CHECK(sink_item == 1);
      sink_item = 0;
      source_cv.notify_one();
    }
  };

  SECTION("launch source before sink, get source before sink") {
    auto fut_a = std::async(std::launch::async, source_node);
    auto fut_b = std::async(std::launch::async, sink_node);

    fut_a.get();
    fut_b.get();
  }

  SECTION("launch sink before source, get source before sink") {
    auto fut_b = std::async(std::launch::async, sink_node);
    auto fut_a = std::async(std::launch::async, source_node);

    fut_a.get();
    fut_b.get();
  }

  SECTION("launch source before sink, get sink before source") {
    auto fut_a = std::async(std::launch::async, source_node);
    auto fut_b = std::async(std::launch::async, sink_node);

    fut_b.get();
    fut_a.get();
  }

  SECTION("launch sink before source, get sink before source") {
    auto fut_b = std::async(std::launch::async, sink_node);
    auto fut_a = std::async(std::launch::async, source_node);

    fut_b.get();
    fut_a.get();
  }

  CHECK(is_src_empty(a.state()) == "");
  CHECK(is_snk_empty(a.state()) == "");
  CHECK(source_item == 0);
  CHECK(sink_item == 0);
}
