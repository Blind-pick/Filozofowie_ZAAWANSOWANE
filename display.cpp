#include "Display.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

Display::Display(const std::vector<Philosopher*>& philosophers,
                 const std::vector<Waiter*>& waiters,
                 const std::vector<Cook*>& cooks,
                 std::shared_ptr<Kitchen> kitchen)
    : philosophers(philosophers), waiters(waiters), cooks(cooks), kitchen(kitchen) {}

void Display::start() {
    running = true;
    displayThread = std::thread(&Display::displayLoop, this);
}

void Display::stop() {
    running = false;
    if (displayThread.joinable()) displayThread.join();
}

void Display::displayLoop() {
    while (running) {
        show();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void Display::show() {
    std::cout << std::left;

    // === Filozofowie ===
    std::cout << "===== Philosophers =====\n";
    std::cout << std::setw(5) << "ID"
              << std::setw(15) << "Name"
              << std::setw(12) << "State"
              << std::setw(15) << "Dish"
              << std::setw(20) << "ExtraWaitTime(s)" << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (auto* p : philosophers) {
        std::string state;
        switch (p->getState()) {
            case Philosopher::State::Thinking: state = "Thinking"; break;
            case Philosopher::State::Hungry: state = "Hungry"; break;
            case Philosopher::State::Ordering: state = "Ordering"; break;
            case Philosopher::State::Waiting: state = "Waiting"; break;
            case Philosopher::State::Eating: state = "Eating"; break;
            case Philosopher::State::Paying: state = "Paying"; break;
            default: state = "Unknown"; break;
        }
        std::cout << std::setw(5) << p->getId()
                  << std::setw(15) << p->getName()
                  << std::setw(12) << state
                  << std::setw(15) << p->getCurrentOrder()
                  << std::setw(20) << std::fixed << std::setprecision(2) << p->getTotalExtraWaitTime()
                  << "\n";
    }

    // === Kelnerzy ===
    std::cout << "\n===== Waiters =====\n";
    std::cout << std::setw(5) << "ID" << std::setw(12) << "State" << "\n";
    std::cout << std::string(20, '-') << "\n";
    for (auto* w : waiters) {
        std::string state = (w->getState() == Waiter::State::Free) ? "Free" : "Busy";
        std::cout << std::setw(5) << w->getId() << std::setw(12) << state << "\n";
    }

    // === Kucharze ===
    std::cout << "\n===== Cooks =====\n";
    std::cout << std::setw(5) << "ID" << std::setw(12) << "State" << "\n";
    std::cout << std::string(20, '-') << "\n";
    for (auto* c : cooks) {
        std::string state = (c->getState() == Cook::State::Free) ? "Free" : "Busy";
        std::cout << std::setw(5) << c->getId() << std::setw(12) << state << "\n";
    }

    // === Magazyn i sztućce ===
    std::cout << "\n===== Pantry & Cutlery =====\n";
    std::cout << std::setw(20) << "Ingredient" << std::setw(10) << "Amount"
              << std::setw(20) << "Cutlery" << std::setw(10) << "Amount" << "\n";
    std::cout << std::string(60, '-') << "\n";

    const auto& pantry = kitchen->getPantry();
    const auto& cutlery = kitchen->getCutleryStock();

    auto panIt = pantry.begin();
    auto cutIt = cutlery.begin();

    while (panIt != pantry.end() || cutIt != cutlery.end()) {
        if (panIt != pantry.end()) {
            std::cout << std::setw(20) << panIt->first << std::setw(10) << panIt->second;
            ++panIt;
        } else {
            std::cout << std::setw(30) << " ";
        }

        if (cutIt != cutlery.end()) {
            std::cout << std::setw(20) << cutIt->first << std::setw(10) << cutIt->second;
            ++cutIt;
        }
        std::cout << "\n";
    }

    // === Przychód restauracji ===
    std::cout << "\n===== Restaurant Income =====\n";
    std::cout << "Total Income: " << std::fixed << std::setprecision(2)
              << kitchen->getIncome() << " zl\n";

    std::cout << std::string(80, '-') << "\n";
    std::cout << std::flush;
}
