"""Time HUB75 DATA writes separately from firmware acknowledgement and SHOW."""

import argparse
import json
import statistics
import time

if __package__:
    from .hub75_smoke import Session, frame_bytes, pattern
else:
    from hub75_smoke import Session, frame_bytes, pattern


def benchmark(link, frames=30, chunk_bytes=2000, show=False):
    if frames < 1 or not 1 <= chunk_bytes <= 2000:
        raise ValueError("Frames must be positive and chunk size must be 1..2000")
    session = Session(link)
    identity = session.query("*IDN?", timeout=8, prefix="CTI,")
    width, height = map(int, session.query("DISP:HUB75:DIM?").split(","))
    pixel_format = session.query("DISP:HUB75:FORM?")
    expected = frame_bytes(width, height, pixel_format)
    status = session.wait_swap()
    session.check_error()
    image = pattern("ramp", width, height, pixel_format)
    commands = []
    for offset in range(0, expected, chunk_bytes):
        chunk = image[offset:offset + chunk_bytes]
        length = str(len(chunk))
        commands.append(f"DISP:HUB75:DATA {offset},#{len(length)}{length}".encode() + chunk + b"\n")
    measurements = {"write_ms": [], "data_ack_ms": []}
    if show:
        measurements["frame_ms"] = []
    for _ in range(frames):
        started = time.perf_counter()
        for command in commands:
            session.write(command)
        written = time.perf_counter()
        session.check_error()  # Barrier: firmware has actually handled all preceding DATA.
        acknowledged = time.perf_counter()
        if show:
            session.write(b"DISP:HUB75:SHOW\n")
            session.wait_swap()
            session.check_error()
            measurements["frame_ms"].append((time.perf_counter() - started) * 1000)
        measurements["write_ms"].append((written - started) * 1000)
        measurements["data_ack_ms"].append((acknowledged - started) * 1000)
    final = session.status()
    session.check_error()
    if not final[0] or final[1] != (0 if show else expected):
        raise RuntimeError(f"Unexpected final display state: {final}")
    return {
        "identity": identity, "dimensions": [width, height], "format": pixel_format,
        "frame_bytes": expected, "chunk_bytes": chunk_bytes, "frames": frames, "show": show,
        "initial_status": status, "final_status": final,
        "timings": {
            key: {"median": statistics.median(values), "min": min(values), "max": max(values)}
            for key, values in measurements.items()
        },
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--frames", type=int, default=30)
    parser.add_argument("--chunk-bytes", type=int, default=2000)
    parser.add_argument("--show", action="store_true", help="Display test ramp; otherwise only replace staging")
    args = parser.parse_args()
    if args.frames < 1 or not 1 <= args.chunk_bytes <= 2000:
        parser.error("--frames must be positive; --chunk-bytes must be 1..2000")
    import serial
    with serial.Serial(args.port, 115200, timeout=0.25, write_timeout=2) as link:
        print(json.dumps(benchmark(link, args.frames, args.chunk_bytes, args.show), indent=2))


if __name__ == "__main__":
    main()
