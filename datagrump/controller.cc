#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller(const bool debug) : debug_(debug) {}

/* Get current window size, in datagrams */
unsigned int Controller::window_size(void) {
    if (debug_) {
        // cerr << "At time " << timestamp_ms() << " window size is "
        //<< window_size_ << endl;
    }

    window_size_ = link_speed_estimate_ * predicable_interval_ms_;
    return (uint64_t)window_size_;
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

void Controller::estimate_link_speed(const uint64_t timestamp_ack_received,
                                     const uint64_t delay) {
    // New tick!
    if (timestamp_ack_received - current_tick_ > tick_duration_ms_) {
        cerr << "Packets in prev tick: " << packets_in_tick_ << endl;
        cerr << "Estimate link speed: " << link_speed_estimate_ << endl;
        double estimate_in_tick =
            max((double)packets_in_tick_ / tick_duration_ms_, 1.0 / delay);
        link_speed_estimate_ =
            alpha_ * link_speed_estimate_ + (1 - alpha_) * estimate_in_tick;
        cerr << "New estimate link speed: " << link_speed_estimate_ << endl;

        current_tick_ =
            timestamp_ack_received - timestamp_ack_received % tick_duration_ms_;
        packets_in_tick_ = 1;
    } else {
        ++packets_in_tick_;
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
    (void)sequence_number_acked;
    (void)send_timestamp_acked;
    (void)recv_timestamp_acked;
    if (debug_) {
         cerr << "At time " << timestamp_ack_received
        << " received ack for datagram " << sequence_number_acked
        << " (send @ time " << send_timestamp_acked << ", received @ time "
        << recv_timestamp_acked << " by receiver's clock)" << endl;
    }

    estimate_link_speed(timestamp_ack_received,
                        (timestamp_ack_received - send_timestamp_acked) / 2);
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms(void) {
    return timeout_loss_ms_; /* timeout of one second */
}
