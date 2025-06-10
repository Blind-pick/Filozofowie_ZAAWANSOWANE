#include "Philosopher.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <random>

Philosopher::Philosopher(int id, const std::string &name, const std::string &favoriteDish,
                         std::shared_ptr<Kitchen> kitchen)
    : id(id), name(name), favoriteDish(favoriteDish), kitchen(kitchen), currentState(State::Thinking),
      running(true), wantsToOrder(false), foodReady(false), orderTaken(false) {
}

Philosopher::~Philosopher() {
    stop();
    if (thread.joinable()) thread.join();
}

void Philosopher::start() {
    running = true;
    thread = std::thread(&Philosopher::lifeCycle, this);
}

void Philosopher::stop() {
    running = false;

    // Obudź filozofa jeśli czeka na kelnera, żeby nie wisiał na wait()
    {
        std::lock_guard<std::mutex> lock(waiterMutex);
        orderTaken = true;
    }
    waiterCondition.notify_one();
}

void Philosopher::lifeCycle() {
    while (running) {
        think();
        getHungry();
        orderFood();
        if (!running) break;
        waitForFood();
        if (!running) break;
        eat();
        pay();
    }
}

void Philosopher::think() {
    currentState = State::Thinking;
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 3000 + 1000));
}

void Philosopher::getHungry() {
    currentState = State::Hungry;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void Philosopher::orderFood() {
    currentState = State::Ordering;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    std::string chosenDish;

    if (dis(gen) < 0.6) {
        chosenDish = favoriteDish;
    } else {
        std::vector<std::string> allDishes;
        auto menu = kitchen->getMenu();
        for (const auto &pair : menu) {
            if (pair.first != favoriteDish) {
                allDishes.push_back(pair.first);
            }
        }
        if (!allDishes.empty()) {
            std::uniform_int_distribution<> indexDist(0, allDishes.size() - 1);
            chosenDish = allDishes[indexDist(gen)];
        } else {
            chosenDish = favoriteDish;
        }
    }

    {
        std::lock_guard<std::mutex> lock(stateMutex);
        currentOrder = chosenDish;
        wantsToOrder = true;
    }

    // Czekaj na kelnera (orderTaken == true)
    {
        std::unique_lock<std::mutex> lock(waiterMutex);
        orderTaken = false;
        waiterCondition.wait(lock, [this] { return orderTaken || !running; });
    }

    if (!running) return;

    wantsToOrder = false;

    auto menu = kitchen->getMenu();
    if (menu.count(currentOrder)) {
        markOrderStart(menu.at(currentOrder).cookTimeMs / 1000.0);
    }
}

void Philosopher::waitForFood() {
    currentState = State::Waiting;
    foodReady = false;
    while (!foodReady && running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Philosopher::eat() {
    currentState = State::Eating;
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 3000 + 1000));

    std::string cutleryType;
    auto menu = kitchen->getMenu();
    if (menu.count(currentOrder)) {
        cutleryType = menu.at(currentOrder).cutlery;
    }

    if (!cutleryType.empty()) {
        kitchen->returnUsedCutlery(cutleryType);
    }
}

void Philosopher::pay() {
    currentState = State::Paying;

    double price = 0.0;
    auto menu = kitchen->getMenu();
    if (menu.count(currentOrder)) {
        price = menu.at(currentOrder).price;
    }

    if (price > 0.0) {
        kitchen->addIncome(price);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void Philosopher::receiveFood() {
    foodReady = true;
    markDishServed();
}

Philosopher::State Philosopher::getState() const {
    return currentState;
}

std::string Philosopher::getName() const {
    return name;
}

int Philosopher::getId() const {
    return id;
}

bool Philosopher::isWaitingToOrder() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return wantsToOrder;
}

std::string Philosopher::getCurrentOrder() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return currentOrder;
}

void Philosopher::markOrderTaken() {
    {
        std::lock_guard<std::mutex> lock(waiterMutex);
        orderTaken = true;
    }
    waiterCondition.notify_one();
}

void Philosopher::markOrderStart(double cookTimeSeconds) {
    orderStartTime = std::chrono::steady_clock::now();
    currentDishCookTime = cookTimeSeconds;
}
