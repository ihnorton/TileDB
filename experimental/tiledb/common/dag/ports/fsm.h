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

#include <stdexcept>

namespace tiledb::common {

enum class PortEvent : unsigned short {
  src_data_fill = 0,
  sink_drained,
  source_swap,
  source_filled,
  sink_swap,
  sink_data_drain,
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

}  // namespace

// clang-format off
/**
 *    +-----------------+------------------------------------------------------------------------------------------------------+
 *    |      States     |                                             Events                                                   |
 *    +--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *    | Source |  Sink  | src_data_fill | sink_drained | source_swap | source_filled | sink_swap  | sink_data_drain | shutdown |
 *    |--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *    | empty  | empty  | full/empty    |              |             |               |            |                 |          |
 *    |--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *    | empty  | ready  |               |              |             |               |            |                 |          |
 *    |--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *    | empty  | full   | full/full     |              |             |               |            | empty/empty     |          |
 *    |--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *    | full   | empty  |               | ready/empty  |             | full/ready    |            |                 |          |
 *    |--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *    | full   | ready  |               | ready/ready  |             |               | empty/full |                 |          |
 *    |--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *    | full   | full   |               |              |             |               |            | full/empty      |          |
 *    |--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *    | ready  | empty  |               |              | empty/full  | ready/ready   |            |                 |          |
 *    |--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *    | ready  | ready  |               |              | empty/full  |               | empty/full |                 |          |
 *    |--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *    | ready  | full   |               |              |             |               |            |                 |          |
 *    +--------+--------+---------------+--------------+-------------+---------------+------------+-----------------+----------+
 *
 */
// clang-format on

enum class PortState {
  empty_empty = 0,
  empty_ready,
  empty_full,
  full_empty,
  full_ready,
  full_full,
  ready_empty,
  ready_ready,
  ready_full,
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

enum class PortAction : unsigned short {
  none = 0,
  fill_source,
  drain_sink,
  source_swap,
  sink_swap
};

namespace {
// clang-format off
  constexpr PortState transition_table[n_states][n_events] {
                       /* src_data_fill */    /* sink_drained */      /* source_swap */      /* source_filled */    /* sink_swap */         /* sink_data_drain */   /* shutdown */
  /* empty_empty */  { PortState::full_empty, PortState::error,       PortState::error,      PortState::error,       PortState::error,      PortState::error,       PortState::error },
  /* empty_ready */  { PortState::error,      PortState::error,       PortState::error,      PortState::error,       PortState::error,      PortState::error,       PortState::error },
  /* empty_full  */  { PortState::full_full,  PortState::error,       PortState::error,      PortState::error,       PortState::error,      PortState::empty_empty, PortState::error },
  /* full_empty  */  { PortState::error,      PortState::ready_empty, PortState::error,      PortState::full_ready,  PortState::error,      PortState::error,       PortState::error },
  /* full_ready  */  { PortState::error,      PortState::ready_ready, PortState::error,      PortState::error,       PortState::empty_full, PortState::error,       PortState::error },
  /* full_full   */  { PortState::error,      PortState::error,       PortState::error,      PortState::error,       PortState::error,      PortState::full_empty,  PortState::error },
  /* ready_empty */  { PortState::error,      PortState::error,       PortState::empty_full, PortState::ready_ready, PortState::error,      PortState::error,       PortState::error },
  /* ready_ready */  { PortState::error,      PortState::error,       PortState::empty_full, PortState::error,       PortState::empty_full, PortState::error,       PortState::error },
  /* ready_full  */  { PortState::error,      PortState::error,       PortState::error,      PortState::error,       PortState::error,      PortState::error,       PortState::error },
  /* error       */  { PortState::error,      PortState::error,       PortState::error,      PortState::error,       PortState::error,      PortState::error,       PortState::error },
  /* done        */  { PortState::error,      PortState::error,       PortState::error,      PortState::error,       PortState::error,      PortState::error,       PortState::error },
  };

  /**
   * Transition actions table.
   */
  constexpr PortAction action_table[n_states][n_events] {
                       /* src_data_fill */      /* sink_drained */      /* source_swap */        /* source_filled */      /* sink_swap */        /* sink_data_drain */   /* shutdown */
  /* empty_empty */  { PortAction::fill_source, PortAction::none,       PortAction::none,        PortAction::none,        PortAction::none,      PortAction::none,       PortAction::none },
  /* empty_ready */  { PortAction::none,        PortAction::none,       PortAction::none,        PortAction::none,        PortAction::none,      PortAction::none,       PortAction::none },
  /* empty_full  */  { PortAction::fill_source, PortAction::none,       PortAction::none,        PortAction::none,        PortAction::none,      PortAction::drain_sink, PortAction::none },
  /* full_empty  */  { PortAction::none,        PortAction::drain_sink, PortAction::none,        PortAction::fill_source, PortAction::none,      PortAction::none,       PortAction::none },
  /* full_ready  */  { PortAction::none,        PortAction::drain_sink, PortAction::none,        PortAction::none,        PortAction::sink_swap, PortAction::none,       PortAction::none },
  /* full_full   */  { PortAction::none,        PortAction::none,       PortAction::none,        PortAction::none,        PortAction::none,      PortAction::drain_sink, PortAction::none },
  /* ready_empty */  { PortAction::none,        PortAction::none,       PortAction::source_swap, PortAction::fill_source, PortAction::none,      PortAction::none,       PortAction::none },
  /* ready_ready */  { PortAction::none,        PortAction::none,       PortAction::source_swap, PortAction::none,        PortAction::sink_swap, PortAction::none,       PortAction::none },
  /* ready_full  */  { PortAction::none,        PortAction::none,       PortAction::none,        PortAction::none,        PortAction::none,      PortAction::none,       PortAction::none },
  /* error       */  { PortAction::none,        PortAction::none,       PortAction::none,        PortAction::none,        PortAction::none,      PortAction::none,       PortAction::none },
  /* done        */  { PortAction::none,        PortAction::none,       PortAction::none,        PortAction::none,        PortAction::none,      PortAction::none,       PortAction::none },
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
      : state_(PortState::empty_empty){};

  [[nodiscard]] inline PortState state() const {
    return state_;
  }

 public:
  void event(PortEvent event) {
    auto new_state{transition_table[to_index(state_)][to_index(event)]};
    auto action{action_table[to_index(state_)][to_index(event)]};

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
};

}  // namespace tiledb::common

#endif  // TILEDB_DAG_FSM_H
