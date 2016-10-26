#include <algorithm>
#include <cmath>
#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller(const bool debug) : debug_(debug) {}

double mean(const std::deque<int64_t>& hist) {
    double sum = 0.;
    for (int64_t elem : hist) {
        sum += elem;
    }
    return sum / hist.size();
}

double median(const std::deque<int64_t>& hist) {
    std::deque<int64_t> x = hist;
    std::sort(x.begin(), x.end());
    return x[x.size() / 2];
}

double variance(const std::deque<int64_t>& hist) {
    double mu = mean(hist);

    double sum = 0.;
    for (int64_t elem : hist) sum += (elem - mu) * (elem - mu);
    return sum / hist.size();
}

double stdev(const std::deque<int64_t>& hist) {
    return sqrt(variance(hist));
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size(void) {
    if (debug_) {
        // cerr << "At time " << timestamp_ms() << " window size is "
        //<< window_size_ << endl;
    }
    cout << link_speed_estimate_ << " " << mean(delta_history_) << " "
         << stdev(delta_history_) << endl;

    double mu = mean(delta_history_);
    double s = stdev(delta_history_);

    double aggressiveness;
    if (mu < 0) {
        aggressiveness = 0.9 + (0.00033 * s * s - 0.0116 * s + 0.08);
        if (s >= 10) aggressiveness = 0.9;
    } else {
        aggressiveness = 0.7 + (0.00067 * s * s - 0.023 * s + 0.18);
        if (s >= 10) aggressiveness = 0.7;
    }

    double model = 0.3;
    window_size_ = model * 65 * link_speed_estimate_ * aggressiveness +
                   (1 - model) * aimd_;
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

    /* Keep track of the history of RTTs */
    int64_t rtt = timestamp_ack_received - send_timestamp_acked;
    if (history_.size() > 0) delta_history_.push_back(rtt - history_.back());

    history_.push_back(rtt);

    /* Keep at most history_size_max in the history */
    if (history_.size() > history_size_max) history_.pop_front();

    if (delta_history_.size() > history_size_max) delta_history_.pop_front();

    /* Keep a separate AIMD number */
    cout << aimd_ << endl;
    if (rtt > rtt_thresh_ms_)
        aimd_ /= 1.02;
    else
        aimd_ += 3 / aimd_;

    estimate_link_speed(timestamp_ack_received, rtt / 2);
}

void Controller::timeout_callback() {
    if (debug_) {
        cerr << "Timeout occured (no-op)." << endl;
    }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms(void) {
    return timeout_loss_ms_; /* timeout of one second */
}
