#ifndef C_KAFKA_EXAMPLES_PRODUCER_H
#define C_KAFKA_EXAMPLES_PRODUCER_H
#include <librdkafka/rdkafka.h>

int create_producer(rd_kafka_t **rk, rd_kafka_conf_t *conf, char *errstr);
void *produce(rd_kafka_t **rk, char* topic, char* key, char* msg);

#endif //C_KAFKA_EXAMPLES_PRODUCER_H
