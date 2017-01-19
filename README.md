Platform Calibration (for SimGrid)
==================================

This project gathers all efforts regarding platform calibration for HPC experiments with SimGrid's SMPI.

Calibration
-----------

The `calibrate` binary is an MPI application that introduces a bunch
of MPI experiments. The goal is to profile the amount of time taken to
execute some MPI operations in a real environment. The timings for
such operations are obtained using the most precise clock available in
the local node where the binary is executed. Measurements, along
with their corresponding parameters such as message size, are dumped
to multiple CSV files, one per operation. In a second stage, after the experiment has
finished, an R script takes as input those measurements and a break
points textual file informing the message sizes where the MPI protocol
changes. As output, the R script generates the values that need to be
included to the corresponding platform file, in order to obtain a
faithful simulation.

#### The Profiled Operations

* **Blocking Receive** (R): time spent inside the _MPI_Recv_
    function. During the benchmark initialization, through a ping-pong
    operation, the program measures how much time the sender must
    sleep between two benchmark procedures. Such sleep time is to be
    sure the message that has been sent has arrived in the receiver
    side.  After that, for a number of replications, the program
    measures how long it took to execute the blocking receive
    (assuming the message has already available). To guarantee all
    replications are finished correctly, at the end, a message is sent
    to the receiver side.

* **Asynchronous Send** (AS): time spent inside the _MPI_Isend_
    asynchronous operation. This measurement captures the time spent
    in the buffering data and sending it to the network card. For a
    number of replications, the program measures how long it took to
    execute this operation when changing the message size.

* **Ping-Pong** (PP): time spent to execute a sequence of a _MPI_Send_
    followed by a _MPI_Recv_, representing together a ping pong
    operation. The time spent in each of these two operations (send
    and receive) is registered separately. The time spent for the
    whole synchronous ping-pong can be obtained by summing these two
    durations.

* **Other operations** (OT): The network-unrelated functions
    _MPI_Test_, _MPI_Iprobe_ and _MPI_Wtime_ are also profiled during
    calibration since their CPU timings are neces

#### Network Modeling

The R and AS measurements (see above) represents the software overhead
of the receive and send operations. Since the R is measured when the
message is already available to be actually received, it is assumed
that its duration consists exclusively in the message handling. The
same is verified for the asynchronous send. Since the operation
returns to the caller as soon as everything has been setup for the
actual message transmission, we consider that it contains only the
software overhead related to sending a message. Therefore, these two
operations account for the software overhead, unrelated to network
modeling itself. The network latency and bandwidth are calculated
using the results of the PP measurements. Since the PP operation uses
_MPI_Send_, we can check coherence of its behavior against the AS
measurements (_MPI_Isend_).

#### The Message Sizes and Variability Check

Addressing variability requires that multiple runs for each
operation. The number of replications can be set through the parameter
`--nb_runs`. Message sizes are taken in the order established in an
input file whose name is passed as parameter, using the `--sizeFile`
parameter. The `zoo_sizes` file that is already in this repository
provides a comprehensive set of sizes, with a good repartition for all
magnitudes.  Regardless of the message sizes available in the input
file, we can filter them by passing `--min_size` followed by the
minimum size and `--max-size` for the maximum. Since sizes can go up
to 1GB in the furnished size file, the analyst can create a cut to
reduce experimental time.

#### CLI Help and Parameter Description

Call the `calibrate` binary passing the `--help` option. You'll get
something like this:

```
$ ./calibrate --help
Usage: calibrate [OPTION...]
Runs MPI point to point benchmarks to compute values used for SMPI calibration

  -d, --dir_name=dir_name    Name/path of the directory to save files into. No
                             trailing slashes.
  -f, --filename=FILENAME    XML filename
  -m, --min_size=MIN SIZE    Minimum size to process
  -M, --max_size=MAX SIZE    Maximum size to process
  -n, --nb_runs=NB RUNS      number of times you want to execute the run
  -p, --prefix=PREFIX        prefix of the csv files
  -s, --sizeFile=SIZEFILE    filename of the size list
  -?, --help                 Give this help list
      --usage                Give a short usage message

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```


Saturation
----------

Forthcoming.
