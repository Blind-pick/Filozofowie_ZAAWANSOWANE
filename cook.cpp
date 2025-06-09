#include "cook.h"
#include <iostream>
#include <thread>
#include <chrono>

Cook::Cook(int id, const std::string& specialtyDish, Kitchen* kitchen)
    : id(id), specialtyDish(specialtyDish), kitchen(kitchen), state(State::Free) {}

void Cook::start() {
    thread = std::thread(&Cook::lifeCycle, this);
}

void Cook::join() {
    if (thread.joinable()) {
        thread.join();
    }
}

void Cook::lifeCycle() {
    while (true) {
        std::optional<Kitchen::Order> orderToProcess;

        // Próbujemy znaleźć możliwe do wykonania zamówienie
        for (int i = 0; i < 10; ++i) {  // sprawdzamy max 10 zamówień z kolejki
            auto maybeOrder = kitchen->getNextOrder();
            if (!maybeOrder) {
                break; // kolejka pusta
            }

            if (kitchen->canPrepare(maybeOrder->dishName)) {
                orderToProcess = maybeOrder; // mamy zamówienie do wykonania
                break;
            } else {
                // Odkładamy niewykonalne zamówienie na koniec
                kitchen->addOrder(maybeOrder->philosopherId, maybeOrder->dishName);
            }
        }

        if (orderToProcess) {
            state = State::Busy;
            cookOrder(orderToProcess->philosopherId, orderToProcess->dishName);
            state = State::Free;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}


void Cook::cookOrder(int philosopherId, const std::string& dishName) {
    if (!kitchen->reserveResourcesFor(dishName)) {
        std::cerr << "[COOK " << id << "] Failed to reserve resources for " << dishName << ", retrying later\n";

        // Odkładamy zamówienie z powrotem do kolejki
        kitchen->addOrder(philosopherId, dishName);

        // Czekamy chwilę, by nie zalać kolejki natychmiast
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return;
    }


    int baseTime = kitchen->getCookingTime(dishName);
    int cookingTime = (dishName == specialtyDish) ? static_cast<int>(baseTime * 0.6) : baseTime;

    std::cout << "[COOK " << id << "] Cooking " << dishName << " for philosopher " << philosopherId << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(cookingTime));
    kitchen->markDishReady(philosopherId, dishName);
    std::cout << "[COOK " << id << "] Finished " << dishName << " for philosopher " << philosopherId << "\n";
}

Cook::State Cook::getState() {
    return state;
}

int Cook::getId() const {
    return id;
}
