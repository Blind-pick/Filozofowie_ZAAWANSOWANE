#include "ConfigLoader.h"
#include "Philosopher.h"
#include "Waiter.h"
#include "Cook.h"
#include "Kitchen.h"
#include "display.h"

#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>

int main() {
    ConfigLoader loader;
    if (!loader.loadFromFile("config.yaml")) {
        std::cerr << "Błąd podczas wczytywania pliku konfiguracyjnego." << std::endl;
        return 1;
    }

    auto philosophersCfg = loader.getPhilosophers();
    auto cooksCfg = loader.getCooks();
    auto pantry = loader.getPantry();
    auto dishesCfg = loader.getDishes();
    int waiterCount = loader.getWaiterCount();

    // Tworzymy kuchnię
    auto kitchen = std::make_shared<Kitchen>();

    for (const auto& [name, amount] : pantry.ingredients)
        kitchen->addIngredient(name, amount);

    for (const auto& [type, amount] : pantry.cutlery)
        kitchen->addCutlery(type, amount);

    for (const auto& [dishName, info] : dishesCfg)
        kitchen->addDish(dishName, Kitchen::DishInfo{info.ingredient, info.cutlery, info.cookTimeMs, info.price});

    // Filozofowie
    std::vector<std::unique_ptr<Philosopher>> philosophers;
    for (const auto& ph : philosophersCfg) {
        auto philosopher = std::make_unique<Philosopher>(ph.id, ph.name, ph.favoriteDish, kitchen);
        philosophers.emplace_back(std::move(philosopher));
    }

    // Mapowanie filozofów dla kelnerów
    std::unordered_map<int, Philosopher*> philosopherMap;
    for (auto& p : philosophers)
        philosopherMap[p->getId()] = p.get();

    std::mutex philosopherMapMutex;

    int interval = rand() % 5000 + 5000; // 5–10 sekund
    int duration = rand() % 2000 + 1000; // 1–3 sekundy
    kitchen->runDishwasher(interval, duration);
    kitchen->startIngredientDelivery(20000); // co 20 sekund

    // Kelnerzy
    std::vector<std::unique_ptr<Waiter>> waiters;
    for (int i = 0; i < waiterCount; ++i) {
        auto waiter = std::make_unique<Waiter>(i);
        waiter->setKitchen(kitchen.get());
        waiter->setPhilosopherMap(philosopherMap, philosopherMapMutex);
        waiter->start();
        waiters.emplace_back(std::move(waiter));
    }

    // Kucharze
    std::vector<std::unique_ptr<Cook>> cooks;
    for (const auto& cookCfg : cooksCfg) {
        auto cook = std::make_unique<Cook>(cookCfg.id, cookCfg.specialtyDish, kitchen.get());
        cook->start();
        cooks.emplace_back(std::move(cook));
    }

    // Uruchom filozofów
    for (auto& philosopher : philosophers) {
        philosopher->start();
    }

    // Przekazanie do display
    std::vector<Philosopher*> philosopherPtrs;
    for (auto& p : philosophers) philosopherPtrs.push_back(p.get());

    std::vector<Waiter*> waiterPtrs;
    for (auto& w : waiters) waiterPtrs.push_back(w.get());

    std::vector<Cook*> cookPtrs;
    for (auto& c : cooks) cookPtrs.push_back(c.get());

    Display display(philosopherPtrs, waiterPtrs, cookPtrs, kitchen);
    display.start();

    std::this_thread::sleep_for(std::chrono::seconds(600));

    // Zatrzymanie wątków
    display.stop();

    for (auto& philosopher : philosophers)
        philosopher->stop();

    for (auto& waiter : waiters)
        waiter->stop();

    for (auto& cook : cooks)
        cook->join();

    return 0;
}
