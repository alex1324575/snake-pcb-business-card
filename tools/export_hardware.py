"""Regenerate public KiCad outputs with KiCad 10's CLI (no PCB mutation).

Usage: python tools/export_hardware.py --kicad-cli /path/to/kicad-cli
Add --renders to create the three native 3D previews. Standard KiCad libraries
must be installed for their models and footprints to be available.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def sanitize(text):
    """Retain report content while making project paths portable."""
    for prefix in (str(ROOT) + os.sep, ROOT.as_posix() + "/"):
        text = text.replace(prefix, "")
    # Preserve host diagnostics without publishing the user's cache locations.
    text = text.replace("\\", "/")
    text = re.sub(re.escape(Path.home().as_posix()), "<user-home>", text, flags=re.IGNORECASE)
    return text


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--kicad-cli", default="kicad-cli")
    parser.add_argument("--renders", action="store_true")
    args = parser.parse_args()
    for directory in ("exports", "fabrication", "verification/hardware", "docs/images"):
        (ROOT / directory).mkdir(parents=True, exist_ok=True)
    pcb = "SnakeBusinessCard.kicad_pcb"
    sch = "SnakeBusinessCard.kicad_sch"
    commands = [
        ["sch", "export", "netlist", "--format", "kicadxml", "-o", "exports/SnakeBusinessCard.net.xml", sch],
        ["sch", "erc", "--format", "json", "--severity-all", "--exit-code-violations", "-o", "verification/hardware/erc.json", sch],
        ["pcb", "drc", "--format", "json", "--schematic-parity", "--all-track-errors", "--severity-all", "--exit-code-violations", "-o", "verification/hardware/pcb_drc.json", pcb],
        ["sch", "export", "pdf", "--exclude-pdf-metadata", "-o", "exports/SnakeBusinessCard_Schematic.pdf", sch],
        ["sch", "export", "bom", "--fields", "Reference,Value,Manufacturer,MPN,Footprint,QUANTITY,DNP,Datasheet", "--labels", "References,Value,Manufacturer,MPN,Footprint,Quantity,DNP,Datasheet", "--group-by", "Value,Manufacturer,MPN,Footprint,DNP,Datasheet", "-o", "exports/SnakeBusinessCard_BOM.csv", sch],
        ["pcb", "export", "gerbers", "--layers", "F.Cu,B.Cu,F.Paste,B.Paste,F.SilkS,B.SilkS,F.Mask,B.Mask,Edge.Cuts", "-o", "fabrication/", pcb],
        ["pcb", "export", "drill", "--excellon-separate-th", "--generate-map", "--map-format", "pdf", "--generate-report", "--report-path", "fabrication/drill_report.txt", "-o", "fabrication/", pcb],
    ]
    if args.renders:
        for name, side, rotation in (("front", "top", None), ("back", "bottom", None), ("isometric", "top", "-30,0,25")):
            command = ["pcb", "render", "--width", "1800", "--height", "1200", "--side", side,
                       "--quality", "high", "--background", "transparent", "--use-board-stackup-colors",
                       "-o", f"docs/images/{name}.png"]
            if rotation:
                command += ["--rotate", rotation]
            commands.append(command + [pcb])
    reports = []
    for command in commands:
        print("kicad-cli " + " ".join(command), flush=True)
        result = subprocess.run([args.kicad_cli, *command], cwd=ROOT, capture_output=True, text=True, encoding="utf-8", errors="replace")
        # wxWidgets may log registry initialization failures on restricted Windows
        # accounts although the requested CLI operation succeeds. Preserve all
        # other diagnostics, and always enforce the process exit code.
        stderr = "\n".join(line for line in result.stderr.splitlines() if "registry key 'HKCU\\Software\\kicad-cli'" not in line)
        reports.append({"command": ["kicad-cli", *command], "exit_code": result.returncode,
                        "stdout": sanitize(result.stdout), "stderr": sanitize(stderr)})
        print(sanitize(result.stdout).strip(), flush=True)
        if result.returncode:
            print(stderr, file=sys.stderr)
            (ROOT / "verification/hardware/export_log.json").write_text(json.dumps(reports, indent=2) + "\n", encoding="utf-8")
            return result.returncode
    # KiCad netlist/library provenance and reports can embed local absolute paths.
    # Only file locations are normalized; electrical content and results remain intact.
    for path in [ROOT / "exports/SnakeBusinessCard.net.xml", * (ROOT / "verification/hardware").glob("*.json"), * (ROOT / "fabrication").glob("*.txt")]:
        content = sanitize(path.read_text(encoding="utf-8-sig"))
        path.write_text(content, encoding="utf-8")
    # The catalog check uses the freshly normalized XML. Write purchasing tables
    # before hashing exports so the manifest describes this same revision.
    procurement = subprocess.run([sys.executable, "tools/procurement.py", "--write-tables"],
                                 cwd=ROOT, capture_output=True, text=True, encoding="utf-8")
    reports.append({"command": ["python", "tools/procurement.py", "--write-tables"],
                    "exit_code": procurement.returncode, "stdout": sanitize(procurement.stdout),
                    "stderr": sanitize(procurement.stderr)})
    if procurement.returncode:
        print(sanitize(procurement.stderr), file=sys.stderr)
        (ROOT / "verification/hardware/export_log.json").write_text(json.dumps(reports, indent=2) + "\n", encoding="utf-8")
        return procurement.returncode
    (ROOT / "verification/hardware/export_log.json").write_text(json.dumps(reports, indent=2) + "\n", encoding="utf-8")
    files = [ROOT / pcb, ROOT / "SnakeBusinessCard.kicad_pro", *ROOT.glob("*.kicad_sch"), *(ROOT / "exports").glob("*"), *(ROOT / "fabrication").glob("*"), *(ROOT / "docs/images").glob("*.png")]
    manifest = {"source": pcb, "hardware_tested": False, "image_provenance": "Native KiCad 10 ray-traced renders of the public board; no photographic or generated-image substitution.",
                "files": {path.relative_to(ROOT).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest() for path in sorted(files) if path.is_file()}}
    (ROOT / "verification/hardware/manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
