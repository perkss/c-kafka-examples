//
// Created by Stuart Perks on 30/11/2020.
//
#include <stdio.h>
#include <pthread.h>
#include <librdkafka/rdkafka.h>

#include "consumer.h"
#include "UppercaseTopology.h"

int runTopology(int argc, char **argv) {
    rd_kafka_t *rk;          /* Consumer instance handle */
    rd_kafka_conf_t *conf;   /* Temporary configuration object */
    rd_kafka_resp_err_t err; /* librdkafka API error code */
    char errstr[512];        /* librdkafka API error reporting buffer */
    const char *brokers;     /* Argument: broker list */
    const char *groupid;     /* Argument: Consumer group id */
    char **topics;           /* Argument: list of topics to subscribe to */
    int topic_cnt;           /* Number of topics to subscribe to */


    /*
     * Argument validation
     */
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


    /*
     * Create Kafka client configuration place-holder
     */
    conf = rd_kafka_conf_new();

    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster. */
    if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers,
                          errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(conf);
        return 1;
    }

    /* Set the consumer group id.
     * All consumers sharing the same group id will join the same
     * group, and the subscribed topic' partitions will be assigned
     * according to the partition.assignment.strategy
     * (consumer config property) to the consumers in the group. */
    if (rd_kafka_conf_set(conf, "group.id", groupid,
                          errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(conf);
        return 1;
    }

    /* If there is no previously committed offset for a partition
     * the auto.offset.reset strategy will be used to decide where
     * in the partition to start fetching messages.
     * By setting this to earliest the consumer will read all messages
     * in the partition if there was no previously committed offset. */
    if (rd_kafka_conf_set(conf, "auto.offset.reset", "earliest",
                          errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(conf);
        return 1;
    }

    /*
     * Create consumer instance.
     *
     * NOTE: rd_kafka_new() takes ownership of the conf object
     *       and the application must not reference it again after
     *       this call.
     */

    int result = createConsumer(&rk, conf, errstr);

    if (result != 0) {
        return result;
    }

    conf = NULL; /* Configuration object is now owned, and freed,
                      * by the rd_kafka_t instance. */

    subscribe(&rk, topic_cnt, topics);

    // Define the consumer thread ID
    pthread_t consumer_thread_id;

    pthread_create(&consumer_thread_id, NULL, consume(&rk), NULL );

    printf("Created thread for consuming");

    pthread_join(consumer_thread_id, NULL);

    printf("Joined thread for consuming");

    cleanUp(&rk);

    return 0;
}