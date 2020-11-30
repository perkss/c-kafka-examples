//
// Created by Stuart Perks on 25/11/2020.
//

#ifndef C_KAFKA_EXAMPLES_PRODUCER_H
#define C_KAFKA_EXAMPLES_PRODUCER_H
#include <librdkafka/rdkafka.h>

int createProducer(rd_kafka_t **rk, rd_kafka_conf_t *conf, char errstr[512]);
void *produce(rd_kafka_t **rk, char* topic, char* msg);

#endif //C_KAFKA_EXAMPLES_PRODUCER_H
