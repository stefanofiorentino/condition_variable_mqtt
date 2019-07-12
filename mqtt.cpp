#include "mqtt.h"

mqtt_client::mqtt_client(std::string const &id, std::string const &host, int port,
std::function<void(std::string const &)> const &onMessage) :
mosquittopp(id.c_str()), onMessage(onMessage), rc(0) {
    int keepalive = DEFAULT_KEEP_ALIVE;
    connect(host.c_str(), port, keepalive);
}

void mqtt_client::on_connect(int rc) {
    if (!rc) {
#ifdef DEBUG
        std::cout << "Connected - code " << rc << std::endl;
#endif
    }
}

void mqtt_client::on_subscribe(int mid, int qos_count, const int *granted_qos) {
#ifdef DEBUG
    std::cout << "Subscription succeeded." << std::endl;
#endif
}

void mqtt_client::on_message(const struct mosquitto_message *message) {
    std::string std_message{static_cast<const char*>(message->payload)};
    // call the callback
    if (onMessage) {
        onMessage(std_message);
    }
}


void mqtt_client::loop_forever() {
    ::mosqpp::mosquittopp::loop_forever();
}