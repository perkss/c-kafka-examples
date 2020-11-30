//
// Created by Stuart Perks on 25/11/2020.
//
#include <signal.h>
#include <string.h>

#include "producer.h"

static volatile sig_atomic_t run = 1;

/**
 * @brief Signal termination of program
 */
static void stop (int sig) {
    run = 0;
    fclose(stdin); /* abort fgets() */
}

int createProducer(rd_kafka_t **rk, rd_kafka_conf_t *conf, char errstr[512]) {
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

void *produce(rd_kafka_t **rk, char* topic, char* msg) {

    rd_kafka_resp_err_t err; /* librdkafka API error code */
// https://stackoverflow.com/questions/57052441/librdkafka-producer-to-take-a-message-from-a-function-and-produce-it-on-topic

    err = rd_kafka_producev(
            /* Producer handle */
            *rk,
            /* Topic name */
            RD_KAFKA_V_TOPIC(topic),
            /* Make a copy of the payload. */
            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
            /* Message value and length */
            RD_KAFKA_V_VALUE(msg, strlen(msg)),
            /* Per-Message opaque, provided in
             * delivery report callback as
             * msg_opaque. */
            RD_KAFKA_V_OPAQUE(NULL),
            /* End sentinel */
            RD_KAFKA_V_END);

    if (err) {
        /*
         * Failed to *enqueue* message for producing.
         */
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
          //TODO  goto retry;
        }
    } else {
        fprintf(stderr, "%% Enqueued message (%zd bytes) "
                        "for topic %s\n",
                strlen(msg), topic);
    }


    /* A producer application should continually serve
     * the delivery report queue by calling rd_kafka_poll()
     * at frequent intervals.
     * Either put the poll call in your main loop, or in a
     * dedicated thread, or call it after every
     * rd_kafka_produce() call.
     * Just make sure that rd_kafka_poll() is still called
     * during periods where you are not producing any messages
     * to make sure previously produced messages have their
     * delivery report callback served (and any other callbacks
     * you register). */
    rd_kafka_poll(*rk, 0/*non-blocking*/);

    return 0;
}

void cleanUp(rd_kafka_t **rk) {
    /* Wait for final messages to be delivered or fail.
         * rd_kafka_flush() is an abstraction over rd_kafka_poll() which
         * waits for all messages to be delivered. */
    fprintf(stderr, "%% Flushing final messages..\n");
    rd_kafka_flush(*rk, 10*1000 /* wait for max 10 seconds */);

    /* If the output queue is still not empty there is an issue
     * with producing messages to the clusters. */
    if (rd_kafka_outq_len(*rk) > 0)
        fprintf(stderr, "%% %d message(s) were not delivered\n",
                rd_kafka_outq_len(*rk));

    /* Destroy the producer instance */
    rd_kafka_destroy(*rk);
}
