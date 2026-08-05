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

#include <stddef.h>  // size_t NOLINT

#include <new>  // placement new

#include "jled_base.h"    // JLedBase, eIdleMode, TJLed
#include "jled_events.h"  // Event, EventSet, HasEvent, GroupUpdateResult
#include "jled_std.h"     // EnableIf, IsBaseOf

// TJLedGroup/TJLedAny/TJLedRef: grouping and type-erasure of TJLed subclasses
// and other TJLedGroup instances, without heap allocation or virtual calls
// for the common cases.

namespace jled {

// Forward declaration, TJLedAny is defined below.
template<size_t BufSize>
struct TJLedAny;

// TJLedGroup groups TJLedAny elements and plays them in parallel or sequentially.
template<typename Clock, typename AnyType>
class TJLedGroup {
 public:
    enum class eMode : uint8_t { SEQUENCE, PARALLEL };

    template<size_t N>
    static TJLedGroup Parallel(AnyType (&leds)[N]) {
        static_assert(N <= 255, "TJLedGroup supports at most 255 elements");
        return TJLedGroup(eMode::PARALLEL, leds, N);
    }
    static TJLedGroup Parallel(AnyType* leds, size_t n) {
        return TJLedGroup(eMode::PARALLEL, leds, n);
    }
    template<size_t N>
    static TJLedGroup Sequential(AnyType (&leds)[N]) {
        static_assert(N <= 255, "TJLedGroup supports at most 255 elements");
        return TJLedGroup(eMode::SEQUENCE, leds, N);
    }
    static TJLedGroup Sequential(AnyType* leds, size_t n) {
        return TJLedGroup(eMode::SEQUENCE, leds, n);
    }

    TJLedGroup& Repeat(uint16_t num_repetitions) {
        num_repetitions_ = num_repetitions;
        return *this;
    }
    TJLedGroup& Forever() { return Repeat(kRepeatForever); }
    bool IsForever() const { return num_repetitions_ == kRepeatForever; }
    bool HasElements() const { return n_ >= 1; }

    // Toggles playback direction for SEQUENCE mode (false = forward, the
    // default; true = backward). A harmless no-op in PARALLEL mode, since
    // UpdateParallel() never reads cur_. Configuration, not run state: it
    // survives Reset(), see Reset()'s own comment.
    TJLedGroup& Reverse() {
        reverse_ = !reverse_;
        return *this;
    }
    bool IsReversed() const { return reverse_; }

    // Advances cur_ by one step in the currently configured direction, without
    // invoking Update() on the skipped element this pass. Clamped to [0, n_):
    // a no-op if there is nowhere left to advance to (n_ <= 1, or cur_ already
    // at the far end for the current direction). Meaningful only in SEQUENCE
    // mode; harmless no-op in PARALLEL mode, same reasoning as Reverse().
    TJLedGroup& Skip() {
        if (n_ == 0) return *this;
        if (reverse_) {
            if (cur_ > 0) --cur_;
        } else {
            if (cur_ < n_ - 1) ++cur_;
        }
        return *this;
    }

    // Update() reads the clock once and delegates to Update(t).
    GroupUpdateResult<TJLedGroup> Update();
    GroupUpdateResult<TJLedGroup> Update(uint32_t t);
    TJLedGroup& Reset();
    void Stop(eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS);
    void Pause(eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS);
    void Resume();

    TJLedGroup(eMode mode, AnyType* leds, size_t n)
        : mode_(mode),
          leds_(leds),
          n_(static_cast<uint8_t>(n > 255 ? 255 : n)),
          is_running_(true),
          started_(false),
          done_(false),
          reverse_(false) {}

 protected:
    void Pause(uint32_t t, eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS);
    void Resume(uint32_t t);

    template<size_t N>
    friend struct TJLedAny;
    friend class TJLedRef;

 private:
    // GroupUpdateResult resolves OnElementEnter/OnElementLeave through IsSequenceMode()
    // and ElementRef(); neither is part of the public group API.
    template<typename Group>
    friend class GroupUpdateResult;

    bool IsSequenceMode() const { return mode_ == eMode::SEQUENCE; }
    AnyType& ElementRef(uint8_t i) { return leds_[i]; }

    bool UpdateParallel(uint32_t t);
    bool UpdateSequentially(uint32_t t);
    void ResetLeds();

    eMode mode_;
    AnyType* leds_;
    uint8_t n_;
    uint8_t cur_ = 0;
    static constexpr uint16_t kRepeatForever = 65535;
    uint16_t num_repetitions_ = 1;
    uint16_t iteration_ = 0;
    uint8_t is_running_ : 1;
    uint8_t started_ : 1;  // gates kStart to the first Update() call of a run
    uint8_t done_ : 1;     // gates kDone to the tick that observes is_running_ becoming false
    uint8_t reverse_ : 1;  // playback direction; false = forward (default), true = backward
    uint8_t last_update_time_ = 0;  // duplicate-tick guard, see Update()
};

// JLed intentionally uses no virtual methods. But holding a heterogeneous mix
// of JLed and JLedGroup objects in one array requires a uniform interface.
// TJLedAny provides this via type erasure: it stores any TJLed subclass or
// TJLedGroup by value in a fixed-size aligned buffer using a manual vtable.
// No heap allocation is required.
// sizeof(TJLedAny<N>) == N + sizeof(void*) (each instance holds one vtable
// pointer; the pointed-to Vtable struct is shared across all instances of the
// same concrete type).
template<size_t BufSize>
struct TJLedAny {
 private:
    struct Vtable {
        bool (*update)(void*, uint32_t);
        void (*reset)(void*);
        void (*stop)(void*, eIdleMode);
        void (*pause)(void*, uint32_t, eIdleMode);
        void (*resume)(void*, uint32_t);
        void (*copy)(void* dst, const void* src);
        void (*dtor)(void*);
    };

    alignas(alignof(max_align_t)) char buf_[BufSize];
    const Vtable* vtable_;

    template<typename T>
    static const Vtable* VtableFor() {
        static const Vtable kVt = {
            [](void* p, uint32_t t) -> bool { return static_cast<T*>(p)->Update(t); },
            [](void* p) { static_cast<T*>(p)->Reset(); },
            [](void* p, eIdleMode m) { static_cast<T*>(p)->Stop(m); },
            [](void* p, uint32_t t, eIdleMode m) { static_cast<T*>(p)->Pause(t, m); },
            [](void* p, uint32_t t) { static_cast<T*>(p)->Resume(t); },
            [](void* dst, const void* src) { new (dst) T(*static_cast<const T*>(src)); },
            [](void* p) { static_cast<T*>(p)->~T(); }};
        return &kVt;
    }

    template<typename T>
    void Init(const T& obj) {
        new (buf_) T(obj);
        vtable_ = VtableFor<T>();
    }

 public:
    // Accepts any TJLed subclass (JLed, JLedHD, and user-defined types).
    template<typename T, typename = typename EnableIf<IsBaseOf<JLedBase, T>::value>::type>
    TJLedAny(T t) {  // NOLINT
        static_assert(sizeof(T) <= BufSize,
                      "LED type exceeds TJLedAny buffer size. "
                      "Use TJLedAny<sizeof(YourType)> to define a custom-sized alias.");
        Init(t);
    }

    // Accepts any TJLedGroup<Clock, AnyType> (covers JLedGroup and clock variants used in tests).
    template<typename Clock, typename AnyType>
    TJLedAny(TJLedGroup<Clock, AnyType> g) {  // NOLINT
        static_assert(sizeof(TJLedGroup<Clock, AnyType>) <= BufSize,
                      "TJLedGroup exceeds TJLedAny buffer size. "
                      "Use TJLedAny<sizeof(YourType)> to define a custom-sized alias.");
        Init(g);
    }

    TJLedAny(const TJLedAny& other) : vtable_(other.vtable_) {
        other.vtable_->copy(buf_, other.buf_);
    }

    TJLedAny& operator=(const TJLedAny&) = delete;

    ~TJLedAny() { vtable_->dtor(buf_); }

    // Recovers a pointer to the stored object if it is exactly of type T (no
    // base-class match), or nullptr otherwise. VtableFor<T>() is one static
    // instance per instantiation, so comparing its address is a no-RTTI
    // stand-in for a dynamic_cast type check.
    template<typename T>
    T* As() {
        return vtable_ == VtableFor<T>() ? reinterpret_cast<T*>(buf_) : nullptr;
    }
    template<typename T>
    const T* As() const {
        return vtable_ == VtableFor<T>() ? reinterpret_cast<const T*>(buf_) : nullptr;
    }

 protected:
    bool Update(uint32_t t) { return vtable_->update(buf_, t); }
    void Reset() { vtable_->reset(buf_); }
    void Stop(eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS) { vtable_->stop(buf_, mode); }
    void Pause(uint32_t t, eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS) {
        vtable_->pause(buf_, t, mode);
    }
    void Resume(uint32_t t) { vtable_->resume(buf_, t); }

    template<typename Clock, typename AnyType>
    friend class TJLedGroup;
};

// TJLedRef is a non-owning type-erased reference to any LED or group.
// It stores a pointer to an externally managed object, no copying, no buffer.
// sizeof(TJLedRef) == 2 * sizeof(void*) on any platform.
// The referenced object must outlive the TJLedRef.
class TJLedRef {
    struct Vtable {
        bool (*update)(void*, uint32_t);
        void (*reset)(void*);
        void (*stop)(void*, eIdleMode);
        void (*pause)(void*, uint32_t, eIdleMode);
        void (*resume)(void*, uint32_t);
    };
    void* obj_;
    const Vtable* vtable_;

    template<typename T>
    static const Vtable* VtableFor() {
        static const Vtable kVt = {
            [](void* p, uint32_t t) -> bool { return static_cast<T*>(p)->Update(t); },
            [](void* p) { static_cast<T*>(p)->Reset(); },
            [](void* p, eIdleMode m) { static_cast<T*>(p)->Stop(m); },
            [](void* p, uint32_t t, eIdleMode m) { static_cast<T*>(p)->Pause(t, m); },
            [](void* p, uint32_t t) { static_cast<T*>(p)->Resume(t); }};
        return &kVt;
    }

 public:
    // Accepts a pointer to any TJLed subclass (JLed, JLedHD, user-defined).
    // Not explicit so that JLedRef refs[] = { &led1, &led2 } compiles directly.
    template<typename T, typename = typename EnableIf<IsBaseOf<JLedBase, T>::value>::type>
    TJLedRef(T* ptr) : obj_(ptr), vtable_(VtableFor<T>()) {}  // NOLINT

    // Accepts a pointer to any TJLedGroup (enables nested groups via JLedRef).
    template<typename Clock, typename AnyType>
    TJLedRef(TJLedGroup<Clock, AnyType>* ptr)  // NOLINT
        : obj_(ptr), vtable_(VtableFor<TJLedGroup<Clock, AnyType>>()) {}

    // Recovers the referenced object as T* if it was constructed from
    // exactly that type, or nullptr otherwise. See TJLedAny::As() for the
    // no-RTTI mechanism.
    template<typename T>
    T* As() {
        return vtable_ == VtableFor<T>() ? static_cast<T*>(obj_) : nullptr;
    }
    template<typename T>
    const T* As() const {
        return vtable_ == VtableFor<T>() ? static_cast<const T*>(obj_) : nullptr;
    }

 protected:
    bool Update(uint32_t t) { return vtable_->update(obj_, t); }
    void Reset() { vtable_->reset(obj_); }
    void Stop(eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS) { vtable_->stop(obj_, mode); }
    void Pause(uint32_t t, eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS) {
        vtable_->pause(obj_, t, mode);
    }
    void Resume(uint32_t t) { vtable_->resume(obj_, t); }

    template<typename Clock, typename AnyType>
    friend class TJLedGroup;
};

// TJLedGroup method bodies, defined after TJLedAny is complete.

template<typename Clock, typename AnyType>
bool TJLedGroup<Clock, AnyType>::UpdateParallel(uint32_t t) {
    auto result = false;
    for (auto i = 0U; i < n_; i++) {
        result |= leds_[i].Update(t);
    }
    return result;
}

template<typename Clock, typename AnyType>
bool TJLedGroup<Clock, AnyType>::UpdateSequentially(uint32_t t) {
    if (!leds_[cur_].Update(t)) {
        if (reverse_) {
            if (cur_ == 0) return false;
            --cur_;
            return true;
        }
        return ++cur_ < n_;
    }
    return true;
}

template<typename Clock, typename AnyType>
void TJLedGroup<Clock, AnyType>::ResetLeds() {
    for (auto i = 0U; i < n_; i++) {
        leds_[i].Reset();
    }
}

template<typename Clock, typename AnyType>
GroupUpdateResult<TJLedGroup<Clock, AnyType>> TJLedGroup<Clock, AnyType>::Update() {
    return Update(Clock::millis());
}

// kStart fires on the first Update() call of a run; kDone fires on the tick
// that first observes is_running_ having become false, whether that happened
// here (repetitions exhausted) or earlier via Stop().
//
// kRepeatStart fires once per repetition, including the first (coincident
// with kStart, mirroring TJLed's own kRepeatStart), and again each time a
// lap through all elements completes and another repetition follows.
//
// kElementChanged fires when the active element in SEQUENCE mode changes,
// including wrapping back to the first element at a new repetition.
// It never fires in PARALLEL mode or for a single-element sequence.
// It does NOT "see" into nested subgroups.
template<typename Clock, typename AnyType>
GroupUpdateResult<TJLedGroup<Clock, AnyType>> TJLedGroup<Clock, AnyType>::Update(uint32_t t) {
    EventSet events = 0;
    const auto cur_before = cur_;
    const bool was_started = started_;
    if (!started_) {
        started_ = true;
        events |= Event::kStart;
        events |= Event::kRepeatStart;
    }

    if (!is_running_ || n_ < 1) {
        if (!done_) {
            done_ = true;
            events |= Event::kDone;
        }
        return GroupUpdateResult<TJLedGroup>(false, events, cur_, cur_before, this);
    }

    // Duplicate-tick guard: a repetition boundary's ResetLeds() puts every
    // element back in ST_INIT, bypassing its own per-tick guard. Without
    // this check, a second Update() call at the same t would let a
    // one-tick effect (e.g. a plain On()) finish instantly, silently
    // advancing an extra lap.
    if (was_started && static_cast<uint8_t>(t & 255) == last_update_time_) {
        return GroupUpdateResult<TJLedGroup>(true, events, cur_, cur_before, this);
    }
    last_update_time_ = static_cast<uint8_t>(t & 255);

    const auto led_running = (mode_ == eMode::PARALLEL) ? UpdateParallel(t) : UpdateSequentially(t);

    if (led_running) {
        if (mode_ == eMode::SEQUENCE && cur_ != cur_before) {
            events |= Event::kElementChanged;
        }
        return GroupUpdateResult<TJLedGroup>(true, events, cur_, cur_before, this);
    }

    cur_ = 0;
    is_running_ = ++iteration_ < num_repetitions_ || num_repetitions_ == kRepeatForever;

    if (!is_running_) {
        if (!done_) {
            done_ = true;
            events |= Event::kDone;
        }
    } else {
        // Only rearm elements when another repetition is actually coming.
        // On final completion, each element already reached its own correct
        // terminal (stopped) state on this tick and must be left there.
        ResetLeds();
        events |= Event::kRepeatStart;
        if (mode_ == eMode::SEQUENCE && cur_ != cur_before) {
            events |= Event::kElementChanged;
        }
    }

    return GroupUpdateResult<TJLedGroup>(is_running_, events, cur_, cur_before, this);
}

template<typename Clock, typename AnyType>
TJLedGroup<Clock, AnyType>& TJLedGroup<Clock, AnyType>::Reset() {
    ResetLeds();
    // "Restart from the beginning" means the beginning of the currently
    // configured direction. Guard n_ > 0: n_ - 1 would underflow uint8_t for
    // an empty (n_ == 0) reversed group. reverse_ itself is untouched (it is
    // configuration, not run state, like num_repetitions_).
    cur_ = (reverse_ && n_ > 0) ? n_ - 1 : 0;
    iteration_ = 0;
    is_running_ = true;
    started_ = false;
    done_ = false;
    last_update_time_ = 0;
    return *this;
}

template<typename Clock, typename AnyType>
void TJLedGroup<Clock, AnyType>::Stop(eIdleMode mode) {
    is_running_ = false;
    for (auto i = 0U; i < n_; i++) {
        leds_[i].Stop(mode);
    }
}

template<typename Clock, typename AnyType>
void TJLedGroup<Clock, AnyType>::Pause(uint32_t t, eIdleMode mode) {
    for (auto i = 0U; i < n_; i++) leds_[i].Pause(t, mode);
}

template<typename Clock, typename AnyType>
void TJLedGroup<Clock, AnyType>::Pause(eIdleMode mode) {
    Pause(Clock::millis(), mode);
}

template<typename Clock, typename AnyType>
void TJLedGroup<Clock, AnyType>::Resume(uint32_t t) {
    for (auto i = 0U; i < n_; i++) leds_[i].Resume(t);
}

template<typename Clock, typename AnyType>
void TJLedGroup<Clock, AnyType>::Resume() {
    Resume(Clock::millis());
}

};  // namespace jled
