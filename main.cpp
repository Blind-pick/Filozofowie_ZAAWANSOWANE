#include "ConfigLoader.h"
#include "Philosopher.h"
#include "Waiter.h"
#include "Cook.h"
#include "Kitchen.h"
#include "display.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>
#include <iomanip>

int main() {
    std::vector<std::string> configFiles = {
        "1_balanced.yaml",
        "2_few_cutlery.yaml",
        "3_few_waiters.yaml",
        "4_few_cooks.yaml",
        "5_less_ingredients.yaml",
        "6_same_dish.yaml"
    };

    std::cout << "Wybierz scenariusz testowy:\n";
    for (size_t i = 0; i < configFiles.size(); ++i) {
        std::cout << (i + 1) << ". " << configFiles[i] << "\n";
    }
    std::cout << "Twoj wybor (1-" << configFiles.size() << "): ";

    int choice = 0;
    std::cin >> choice;
    if (choice < 1 || choice > static_cast<int>(configFiles.size())) {
        std::cerr << "Nieprawidlowy wybor.\n";
        return 1;
    }

    std::string selectedFile = configFiles[choice - 1];
    std::string resultsFile = "wyniki_" + selectedFile + ".txt";

    std::cout << "Podaj liczbe powtorzen symulacji: ";
    int repeatCount = 1;
    std::cin >> repeatCount;

    for (int sim = 1; sim <= repeatCount; ++sim) {
        std::cout << "\nSymulacja nr " << sim << "...\n";

        ConfigLoader loader;
        if (!loader.loadFromFile(selectedFile)) {
            std::cerr << "Bląd podczas wczytywania pliku konfiguracyjnego." << std::endl;
            return 1;
        }

        auto philosophersCfg = loader.getPhilosophers();
        auto cooksCfg = loader.getCooks();
        auto pantry = loader.getPantry();
        auto dishesCfg = loader.getDishes();
        int waiterCount = loader.getWaiterCount();

        auto kitchen = std::make_shared<Kitchen>();
        for (const auto &[name, amount]: pantry.ingredients)
            kitchen->addIngredient(name, amount);
        for (const auto &[type, amount]: pantry.cutlery)
            kitchen->addCutlery(type, amount);
        for (const auto &[dishName, info]: dishesCfg)
            kitchen->addDish(dishName, Kitchen::DishInfo{info.ingredient, info.cutlery, info.cookTimeMs, info.price});

        std::vector<std::unique_ptr<Philosopher> > philosophers;
        for (const auto &ph: philosophersCfg) {
            auto philosopher = std::make_unique<Philosopher>(ph.id, ph.name, ph.favoriteDish, kitchen);
            philosophers.emplace_back(std::move(philosopher));
        }

        std::unordered_map<int, Philosopher *> philosopherMap;
        for (auto &p: philosophers)
            philosopherMap[p->getId()] = p.get();
        std::mutex philosopherMapMutex;

        kitchen->runDishwasher(6000, 1500);
        kitchen->startIngredientDelivery(20000);

        std::vector<std::unique_ptr<Waiter> > waiters;
        for (int i = 0; i < waiterCount; ++i) {
            auto waiter = std::make_unique<Waiter>(i);
            waiter->setKitchen(kitchen.get());
            waiter->setPhilosopherMap(philosopherMap, philosopherMapMutex);
            waiter->start();
            waiters.emplace_back(std::move(waiter));
        }

        std::vector<std::unique_ptr<Cook> > cooks;
        for (const auto &cookCfg: cooksCfg) {
            auto cook = std::make_unique<Cook>(cookCfg.id, cookCfg.specialtyDish, kitchen.get());
            cook->start();
            cooks.emplace_back(std::move(cook));
        }

        for (auto &philosopher: philosophers)
            philosopher->start();

        std::vector<Philosopher *> philosopherPtrs;
        for (auto &p: philosophers) philosopherPtrs.push_back(p.get());
        std::vector<Waiter *> waiterPtrs;
        for (auto &w: waiters) waiterPtrs.push_back(w.get());
        std::vector<Cook *> cookPtrs;
        for (auto &c: cooks) cookPtrs.push_back(c.get());

        Display display(philosopherPtrs, waiterPtrs, cookPtrs, kitchen);
        display.start();

        std::this_thread::sleep_for(std::chrono::seconds(600));

        display.stop();
        for (auto &philosopher: philosophers)
            philosopher->stop();
        for (auto &waiter: waiters)
            waiter->stop();
        for (auto &cook: cooks)
            cook->stop();
        kitchen->stopBackgroundTasks();

        // Zapis wyników
        std::ofstream out(resultsFile, std::ios::app);
        out << "Symulacja #" << sim << "\n";
        out << "-------------------------------------\n";

        long long totalWait = 0;
        for (auto &p: philosophers) {
            long long waitTime = p->getTotalExtraWaitTime();
            totalWait += waitTime;
            out << "Filozof " << p->getName() << " - czas oczekiwania: " << waitTime << " s\n";
        }

        double averageWait = philosophers.empty() ? 0.0 : static_cast<double>(totalWait) / philosophers.size();
        out << "Średni czas oczekiwania: " << averageWait << " s\n";

        double revenue = kitchen->getIncome();
        out << "Przychód restauracji: " << std::fixed << std::setprecision(2) << revenue << " zł\n\n";
        out.close();
    }

    std::cout << "\nZapisano wyniki do pliku: " << resultsFile << std::endl;
    return 0;
}
