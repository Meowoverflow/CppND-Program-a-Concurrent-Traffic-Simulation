#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::unique_lock<std::mutex> uLock(_msgQMtx);
    _msgQCondVar.wait(uLock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.back());
    _queue.pop_back();
    _queue.clear();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::lock_guard<std::mutex> lck(_msgQMtx);
    _queue.push_back(std::move(msg));
    _msgQCondVar.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight() : _currentPhase(TrafficLightPhase::red){ }

void TrafficLight::waitForGreen()
{
    const long cycleDuration = 1; //1 ms
    auto thisMoment = std::chrono::system_clock::now();
    while (true)
    {
        auto msg = _msgQ.receive();
        if ( msg == TrafficLightPhase::green)
            return;

    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases , this));
}

void TrafficLight::toggleLight()
{
    std::lock_guard<std::mutex> lck(_mutex);
    if(_currentPhase == TrafficLightPhase::red)
        _currentPhase = TrafficLightPhase::green;
    else
        _currentPhase = TrafficLightPhase::red;
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    long cycleDuration = std::rand()%(6000 - 4000 + 1 ) + 4000; //random value between 4000 to 6000ms
    auto thisMoment = std::chrono::system_clock::now();
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - thisMoment).count() >= cycleDuration)
        {
            toggleLight();
            _msgQ.send(std::move(_currentPhase));
            thisMoment = std::chrono::system_clock::now();
            cycleDuration = std::rand()%(6000 - 4000 + 1 ) + 4000;//another random value between 4000 to 6000ms
        }

    }
}


