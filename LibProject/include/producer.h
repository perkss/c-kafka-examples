#ifndef C_KAFKA_EXAMPLES_PRODUCER_H
#define C_KAFKA_EXAMPLES_PRODUCER_H
#include <librdkafka/rdkafka.h>

typedef struct producer_s {
    rd_kafka_t **rk;
    rd_kafka_conf_t *conf; // change to topic
    char errstr[512];
} producer;

int createProducer(rd_kafka_t **rk, rd_kafka_conf_t *conf, char errstr[512]);
void *produce(rd_kafka_t **rk, char* topic, char* key, char* msg);

#endif //C_KAFKA_EXAMPLES_PRODUCER_H
