#include <signal.h>
#include <string.h>
#include <ctype.h>

#include "consumer.h"

int createConsumer(rd_kafka_t **rk, rd_kafka_conf_t *conf, char errstr[512]) {
    /* Consumer instance handle */
    *rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, strlen(errstr));
    if (!*rk) {
        fprintf(stderr,
                "%% Failed to create new consumer: %s\n", errstr);
        return 1;
    } else {
        printf("Successfully created consumer \n");
        return 0;
    }
}

int subscribe(rd_kafka_t **rk, int topic_cnt, char **topics) {
    /* Redirect all messages from per-partition queues to
    * the main queue so that messages can be consumed with one
    * call from all assigned partitions.
    *
    * The alternative is to poll the main queue (for events)
    * and each partition queue separately, which requires setting
    * up a rebalance callback and keeping track of the assignment:
    * but that is more complex and typically not recommended. */
    rd_kafka_poll_set_consumer(*rk);

    rd_kafka_topic_partition_list_t *subscription; /* Subscribed topics */
    /* Convert the list of topics to a format suitable for librdkafka */
    subscription = rd_kafka_topic_partition_list_new(topic_cnt);
    int i;
    for (i = 0; i < topic_cnt; i++)
        rd_kafka_topic_partition_list_add(subscription,
                                          topics[i],
                /* the partition is ignored
                 * by subscribe() */
                                          RD_KAFKA_PARTITION_UA);

    rd_kafka_resp_err_t err; /* librdkafka API error code */
    /* Subscribe to the list of topics */
    err = rd_kafka_subscribe(*rk, subscription);
    if (err) {
        fprintf(stderr,
                "%% Failed to subscribe to %d topics: %s\n",
                subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(*rk);
        return 1;
    }

    fprintf(stderr,
            "%% Subscribed to %d topic(s), "
            "waiting for rebalance and messages...\n",
            subscription->cnt);

    rd_kafka_topic_partition_list_destroy(subscription);
    return 0;
}

static volatile sig_atomic_t run = 1;

/**
 * @brief Signal termination of program
 */
static void stop(int sig) {
    run = 0;
}


/**
 * @returns 1 if all bytes are printable, else 0.
 */
static int is_printable(const char *buf, size_t size) {
    size_t i;

    for (i = 0; i < size; i++)
        if (!isprint((int) buf[i]))
            return 0;

    return 1;
}

void *consume(rd_kafka_t **rk) {
    /* Subscribing to topics will trigger a group rebalance
 * which may take some time to finish, but there is no need
 * for the application to handle this idle period in a special way
 * since a rebalance may happen at any time.
 * Start polling for messages. */
    while (run) {
        rd_kafka_message_t *rkm;

        rkm = rd_kafka_consumer_poll(*rk, 100);
        if (!rkm)
            continue; /* Timeout: no message within 100ms,
                                   *  try again. This short timeout allows
                                   *  checking for `run` at frequent intervals.
                                   */

        /* consumer_poll() will return either a proper message
         * or a consumer error (rkm->err is set). */
        if (rkm->err) {
            /* Consumer errors are generally to be considered
             * informational as the consumer will automatically
             * try to recover from all types of errors. */
            fprintf(stderr,
                    "%% Consumer error: %s\n",
                    rd_kafka_message_errstr(rkm));
            rd_kafka_message_destroy(rkm);
            continue;
        }

        /* Proper message. */
        printf("Message on %s [%"PRId32"] at offset %"PRId64":\n",
               rd_kafka_topic_name(rkm->rkt), rkm->partition,
               rkm->offset);

        /* Print the message key. */
        if (rkm->key && is_printable(rkm->key, rkm->key_len))
            printf(" Key: %.*s\n",
                   (int) rkm->key_len, (const char *) rkm->key);
        else if (rkm->key)
            printf(" Key: (%d bytes)\n", (int) rkm->key_len);

        /* Print the message value/payload. */
        if (rkm->payload && is_printable(rkm->payload, rkm->len))
            printf(" Value: %.*s\n",
                   (int) rkm->len, (const char *) rkm->payload);
        else if (rkm->payload)
            printf(" Value: (%d bytes)\n", (int) rkm->len);

        rd_kafka_message_destroy(rkm);
    }

    return NULL;
}

void cleanUp(rd_kafka_t **rk) {
    /* Signal handler for clean shutdown */
    signal(SIGINT, stop);

    /* Close the consumer: commit final offsets and leave the group. */
    fprintf(stderr, "%% Closing consumer\n");
    rd_kafka_consumer_close(*rk);

    /* Destroy the consumer */
    rd_kafka_destroy(*rk);
}