// Copyright (c) 2017-2026 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
#pragma once

#include <inttypes.h>  // types, e.g. uint8_t NOLINT

// Lifecycle events shared by TJLed and TJLedGroup

namespace jled {

enum class Event : uint8_t {
    kStart = 1 << 0,            // effect started this tick (INIT -> RUNNING)
    kEnterDelayAfter = 1 << 1,  // TJLed only: effect entered a delay-after phase this tick
    kDone = 1 << 2,             // effect stopped this tick
    kRepeatStart = 1 << 3,      // a repetition cycle started this tick
    kActive =
        1 << 4,  // TJLed Only: first actual output tick (fires with kRepeatStart on iteration 0;
    // TJLedGroup only: the active element changed this tick. Only ever set in
    // SEQUENCE mode, where the group has a single "current" element; PARALLEL
    // groups have no such notion and never set this bit.
    kElementChanged = 1 << 5,
};

// EventSet holds a combination of Event bits.
using EventSet = uint8_t;

constexpr EventSet operator|(Event a, Event b) {
    return static_cast<EventSet>(a) | static_cast<EventSet>(b);
}

constexpr EventSet operator|(EventSet a, Event b) {
    return a | static_cast<EventSet>(b);
}

inline EventSet& operator|=(EventSet& a, Event b) {
    a |= static_cast<EventSet>(b);
    return a;
}

constexpr bool HasEvent(EventSet mask, Event flag) {
    return (mask & static_cast<EventSet>(flag)) != 0;
}

// Result of a single TJLed::Update() call: whether the effect is still
// running, which lifecycle events fired this tick, and the brightness value
// (if any) written to the HAL this tick.
template<typename T>
class UpdateResult {
    // Member order matters here: obj_ (the widest-aligned member, a pointer)
    // comes first so no alignment padding is inserted before it, and running_/
    // has_brightness_ are 1-bit fields sharing a single trailing byte instead of
    // two separate bool bytes. On a 32-bit MCU this keeps sizeof(UpdateResult<JLedHD>)
    // at 8 bytes instead of 12; on 8-bit AVR it saves a further byte per instance.
    T* obj_;
    typename T::brightness_t brightness_;  // value written this tick; valid iff has_brightness_
    EventSet event_;                       // bitmask of events that fired this tick
    uint8_t running_ : 1;
    uint8_t has_brightness_ : 1;  // true iff the HAL was written this tick

 public:
    UpdateResult(bool running, EventSet event, typename T::brightness_t brightness,
                 bool has_brightness, T* obj)
        : obj_(obj),
          brightness_(brightness),
          event_(event),
          running_(running),
          has_brightness_(has_brightness) {}

    // Implicit bool: preserves all existing if/while/|= usage
    operator bool() const { return running_; }  // NOLINT(runtime/explicit)

    bool IsRunning() const { return running_; }
    bool IsStarted() const { return HasEvent(event_, Event::kStart); }
    bool IsActive() const { return HasEvent(event_, Event::kActive); }
    bool IsRepeatStarted() const { return HasEvent(event_, Event::kRepeatStart); }
    bool IsEnteringDelayAfter() const { return HasEvent(event_, Event::kEnterDelayAfter); }
    bool IsDone() const { return HasEvent(event_, Event::kDone); }
    EventSet GetEvents() const { return event_; }

    // Value written to the HAL this tick. HasBrightness() distinguishes "wrote value 0"
    // from "wrote nothing". Holds the full range for both 8-bit (JLed) and 16-bit (JLedHD)
    // resolutions.
    bool HasBrightness() const { return has_brightness_; }
    typename T::brightness_t Brightness() const { return brightness_; }

    // Callback API
    // Template parameter F accepts any callable (lambda with or without capture,
    // function pointer). No heap allocation. Callbacks fire synchronously,
    // inline in the same statement as Update(). With the bitmask, more than one
    // of these may fire in a single call; they run in the order chained.

    template<typename F>
    UpdateResult& OnStart(F cb) {
        if (IsStarted()) cb(obj_);
        return *this;
    }

    template<typename F>
    UpdateResult& OnActive(F cb) {
        if (IsActive()) cb(obj_);
        return *this;
    }

    template<typename F>
    UpdateResult& OnRepeatStart(F cb) {
        if (IsRepeatStarted()) cb(obj_);
        return *this;
    }

    template<typename F>
    UpdateResult& OnEnterDelayAfter(F cb) {
        if (IsEnteringDelayAfter()) cb(obj_);
        return *this;
    }

    template<typename F>
    UpdateResult& OnDone(F cb) {
        if (IsDone()) cb(obj_);
        return *this;
    }
};

// Result of a single TJLedGroup::Update() call: whether the group is still
// running and which group-level events (kStart, kDone, kRepeatStart,
// kElementChanged) fired this tick. Distinct from UpdateResult<T>: a group
// has no brightness value, no delay-after phase, and no single "active"
// output tick of its own.
template<typename Group>
class GroupUpdateResult {
    Group* obj_;
    EventSet event_;
    uint8_t enter_index_;  // valid iff IsEnter(); index of the element that just became active
    uint8_t leave_index_;  // valid iff IsLeave(); index of the element that just stopped
    uint8_t running_ : 1;

 public:
    GroupUpdateResult(bool running, EventSet event, uint8_t enter_index, uint8_t leave_index,
                      Group* obj)
        : obj_(obj),
          event_(event),
          enter_index_(enter_index),
          leave_index_(leave_index),
          running_(running) {}

    // Implicit bool: preserves all existing if/while/|= usage
    operator bool() const { return running_; }  // NOLINT(runtime/explicit)

    bool IsRunning() const { return running_; }
    bool IsStarted() const { return HasEvent(event_, Event::kStart); }
    bool IsDone() const { return HasEvent(event_, Event::kDone); }
    bool IsRepeatStarted() const { return HasEvent(event_, Event::kRepeatStart); }
    bool IsElementChanged() const { return HasEvent(event_, Event::kElementChanged); }
    // IsEnter()/IsLeave(): SEQUENCE mode, some element became active / stopped being
    // active this tick, regardless of position. In PARALLEL mode these alias
    // IsStarted()/IsDone() exactly, since kElementChanged never fires there. Neither
    // ever fires for a group with zero elements: kStart/kDone still fire there (the
    // group itself starts and finishes), but there is no element to enter or leave.
    bool IsEnter() const {
        return obj_->HasElements() &&
               (HasEvent(event_, Event::kStart) || HasEvent(event_, Event::kElementChanged));
    }
    bool IsLeave() const {
        return obj_->HasElements() &&
               (HasEvent(event_, Event::kElementChanged) || HasEvent(event_, Event::kDone));
    }
    EventSet GetEvents() const { return event_; }

    template<typename F>
    GroupUpdateResult& OnStart(F cb) {
        if (IsStarted()) cb(obj_);
        return *this;
    }

    template<typename F>
    GroupUpdateResult& OnDone(F cb) {
        if (IsDone()) cb(obj_);
        return *this;
    }

    template<typename F>
    GroupUpdateResult& OnRepeatStart(F cb) {
        if (IsRepeatStarted()) cb(obj_);
        return *this;
    }

    template<typename F>
    GroupUpdateResult& OnElementChanged(F cb) {
        if (IsElementChanged()) cb(obj_);
        return *this;
    }

    // index is 0-based, into the array passed to Sequential()/Parallel(). In
    // PARALLEL mode, or on an empty group, the index is always 0 and does
    // not identify a specific element.
    template<typename F>
    GroupUpdateResult& OnEnter(F cb) {
        if (IsEnter()) cb(obj_, enter_index_);
        return *this;
    }

    template<typename F>
    GroupUpdateResult& OnLeave(F cb) {
        if (IsLeave()) cb(obj_, leave_index_);
        return *this;
    }
};

}  // namespace jled
