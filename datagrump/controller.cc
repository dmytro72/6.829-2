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

int clamp(int n, int min, int max) {
    if (n < min)
        return min;
    else if (n > max)
        return max;
    else
        return n;
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

    uint64_t measured_rtt = timestamp_ack_received - send_timestamp_acked;
    avg_rtt_ = alpha_ * avg_rtt_ + (1 - alpha_) * measured_rtt;

    float P = 0.02;
    float D = 0.2;
    float I = 0.000001;
    int64_t error = rtt_thresh_ms_ - avg_rtt_;
    float derivative = error - last_error_;

    int64_t delta = error * P + D * derivative + I * integral_;

    last_error_ = error;
    integral_ += error;

    if (debug_) {
        cout << "Measured "  << measured_rtt << " Error " << error <<
            " Weighted error " << error * P << endl;
        cout << "Derivative "  << derivative << " Weighted derivative " << derivative * D << endl;
        cout << "Integral "  << integral_ << " Weighted integral " << integral_ * I << endl;
    }

    // We clamp the delta to prevent window_size_ from diverging
    window_size_ += clamp(delta, -10, 10);
    window_size_ = max(window_size_ , (int64_t) 1);
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms(void) {
    return timeout_loss_ms_; /* timeout of one second */
}
