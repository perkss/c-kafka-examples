#ifndef C_KAFKA_EXAMPLES_CONSUMER_H
#define C_KAFKA_EXAMPLES_CONSUMER_H
#include <librdkafka/rdkafka.h>

/* Has to be defined here due to parameter used*/
typedef struct message_s {
    size_t key_length;
    char* key;
    size_t payload_length;
    char* payload;
} message_t;

int createConsumer(rd_kafka_t **rk, rd_kafka_conf_t *conf, char errstr[512]);

int subscribe(rd_kafka_t **rk, int topic_cnt, char** topics);

void *consume(rd_kafka_t **rk, int (*cc)(message_t, rd_kafka_t**), rd_kafka_t **producer);

void cleanUp(rd_kafka_t **rk);

#endif //C_KAFKA_EXAMPLES_CONSUMER_H