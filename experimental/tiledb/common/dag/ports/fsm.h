/**
 * @file   ports.h
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
 * This file declares the finite state machine for communicating ports.
 *
 */

#ifndef TILEDB_DAG_FSM_H
#define TILEDB_DAG_FSM_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tiledb::common {

enum class PortEvent : unsigned short {
  source_fill = 0,
  source_swap,
  sink_swap,
  sink_drain,
  shutdown
};

namespace {
inline constexpr unsigned short to_index(PortEvent x) {
  return static_cast<unsigned short>(x);
}

/**
 * Number of events in the PortEvent state machine
 */
constexpr unsigned int n_events = to_index(PortEvent::shutdown) + 1;

static std::vector<std::string> event_strings{
    "source_fill",
    "source_swap",
    "sink_swap",
    "sink_drain",
    "shutdown",
};

static auto str(PortEvent ev) {
  return event_strings[static_cast<int>(ev)];
}

}  // namespace

// clang-format off
/**
 *
 *    +-----------------+----------------------------------------------------------------------------------------+
 *    |      States     |                           Events                                                       |
 *    +--------+--------+------------------+-------------+-------------+------------+-----------------+----------+
 *    | Source |  Sink  | source_fill | source_swap | sink_start  | sink_swap  | sink_drain | shutdown |
 *    |--------+--------+------------------+-------------+-------------+------------+-----------------+----------+
 *    | empty  | empty  | full/empty       |             |             |            |                 |          |
 *    |--------+--------+------------------+-------------+-------------+------------+-----------------+----------+
 *    | empty  | full   | full/full        |             |             |            | empty/empty     |          |
 *    |--------+--------+------------------+-------------+-------------+------------+-----------------+----------+
 *    | full   | empty  |                  | empty/full  |             | empty/full |                 |          |
 *    |--------+--------+------------------+-------------+-------------+------------+-----------------+----------+
 *    | full   | full   |                  |             |             |            | full/empty      |          |
 *    +--------+--------+------------------+-------------+-------------+------------+-----------------+----------+
 *
 */
// clang-format on

enum class PortState {
  empty_empty,
  empty_full,
  full_empty,
  full_full,
  error,
  done
};

namespace {
constexpr unsigned short to_index(PortState x) {
  return static_cast<unsigned short>(x);
}
/**
 * Number of states in the Port state machine
 */
constexpr unsigned short n_states = to_index(PortState::done) + 1;
}  // namespace

static std::vector<std::string> port_state_strings{
    "empty_empty",
    "empty_full",
    "full_empty",
    "full_full",
    "error",
    "done",
};

static auto str(PortState st) {
  return port_state_strings[static_cast<int>(st)];
}

enum class PortAction : unsigned short {
  none = 0,
  fill_source,
  drain_sink,
  source_swap,
  sink_swap,
  shutdown
};

/**
 * State transition table.
 *
 * Originally included start events and separate sink_drained and source_filled
 * events. The start events are not necessary for a connected source and sink
 * if the initial state of the fsm is empty_full. The source_fill and
 * source_filled are really the same event, as are sink_drained and
 * sink_drain. Without start events, there are also no start states.
 *
 * Todo: Protect state transitions with mutex
 */
namespace {
// clang-format off
  constexpr PortState transition_table[n_states][n_events] {
  /* source_sink */    /* source_fill */      /* source_swap */      /* sink_swap */        /* sink_drain */        /* shutdown */
  /* empty_empty */  { PortState::full_empty, PortState::error,      PortState::error,      PortState::error,       PortState::error },
  /* empty_full  */  { PortState::full_full,  PortState::error,      PortState::error,      PortState::empty_empty, PortState::error },
  /* full_empty  */  { PortState::error,      PortState::empty_full, PortState::empty_full, PortState::error,       PortState::error },
  /* full_full   */  { PortState::error,      PortState::error,      PortState::error,      PortState::full_empty,  PortState::error },
		                             			     
  /* error       */  { PortState::error,      PortState::error,      PortState::error,      PortState::error,       PortState::error },
  /* done        */  { PortState::error,      PortState::error,      PortState::error,      PortState::error,       PortState::error },
  };

  /**
   * Transition actions table.
   */
  constexpr PortAction action_table[n_states][n_events] {
  /* source_sink */    /*source_fill */        /* source_swap */     /* sink_swap */        /* sink_drain */        /* shutdown */
  /* empty_empty */  { PortAction::none,       PortAction::none,     PortAction::none,      PortAction::none,       PortAction::none },
  /* empty_full  */  { PortAction::none,       PortAction::none,     PortAction::none,      PortAction::none,       PortAction::none },
  /* full_empty  */  { PortAction::none,       PortAction::none,     PortAction::none,      PortAction::none,       PortAction::none },
  /* full_full   */  { PortAction::none,       PortAction::none,     PortAction::none,      PortAction::none,       PortAction::none },

  /* error       */  { PortAction::none,       PortAction::none,       PortAction::none,     PortAction::none,      PortAction::none },
  /* done        */  { PortAction::none,       PortAction::none,       PortAction::none,     PortAction::none,      PortAction::none },
  };

  // clang-format on
}  // namespace

class PortStateMachine {
 private:
  PortState state_;

 public:
  /**
   * Default constructor
   */
  PortStateMachine()
      : state_(PortState::empty_full){};

  [[nodiscard]] inline PortState state() const {
    return state_;
  }

 public:
  void event(PortEvent event, const std::string msg = "") {
    auto new_state{transition_table[to_index(state_)][to_index(event)]};
    auto action{action_table[to_index(state_)][to_index(event)]};

    if (msg != "") {
      std::cout << msg + " " + str(event) + ": " + str(state_) + " -> " +
                       str(new_state)
                << std::endl;
    }

    switch (action) {
      case PortAction::fill_source:
        break;

      case PortAction::drain_sink:
        break;

      case PortAction::source_swap:
        break;

      case PortAction::sink_swap:
        break;

      case PortAction::none:
        break;

      default:
        throw std::logic_error("Port state action not reachable");
    }
    /*
     * Assign new state only after action returns without throwing.
     */
    state_ = new_state;
  }

  /*
   * Try to apply source / sink event
   */
  bool try_source_swap() {
    auto old_state = state_;
    event(PortEvent::source_swap);
    if (old_state == state_) {
      return false;
    }
    return true;
  }

  bool try_sink_swap() {
    return true;
  }
};

}  // namespace tiledb::common

#endif  // TILEDB_DAG_FSM_H
