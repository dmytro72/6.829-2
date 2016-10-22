#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller(const bool debug) : debug_(debug) {}

/* Get current window size, in datagrams */
unsigned int Controller::window_size(void) {
    if (debug_) {
        cerr << "At time " << timestamp_ms() << " window size is "
             << window_size_ << endl;
    }

    return window_size_;
}

/* A datagram was sent */
void Controller::datagram_was_sent(const uint64_t sequence_number,
                                   /* of the sent datagram */
                                   const uint64_t send_timestamp)
/* in milliseconds */
{
    if (debug_) {
        cerr << "At time " << send_timestamp << " sent datagram "
             << sequence_number << endl;
    }
}

/* An ack was received */
void Controller::ack_received(
    const uint64_t sequence_number_acked,
    /* what sequence number was acknowledged */
    const uint64_t send_timestamp_acked,
    /* when the acked datagram was sent (sender's clock) */
    const uint64_t recv_timestamp_acked,
    /* when the acked datagram was received (receiver's clock)*/
    const uint64_t timestamp_ack_received)
/* when the ack was received (by sender) */
{
    if (debug_) {
        cerr << "At time " << timestamp_ack_received
             << " received ack for datagram " << sequence_number_acked
             << " (send @ time " << send_timestamp_acked << ", received @ time "
             << recv_timestamp_acked << " by receiver's clock)" << endl;
    }

    /* Multiplicative decrease: time between acks is too big; a packet was
     * probably dropped, and we timed out in the sender. */
    if (timestamp_ack_received > last_ack_timestamp_ + timeout_loss_ms_) {
        if (debug_) {
            cerr << "Timeout: halving window size." << endl;
        }
        last_ack_timestamp_ = timestamp_ack_received;
        window_size_ = max(window_size_ >> 1, (uint64_t) 1);
        acked_in_window_ = 0;
        return;
    }

    /* Additive increase: increase window size by 1 when received acks for
     * window_size_ packets */
    ++acked_in_window_;
    if (acked_in_window_ >= window_size_) {
        if (debug_) {
            cerr << "Additive increase: increasing window size by 1." << endl;
        }
        last_ack_timestamp_ = timestamp_ack_received;
        acked_in_window_ = 0;
        ++window_size_;
    }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms(void) {
    return timeout_loss_ms_; /* timeout of one second */
}
