import unittest

from test_hub75_smoke import FakeSerial
from tools.hub75_benchmark import benchmark


class BenchmarkTests(unittest.TestCase):
    def test_existing_configuration_and_separate_timing_stages(self):
        for show in (False, True):
            link = FakeSerial()
            link.running = True
            result = benchmark(link, frames=3, chunk_bytes=2000, show=show)
            self.assertEqual(result["frame_bytes"], 12288)
            self.assertEqual(result["frames"], 3)
            self.assertEqual(result["show"], show)
            self.assertEqual(link.received, 0 if show else 12288)
            self.assertEqual(link.writes.count(b"DISP:HUB75:SHOW\n"), 3 if show else 0)
            self.assertEqual(set(result["timings"]),
                             {"write_ms", "data_ack_ms", "frame_ms"} if show else {"write_ms", "data_ack_ms"})
            self.assertFalse(any(command.startswith((b"DISP:HUB75:STOP", b"DISP:HUB75:START",
                                                    b"DISP:HUB75:DIM ", b"DISP:HUB75:FORM "))
                                 for command in link.writes))
            self.assertGreaterEqual(result["timings"]["data_ack_ms"]["median"],
                                    result["timings"]["write_ms"]["median"])

    def test_invalid_options_do_not_touch_hardware(self):
        for frames, chunk in ((0, 1000), (1, 0), (1, 2001)):
            link = FakeSerial()
            with self.assertRaises(ValueError):
                benchmark(link, frames=frames, chunk_bytes=chunk)
            self.assertEqual(link.writes, [])


if __name__ == "__main__":
    unittest.main()
