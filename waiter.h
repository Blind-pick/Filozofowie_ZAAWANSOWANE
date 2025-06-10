#ifndef WAITER_H
#define WAITER_H

#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>

class Kitchen;
class Philosopher;

class Waiter {
public:
    enum class State { Free, Busy };

    Waiter(int id);

    ~Waiter();

    void start();

    void stop();

    void setKitchen(Kitchen *kitchen);

    void setPhilosopherMap(std::unordered_map<int, Philosopher *> &map, std::mutex &mutex);

    State getState() const;

    int getServingPhilosopherId() const;

    int getId() const { return id; }

    int id;
    std::atomic<State> state;
    std::atomic<int> servingPhilosopherId;
    std::atomic<bool> running;

    std::thread thread;

    Kitchen *kitchen = nullptr;
    std::unordered_map<int, Philosopher *> philosopherMap;
    std::mutex *philosopherMapMutex = nullptr;

    void lifeCycle();

    void deliverOrderToKitchen(int philosopherId, const std::string &dish);
};

#endif // WAITER_H
