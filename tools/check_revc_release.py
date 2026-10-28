"""Check published Rev C artifact integrity, revision links and release limits.

Uses the Python standard library. This does not run Altium or qualify hardware.
"""
from pathlib import Path
import hashlib
import json
import re
import sys
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / 'verification/revc/PUBLICATION_MANIFEST.json'


def check():
    errors = []
    manifest = json.loads(MANIFEST.read_text(encoding='utf-8'))
    seen = set()
    machine_count = 0
    for row in manifest['files']:
        name = row['path']
        path = (ROOT/name).resolve()
        if name in seen or ROOT not in path.parents:
            errors.append(f'Invalid or duplicate manifest path: {name}')
            continue
        seen.add(name)
        if not path.is_file():
            errors.append(f'Missing artifact: {name}')
            continue
        data = path.read_bytes()
        digest = hashlib.sha256(data).hexdigest()
        if digest != row['sha256'] or len(data) != row['bytes']:
            errors.append(f'Artifact changed without a manifest update: {name}')
        if name.startswith('fabrication/revc/') and path.suffix.upper() in {
                '.GTL','.GBL','.GTO','.GBO','.GTS','.GBS','.GTP','.GBP','.GM','.TXT'}:
            machine_count += 1
            if digest != row['verified_original_sha256']:
                errors.append(f'Machine data differs from reviewed native export: {name}')
    if machine_count != 11:
        errors.append(f'Expected 11 Rev C machine files, found {machine_count}')

    status = json.loads((ROOT/'verification/revc/FINAL_CHECK_STATUS.json').read_text())
    expected_tests = {f'P{n:02}':'NOT TESTED' for n in range(1,15)}
    if status['physical_tests'] != expected_tests:
        errors.append('Physical qualification disposition changed; review evidence and this gate together')
    if status['native_pcb_drc']['frozen_rule_records'] != 43:
        errors.append('Unexpected native rule count')

    project = (ROOT/'altium/SnakeCard_RevC.PrjPcb').read_text(encoding='cp1252')
    documents = re.findall(r'^DocumentPath=(.*)$', project, re.M)
    if len(documents) != 4:
        errors.append('Public project must contain exactly four source/library documents')
    for name in documents:
        if not (ROOT/'altium'/name.strip()).is_file():
            errors.append(f'Unresolved Altium source document: {name.strip()}')

    # Check repository Markdown, including historical guides, without parsing
    # example snippets as links. External URLs and fragment-only links are skipped.
    link_count = 0
    for path in ROOT.rglob('*.md'):
        if any(part in {'.git','build','toolchain','__pycache__'} for part in path.relative_to(ROOT).parts):
            continue
        text = path.read_text(encoding='utf-8-sig')
        text = re.sub(r'```.*?```','',text,flags=re.S)
        for target in re.findall(r'\[[^\]\n]*\]\(([^)\n]+)\)',text):
            target = target.strip().split(' "')[0].strip('<>')
            if re.match(r'[a-z][a-z0-9+.-]*:',target,re.I) or target.startswith('#'):
                continue
            local = unquote(target.split('#')[0])
            if not local:
                continue
            link_count += 1
            resolved = (path.parent/local).resolve()
            if ROOT not in (resolved,*resolved.parents) or not resolved.exists():
                errors.append(f'Broken local link in {path.relative_to(ROOT).as_posix()}: {target}')

    if errors:
        for error in errors:
            print('ERROR: '+error, file=sys.stderr)
        return 1
    print(f'PASS: {len(seen)} manifest artifacts, 11 native machine files, four project documents, {link_count} local links.')
    print('Scope: publication integrity; native checks are recorded evidence and physical tests remain NOT TESTED.')
    return 0


if __name__ == '__main__':
    raise SystemExit(check())
