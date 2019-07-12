#include <condition_variable>
#include <iostream>
#include <thread>
#include "mqtt.h"

// https://www.modernescpp.com/index.php/c-core-guidelines-be-aware-of-the-traps-of-condition-variables
// mosquitto_pub -t "topic" -m "NO"
// mosquitto_pub -t "topic" -m "OK"

using namespace std::chrono_literals;

int main()
{
    std::mutex mutex_;
    std::condition_variable condVar;
    bool dataReady{false};
    bool timeout{false};

    auto setDataReady = [&]()
    {
        mqtt_client client_res("bt_bluetooth", "localhost", 60000, [&](std::string const &message)
        {
            if ("OK" == message)
            {
                {
                    std::lock_guard<std::mutex> lck(mutex_);
                    dataReady = true;
                }
                std::cout << "Data prepared" << std::endl;
                condVar.notify_one();
            }
        });
        int sub_mid;
        std::string topic = "topic";
        client_res.subscribe(&sub_mid, topic.c_str());
        auto start = std::chrono::steady_clock::now();
        for (;;)
        {
            client_res.loop();
            {
                std::lock_guard<std::mutex> lck(mutex_);
                if (dataReady)
                {
                    break;
                }
                else if (std::chrono::steady_clock::now() > start + 10s)
                {
                    timeout = !timeout;
                    break;
                }
            }
        }
        condVar.notify_one();
    };

    std::thread t2(setDataReady);

    std::cout << "Waiting " << std::endl;
    std::unique_lock<std::mutex> lck(mutex_);
    condVar.wait(lck, [&]
    {
        return dataReady || timeout;
    });   // (4)
    if (timeout)
    {
        std::cout << "Timed Out " << std::endl;
        std::_Exit(EXIT_FAILURE);
    }
    std::cout << "Running " << std::endl;

    t2.join();

    return EXIT_SUCCESS;
}