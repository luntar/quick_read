# influx_python_reader

A lightweight Python example that queries data from InfluxDB 2.x using the Flux query API.

## Prerequisites

* Python 3.9+
* Access to an InfluxDB 2.x instance with read permissions

Install dependencies:

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Configuration

Set the following environment variables before running the script:

* `INFLUX_URL` – Base URL of the InfluxDB instance (e.g. `http://localhost:8086`).
* `INFLUX_TOKEN` – API token with read access to the bucket.
* `INFLUX_ORG` – InfluxDB organisation name.
* `INFLUX_BUCKET` – Bucket to query from.

## Run

```bash
export INFLUX_URL="http://localhost:8086"
export INFLUX_TOKEN="your-token"
export INFLUX_ORG="your-org"
export INFLUX_BUCKET="your-bucket"
python main.py
```

By default the script retrieves the last hour of `sensor_data` measurements and prints the results. Adjust the Flux query in `main.py` to suit your data shape.
