"""Synchronize and audit procurement metadata without changing circuit geometry.

Run --sync after editing procurement/parts.json, then export hardware with
export_hardware.py. Run --check to verify native sources, XML and generated BOMs.
Only Manufacturer, MPN, Datasheet and explicitly selected Value fields are edited.
"""
import argparse
import csv
import hashlib
import io
import json
from pathlib import Path
import re
import sys
import uuid
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "firmware/tools"))
from check_board_contract import children, sexpr

CATALOG = ROOT / "procurement/parts.json"
FIELDS = ("Manufacturer", "MPN", "Datasheet", "Value")


def require(condition, message):
    if not condition:
        raise ValueError(message)


def ref_key(ref):
    prefix, number = re.fullmatch(r"([A-Za-z]+)(\d+)", ref).groups()
    return prefix, int(number)


def catalog():
    data = json.loads(CATALOG.read_text(encoding="utf-8"))
    by_ref = {}
    for part in data["entries"]:
        require(part["population"] in ("fit", "dnp", "pcb_feature"), "Invalid population")
        if part["population"] == "fit":
            require(all(part.get(k) for k in ("manufacturer", "mpn", "datasheet", "review")),
                    "Fitted part lacks procurement evidence")
        else:
            require(not part["mpn"], "DNP/PCB feature must not have an orderable part")
        for ref in part["references"]:
            require(ref not in by_ref, "Duplicate catalog reference: " + ref)
            by_ref[ref] = part
    require({r for r, p in by_ref.items() if p["population"] == "pcb_feature"} ==
            {"L1", *("TP" + str(i) for i in range(1, 9))}, "Unexpected PCB-feature classification")
    require({r for r, p in by_ref.items() if p["population"] == "dnp"} == {"C30"},
            "Unexpected DNP classification")
    return data, by_ref


def spans(text):
    """Return direct child list spans, respecting quoted KiCad strings."""
    depth, start = 0, None
    for match in re.finditer(r'"(?:\\.|[^"\\])*"|[()]', text):
        token = match.group()
        if token == "(":
            depth += 1
            if depth == 2:
                start = match.start()
        elif token == ")":
            if depth == 2:
                yield start, match.end()
            depth -= 1
    require(depth == 0, "Unbalanced native KiCad file")


def properties(node):
    return {p[1]: p[2] for p in children(node, "property")}


def set_property(block, name, value, ref, is_pcb):
    field_spans = {}
    for start, end in spans(block):
        child_text = block[start:end]
        if child_text.startswith("(property "):
            node = sexpr(child_text)
            field_spans[node[1]] = (start, end)
    if name in field_spans:
        start, end = field_spans[name]
        original = block[start:end]
        updated = re.sub(r'^(\(property\s+"(?:\\.|[^"\\])*"\s+)"(?:\\.|[^"\\])*"',
                         lambda match: match[1] + json.dumps(value), original, count=1)
        return block[:start] + updated + block[end:]
    require(name == "Manufacturer", "Expected native property absent: " + ref + "/" + name)
    start, end = field_spans["MPN"]
    original = block[start:end]
    updated = re.sub(r'^\(property\s+"MPN"\s+"(?:\\.|[^"\\])*"',
                     lambda _: '(property "Manufacturer" ' + json.dumps(value), original, count=1)
    if is_pcb:
        field_id = str(uuid.uuid5(uuid.NAMESPACE_URL, "snake-card/" + ref + "/Manufacturer"))
        updated = re.sub(r'\(uuid "[^"]+"\)', '(uuid "' + field_id + '")', updated)
    indent = "\t\t" if is_pcb else "    "
    newline = "\r\n" if "\r\n" in block else "\n"
    return block[:end] + newline + indent + updated + block[end:]


def invariant(text, is_pcb, by_ref):
    """Full parsed native document, excluding only the allowed field changes."""
    tree = sexpr(text)
    for node in children(tree, "footprint" if is_pcb else "symbol"):
        if not is_pcb and not children(node, "lib_id"):
            continue
        ref = properties(node).get("Reference")
        if ref not in by_ref:
            continue
        for prop in children(node, "property"):
            if prop[1] in ("MPN", "Datasheet") or (prop[1] == "Value" and by_ref[ref].get("value")):
                prop[2] = "<selected metadata>"  # Preserve position/font/layer/visibility/UUID.
        # A new hidden Manufacturer field is deliberately inserted by --sync.
        node[:] = [item for item in node if not (isinstance(item, list) and
                   len(item) > 2 and item[0] == "property" and item[1] == "Manufacturer")]
    return tree


def native_files():
    return [*sorted(ROOT.glob("*.kicad_sch")), ROOT / "SnakeBusinessCard.kicad_pcb"]


def sync(by_ref):
    report = {"passed": True, "scope": "Procurement metadata and selected value labels only",
              "allowed_fields": list(FIELDS), "files": []}
    for path in native_files():
        original = path.read_bytes().decode("utf-8")
        is_pcb = path.suffix == ".kicad_pcb"
        updates = []
        count = 0
        for start, end in spans(original):
            block = original[start:end]
            kind = "footprint" if is_pcb else "symbol"
            if not re.match(r"\(" + kind + r"(?:\s|$)", block):
                continue
            node = sexpr(block)
            if not is_pcb and not children(node, "lib_id"):
                continue
            ref = properties(node).get("Reference")
            if ref not in by_ref:
                continue
            part = by_ref[ref]
            selected = {"Manufacturer": part["manufacturer"], "MPN": part["mpn"],
                        "Datasheet": part["datasheet"]}
            if part.get("value"):
                selected["Value"] = part["value"]
            updated = block
            for name, value in selected.items():
                updated = set_property(updated, name, value, ref, is_pcb)
            updates.append((start, end, updated))
            count += 1
        result = original
        for start, end, updated in reversed(updates):
            result = result[:start] + updated + result[end:]
        require(invariant(original, is_pcb, by_ref) == invariant(result, is_pcb, by_ref),
                "Unexpected native change: " + path.name)
        path.write_bytes(result.encode("utf-8"))
        report["files"].append({"path": path.name, "matched_components": count,
                                "before_sha256": hashlib.sha256(original.encode()).hexdigest(),
                                "after_sha256": hashlib.sha256(result.encode()).hexdigest(),
                                "non_metadata_content_preserved": True})
    (ROOT / "verification/hardware/procurement_metadata_audit.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")


def xml_components():
    result = {}
    for component in ET.parse(ROOT / "exports/SnakeBusinessCard.net.xml").getroot().find("components"):
        ref = component.attrib["ref"]
        require(ref not in result, "Duplicate XML reference: " + ref)
        result[ref] = component
    return result


def expand_refs(value):
    result = []
    for group in value.split(","):
        if "-" not in group:
            result.append(group.strip())
            continue
        first, last = (ref_key(ref.strip()) for ref in group.split("-"))
        require(first[0] == last[0] and first[1] <= last[1], "Invalid BOM reference range")
        result.extend(first[0] + str(i) for i in range(first[1], last[1] + 1))
    return result


def csv_text(headers, rows):
    stream = io.StringIO(newline="")
    writer = csv.writer(stream, lineterminator="\n")
    writer.writerow(headers)
    writer.writerows(rows)
    return stream.getvalue()


def generated(data, by_ref, components):
    bom, review = [], []
    for part in data["entries"]:
        if part["population"] != "fit":
            continue
        refs = sorted(part["references"], key=ref_key)
        footprints = {components[r].findtext("footprint", "") for r in refs}
        require(len(footprints) == 1, "Mixed footprints in part group: " + part["mpn"])
        bom.append([", ".join(refs), len(refs), part["manufacturer"], part["mpn"],
                    part["description"], footprints.pop(), part["datasheet"],
                    part["review_status"], part["open_items"]])
    for ref in sorted(by_ref, key=ref_key):
        part, component = by_ref[ref], components[ref]
        review.append([ref, component.findtext("value"), part["population"],
                       part["manufacturer"], part["mpn"], component.findtext("footprint", ""),
                       part["review_status"], part["review"], part["datasheet"],
                       part["open_items"], "No"])
    return {
        "exports/Procurement_BOM.csv": csv_text(
            ["References", "Quantity per board", "Manufacturer", "MPN", "Description",
             "Footprint", "Datasheet", "Review status", "Open items"], bom),
        "verification/hardware/component_review.csv": csv_text(
            ["Reference", "Value", "Population", "Manufacturer", "MPN", "Footprint",
             "Review status", "Review document", "Datasheet", "Open items", "Hardware tested"], review)}


def check(data, by_ref, write):
    components = xml_components()
    require(set(components) == set(by_ref), "Catalog must cover every XML component exactly")
    schematic_refs, pcb_refs = set(), set()
    for path in native_files():
        is_pcb = path.suffix == ".kicad_pcb"
        for node in children(sexpr(path.read_text(encoding="utf-8")),
                             "footprint" if is_pcb else "symbol"):
            if not is_pcb and not children(node, "lib_id"):
                continue
            props = properties(node)
            ref = props.get("Reference")
            if ref not in by_ref:
                require(ref.startswith("#") or (is_pcb and ref in ("NT1", "QR1")),
                        "Uncatalogued native reference: " + str(ref))
                continue
            part = by_ref[ref]
            seen = pcb_refs if is_pcb else schematic_refs
            require(ref not in seen, "Duplicate native reference: " + ref)
            seen.add(ref)
            for field, key in (("Manufacturer", "manufacturer"), ("MPN", "mpn"),
                               ("Datasheet", "datasheet")):
                require(props.get(field, "") == part[key], path.name + ": " + ref + "/" + field)
            if part.get("value"):
                require(props["Value"] == part["value"], "Native value mismatch: " + ref)
            component = components[ref]
            require(props["Value"] == component.findtext("value"), "XML/native value mismatch: " + ref)
            footprint = node[1] if is_pcb else props.get("Footprint", "")
            require(footprint == component.findtext("footprint", ""), "XML/native footprint mismatch: " + ref)
    require(schematic_refs == set(by_ref), "Schematic catalog coverage mismatch")
    require(pcb_refs == set(by_ref) - {"L1"}, "PCB catalog coverage mismatch")
    for ref, component in components.items():
        part = by_ref[ref]
        fields = {f.attrib["name"]: f.text or "" for f in component.findall("./fields/field")}
        for field, key in (("Manufacturer", "manufacturer"), ("MPN", "mpn")):
            require(fields.get(field, "") == part[key], "Stale XML field: " + ref + "/" + field)
        require(component.findtext("datasheet", "") == part["datasheet"], "Stale XML datasheet: " + ref)
        if part.get("value"):
            require(component.findtext("value") == part["value"], "Stale XML value: " + ref)
        dnp = any(p.attrib.get("name") == "dnp" for p in component.findall("property"))
        require(dnp == (part["population"] == "dnp"), "DNP mismatch: " + ref)
    # Also check the raw KiCad BOM rather than trusting its export command.
    bom_refs = set()
    with (ROOT / "exports/SnakeBusinessCard_BOM.csv").open(encoding="utf-8-sig", newline="") as stream:
        for row in csv.DictReader(stream):
            refs = expand_refs(row["References"])
            require(int(row["Quantity"]) == len(refs), "Native BOM quantity mismatch")
            for ref in refs:
                require(ref in by_ref and ref not in bom_refs, "Native BOM reference mismatch: " + ref)
                bom_refs.add(ref)
                part, component = by_ref[ref], components[ref]
                for field, key in (("Manufacturer", "manufacturer"), ("MPN", "mpn"), ("Datasheet", "datasheet")):
                    require(row[field] == part[key], "Stale native BOM: " + ref + "/" + field)
                require(row["Value"] == component.findtext("value"), "Native BOM value mismatch: " + ref)
                require(row["Footprint"] == component.findtext("footprint", ""), "Native BOM footprint mismatch: " + ref)
                require(bool(row["DNP"]) == (part["population"] == "dnp"), "Native BOM DNP mismatch: " + ref)
    expected_bom = {r for r, c in components.items() if not any(
        p.attrib.get("name") == "exclude_from_bom" for p in c.findall("property"))}
    require(bom_refs == expected_bom, "Incomplete native BOM coverage")
    for name, content in generated(data, by_ref, components).items():
        path = ROOT / name
        if write:
            path.write_bytes(content.encode("utf-8"))
        require(path.read_bytes() == content.encode(), "Stale generated table: " + name)
    summary = {"passed": True, "catalog_sha256": hashlib.sha256(CATALOG.read_bytes()).hexdigest(),
               "schematic_components": len(components), "fitted_components": sum(
                   p["population"] == "fit" for p in by_ref.values()),
               "fitted_orderable_mpns": len({p["mpn"] for p in by_ref.values() if p["population"] == "fit"}),
               "dnp_components": [r for r, p in by_ref.items() if p["population"] == "dnp"],
               "pcb_features": [r for r, p in by_ref.items() if p["population"] == "pcb_feature"],
               "hardware_tested": False}
    report = ROOT / "verification/hardware/procurement_check.json"
    expected = (json.dumps(summary, indent=2) + "\n").encode("utf-8")
    if write:
        report.write_bytes(expected)
    require(report.read_bytes() == expected, "Stale procurement check report")
    print(json.dumps(summary, indent=2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sync", action="store_true")
    parser.add_argument("--write-tables", action="store_true")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    data, by_ref = catalog()
    if args.sync:
        sync(by_ref)
        print("PASS: metadata synchronized; all other native content preserved. Re-export XML next.")
    if args.check or args.write_tables:
        check(data, by_ref, args.write_tables)
    require(args.sync or args.check or args.write_tables, "Choose --sync, --write-tables or --check")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, AssertionError, KeyError, OSError) as error:
        print("FAILED: " + str(error), file=sys.stderr)
        sys.exit(1)
