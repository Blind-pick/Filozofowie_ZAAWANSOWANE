#ifndef COOK_H
#define COOK_H

#include <string>
#include <thread>
#include <atomic>
#include "kitchen.h"

class Cook {
public:
    enum class State {
        Free,
        Busy
    };

    Cook(int id, const std::string &specialtyDish, Kitchen *kitchen);

    void start();

    void join();

    void stop();

    State getState();

    int getId() const;

    int id;
    std::atomic<bool> running;
    std::string specialtyDish;
    Kitchen *kitchen;

    std::thread thread;
    std::atomic<State> state = State::Free;

    void lifeCycle();

    void cookOrder(int philosopherId, const std::string &dishName);
};

#endif // COOK_H
