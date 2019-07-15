#include <condition_variable>
#include <iostream>
#include <thread>
#include "mqtt.h"

// https://www.modernescpp.com/index.php/c-core-guidelines-be-aware-of-the-traps-of-condition-variables
// mosquitto_pub -t "topic" -m "NO"
// mosquitto_pub -t "topic" -m "OK"

void server_launcher();

void client_launcher(std::condition_variable &condVar, std::mutex &mutex_, bool &dataReady, bool &timeout);

using namespace std::chrono_literals;

int main()
{
    std::mutex mutex_;
    std::condition_variable condVar;
    bool dataReady{false};
    bool timeout{false};

    auto setDataReady = [&]()
    {
        client_launcher(condVar, mutex_, dataReady, timeout);
    };

    std::thread client_response_thread(setDataReady);

    std::cout << "Waiting " << std::endl;

    server_launcher();

    std::unique_lock<std::mutex> lck(mutex_);
    condVar.wait(lck, [&]
    {
        return dataReady || timeout;
    });
    if (timeout)
    {
        std::cout << "Timed Out " << std::endl;
        std::_Exit(EXIT_FAILURE);
    }
    std::cout << "Running " << std::endl;

    if (client_response_thread.joinable())
    {
        client_response_thread.join();
    }
    else
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void client_launcher(std::condition_variable &condVar, std::mutex &mutex_, bool &dataReady, bool &timeout)
{
    mqtt_client client_res("bt_bluetooth_client", "localhost", 60000, [&](std::string const &message)
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
}

void server_launcher()
{
    std::thread server_response_thread([]
                                       {
                                           mqtt_client server_res("bt_bluetooth_server", "localhost", 60000, nullptr);
                                           int sub_mid;
                                           std::string topic = "topic";
                                           auto const &message = "OK";
                                           std::this_thread::sleep_for(5s);
                                           server_res.publish(&sub_mid, topic.c_str(), strlen(message), message);
                                           server_res.loop();
                                       });

    server_response_thread.detach();
}