#ifndef AUTHORIZATION_MQTT_HPP
#define AUTHORIZATION_MQTT_HPP


#include <mosquittopp.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <functional>
#include <thread>
#include <vector>
#include <uuid/uuid.h>
#include <sstream>

#ifdef DEBUG
#include <iostream>
#endif

#define DEFAULT_KEEP_ALIVE 60

class mqtt_client : public mosqpp::mosquittopp
{
    const std::function<void(std::string const &)> onMessage;
    int rc;

public:
    mqtt_client(std::string const &, std::string const &, int, std::function<void(std::string const &)> const &);

    void on_connect(int rc) final;

    void on_subscribe(int mid, int qos_count, const int *granted_qos) final;

    void on_message(const struct mosquitto_message *message) final;

    void loop_forever();
};

#endif //AUTHORIZATION_MQTT_HPP
