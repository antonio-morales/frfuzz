/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <thread>

// include uint64_t
#include <cstdint>

#include <typeinfo>

#include <chrono>

// #include "utils.h"

class grFunction {};

class grThreadStatus {
  public:
    float progress;
    std::string currentItem;
    uint64_t timeElapsed;
};

template <typename _Callable> class grThread {
  private:
    std::thread *t;
    uint64_t ID;

    std::chrono::_V2::system_clock::time_point start_time;
    std::chrono::_V2::system_clock::time_point end_time;

    // state
    bool running = false;

    float prv_progress;
    std::string prv_currentItem;

    _Callable *prv_f;

    // std::initializer_list<_Args...> prv_args;

  public:
    grThread() {
        t = NULL;
        prv_f = NULL;

        prv_progress = 0.0;
        prv_currentItem = "";
    }

    grThread(_Callable &&f) {

        t = NULL;
        prv_f = f;

        prv_progress = 0.0;
        prv_currentItem = "";

        // std::initializer_list<_Args...> prv_args;

        // prv_args = {args ...};

        // std::cout << typeid(prv_args).name() << std::endl;

        // prv_args = args;

        // std::thread th1(f, args);
    }

    ~grThread() {}

    template <typename... _Args> // Parameter pack
    void run(_Args &&...args) {

        // progress::start();

        start_time = std::chrono::high_resolution_clock::now();

        // TODO: Store _Args...
        t = new std::thread(prv_f, args...);

        running = true;

        ID = toint(t->get_id());

        std::cout << "ID: " << ID << std::endl;

        // t->join();
        // progress::end();
    }

    grThreadStatus status() {

        progress::status tmp = progress::get(ID);
        prv_progress = tmp.progress;
        prv_currentItem = tmp.currentItem;

        grThreadStatus s;

        s.progress = prv_progress;
        s.currentItem = prv_currentItem;

        if (running) {
            auto duration = std::chrono::high_resolution_clock::now() - start_time;
            s.timeElapsed = duration.count();
        } else {
            s.timeElapsed = 0;
        }

        return s;
    }
};
