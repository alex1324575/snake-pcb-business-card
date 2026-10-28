# Rev C Saved Source Audit

Result: **PASS**

The saved SchDoc and SchLib preserve electrical pin geometry, wiring, net labels, footprint mappings and unrelated sheet artwork. The controlled changes are procurement metadata, the U3/D91 aliases, D91 K/A pin names and eight symbol lines, and one Rev C drawing title.

| Check | Result |
|---|---|
| Native source instances / library symbols / footprints | 135 / 29 / 15 |
| PCB model instances | 136, including board artwork |
| Independent schematic graph | 346 pins; 39 named nets; 30 isolated NC; no shorts or mismatches |
| Firmware-to-schematic-and-PCB contract | 217 checks |
| LED grid | 90 LEDs; 3.5 mm pitch; cathode at physical pad 1, left in Top view |
| PCB / reviewed physical inventory binding | Complete pad bytes after owner-index rebinding; exact net order and placement |
| Unrelated sheet records | 855 protected records; one explicit Rev B to Rev C title replacement; derived header count independently checked |

## D91 polarity and symbol

The selected PESD5Z6.0,115 uses pin 1 K to VBUS_5V and pin 2 A to GND. The redrawn unidirectional zener body meets both unchanged pin roots; the triangle points toward the left cathode bar.

## Native serialization details

Altium regenerated the own GUID of 346 hidden PinUniqueId auxiliary parameters. Each parameter still references the exact same parent pin UID; the actual pin and component UIDs, geometry, owner association and nets remain unchanged. The SchLib catalogue table was reordered; entries compare by exact symbol identity. Its font table and 59 auxiliary streams remain protected. These serialization effects are recorded explicitly, rather than described as raw file identity.

## Evidence boundary

This is a saved-file semantic and binding audit. Native ERC/DRC reports are separate evidence. Retained firmware host tests are bound by exact source hashes; no new bench test is implied. Physical qualification remains NOT TESTED. Full field-level results, source hashes and limitations are in FINAL_SOURCE_AUDIT.json.
