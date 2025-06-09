#include "ConfigLoader.h"
#include <yaml-cpp/yaml.h>
#include <iostream>

bool ConfigLoader::loadFromFile(const std::string& filename) {
    try {
        YAML::Node config = YAML::LoadFile(filename);

        // Filozofowie
        if (config["philosophers"]) {
            for (const auto& phNode : config["philosophers"]) {
                PhilosopherConfig ph;
                ph.id = phNode["id"].as<int>();
                ph.name = phNode["name"].as<std::string>();
                ph.favoriteDish = phNode["favoriteDish"].as<std::string>();
                philosophers.push_back(ph);
            }
        } else {
            std::cerr << "Brak sekcji 'philosophers' w pliku konfiguracyjnym\n";
            return false;
        }

        // Kelnerzy
        if (config["waiters"]) {
            if (config["waiters"]["count"]) {
                waiterCount = config["waiters"]["count"].as<int>();
            } else {
                std::cerr << "Brak 'count' w 'waiters'\n";
                return false;
            }
        } else {
            std::cerr << "Brak sekcji 'waiters' w pliku konfiguracyjnym\n";
            return false;
        }


        // Kucharze
        if (config["cooks"]) {
            for (const auto& cookNode : config["cooks"]) {
                CookConfig cook;
                cook.id = cookNode["id"].as<int>();
                cook.specialtyDish = cookNode["specialtyDish"].as<std::string>();
                cooks.push_back(cook);
            }
        } else {
            std::cerr << "Brak sekcji 'cooks' w pliku konfiguracyjnym\n";
            return false;
        }

        // Spiżarnia
        if (config["pantry"]) {
            if (config["pantry"]["ingredients"]) {
                for (const auto& ingr : config["pantry"]["ingredients"]) {
                    std::string name = ingr.first.as<std::string>();
                    int amount = ingr.second.as<int>();
                    pantry.ingredients[name] = amount;
                }
            }

            if (config["pantry"]["cutlery"]) {
                for (const auto& cut : config["pantry"]["cutlery"]) {
                    std::string type = cut.first.as<std::string>();
                    int amount = cut.second.as<int>();
                    pantry.cutlery[type] = amount;
                }
            }
        } else {
            std::cerr << "Brak sekcji 'pantry' w pliku konfiguracyjnym\n";
            return false;
        }

        // Dania
        if (config["dishes"]) {
            for (const auto& dishNode : config["dishes"]) {
                std::string dishName = dishNode.first.as<std::string>();
                const YAML::Node& details = dishNode.second;

                DishConfig dish;
                dish.ingredient = details["ingredient"].as<std::string>();
                dish.cutlery = details["cutlery"].as<std::string>();
                dish.cookTimeMs = details["cookTimeMs"].as<int>();
                dish.price = details["price"].as<double>();

                dishes[dishName] = dish;
            }
        } else {
            std::cerr << "Brak sekcji 'dishes' w pliku konfiguracyjnym\n";
            return false;
        }

    } catch (const YAML::BadFile& e) {
        std::cerr << "Nie można otworzyć pliku: " << e.what() << std::endl;
        return false;
    } catch (const YAML::Exception& e) {
        std::cerr << "Błąd podczas parsowania YAML: " << e.what() << std::endl;
        return false;
    }

    return true;
}

std::vector<PhilosopherConfig> ConfigLoader::getPhilosophers() const {
    return philosophers;
}

int ConfigLoader::getWaiterCount() const {
    return waiterCount;
}

std::vector<CookConfig> ConfigLoader::getCooks() const {
    return cooks;
}

PantryConfig ConfigLoader::getPantry() const {
    return pantry;
}

std::unordered_map<std::string, DishConfig> ConfigLoader::getDishes() const {
    return dishes;
}
