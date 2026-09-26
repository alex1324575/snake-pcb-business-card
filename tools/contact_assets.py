"""Example QR and NFC NDEF assets; no network access or tag writes.

Run with the standard Python/OpenCV runtime to regenerate contact assets.
The pcbnew helper imports neither OpenCV nor Pillow and can run with KiCad's
Python. It only changes back silkscreen and a board-only graphic footprint.
"""
from pathlib import Path
import hashlib
import json

ROOT = Path(__file__).resolve().parents[1]
CONTACT = ROOT / 'contact'
URL = 'https://example.com/'
BOUNDS = (58.0, 11.5, 79.0, 32.5)  # mm, readable back view


def light_runs(matrix):
    """Horizontal runs of light cells, including the 4-module quiet zone."""
    runs = []
    for y, row in enumerate(matrix):
        x = 0
        while x < len(row):
            if row[x]:
                x += 1
                continue
            start = x
            while x < len(row) and not row[x]:
                x += 1
            runs.append((start, y, x, y + 1))
    return runs


def decode_ndef(message):
    """Independent minimal reader for this one-record NFC Forum URI message."""
    assert message[0] == 0xD1  # MB | ME | SR | TNF well-known
    type_length, payload_length = message[1:3]
    assert type_length == 1 and len(message) == 3 + type_length + payload_length
    assert message[3:4] == b'U'
    payload = message[4:]
    prefixes = {0x01: 'http://www.', 0x02: 'https://www.',
                0x03: 'http://', 0x04: 'https://'}
    return prefixes[payload[0]] + payload[1:].decode('utf-8')


def generate_assets():
    import cv2
    import numpy as np
    from PIL import Image

    CONTACT.mkdir(exist_ok=True)
    params = cv2.QRCodeEncoder_Params()
    params.correction_level = cv2.QRCodeEncoder_CORRECT_LEVEL_Q
    params.mode = cv2.QRCodeEncoder_MODE_BYTE
    encoded = cv2.QRCodeEncoder_create(params).encode(URL)
    # OpenCV adds its own border. Derive the actual symbol bounds, then add the
    # required four modules explicitly; no assumed encoder-border thickness.
    yy, xx = np.where(encoded == 0)
    symbol = encoded[yy.min():yy.max()+1, xx.min():xx.max()+1]
    matrix = np.pad(symbol == 0, 4, constant_values=False)
    pixels = np.where(matrix, 0, 255).astype('uint8')
    png = np.repeat(np.repeat(pixels, 16, axis=0), 16, axis=1)
    Image.fromarray(png).save(CONTACT / 'Example_QR.png')
    decoded, points, _ = cv2.QRCodeDetector().detectAndDecode(png)
    assert points is not None and decoded == URL
    cells = matrix.astype(int).tolist()
    metadata = {
        'url': URL, 'qr_ecc': 'Q', 'qr_version': (len(symbol)-17)//4,
        'symbol_modules': len(symbol), 'quiet_zone_modules': 4,
        'total_modules': len(cells), 'printed_size_mm': 21.0,
        'module_pitch_mm': 21.0/len(cells),
        'back_view_bounds_mm': list(BOUNDS),
        'silkscreen_layer': 'B.SilkS',
        'dark_modules': 'unprinted graphite black solder mask',
        'light_modules': 'white silkscreen including four-module quiet zone',
        'matrix': cells,
    }
    (CONTACT / 'qr_matrix.json').write_text(json.dumps(metadata, indent=2), encoding='utf-8')

    # One NFC Forum well-known URI record. 0x04 abbreviates https://
    payload = b'\x04' + URL.removeprefix('https://').encode('utf-8')
    message = bytes([0xD1, 0x01, len(payload), 0x55]) + payload
    tlv = bytes([0x03, len(message)]) + message + b'\xFE'
    pages = tlv + bytes((-len(tlv)) % 4)
    assert len(tlv) < 160
    assert decode_ndef(message) == URL
    assert tlv[0] == 3 and tlv[1] == len(message) and tlv[-1] == 0xFE
    (CONTACT / 'Example_URI.ndef').write_bytes(message)
    (CONTACT / 'Example_T2T_TLV.bin').write_bytes(tlv)
    (CONTACT / 'Example_T2T_user_pages.bin').write_bytes(pages)
    (CONTACT / 'Example_T2T_user_pages.hex').write_text(
        '# ST25TN01K user pages only, first page 0x04. No UID/CC/lock/config bytes.\n'
        + '\n'.join(f'{4+i//4:02X}: ' + pages[i:i+4].hex(' ').upper()
                    for i in range(0, len(pages), 4)) + '\n', encoding='utf-8')
    (CONTACT / 'example.url.txt').write_text(URL + '\n', encoding='utf-8')
    report = {
        'url': URL,
        'qr_decoder': f'OpenCV QRCodeDetector {cv2.__version__}',
        'qr_decoded_url': decoded, 'qr_decode_pass': decoded == URL,
        'qr_image_sha256': hashlib.sha256((CONTACT/'Example_QR.png').read_bytes()).hexdigest(),
        'matrix_sha256': hashlib.sha256((CONTACT/'qr_matrix.json').read_bytes()).hexdigest(),
        'ndef_decoded_url': decode_ndef(message),
        'ndef_parse_pass': decode_ndef(message) == URL,
        'ndef_record_hex': message.hex(' ').upper(),
        'ndef_message_bytes': len(message), 't2t_tlv_bytes': len(tlv),
        'page_padded_bytes': len(pages), 'start_user_page_hex': '04',
        'tag_write_performed': False, 'hardware_scan_performed': False,
        'silkscreen_scan_on_finished_pcb_performed': False,
    }
    (CONTACT / 'contact_validation.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(json.dumps({k:v for k,v in report.items() if not k.endswith('_hex')}, indent=2))


def add_contact_art(board, origin=(100., 80.), width=85.6, remove_placeholder=True):
    """Place actual QR1 art in B.SilkS; remove only known placeholder graphics.

    Input matrix is 1=dark. Print only light-cell runs so normal dark modules
    are provided by black mask. This adds no copper, mask opening, or paste.
    Native front-view X coordinates are mirrored around the board width so
    the QR reads correctly when the physical card is turned over.
    """
    import pcbnew as p
    data = json.loads((CONTACT / 'qr_matrix.json').read_text(encoding='utf-8'))
    assert data['url'] == URL
    mm = p.FromMM
    def at(x, y):
        return p.VECTOR2I(mm(origin[0] + width - x), mm(origin[1] + y))
    def back_xy(v):
        return (round(width - (p.ToMM(v.x) - origin[0]), 3),
                round(p.ToMM(v.y) - origin[1], 3))
    for old in list(board.GetFootprints()):
        if old.GetReference() == 'QR1':
            board.Delete(old)
    if remove_placeholder:
        corners = {(58.,11.5), (79.,11.5), (79.,32.5), (58.,32.5)}
        for item in list(board.GetDrawings()):
            if item.GetLayer() != p.B_SilkS:
                continue
            if isinstance(item, p.PCB_TEXT):
                if item.GetText() in ('PLACEHOLDER', 'LINKEDIN / URL PENDING',
                                      'EXAMPLE / SCAN TO CONNECT'):
                    board.Delete(item)
            elif isinstance(item, p.PCB_SHAPE):
                if back_xy(item.GetStart()) in corners and back_xy(item.GetEnd()) in corners:
                    board.Delete(item)
    fp = p.FOOTPRINT(board)
    fp.SetReference('QR1'); fp.SetValue('Example QR / ' + URL)
    fp.SetFPID(p.LIB_ID('SnakeCard', 'Example_QR_21mm_BSilk'))
    fp.SetLayer(p.B_Cu)
    fp.SetAttributes(p.FP_BOARD_ONLY | p.FP_EXCLUDE_FROM_BOM | p.FP_EXCLUDE_FROM_POS_FILES)
    fp.SetPosition(at(68.5, 22.0))
    fp.Reference().SetVisible(False); fp.Value().SetVisible(False)
    board.Add(fp)
    pitch = 21. / data['total_modules']
    for x0, y0, x1, y1 in light_runs(data['matrix']):
        shape = p.PCB_SHAPE(fp, p.SHAPE_T_RECT)
        a = at(58. + x0*pitch, 11.5 + y0*pitch)
        b = at(58. + x1*pitch, 11.5 + y1*pitch)
        shape.SetStart(p.VECTOR2I(min(a.x,b.x), min(a.y,b.y)))
        shape.SetEnd(p.VECTOR2I(max(a.x,b.x), max(a.y,b.y)))
        shape.SetLayer(p.B_SilkS); shape.SetWidth(0); shape.SetFilled(True)
        fp.Add(shape)
    label = p.PCB_TEXT(board)
    label.SetText('EXAMPLE / SCAN TO CONNECT')
    label.SetPosition(at(58.,35.5)); label.SetLayer(p.B_SilkS)
    label.SetTextSize(p.VECTOR2I(mm(.85), mm(.85))); label.SetTextThickness(mm(.15))
    label.SetHorizJustify(p.GR_TEXT_H_ALIGN_LEFT); label.SetMirrored(True)
    board.Add(label)
    return {'footprint': fp, 'url': URL, 'light_rectangles': len(light_runs(data['matrix'])),
            'module_pitch_mm': pitch, 'quiet_zone_modules': 4,
            'bounds_back_view_mm': BOUNDS, 'copper_added': False}


if __name__ == '__main__':
    generate_assets()
