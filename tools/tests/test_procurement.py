"""Guard against unintended native edits by the procurement synchronizer."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location(
    "procurement", Path(__file__).resolve().parents[1] / "procurement.py")
procurement = importlib.util.module_from_spec(spec)
spec.loader.exec_module(procurement)

BLOCK = '''(symbol
  (lib_id "Device:R")
  (property "Reference" "R1" (at 1 2 0))
  (property "Value" "10k" (at 3 4 0))
  (property "MPN" "" (at 5 6 0) (effects (font (size 1 1)) hide))
  (property "Datasheet" "" (at 5 6 0) (effects (font (size 1 1)) hide))
  (pin "1" (uuid "pin-one")))'''


class ProcurementTests(unittest.TestCase):
    def invariant(self, block, selected=None):
        return procurement.invariant("(kicad_sch " + block + ")", False,
                                     {"R1": {"value": "10k 1%"}} if selected is None else selected)

    def test_metadata_changes_are_allowed(self):
        updated = BLOCK
        for field, value in (("MPN", "ERJ3EKF1002V"), ("Manufacturer", "Panasonic Industry"),
                             ("Value", "10k 1%"), ("Datasheet", "https://example.com/part.pdf")):
            updated = procurement.set_property(updated, field, value, "R1", False)
        self.assertEqual(self.invariant(BLOCK), self.invariant(updated))
        self.assertIn('"ERJ3EKF1002V"', updated)
        self.assertIn('"Manufacturer" "Panasonic Industry"', updated)

    def test_label_geometry_and_visibility_are_preserved(self):
        for original, changed in (("(at 3 4 0)", "(at 99 4 0)"),
                                  ("(size 1 1)", "(size 2 2)"), (" hide)", ")")):
            with self.subTest(changed=changed):
                self.assertNotEqual(self.invariant(BLOCK), self.invariant(BLOCK.replace(original, changed)))

    def test_pin_identity_changes_are_rejected(self):
        self.assertNotEqual(self.invariant(BLOCK), self.invariant(BLOCK.replace('"pin-one"', '"pin-two"')))

    def test_unselected_value_cannot_change(self):
        self.assertNotEqual(self.invariant(BLOCK, {"R1": {}}),
                            self.invariant(BLOCK.replace('"10k"', '"20k"'), {"R1": {}}))

    def test_quoted_parentheses_do_not_break_spans(self):
        updated = procurement.set_property(BLOCK, "MPN", 'example("quoted")', "R1", False)
        props = procurement.properties(procurement.sexpr(updated))
        self.assertEqual(props["MPN"], 'example("quoted")')
        self.assertEqual(self.invariant(BLOCK), self.invariant(updated))

    def test_bom_ranges(self):
        self.assertEqual(procurement.expand_refs("C10,C12,C20-C22"),
                         ["C10", "C12", "C20", "C21", "C22"])
        with self.assertRaises(ValueError):
            procurement.expand_refs("R1-C3")


if __name__ == "__main__":
    unittest.main()
