/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <atomic>
#include <iostream>

class Debug {
  public:
    explicit Debug(std::atomic<bool> &enabled, std::ostream &out = std::cout) : enabled_(&enabled), out_(&out) {}

    template <class T> Debug &operator<<(const T &v) {
        if (enabled_->load(std::memory_order_relaxed))
            *out_ << v;
        return *this;
    }

    using Manip = std::ostream &(*)(std::ostream &);
    Debug &operator<<(Manip m) {
        if (enabled_->load(std::memory_order_relaxed))
            *out_ << m;
        return *this;
    }

  private:
    std::atomic<bool> *enabled_;
    std::ostream *out_;
};

// ---- Global accessors (function-local statics are init-safe since C++11) ----

// Single shared flag and stream holder
inline std::atomic<bool> &flag() {
    static std::atomic<bool> enabled{false}; // default off
    return enabled;
}
inline std::ostream *&sink() {
    static std::ostream *out = &std::cout;
    return out;
}

// The global logger object
inline Debug &debug() {
    static Debug d(flag(), *sink());
    return d;
}

// Control functions
inline void enable_debug(bool on) { flag().store(on, std::memory_order_relaxed); }
inline bool is_debug_enabled() { return flag().load(std::memory_order_relaxed); }

// You can retarget to std::clog, std::cerr, or a file stream you manage
inline void set_debug_stream(std::ostream &os) {
    sink() = &os;
    // Reconstruct the Debug to point at the new stream (safe; runs once-per-call)
    static Debug &d = debug();
    d = Debug(flag(), os);
}

// Optional RAII helper for temporary enabling
struct scoped_enable {
    bool old;
    explicit scoped_enable(bool on = true) : old(is_debug_enabled()) { enable_debug(on); }
    ~scoped_enable() { enable_debug(old); }
};
