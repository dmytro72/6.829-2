# Files

On this directory, we have a few sets of files:

* ``controller.cc, controller.hh``: Files that will be used in compilation.
* ``controller_mixture.cc, controller_mixture.hh``: Files related to the mixture model approach.
* ``controller_interval.cc, controller_interval.hh``: Files related to the interval prediction approach.
* ``controller_pid.cc, controller_pid.hh``: Files related to the pid approach.
* ``controller_aimd.cc, controller_aimd.hh``: Files related to the TCP-like AIMD approach.
* ``controller_rtt.cc, controller_rtt.hh``: Files realted to the Delay-based approach.

# Running different schemes

To run a different scheme, simply overwrite the ``controller.cc, controller.hh`` files
with their corresponding scheme files as described above, and run ``make``.

# Report

The report can be found under ``datagrump/report.pdf``.

A summary of the results on the Verizon trace is below:

* **Mixture Model**: 4Mbits/s, 112ms, 33.03 Score
* **Interval Prediction**: 3.34Mbits/s, 128ms, 26.09 Score
* **Delay-based**: 4Mbits/s, 112ms, 35.71 Score
* **TCP-like AIMD**: 2.26Mbits/s, 99ms, 22.82 Score

On the TMobile trace, used to assess overfit, we obtained:

* **Delay-based**: 7.8Mbits/s, 194ms, 40.20 Score
* **Interval Prediction**: 11.94Mbits/s, 206ms, 57.96 Score
* **Mixture Model**: 9.4Mbits/s, 182ms, 51.64 Score
