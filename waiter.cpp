#include "Waiter.h"
#include "Kitchen.h"
#include "Philosopher.h"
#include <chrono>
#include <iostream>
#include <cstdlib>

Waiter::Waiter(int id)
    : id(id), servingPhilosopherId(-1), state(State::Free), running(false) {}

Waiter::~Waiter() {
    stop();
    if (thread.joinable()) thread.join();
}

void Waiter::start() {
    running = true;
    thread = std::thread(&Waiter::lifeCycle, this);
}

void Waiter::stop() {
    running = false;
}

void Waiter::setKitchen(Kitchen* k) {
    kitchen = k;
}

void Waiter::setPhilosopherMap(std::unordered_map<int, Philosopher*>& map, std::mutex& mutex) {
    philosopherMap = map;
    philosopherMapMutex = &mutex;
}

void Waiter::deliverOrderToKitchen(int philosopherId, const std::string& dish) {
    if (kitchen) {
        kitchen->addOrder(philosopherId, dish);
    }
}

void Waiter::lifeCycle() {
    while (running) {
        bool wasBusy = false;

        // Obsługa filozofów czekających na kelnera
        {
            std::lock_guard<std::mutex> lock(*philosopherMapMutex);
            for (auto& [id, philosopher] : philosopherMap) {
                if (philosopher && philosopher->isWaitingToOrder()) {
                    std::string dish = philosopher->getCurrentOrder();

                    state = State::Busy;
                    servingPhilosopherId = id;

                    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 400 + 200));
                    deliverOrderToKitchen(id, dish);
                    philosopher->markOrderTaken();

                    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 400 + 200));

                    servingPhilosopherId = -1;
                    state = State::Free;

                    wasBusy = true;
                    break;
                }
            }
        }

        // Sprawdź gotowe dania
        if (kitchen && kitchen->hasReadyDish()) {
            auto readyOrder = kitchen->getReadyDish();
            state = State::Busy;
            servingPhilosopherId = readyOrder.first;
            wasBusy = true;

            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 400 + 200));

            {
                std::lock_guard<std::mutex> lock(*philosopherMapMutex);
                if (philosopherMap.count(readyOrder.first)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 500 + 300));
                    philosopherMap[readyOrder.first]->receiveFood();
                }
            }

            servingPhilosopherId = -1;
            state = State::Free;
        }

        if (!wasBusy) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    }
}

Waiter::State Waiter::getState() const {
    return state;
}

int Waiter::getServingPhilosopherId() const {
    return servingPhilosopherId;
}
