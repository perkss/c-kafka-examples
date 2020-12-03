#include <string.h>
#include "producer.h"

int create_producer(rd_kafka_t **rk, rd_kafka_conf_t *conf, char *errstr) {
    *rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!*rk) {
        fprintf(stderr,
                "%% Failed to create new producer: %s\n", errstr);
        return 1;
    } else {
        printf("Successfully created producer \n");
        return 0;
    }
}

void *produce(rd_kafka_t **rk, char *topic, char *key, char *msg) {
    rd_kafka_resp_err_t err;

    retry:
    err = rd_kafka_producev(
            /* Producer handle */
            *rk,
            /* Topic name */
            RD_KAFKA_V_TOPIC(topic),
            /* Make a copy of the payload. */
            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
            RD_KAFKA_V_KEY(key, strlen(key)),
            /* Message value and length */
            RD_KAFKA_V_VALUE(msg, strlen(msg)),
            /* Per-Message opaque, provided in
             * delivery report callback as
             * msg_opaque. */
            RD_KAFKA_V_OPAQUE(NULL),
            /* End sentinel */
            RD_KAFKA_V_END);

    if (err) {

        fprintf(stderr,
                "%% Failed to produce to topic %s: %s\n",
                topic, rd_kafka_err2str(err));

        if (err == RD_KAFKA_RESP_ERR__QUEUE_FULL) {
            /* If the internal queue is full, wait for
             * messages to be delivered and then retry.
             * The internal queue represents both
             * messages to be sent and messages that have
             * been sent or failed, awaiting their
             * delivery report callback to be called.
             *
             * The internal queue is limited by the
             * configuration property
             * queue.buffering.max.messages */
            rd_kafka_poll(*rk, 1000/*block for max 1000ms*/);
            goto retry;
        }
    } else {
        fprintf(stderr, "%% Enqueued message (%zd bytes) "
                        "for topic %s\n",
                strlen(msg), topic);
    }

    rd_kafka_poll(*rk, 0/*non-blocking*/);

    return 0;
}

void clean_up_producer(rd_kafka_t **rk) {
    fprintf(stderr, "%% Flushing final messages..\n");
    rd_kafka_flush(*rk, 10 * 1000 /* wait for max 10 seconds */);

    if (rd_kafka_outq_len(*rk) > 0)
        fprintf(stderr, "%% %d message(s) were not delivered\n",
                rd_kafka_outq_len(*rk));

    rd_kafka_destroy(*rk);
}
