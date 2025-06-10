#include <iostream>
#include <thread>

#include "Kitchen.h"

Kitchen::Kitchen() {
}

void Kitchen::addIngredient(const std::string &name, int amount) {
    std::lock_guard<std::mutex> lock(pantryMutex);
    pantry[name] += amount;
}

void Kitchen::addCutlery(const std::string &type, int amount) {
    std::lock_guard<std::mutex> lock(cutleryMutex);
    cutlery[type] += amount;
}

void Kitchen::addDish(const std::string &dishName, const DishInfo &info) {
    std::lock_guard<std::mutex> lock(menuMutex);
    menu[dishName] = info;
}

void Kitchen::addOrder(int philosopherId, const std::string &dishName) {
    std::lock_guard<std::mutex> lock(orderQueueMutex);
    orderQueue.emplace(Order{philosopherId, dishName});
}

std::optional<Kitchen::Order> Kitchen::getNextOrder() {
    std::lock_guard<std::mutex> lock(orderQueueMutex);
    if (orderQueue.empty()) return std::nullopt;

    auto next = orderQueue.front();
    orderQueue.pop();
    return next;
}

void Kitchen::markDishReady(int philosopherId, const std::string &dishName) {
    std::lock_guard<std::mutex> lock(readyQueueMutex);
    readyDishes.emplace(philosopherId, dishName);
}

bool Kitchen::hasReadyDish() {
    std::lock_guard<std::mutex> lock(readyQueueMutex);
    return !readyDishes.empty();
}

std::pair<int, std::string> Kitchen::getReadyDish() {
    std::lock_guard<std::mutex> lock(readyQueueMutex);
    auto dish = readyDishes.front();
    readyDishes.pop();
    return dish;
}

int Kitchen::getCookingTime(const std::string &dishName) {
    std::lock_guard<std::mutex> lock(menuMutex);
    if (!menu.count(dishName)) return 2000; // default 2s
    return menu.at(dishName).cookTimeMs;
}

const std::unordered_map<std::string, Kitchen::DishInfo> &Kitchen::getMenu() const {
    return menu;
}

const std::unordered_map<std::string, int> &Kitchen::getPantry() const {
    return pantry;
}

const std::unordered_map<std::string, int> &Kitchen::getCutleryStock() const {
    return cutlery;
}

bool Kitchen::canPrepare(const std::string &dishName) {
    std::lock_guard<std::mutex> lock1(pantryMutex);
    std::lock_guard<std::mutex> lock2(cutleryMutex);
    std::lock_guard<std::mutex> lock3(menuMutex);

    if (!menu.count(dishName)) return false;

    const auto &dish = menu.at(dishName);
    return pantry[dish.ingredient] > 0 && cutlery[dish.cutlery] > 0;
}

bool Kitchen::reserveResourcesFor(const std::string &dishName) {
    std::lock_guard<std::mutex> lock1(pantryMutex);
    std::lock_guard<std::mutex> lock2(cutleryMutex);
    std::lock_guard<std::mutex> lock3(menuMutex);

    if (!menu.count(dishName)) return false;

    auto &dish = menu.at(dishName);
    if (pantry[dish.ingredient] <= 0 || cutlery[dish.cutlery] <= 0) return false;

    pantry[dish.ingredient]--;
    cutlery[dish.cutlery]--;
    return true;
}

void Kitchen::addIncome(double amount) {
    std::lock_guard<std::mutex> lock(incomeMutex);
    income += amount;
}

double Kitchen::getIncome() {
    std::lock_guard<std::mutex> lock(incomeMutex);
    return income;
}

void Kitchen::returnUsedCutlery(const std::string &type) {
    std::lock_guard<std::mutex> lock(dirtyMutex);
    dirtyCutlery[type]++;
}

void Kitchen::runDishwasher(int intervalMs, int durationMs) {
    dishwasherThread = std::thread([=]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
            if (!running) break; {
                std::lock_guard<std::mutex> dirtyLock(dirtyMutex);
                std::lock_guard<std::mutex> cleanLock(cutleryMutex);
                for (auto &pair: dirtyCutlery) {
                    cutlery[pair.first] += pair.second;
                }
                dirtyCutlery.clear();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
        }
    });
}

void Kitchen::startIngredientDelivery(int intervalMs) {
    deliveryThread = std::thread([this, intervalMs]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
            if (!running) break;

            std::lock_guard<std::mutex> lock(pantryMutex);
            std::lock_guard<std::mutex> deliveryLock(deliveryMutex);

            std::cout << "[DELIVERY] Food delivery has arrived\n";

            for (auto &[ingredient, amount]: pantry) {
                int &plannedAmount = deliveryPlan[ingredient];
                if (plannedAmount == 0) plannedAmount = 5;
                if (amount == 0) {
                    plannedAmount += 2;
                } else if (amount > plannedAmount) {
                    plannedAmount = std::max(1, plannedAmount - 1);
                }
                pantry[ingredient] += plannedAmount;
            }
        }
    });
}

void Kitchen::stopBackgroundTasks() {
    running = false;
    if (dishwasherThread.joinable()) dishwasherThread.join();
    if (deliveryThread.joinable()) deliveryThread.join();
}
