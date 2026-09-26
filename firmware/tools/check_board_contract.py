#!/usr/bin/env python3
"""Check firmware assumptions against exported XML and native PCB, using stdlib.

Run: python firmware/tools/check_board_contract.py
This is a source/geometry consistency audit, not ERC/DRC or a hardware test.
"""
from __future__ import annotations

import argparse
import ast
import copy
import hashlib
import json
import operator
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

FIRMWARE = Path(__file__).resolve().parents[1]
PROJECT = FIRMWARE.parent


class ContractError(Exception):
    pass


def require(condition, message):
    if not condition:
        raise ContractError(message)


def sexpr(text):
    """Parse quoted strings and balanced lists; never executes board contents."""
    roots, stack = [], []
    for token in re.findall(r'"(?:\\.|[^"\\])*"|[()]|[^()\s]+', text):
        if token == '(':
            current = []
            (stack[-1] if stack else roots).append(current)
            stack.append(current)
        elif token == ')':
            require(bool(stack), "PCB has unmatched closing parenthesis")
            stack.pop()
        else:
            require(bool(stack), "PCB has an atom outside the root")
            stack[-1].append(json.loads(token) if token.startswith('"') else token)
    require(not stack and len(roots) == 1, "PCB has incomplete/multiple roots")
    return roots[0]


def children(node, name):
    return [entry for entry in node if isinstance(entry, list) and entry and entry[0] == name]


def child(node, name):
    entries = children(node, name)
    require(len(entries) == 1, f"Expected one {name}, got {len(entries)}")
    return entries[0]


def load_board(path):
    result = {}
    for fp in children(sexpr(path.read_text(encoding='utf-8')), 'footprint'):
        refs = [p[2] for p in children(fp, 'property') if p[1] == 'Reference']
        require(len(refs) == 1, 'Footprint reference missing/ambiguous')
        ref = refs[0]
        require(ref not in result, f'Duplicate PCB reference: {ref}')
        pads = {}
        for pad in children(fp, 'pad'):
            nets = children(pad, 'net')
            net = nets[0][-1] if nets else ''
            if pad[1] in pads:
                require(pads[pad[1]] == net, f'{ref}.{pad[1]} duplicate pads disagree')
            pads[pad[1]] = net
        at = child(fp, 'at')
        result[ref] = {'x': float(at[1]), 'y': float(at[2]),
                       'layer': child(fp, 'layer')[1], 'pads': pads}
    return result


def load_netlist(path):
    root = ET.parse(path).getroot()
    pins, nets = {}, {}
    for net in root.findall('./nets/net'):
        name = net.attrib['name']
        require(name not in nets, f'Duplicate XML net {name}')
        nets[name] = []
        for node in net.findall('node'):
            key = (node.attrib['ref'], node.attrib['pin'])
            require(key not in pins, f'XML pin appears twice: {key}')
            pins[key] = dict(node.attrib, net=name)
            nets[name].append(key)
    values = {comp.attrib['ref']: comp.findtext('value', '')
              for comp in root.findall('./components/comp')}
    return {'pins': pins, 'nets': nets, 'values': values}


def macro_definitions(paths):
    definitions = {}
    for path in paths:
        text = re.sub(r'/\*.*?\*/|//[^\n]*', '', path.read_text(encoding='utf-8'), flags=re.S)
        for name, value in re.findall(r'^\s*#define\s+(\w+)[ \t]+([^\n]+)', text, re.M):
            definitions[name] = value.strip()
    return definitions


def macro_int(name, definitions):
    """Small explicit arithmetic evaluator, not eval()/C preprocessing."""
    operations = {ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul,
                  ast.LShift: operator.lshift, ast.BitOr: operator.or_,
                  ast.BitAnd: operator.and_}
    visiting = set()

    def parse(symbol):
        require(symbol in definitions and symbol not in visiting, f'Missing/cyclic macro {symbol}')
        visiting.add(symbol)
        value = re.sub(r'\b(0[xX][0-9a-fA-F]+|\d+)[uUlL]+\b', r'\1', definitions[symbol])
        result = walk(ast.parse(value, mode='eval').body)
        visiting.remove(symbol)
        return result

    def walk(node):
        if isinstance(node, ast.Constant) and type(node.value) is int:
            return node.value
        if isinstance(node, ast.Name):
            return parse(node.id)
        if isinstance(node, ast.BinOp) and type(node.op) in operations:
            return operations[type(node.op)](walk(node.left), walk(node.right))
        if (isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and
                len(node.args) == 1 and not node.keywords):
            if node.func.id == 'UINT32_C':
                return walk(node.args[0])
            if node.func.id == 'SC_BIT':
                return 1 << walk(node.args[0])
        raise ContractError(f'Unsupported numeric expression in macro {name}')

    return parse(name)


def audit(netlist, board, macros):
    checks = 0
    pins, nets, values = netlist['pins'], netlist['nets'], netlist['values']

    def pin(ref, number, net, function=None):
        nonlocal checks
        key = (ref, str(number))
        require(key in pins, f'Missing XML pin {ref}.{number}')
        require(pins[key]['net'] == net, f'{ref}.{number}: {pins[key]["net"]} != {net}')
        if function is not None:
            require(pins[key].get('pinfunction') == f'{function}_{number}',
                    f'{ref}.{number} function differs from {function}')
        require(ref in board and board[ref]['pads'].get(str(number)) == net,
                f'PCB pad {ref}.{number} is not on XML net {net}')
        checks += 1

    def value(ref, wanted):
        nonlocal checks
        require(values.get(ref) == wanted, f'{ref} value {values.get(ref)!r} != {wanted!r}')
        checks += 1

    def macro(name, wanted):
        nonlocal checks
        actual = macro_int(name, macros)
        require(actual == wanted, f'Firmware {name}={actual}, expected {wanted}')
        checks += 1

    value('U1', 'STM32G030K8T6')
    value('U2', 'IS31FL3737-QFLS4-TR')
    controls = []
    for index, name in enumerate(('UP', 'DOWN', 'LEFT', 'RIGHT', 'START')):
        net = 'BTN_' + name
        pin('U1', 7 + index, net, f'PA{index}')
        pin(f'SW{index + 1}', 1, net)
        pin(f'SW{index + 1}', 2, 'GND')
        pin(f'R{index + 11}', 1, '+3V3')
        pin(f'R{index + 11}', 2, net)
        value(f'R{index + 11}', '10k')
        macro('BUTTON_' + name, 1 << index)
        controls.append((net, str(7 + index), f'PA{index}', f'SW{index + 1}'))
    macro('BUTTON_MASK', 31)
    for number, gpio, net, driver_number, driver_function, resistor in (
            (30, 'PB6', 'I2C_SCL', 35, 'SCL', 'R20'),
            (31, 'PB7', 'I2C_SDA', 34, 'SDA', 'R21')):
        pin('U1', number, net, gpio)
        pin('U2', driver_number, net, driver_function)
        pin(resistor, 1, '+3V3')
        pin(resistor, 2, net)
        value(resistor, '2.2k')
        controls.append((net, str(number), gpio, 'U2.' + str(driver_number)))
    pin('U1', 12, 'LED_EN', 'PA5')
    pin('U2', 38, 'LED_EN', '~{SDB}')
    pin('R22', 1, 'LED_EN')
    pin('R22', 2, 'GND')
    value('R22', '100k')
    controls.append(('LED_EN', '12', 'PA5', 'U2.38'))
    for number, function, net, tp in ((24, 'PA13', 'SWDIO', 'TP6'),
                                       (25, 'PA14', 'SWCLK', 'TP7'),
                                       (6, 'NRST', 'NRST', 'TP8')):
        pin('U1', number, net, function)
        pin(tp, 1, net)
        controls.append((net, str(number), function, tp))
    pin('TP4', 1, '+3V3')
    pin('TP5', 1, 'GND')
    pin('U1', 4, '+3V3')
    pin('U1', 5, 'GND')
    for number in (19, 26, 31, 32):
        pin('U2', number, '+3V3')
    for number in (4, 11, 29, 40, 41):
        pin('U2', number, 'GND')
    pin('U2', 36, 'GND', 'ADDR')
    macro('IS31FL3737_ADDRESS', 0x50)
    rext_net = pins[('U2', '30')]['net']
    pin('U2', 30, rext_net, 'R_EXT')
    pin('R24', 1, rext_net)
    pin('R24', 2, 'GND')
    value('R24', '20k 1%')
    require(set(nets[rext_net]) == {('R24', '1'), ('U2', '30')}, 'REXT has unexpected connections')
    reset_net = pins[('U2', '39')]['net']
    pin('U2', 39, reset_net, 'IICRST')
    pin('R23', 1, reset_net)
    pin('R23', 2, 'GND')
    value('R23', '10k')
    require(set(nets[reset_net]) == {('R23', '1'), ('U2', '39')},
            'IICRST no longer has only its pull-down; revise firmware documentation')

    for number in (12, 13, 14, 27, 28, 33, 37):
        node = pins[('U2', str(number))]
        require('no_connect' in node['pintype'] and
                nets[node['net']] == [('U2', str(number))], f'U2.{number} must be unused')
        checks += 1
    cs_pads = [15, 16, 17, 18, 20, 21, 22, 23, 24, 25]
    sw_pads = [1, 2, 3, 5, 6, 7, 8, 9, 10]
    for column, number in enumerate(cs_pads, 1):
        pin('U2', number, f'LED_COL{column}', f'CS{column}')
    for row, number in enumerate(sw_pads, 1):
        pin('U2', number, f'LED_ROW{row}', f'SW{row}')
    for name, wanted in (('SNAKE_WIDTH', 10), ('SNAKE_HEIGHT', 9), ('SNAKE_CAPACITY', 90),
                         ('IS31FL3737_WIDTH', 10), ('IS31FL3737_HEIGHT', 9),
                         ('IS31FL3737_PIXELS', 90), ('IS31FL3737_MAX_GCC', 32)):
        macro(name, wanted)
    for name, wanted in (('SDB_PIN', 1 << 5), ('SCL_PIN', 1 << 6), ('SDA_PIN', 1 << 7),
                         ('BUS_PINS', (1 << 6) | (1 << 7)),
                         ('BUS_MODE_MASK', (3 << 12) | (3 << 14)),
                         ('BUS_AF_MODE', (2 << 12) | (2 << 14)),
                         ('SC_GPIOA', 0x50000000), ('SC_GPIOB', 0x50000400)):
        macro(name, wanted)
    xs = sorted({board[f'D{n}']['x'] for n in range(1, 91)})
    ys = sorted({board[f'D{n}']['y'] for n in range(1, 91)})
    require(len(xs) == 10 and len(ys) == 9, 'LED positions are not a 10 by 9 grid')
    leds = []
    for n in range(1, 91):
        ref = f'D{n}'
        x, y = (n - 1) % 10, (n - 1) // 10
        column_net, row_net = f'LED_COL{x + 1}', f'LED_ROW{y + 1}'
        pin(ref, 1, row_net, 'K')
        pin(ref, 2, column_net, 'A')
        fp = board[ref]
        require(fp['layer'] == 'F.Cu', f'{ref} is not on front')
        require(abs(fp['x'] - xs[x]) < 1e-6 and abs(fp['y'] - ys[y]) < 1e-6,
                f'{ref} physical position disagrees with front-view ({x},{y})')
        checks += 1
        leds.append((ref, x, y, fp['x'], fp['y'], column_net, row_net,
                     16 * y + (x if x < 6 else x + 2)))
    for column in range(1, 11):
        require(len(nets[f'LED_COL{column}']) == 10, 'Unexpected LED column fanout')
    for row in range(1, 10):
        require(len(nets[f'LED_ROW{row}']) == 11, 'Unexpected LED row fanout')
    return {'checks': checks, 'controls': controls, 'leds': leds, 'rext_net': rext_net,
            'iicrst_net': reset_net, 'xs': xs, 'ys': ys}


def audit_bsp_source(board_source, min_header):
    """Small source-contract guards; this deliberately is not a C parser."""
    def compact(text):
        return re.sub(r'\s+', '', re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S))

    source, header = compact(board_source), compact(min_header)

    def body(name):
        match = re.search(r'\b' + name + r'\([^)]*\)\{', source)
        # Whitespace removal joins the return type with the function name.
        if match is None:
            match = re.search(name + r'\([^)]*\)\{', source)
        require(match is not None, f'Missing BSP function {name}')
        start = match.end()
        depth = 1
        for index in range(start, len(source)):
            depth += (source[index] == '{') - (source[index] == '}')
            if depth == 0:
                return source[start:index]
        raise ContractError(f'Unterminated BSP function {name}')

    init = body('board_init')
    required = {
        'Buttons read PA0..PA4 active-low':
            (body('board_buttons'), 'return(uint8_t)(~SC_GPIO_IDR(SC_GPIOA)&UINT32_C(0x1f));'),
        'SDB callback drives port A':
            (body('board_sdb'), 'SC_GPIO_BSRR(SC_GPIOA)=high?SDB_PIN:SDB_PIN<<16;'),
        'SDB is preloaded low before configuration':
            (init, 'SC_GPIO_BSRR(SC_GPIOA)=SDB_PIN<<16;'),
        'I2C PB6/PB7 are open-drain':
            (init, 'SC_GPIO_OTYPER(SC_GPIOB)|=BUS_PINS;'),
        'I2C PB6/PB7 select AF6':
            (init, 'SC_GPIO_AFRL(SC_GPIOB)=(SC_GPIO_AFRL(SC_GPIOB)&~UINT32_C(0xff000000))|UINT32_C(0x66000000);'),
    }
    for name, (scope, snippet) in required.items():
        require(snippet in scope, f'BSP source contract changed: {name}; review intentional refactors')
    require('SC_GPIO_MODER(SC_GPIOA)' in init and
            init.index('SC_GPIO_BSRR(SC_GPIOA)=SDB_PIN<<16;') < init.index('SC_GPIO_MODER(SC_GPIOA)'),
            'SDB preload must precede GPIOA mode configuration')
    require('#defineSC_BIT(n)(UINT32_C(1)<<(n))' in header,
            'SC_BIT implementation changed; update the safe macro evaluator after review')
    return list(required)


def negative_controls(netlist, board, macros):
    scenarios = []
    n, b, m = copy.deepcopy(netlist), copy.deepcopy(board), dict(macros)
    n['pins'][('D7', '2')]['net'] = 'LED_COL6'
    scenarios.append(('LED anode on wrong column', n, b, m))
    n, b, m = copy.deepcopy(netlist), copy.deepcopy(board), dict(macros)
    b['D1']['x'], b['D10']['x'] = b['D10']['x'], b['D1']['x']
    scenarios.append(('Front corner LEDs physically swapped', n, b, m))
    n, b, m = copy.deepcopy(netlist), copy.deepcopy(board), dict(macros)
    m['BUTTON_UP'] = '(1u << 1)'
    scenarios.append(('Firmware UP assigned DOWN bit', n, b, m))
    n, b, m = copy.deepcopy(netlist), copy.deepcopy(board), dict(macros)
    n['values']['R24'] = '10k'
    scenarios.append(('Current resistor changed', n, b, m))
    n, b, m = copy.deepcopy(netlist), copy.deepcopy(board), dict(macros)
    m['IS31FL3737_MAX_GCC'] = '255u'
    scenarios.append(('Firmware GCC ceiling removed', n, b, m))
    caught = []
    for label, n, b, m in scenarios:
        try:
            audit(n, b, m)
        except ContractError:
            caught.append(label)
        else:
            raise ContractError(f'Negative control not detected: {label}')
    return caught


def write_report(path, result, negatives, inputs):
    lines = ['# Firmware / Rev B board contract audit', '',
             '**PASS** — XML connectivity, physical front orientation and firmware constants agree.', '',
             f'{result["checks"]} counted pin/value/macro/LED-position checks passed; '
             f'{len(negatives)} deliberate bad inputs were rejected.', '',
             'Reproduce from any directory: `python firmware/tools/check_board_contract.py` '
             '(adjust the script path to the project location). Uses Python standard library only.', '',
             '## Inputs', '', '| Input | SHA-256 |', '|---|---|']
    for item in inputs:
        try:
            label = item.resolve().relative_to(PROJECT).as_posix()
        except ValueError:
            label = item.as_posix()
        lines.append(f'| `{label}` | '
                     f'`{hashlib.sha256(item.read_bytes()).hexdigest()}` |')
    lines += ['', '## Control and debug connections', '',
              '| Exact net name | U1 pad | MCU function | Other endpoint |', '|---|---:|---|---|']
    for record in result['controls']:
        lines.append('| ' + ' | '.join(record) + ' |')
    lines += ['', 'Each button has a 10k pull-up to +3V3 and a switch to GND; pressed is low.',
              'I²C has 2.2k pull-ups. LED_EN has a 100k pull-down. TP4 is +3V3 and TP5 is GND.',
              'U2 ADDR is GND, matching address 0x50; R24 is 20k 1% to ground. '
              f'The exact R_EXT net is `{result["rext_net"]}`. '
              f'`{result["iicrst_net"]}` connects only U2 IICRST and its 10k pull-down.',
              'Unused CS11–12 / SW10–12 and SYNC/INTB are explicit unconnected pins.', '',
              '## BSP source guards', '',
              'Checks actual SDB/SCL/SDA mask macros and GPIO base constants in the BSP, '
              'plus these narrow source patterns (intentional refactoring requires reviewing the guard):', '']
    lines += ['- ' + item for item in result['bsp_guards']]
    lines += ['', 'The separate BSP review verifies register semantics and timing. These source '
              'guards verify board binding; they do not execute hardware register writes.', '',
              '## Physical matrix', '',
              'The checker derives sorted X/Y coordinates directly from the native PCB, '
              'then requires all 90 front-side footprints to match the row-major firmware index. '
              'Front-view X increases rightward; KiCad Y increases downward. '
              'LED pad 1 is cathode → SW row; pad 2 is anode → CS column. '
              'PCB pad net names are independently compared with the XML for every checked pin.', '',
              f'X positions (mm): `{result["xs"]}`', '', f'Y positions (mm): `{result["ys"]}`', '',
              '| LED | x,y | PCB X,Y mm | Anode net | Cathode net | PWM register |',
              '|---|---|---|---|---|---|']
    for ref, x, y, px, py, anode, cathode, reg in result['leds']:
        lines.append(f'| {ref} | {x},{y} | {px:g},{py:g} | {anode} | {cathode} | 0x{reg:02X} |')
    lines += ['', '## Negative controls', ''] + ['- ' + item for item in negatives]
    lines += ['', '## Scope', '',
              'Checks the exported design and firmware source constants. It does not measure the '
              'assembled board, prove copper connectivity (see KiCad DRC), validate a C preprocessor '
              'configuration, or replace functional host tests and hardware bring-up. '
              'The numerical PWM register column follows the reviewed IS31FL3737 map; the driver '
              'mock-bus tests independently exercise actual generated writes.', '']
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('\n'.join(lines), encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--netlist', type=Path, default=PROJECT / 'exports/SnakeBusinessCard.net.xml')
    parser.add_argument('--pcb', type=Path, default=PROJECT / 'SnakeBusinessCard.kicad_pcb')
    parser.add_argument('--report', type=Path, default=FIRMWARE / 'references/BOARD_CONTRACT_REPORT.md')
    args = parser.parse_args()
    headers = [FIRMWARE / 'include' / name for name in
               ('buttons.h', 'snake.h', 'is31fl3737.h', 'stm32g030_min.h')]
    bsp = FIRMWARE / 'src/board.c'
    macro_sources = [*headers, bsp]
    netlist, board, macros = load_netlist(args.netlist), load_board(args.pcb), macro_definitions(macro_sources)
    result = audit(netlist, board, macros)
    result['bsp_guards'] = audit_bsp_source(bsp.read_text(encoding='utf-8'),
                                         headers[-1].read_text(encoding='utf-8'))
    negatives = negative_controls(netlist, board, macros)
    write_report(args.report, result, negatives, [args.netlist, args.pcb, *macro_sources])
    print(f'PASS: {result["checks"]} board contract checks; 90 front LED positions; '
          f'{len(negatives)} negative controls rejected.')
    print(f'Report: {args.report}')


if __name__ == '__main__':
    try:
        main()
    except (ContractError, OSError, ET.ParseError, SyntaxError, ValueError, KeyError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        sys.exit(1)
