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
#include "experimental/tiledb/common/dag/ports/fsm.h"

using namespace tiledb::common;

TEST_CASE("Port FSM: Construct", "[fsm]") {
  [[maybe_unused]] auto a = PortStateMachine{};

  CHECK(a.state() == PortState::empty_empty);
}

TEST_CASE("Port FSM: Source sequence", "[fsm]") {
  [[maybe_unused]] auto a = PortStateMachine{};

  CHECK(a.state() == PortState::empty_empty);
  a.event(PortEvent::src_data_fill);

  CHECK(a.state() == PortState::full_empty);

  a.event(PortEvent::source_filled);

  CHECK(a.state() == PortState::full_ready);

  a.event(PortEvent::sink_drained);

  CHECK(a.state() == PortState::ready_ready);

  a.event(PortEvent::source_swap);

  CHECK(a.state() == PortState::empty_full);

  a.event(PortEvent::src_data_fill);

  CHECK(a.state() == PortState::full_full);

  a.event(PortEvent::sink_data_drain);

  CHECK(a.state() == PortState::full_empty);

  a.event(PortEvent::source_filled);

  CHECK(a.state() == PortState::full_ready);

  a.event(PortEvent::sink_swap);

  CHECK(a.state() == PortState::empty_full);

  a.event(PortEvent::sink_data_drain);

  CHECK(a.state() == PortState::empty_empty);

  a.event(PortEvent::src_data_fill);

  CHECK(a.state() == PortState::full_empty);

  a.event(PortEvent::sink_drained);

  CHECK(a.state() == PortState::ready_empty);

  a.event(PortEvent::source_swap);

  CHECK(a.state() == PortState::empty_full);

  a.event(PortEvent::src_data_fill);

  CHECK(a.state() == PortState::full_full);

  a.event(PortEvent::sink_data_drain);

  CHECK(a.state() == PortState::full_empty);

  a.event(PortEvent::source_filled);

  CHECK(a.state() == PortState::full_ready);

  a.event(PortEvent::sink_swap);

  CHECK(a.state() == PortState::empty_full);

  a.event(PortEvent::src_data_fill);

  CHECK(a.state() == PortState::full_full);

  a.event(PortEvent::sink_data_drain);

  CHECK(a.state() == PortState::full_empty);

  a.event(PortEvent::source_filled);

  CHECK(a.state() == PortState::full_ready);

  a.event(PortEvent::sink_drained);

  CHECK(a.state() == PortState::ready_ready);

  a.event(PortEvent::sink_swap);

  CHECK(a.state() == PortState::empty_full);

  a.event(PortEvent::sink_data_drain);

  CHECK(a.state() == PortState::empty_empty);

  a.event(PortEvent::src_data_fill);

  CHECK(a.state() == PortState::full_empty);

  a.event(PortEvent::sink_drained);

  CHECK(a.state() == PortState::ready_empty);

  a.event(PortEvent::source_filled);

  CHECK(a.state() == PortState::ready_ready);

  a.event(PortEvent::sink_swap);

  CHECK(a.state() == PortState::empty_full);

  a.event(PortEvent::sink_data_drain);

  CHECK(a.state() == PortState::empty_empty);
}
