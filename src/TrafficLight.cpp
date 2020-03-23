#include <iostream>
#include <random>
#include <future>
#include <memory>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
	std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this]{ return !_queue.empty(); });

    T msg = std::move(_queue.back());
    _queue.pop_back();
    
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.clear();
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _messages = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    while (true) {
        TrafficLightPhase message = _messages->receive();
        if (message == green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::mt19937_64 eng{std::random_device{}()};
    std::uniform_int_distribution<> dist{4000, 6000};
    auto cycleDuration = std::chrono::milliseconds(dist(eng));

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(cycleDuration)+std::chrono::milliseconds(1));
        _currentPhase == green ? _currentPhase = red : _currentPhase = green;
        std::async(&MessageQueue<TrafficLightPhase>::send, _messages, std::move(_currentPhase)); 
    }
}
