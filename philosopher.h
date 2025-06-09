#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include "Kitchen.h"

class Philosopher {
public:
    enum class State { Thinking, Hungry, Ordering, Waiting, Eating, Paying };

    Philosopher(int id, const std::string& name, const std::string& favoriteDish, std::shared_ptr<Kitchen> kitchen);
    ~Philosopher();

    void start();
    void stop();
    void receiveFood();
    void markOrderTime() {
        orderTime = std::chrono::steady_clock::now();
    }

    void recordWaitTime(double cookTimeSeconds) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> waitDuration = now - orderTime;
        double extraWait = waitDuration.count() - cookTimeSeconds;
        if (extraWait > 0)
            totalExtraWaitTime += extraWait;
    }

    double getTotalExtraWaitTime() const {
        return totalExtraWaitTime;
    }

    void markOrderStart(int cookTime) {
        orderStartTime = std::chrono::steady_clock::now();
        currentDishCookTime = cookTime;
    }

    void markDishServed() {
        auto now = std::chrono::steady_clock::now();
        double waitSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - orderStartTime).count() / 1000.0;
        double extra = waitSeconds - currentDishCookTime;
        if (extra > 0)
            totalExtraWaitTime += extra;
    }

    State getState() const;
    std::string getName() const;
    int getId() const;
    bool isWaitingToOrder() const;

    std::string getCurrentOrder() const;

    std::string getCurrentOrder();
    void markOrderTaken();

    void markOrderStart(double cookTimeSeconds);

    static std::vector<std::string> availableDishes;

    std::chrono::steady_clock::time_point orderTime;
    double totalExtraWaitTime = 0.0;

private:
    void lifeCycle();
    void think();
    void getHungry();
    void orderFood();
    void waitForFood();
    void eat();
    void pay();

    std::chrono::steady_clock::time_point orderStartTime;
    int currentDishCookTime = 0; // w sekundach

    int id;
    std::string name;
    std::string favoriteDish;
    std::string currentOrder;

    State currentState;
    std::thread thread;
    std::mutex stateMutex;

    bool running;
    bool wantsToOrder = false;
    bool foodReady = false;

    std::shared_ptr<Kitchen> kitchen;
};
