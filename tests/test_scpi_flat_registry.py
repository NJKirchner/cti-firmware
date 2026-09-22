import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def command_patterns():
    patterns = {}
    for source in (ROOT / "source" / "visa").glob("*.cpp"):
        text = source.read_text(encoding="utf-8")
        for pattern, command, query in re.findall(
            r'addCommand\("([^"]+)",\s*([^,]+),\s*([^)]+)\)', text
        ):
            patterns[pattern] = (command.strip(), query.strip())
    return patterns


def match_segment(pattern, candidate):
    has_number = pattern.endswith("#")
    if has_number:
        pattern = pattern[:-1]

    text = candidate
    number = -1
    if has_number:
        match = re.search(r"(\d+)$", candidate)
        if not match or int(match.group(1)) > 127:
            return None
        text = candidate[: match.start()]
        number = int(match.group(1))

    required = 0
    while required < len(pattern) and not pattern[required].islower():
        required += 1
    if len(text) not in (required, len(pattern)):
        return None
    if text.upper() != pattern[: len(text)].upper():
        return None
    return number


def matches(pattern, candidate):
    pattern_parts = pattern.removeprefix(":").split(":")
    candidate_parts = candidate.removeprefix(":").split(":")
    if len(pattern_parts) != len(candidate_parts):
        return False
    return all(
        match_segment(pattern_part, candidate_part) is not None
        for pattern_part, candidate_part in zip(pattern_parts, candidate_parts)
    )


class FlatRegistryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.patterns = command_patterns()

    def handlers_for(self, command):
        return [
            handlers
            for pattern, handlers in self.patterns.items()
            if matches(pattern, command)
        ]

    def test_digital_available_short_form_selects_only_digital_handler(self):
        self.assertEqual(
            self.handlers_for("DIG:AVAIL"),
            [("nullptr", "digital_available")],
        )

    def test_digital_available_long_form_selects_only_digital_handler(self):
        self.assertEqual(
            self.handlers_for("DIGITAL:AVAILABLE"),
            [("nullptr", "digital_available")],
        )

    def test_idn_selects_only_idn_handler(self):
        self.assertEqual(
            self.handlers_for("*IDN"),
            [("nullptr", "SCPI_CoreIdnQ")],
        )

    def test_unknown_query_has_no_default_handler(self):
        self.assertEqual(self.handlers_for("DIG:INVALID"), [])
        self.assertEqual(self.handlers_for("UNKNOWN"), [])


if __name__ == "__main__":
    unittest.main()
