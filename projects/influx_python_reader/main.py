"""Simple script to query data from InfluxDB 2.x using the Flux query API."""
from __future__ import annotations

import csv
import os
from dataclasses import dataclass
from io import StringIO
from typing import List

import requests


@dataclass
class InfluxConfig:
    url: str
    token: str
    org: str
    bucket: str

    @classmethod
    def from_env(cls) -> "InfluxConfig":
        try:
            return cls(
                url=os.environ["INFLUX_URL"],
                token=os.environ["INFLUX_TOKEN"],
                org=os.environ["INFLUX_ORG"],
                bucket=os.environ["INFLUX_BUCKET"],
            )
        except KeyError as exc:
            raise RuntimeError(f"Missing required environment variable: {exc.args[0]}") from exc


def execute_flux_query(config: InfluxConfig, flux_query: str) -> List[dict]:
    """Execute a Flux query and return parsed records.

    InfluxDB responds with annotated CSV by default. We parse the payload and
    collapse identical column groups into dictionaries for easier consumption.
    """

    headers = {
        "Authorization": f"Token {config.token}",
        "Accept": "application/csv",
        "Content-Type": "application/vnd.flux",
    }

    response = requests.post(
        f"{config.url.rstrip('/')}/api/v2/query",
        params={"org": config.org},
        data=flux_query,
        headers=headers,
        timeout=30,
    )
    response.raise_for_status()

    csv_buffer = StringIO(response.text)
    reader = csv.reader(csv_buffer)

    tables: List[dict] = []
    column_names: List[str] = []
    for row in reader:
        if not row:
            continue
        if row[0].startswith("#"):  # annotations
            if row[0] == "#columns":
                column_names = row[1:]
            continue
        if not column_names:
            raise RuntimeError("Flux response missing #columns annotation")
        record = {column: value for column, value in zip(column_names, row[1:])}
        tables.append(record)
    return tables


def main() -> None:
    config = InfluxConfig.from_env()

    flux_query = f"""
from(bucket: "{config.bucket}")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "sensor_data")
"""

    results = execute_flux_query(config, flux_query)

    if not results:
        print("No data returned from InfluxDB")
        return

    print("Received rows:")
    for row in results:
        measurement = row.get("_measurement", "<unknown>")
        value = row.get("_value", "<unknown>")
        timestamp = row.get("_time", "<unknown>")
        print(f"measurement={measurement} value={value} time={timestamp}")


if __name__ == "__main__":
    main()
