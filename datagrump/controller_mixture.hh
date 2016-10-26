#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>
#include <deque>

/* Congestion controller interface */

class Controller {
   private:
    const uint64_t tick_duration_ms_ = 30; /* Duration of a tick, in ms */
    const uint64_t predictable_interval_ms_ =
        60; /* Duration of an interval in which we can predict the link speed,
               in ms */
    const int64_t rtt_thresh_ms_ = 80;

    bool debug_;                   /* Enables debugging output */
    double window_size_ = 1.0f;    /* Window size */
    uint64_t acked_in_window_ = 0; /* Number of packets ack'ed in this window */
    uint64_t timeout_loss_ms_ = 100; /* Timeout */
    uint64_t avg_rtt_ = 0;           /* Average measured RTT */
    uint64_t alpha_ = 0.8;           /* Weighting parameter */
    uint64_t current_tick_ = 0;      /* Start of current tick */
    uint64_t packets_in_tick_ = 0;   /* Count of packets in current tick */
    double link_speed_estimate_ = 0; /* Estimate of the link speed */
    double aimd_ = 1;

    uint64_t history_size_max = 20;
    std::deque<int64_t> history_{};
    std::deque<int64_t> delta_history_{};

    void estimate_link_speed(const uint64_t timestamp_ack_received,
                             const uint64_t delay);

   public:
    /* Public interface for the congestion controller */

    /* Default constructor */
    Controller(const bool debug);

    /* To be called when a timeout occurs. */
    void timeout_callback();

    /* Get current window size, in datagrams */
    unsigned int window_size(void);

    /* A datagram was sent */
    void datagram_was_sent(const uint64_t sequence_number,
                           const uint64_t send_timestamp);

    /* An ack was received */
    void ack_received(const uint64_t sequence_number_acked,
                      const uint64_t send_timestamp_acked,
                      const uint64_t recv_timestamp_acked,
                      const uint64_t timestamp_ack_received);

    /* How long to wait (in milliseconds) if there are no acks
       before sending one more datagram */
    unsigned int timeout_ms(void);
};

#endif
