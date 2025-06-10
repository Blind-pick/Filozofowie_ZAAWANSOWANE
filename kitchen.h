#ifndef KITCHEN_H
#define KITCHEN_H

#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <optional>
#include <atomic>
#include <thread>

class Kitchen {
public:
    struct DishInfo {
        std::string ingredient;
        std::string cutlery;
        int cookTimeMs;
        double price;
    };

    struct Order {
        int philosopherId;
        std::string dishName;
    };

    Kitchen();

    void addIngredient(const std::string &name, int amount);

    void addCutlery(const std::string &type, int amount);

    void addDish(const std::string &dishName, const DishInfo &info);

    void addOrder(int philosopherId, const std::string &dishName);

    std::optional<Order> getNextOrder();

    void markDishReady(int philosopherId, const std::string &dishName);

    bool hasReadyDish();

    std::pair<int, std::string> getReadyDish();

    int getCookingTime(const std::string &dishName);

    const std::unordered_map<std::string, DishInfo> &getMenu() const;

    const std::unordered_map<std::string, int> &getPantry() const;

    const std::unordered_map<std::string, int> &getCutleryStock() const;

    bool canPrepare(const std::string &dishName);

    bool reserveResourcesFor(const std::string &dishName);

    void addIncome(double amount);

    double getIncome();

    void returnUsedCutlery(const std::string &type);

    void runDishwasher(int intervalMs, int durationMs);

    void startIngredientDelivery(int intervalMs);

    void stopBackgroundTasks(); // <-- nowa funkcja

private:
    std::unordered_map<std::string, DishInfo> menu;
    std::unordered_map<std::string, int> pantry;
    std::unordered_map<std::string, int> cutlery;
    std::unordered_map<std::string, int> dirtyCutlery;
    std::unordered_map<std::string, int> deliveryPlan;

    std::queue<Order> orderQueue;
    std::queue<std::pair<int, std::string> > readyDishes;

    std::mutex menuMutex, pantryMutex, cutleryMutex, dirtyMutex;
    std::mutex orderQueueMutex, readyQueueMutex;
    std::mutex deliveryMutex, incomeMutex;

    double income = 0.0;

    std::atomic<bool> running = true;
    std::thread dishwasherThread;
    std::thread deliveryThread;
};

#endif // KITCHEN_H
