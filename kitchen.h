#ifndef KITCHEN_H
#define KITCHEN_H

#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <optional>

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

    void addIngredient(const std::string& name, int amount);
    void addCutlery(const std::string& type, int amount);
    void addDish(const std::string& dishName, const DishInfo& info);
    void addOrder(int philosopherId, const std::string& dishName);
    void addIncome(double amount);
    void returnUsedCutlery(const std::string& type);
    void runDishwasher(int intervalMs, int durationMs);
    void startIngredientDelivery(int intervalMs);
    double getIncome();

    std::optional<Order> getNextOrder();
    bool canPrepare(const std::string& dishName);
    bool reserveResourcesFor(const std::string& dishName);

    void markDishReady(int philosopherId, const std::string& dishName);
    bool hasReadyDish();
    std::pair<int, std::string> getReadyDish();

    int getCookingTime(const std::string& dishName);
    double income = 0.0;
    std::mutex incomeMutex;

    const std::unordered_map<std::string, DishInfo>& getMenu() const;
    const std::unordered_map<std::string, int>& getPantry() const;
    const std::unordered_map<std::string, int>& getCutleryStock() const;
    std::unordered_map<std::string, int> dirtyCutlery;
    std::mutex dirtyMutex;

    std::unordered_map<std::string, DishInfo> menu;
    std::unordered_map<std::string, int> pantry;
    std::unordered_map<std::string, int> cutlery;
    std::unordered_map<std::string, int> deliveryPlan;
    std::mutex deliveryMutex;


    std::queue<Order> orderQueue;
    std::queue<std::pair<int, std::string>> readyDishes;

    mutable std::mutex menuMutex;
    mutable std::mutex pantryMutex;
    mutable std::mutex cutleryMutex;
    mutable std::mutex orderQueueMutex;
    mutable std::mutex readyQueueMutex;
};

#endif // KITCHEN_H
