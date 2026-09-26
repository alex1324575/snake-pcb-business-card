"""Verify QR from reloaded native KiCad geometry and independently parse NDEF.

Run with the standard Python that has OpenCV, NumPy and Pillow. A subprocess
uses KiCad's Python solely to read the board; the PNG is rasterized from the
actual native footprint rectangles, never from qr_matrix.json.
"""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
EXPECTED_URL = 'https://example.com/'


def extract_native(board_path):
    import pcbnew as p
    board = p.LoadBoard(str(board_path))
    matches = [fp for fp in board.GetFootprints() if fp.GetReference() == 'QR1']
    assert len(matches) == 1, f'Expected one QR1 footprint, found {len(matches)}'
    fp = matches[0]
    rects = []
    for shape in fp.GraphicalItems():
        if isinstance(shape, p.PCB_TEXT):
            assert not shape.IsVisible(), 'Unexpected visible footprint text'
            continue
        assert isinstance(shape, p.PCB_SHAPE)
        assert shape.GetShape() == p.SHAPE_T_RECT
        assert shape.GetLayer() == p.B_SilkS
        assert shape.IsSolidFill()
        assert shape.GetWidth() == 0
        rects.append([p.ToMM(shape.GetStart().x), p.ToMM(shape.GetStart().y),
                      p.ToMM(shape.GetEnd().x), p.ToMM(shape.GetEnd().y)])
    assert len(list(fp.Pads())) == 0, 'QR graphic must add no copper pads'
    attributes = fp.GetAttributes()
    required = p.FP_BOARD_ONLY | p.FP_EXCLUDE_FROM_BOM | p.FP_EXCLUDE_FROM_POS_FILES
    assert attributes & required == required
    x0, y0 = min(r[0] for r in rects), min(r[1] for r in rects)
    x1, y1 = max(r[2] for r in rects), max(r[3] for r in rects)
    qr_box = p.BOX2I(p.VECTOR2I(p.FromMM(x0), p.FromMM(y0)),
                    p.VECTOR2I(p.FromMM(x1-x0), p.FromMM(y1-y0)))
    other_art = list(board.GetDrawings())
    for other_fp in board.GetFootprints():
        if other_fp.GetReference() != 'QR1':
            other_art.extend(other_fp.GraphicalItems())
            other_art.extend((other_fp.Reference(), other_fp.Value()))
    intrusions = []
    for item in other_art:
        if item.GetLayer() != p.B_SilkS:
            continue
        if hasattr(item, 'IsVisible') and not item.IsVisible():
            continue
        if item.GetBoundingBox().Intersects(qr_box):
            intrusions.append(item.GetText() if hasattr(item, 'GetText') else str(type(item)))
    assert not intrusions, f'Other back silkscreen intersects QR/quiet zone: {intrusions}'
    print(json.dumps({'rectangles_native_mm': rects,
                      'rectangles_count': len(rects), 'layer': 'B.SilkS',
                      'footprint_id': fp.GetFPID().GetUniStringLibId(),
                      'footprint_uuid': str(fp.m_Uuid),
                      'pad_count': 0, 'board_only': True,
                      'excluded_from_bom': True, 'excluded_from_positions': True,
                      'other_back_silkscreen_intrusions': intrusions,
                      'copper_or_mask_graphics_added': False}))


def audit(board_path, output_path, kicad_python):
    import cv2
    import numpy as np
    from PIL import Image
    proc = subprocess.run([str(kicad_python), str(Path(__file__).resolve()),
                           '--extract', '--board', str(board_path)],
                          check=True, text=True, capture_output=True)
    native = json.loads(proc.stdout)
    metadata = json.loads((ROOT / 'contact/qr_matrix.json').read_text(encoding='utf-8'))
    assert metadata['url'] == EXPECTED_URL
    # Front-view board coordinates -> readable physical back view.
    origin_x, origin_y, board_width = 100.0, 80.0, 85.6
    bounds = (58.0, 11.5, 79.0, 32.5)
    rects_back = []
    for xa, ya, xb, yb in native['rectangles_native_mm']:
        left, right = sorted((origin_x + board_width - xa, origin_x + board_width - xb))
        top, bottom = sorted((ya - origin_y, yb - origin_y))
        rects_back.append((left, top, right, bottom))
    bounds_measured = [min(r[0] for r in rects_back), min(r[1] for r in rects_back),
                       max(r[2] for r in rects_back), max(r[3] for r in rects_back)]
    assert all(abs(a-b) < 0.000002 for a,b in zip(bounds_measured, bounds))
    # Rasterize actual native geometry at 40 pixels/mm. Test pixel centers
    # against rectangle extents to avoid adding raster stroke widths.
    ppmm, pixels = 40, 840
    xx = bounds[0] + (np.arange(pixels) + 0.5) / ppmm
    yy = bounds[1] + (np.arange(pixels) + 0.5) / ppmm
    raster = np.zeros((pixels, pixels), dtype=np.uint8)
    for left, top, right, bottom in rects_back:
        raster[np.ix_((yy >= top) & (yy < bottom), (xx >= left) & (xx < right))] = 255
    output_path.parent.mkdir(parents=True, exist_ok=True)
    raster_path = output_path.parent / 'QR_Native_Back_Verified.png'
    Image.fromarray(raster).save(raster_path)
    decoded, points, _ = cv2.QRCodeDetector().detectAndDecode(raster)
    assert points is not None and decoded == EXPECTED_URL, f'Native QR decode failed: {decoded!r}'
    # Independently sample the native shapes at each module center. Comparing
    # to the intended matrix proves the physical back is not reflected.
    count = metadata['total_modules']
    pitch = 21.0 / count
    sx = bounds[0] + (np.arange(count) + 0.5) * pitch
    sy = bounds[1] + (np.arange(count) + 0.5) * pitch
    light = np.zeros((count, count), dtype=bool)
    for left, top, right, bottom in rects_back:
        light[np.ix_((sy >= top) & (sy < bottom), (sx >= left) & (sx < right))] = True
    native_dark = ~light
    intended_dark = np.array(metadata['matrix'], dtype=bool)
    assert np.array_equal(native_dark, intended_dark), 'Native back matrix differs from source'
    assert not native_dark[:4].any() and not native_dark[-4:].any()
    assert not native_dark[:, :4].any() and not native_dark[:, -4:].any()

    # Independent, deliberately narrow parser for one short URI NDEF record.
    contact = ROOT / 'contact'
    message = (contact / 'Example_URI.ndef').read_bytes()
    assert message[0] == 0b11010001 and message[1] == 1 and message[3] == ord('U')
    payload_len = message[2]
    assert payload_len == len(message) - 4 and message[4] == 0x04
    ndef_url = 'https://' + message[5:].decode('utf-8')
    assert ndef_url == EXPECTED_URL
    tlv = (contact / 'Example_T2T_TLV.bin').read_bytes()
    assert tlv[:2] == bytes((0x03, len(message)))
    assert tlv[2:-1] == message and tlv[-1] == 0xFE
    pages = (contact / 'Example_T2T_user_pages.bin').read_bytes()
    assert pages.startswith(tlv) and len(pages) % 4 == 0
    assert pages[len(tlv):] == bytes((-len(tlv)) % 4)
    lines = [s for s in (contact / 'Example_T2T_user_pages.hex').read_text().splitlines()
             if s and not s.startswith('#')]
    page_bytes = bytearray()
    for index, line in enumerate(lines):
        page, data = line.split(':')
        assert int(page, 16) == 4 + index
        chunk = bytes.fromhex(data)
        assert len(chunk) == 4
        page_bytes.extend(chunk)
    assert bytes(page_bytes) == pages
    report = {
        'pass': True, 'expected_url': EXPECTED_URL,
        'board_file': board_path.resolve().relative_to(ROOT).as_posix(),
        'board_sha256': hashlib.sha256(board_path.read_bytes()).hexdigest(),
        'native_geometry_sha256': hashlib.sha256(json.dumps(native['rectangles_native_mm']).encode()).hexdigest(),
        'native_rectangle_count': native['rectangles_count'],
        'native_layer': native['layer'], 'native_pads': native['pad_count'],
        'native_back_view_bounds_mm': [round(v, 6) for v in bounds_measured],
        'native_back_matrix_exact_match': True,
        'four_module_white_quiet_zone_verified': True,
        'other_back_silkscreen_intrusions': native['other_back_silkscreen_intrusions'],
        'native_raster_file': raster_path.resolve().relative_to(ROOT).as_posix(),
        'native_qr_decoded_url': decoded,
        'decoder': 'OpenCV QRCodeDetector ' + cv2.__version__,
        'decode_source': 'native KiCad QR1 B.SilkS rectangles reloaded from board file',
        'copper_or_mask_geometry_added': False,
        'ndef_decoded_url': ndef_url,
        'ndef_bytes': len(message), 'type2_tlv_bytes': len(tlv),
        'user_page_bytes': len(pages), 'user_page_start': '0x04',
        'user_page_end': f'0x{3 + len(pages)//4:02X}',
        'hex_pages_match_binary': True,
        'ndef_sha256': hashlib.sha256(message).hexdigest(),
        'tag_write_performed': False, 'hardware_scan_performed': False,
        'finished_pcb_silkscreen_scan_performed': False,
    }
    output_path.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--board', type=Path, default=ROOT / 'SnakeBusinessCard.kicad_pcb')
    ap.add_argument('--output', type=Path, default=ROOT / 'verification/hardware/contact_native_audit.json')
    ap.add_argument('--kicad-python', type=Path, help='Path to the Python interpreter bundled with KiCad')
    ap.add_argument('--extract', action='store_true')
    args = ap.parse_args()
    if args.extract:
        extract_native(args.board)
    else:
        if args.kicad_python is None:
            ap.error('--kicad-python is required unless --extract is used')
        audit(args.board, args.output, args.kicad_python)
