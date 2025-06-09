#ifndef CONFIGLOADER_H
#define CONFIGLOADER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

struct PhilosopherConfig {
    int id;
    std::string name;
    std::string favoriteDish;
};

struct CookConfig {
    int id;
    std::string specialtyDish;
};

struct DishConfig {
    std::string ingredient;
    std::string cutlery;
    int cookTimeMs;
    double price;
};

struct PantryConfig {
    std::unordered_map<std::string, int> ingredients;
    std::unordered_map<std::string, int> cutlery;
};

class ConfigLoader {
public:
    ConfigLoader() = default;

    bool loadFromFile(const std::string& filename);

    // Gettery do pobierania wczytanych danych
    std::vector<PhilosopherConfig> getPhilosophers() const;
    int getWaiterCount() const;
    std::vector<CookConfig> getCooks() const;
    PantryConfig getPantry() const;
    std::unordered_map<std::string, DishConfig> getDishes() const;

private:
    std::vector<PhilosopherConfig> philosophers;
    int waiterCount = 0;
    std::vector<CookConfig> cooks;
    PantryConfig pantry;
    std::unordered_map<std::string, DishConfig> dishes;
};

#endif // CONFIGLOADER_H
