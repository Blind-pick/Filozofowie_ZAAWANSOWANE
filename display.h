#ifndef DISPLAY_H
#define DISPLAY_H

#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "Philosopher.h"
#include "Waiter.h"
#include "Cook.h"
#include "Kitchen.h"

class Display {
public:
    Display(const std::vector<Philosopher *> &philosophers,
            const std::vector<Waiter *> &waiters,
            const std::vector<Cook *> &cooks,
            std::shared_ptr<Kitchen> kitchen);

    void start();

    void stop();

private:
    void displayLoop();

    void show();

    std::vector<Philosopher *> philosophers;
    std::vector<Waiter *> waiters;
    std::vector<Cook *> cooks;
    std::shared_ptr<Kitchen> kitchen;

    bool running = false;
    std::thread displayThread;
};


#endif //DISPLAY_H
