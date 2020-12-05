#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

#include "consumer.h"

int create_consumer(rd_kafka_t **rk, rd_kafka_conf_t *conf, char *errstr) {
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
    rd_kafka_poll_set_consumer(*rk);

    rd_kafka_topic_partition_list_t *subscription; /* Subscribed topics */

    subscription = rd_kafka_topic_partition_list_new(topic_cnt);
    int i;
    for (i = 0; i < topic_cnt; i++)
        rd_kafka_topic_partition_list_add(subscription,
                                          topics[i],
                                          RD_KAFKA_PARTITION_UA);

    rd_kafka_resp_err_t err;

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

static void stop(int sig) {
    run = 0;
}

void free_message(message_t *message) {
    free(message->key);
    free(message);
}

static char *extract_key(const rd_kafka_message_t *rkm) {
    char *key = malloc(rkm->key_len);
    strncpy(key, rkm->key, rkm->key_len);
    return key;
}

static message_t *create_message(const rd_kafka_message_t *rkm) {
    message_t *message = (message_t *) malloc(sizeof(struct message_s));
    message->key = extract_key(rkm);
    message->key_length = rkm->key_len;
    message->payload = rkm->payload;
    message->payload_length = rkm->len;
    return message;
}

void *consume(rd_kafka_t **rk, int (*process_message)(message_t **, rd_kafka_t **), rd_kafka_t **producer) {
    while (run) {
        rd_kafka_message_t *rkm;

        rkm = rd_kafka_consumer_poll(*rk, 100);
        if (!rkm)
            continue;

        if (rkm->err) {
            fprintf(stderr,
                    "%% Consumer error: %s\n",
                    rd_kafka_message_errstr(rkm));
            rd_kafka_message_destroy(rkm);
            continue;
        }

        printf("Message on %s [%"PRId32"] at offset %"PRId64":\n",
               rd_kafka_topic_name(rkm->rkt), rkm->partition,
               rkm->offset);

        message_t *message = create_message(rkm);

        process_message(&message, producer);

        rd_kafka_message_destroy(rkm);
    }

    return NULL;
}

void clean_up(rd_kafka_t **rk) {
    /* Signal handler for clean shutdown */
    signal(SIGINT, stop);

    /* Close the consumer: commit final offsets and leave the group. */
    fprintf(stderr, "%% Closing consumer\n");
    rd_kafka_consumer_close(*rk);

    /* Destroy the consumer */
    rd_kafka_destroy(*rk);
}