#ifndef C_KAFKA_EXAMPLES_CONSUMER_H
#define C_KAFKA_EXAMPLES_CONSUMER_H
#include <librdkafka/rdkafka.h>

int createConsumer(rd_kafka_t **rk, rd_kafka_conf_t *conf, char errstr[512]);

int subscribe(rd_kafka_t **rk, int topic_cnt, char** topics);

void *consume(rd_kafka_t **rk);

void cleanUp(rd_kafka_t **rk);

#endif //C_KAFKA_EXAMPLES_CONSUMER_H