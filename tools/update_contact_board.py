"""Update the board-only rear QR footprint from contact/qr_matrix.json.
Run with KiCad's Python after running contact_assets.py with standard Python.
Only silkscreen graphics are modified; copper, placement and routes are retained.
"""
from pathlib import Path
import json
import pcbnew as p
from contact_assets import add_contact_art
ROOT=Path(__file__).resolve().parents[1]
board_path=ROOT/'SnakeBusinessCard.kicad_pcb'
board=p.LoadBoard(str(board_path))
# Remove the previous generated label before rebuilding to remain idempotent.
for item in list(board.GetDrawings()):
    if isinstance(item,p.PCB_TEXT) and item.GetLayer()==p.B_SilkS and item.GetText()=='EXAMPLE / SCAN TO CONNECT':
        board.Delete(item)
result=add_contact_art(board)
p.FootprintSave(str(ROOT/'library/SnakeCard.pretty'),result['footprint'])
p.SaveBoard(str(board_path),board)
print(json.dumps({k:v for k,v in result.items() if k!='footprint'},indent=2))
