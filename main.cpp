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
        for (auto it = 0; it < 300; ++it)
        {
            std::this_thread::sleep_for(100ms);
            client_res.loop();
            {
                std::lock_guard<std::mutex> lck(mutex_);
                if (dataReady)
                {
                    break;
                }
            }
        }
    };

    std::thread t2(setDataReady);

    std::cout << "Waiting " << std::endl;
    std::unique_lock<std::mutex> lck(mutex_);
    condVar.wait(lck, [&]
    {
        return dataReady;
    });   // (4)
    std::cout << "Running " << std::endl;

    t2.join();

    return EXIT_SUCCESS;
}