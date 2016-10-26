#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>

/* Congestion controller interface */

class Controller {
   private:
    const uint64_t timeout_loss_ms_ = 100; /* Timeout */
    const double add_inc_ = 1.0;           /* Additive increase constant */
    const double mult_dec_ = 4.0; /* Multiplicative decrease constant */

    bool debug_;                   /* Enables debugging output */
    double window_size_ = 1.0;     /* Window size */
    uint64_t acked_in_window_ = 0; /* Number of ack'ed packets in this window */
    uint64_t last_ack_timestamp_ =
        0; /* Timestamp of the last ack'ed datagram */

   public:
    /* Public interface for the congestion controller */

    /* Default constructor */
    Controller(const bool debug);

    /* Get current window size, in datagrams */
    unsigned int window_size(void);

    /* To be called when a timeout occurs. */
    void timeout_callback();

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
