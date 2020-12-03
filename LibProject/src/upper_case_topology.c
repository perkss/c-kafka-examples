#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <librdkafka/rdkafka.h>
#include <ctype.h>

#include "consumer.h"
#include "producer.h"
#include "upper_case_topology.h"

// TODO pass as reference pointer
int process_message_and_send(message_t **param, rd_kafka_t **producer) {
    message_t *msg = *param;

    char *s = msg->payload;
    while (*s) {
        *s = toupper((char) *s);
        s++;
    }

    printf(" Value: %.*s\n",
           (int) msg->payload_length, (const char *) msg->payload);

    produce(producer, "uppercase-topic", msg->key, msg->payload);

    free(msg->key);
    free(msg);

    return 0;
}

static int set_conf_property(rd_kafka_conf_t **conf, const char *propertyName, const char **property) {
    char errstr[512];
    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster. */
    if (rd_kafka_conf_set(*conf, propertyName, *property,
                          errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(*conf);
        return 1;
    } else {
        return 0;
    }
}

int run_uppercase_topology(int argc, char **argv) {
    char errstr[512];        /* librdkafka API error reporting buffer */
    char producer_errstr[512];        /* librdkafka API error reporting buffer */
    const char *brokers;     /* Argument: broker list */
    const char *groupid;     /* Argument: Consumer group id */
    char **topics;           /* Argument: list of topics to subscribe to */
    int topic_cnt;           /* Number of topics to subscribe to */

    if (argc < 4) {
        fprintf(stderr,
                "%% Usage: "
                "%s <broker> <group.id> <topic1> <topic2>..\n",
                argv[0]);
        return 1;
    }

    brokers = argv[1];
    groupid = argv[2];
    topics = &argv[3];
    topic_cnt = argc - 3;

    rd_kafka_conf_t *consumer_conf = rd_kafka_conf_new();
    rd_kafka_conf_t *producer_conf = rd_kafka_conf_new();

    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster. */
    set_conf_property(&consumer_conf, "bootstrap.servers", &brokers);
    set_conf_property(&producer_conf, "bootstrap.servers", &brokers);
    set_conf_property(&consumer_conf, "group.id", &groupid);
    const char *offset = "earliest";
    set_conf_property(&consumer_conf, "auto.offset.reset", &offset);


    rd_kafka_t *rk_consumer;          /* Consumer instance handle */
    rd_kafka_t *rk_producer;          /* Producer instance handle */
    int result = create_consumer(&rk_consumer, consumer_conf, errstr);
    if (result != 0) {
        return result;
    }

    // TODO add topic to producer too
    int producerCreationResult = create_producer(&rk_producer, producer_conf, producer_errstr);

    if (producerCreationResult != 0) {
        return result;
    }

    // TODO create struct to hold a producer and required fields, pass that into function that can call callback.
    //  produce()
    /* Configuration object is now owned, and freed, by the rd_kafka_t instance. */
    consumer_conf = NULL;
    producer_conf = NULL;

    subscribe(&rk_consumer, topic_cnt, topics);

    // Define the consumer thread ID
    pthread_t consumer_thread_id;

    pthread_create(&consumer_thread_id, NULL, consume(&rk_consumer, process_message_and_send, &rk_producer), NULL);

    printf("Created thread for consuming");

    pthread_join(consumer_thread_id, NULL);

    printf("Joined thread for consuming");

    clean_up(&rk_consumer);

    return 0;
}
