# influx_cpp_writer

A minimal C++17 example that writes a single data point to an InfluxDB 2.x bucket using the REST API and libcurl.

## Prerequisites

* CMake 3.15+
* A C++17 compiler (e.g. GCC 9+, Clang 10+)
* libcurl development files
* Access to an InfluxDB 2.x instance and API token with write permissions

## Configuration

The executable reads its configuration from environment variables:

* `INFLUX_URL` – Base URL of the InfluxDB instance (e.g. `http://localhost:8086`).
* `INFLUX_TOKEN` – API token with write permissions to the target bucket.
* `INFLUX_ORG` – InfluxDB organisation name.
* `INFLUX_BUCKET` – Target bucket name.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

After building, set the environment variables and run the program:

```bash
export INFLUX_URL="http://localhost:8086"
export INFLUX_TOKEN="your-token"
export INFLUX_ORG="your-org"
export INFLUX_BUCKET="your-bucket"
./build/influx_cpp_writer
```

The program writes a single point (`sensor_data,host=example-host value=42.0`) using nanosecond precision timestamps. A `204` response from the server indicates success.
