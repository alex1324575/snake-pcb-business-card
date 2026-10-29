"""Verify the gameplay demo's source binding and an optional fresh C capture.

The optional capture is JSONL from tools/demo/record_gameplay.c, linked against
the production C modules. This check does not emulate the firmware in Python.
"""
from pathlib import Path
import argparse
import hashlib
import json

ROOT=Path(__file__).resolve().parents[1]


def require(condition,message):
    if not condition:
        raise ValueError(message)


def verify(capture=None):
    report=json.loads((ROOT/'verification/revc/GAMEPLAY_DEMO.json').read_text())
    trace_path=ROOT/report['trace']['path']
    trace=json.loads(trace_path.read_text())
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    require(sha(trace_path)==report['trace']['sha256'],'Gameplay trace hash mismatch')
    require(report['gameplay']==trace['summary'],'Gameplay summary mismatch')
    require(trace['hardware_tested'] is False,'Unexpected hardware qualification claim')
    for name,digest in trace['source_sha256'].items():
        require(sha(ROOT/name)==digest,'Firmware changed: '+name)
    require(report['firmware_source_sha256']==trace['source_sha256'],'Firmware binding mismatch')
    recorder=ROOT/report['recorder']['path']
    require(sha(recorder)==report['recorder']['sha256']==trace['recorder_sha256'],'Recorder source mismatch')
    artifact=report['artifact'];path=ROOT/artifact['path']
    require(sha(path)==artifact['sha256'],'Gameplay GIF hash mismatch')
    require(path.stat().st_size==artifact['bytes'],'Gameplay GIF size mismatch')
    frames=trace['frames'];interval=trace['frame_interval_ms']
    require([f['time_ms'] for f in frames]==list(range(0,trace['summary']['end_ms']+1,interval)),'Capture timing mismatch')
    require(len(frames)*interval==artifact['duration_ms'],'GIF duration mismatch')
    require(all(len(f['pixels'])==90 and set(f['pixels'])<={0,24,80,180,255} for f in frames),'Invalid LED output frame')
    require(trace['summary']['valid'] and trace['summary']['pickups']==4 and trace['summary']['score']==4,'Unexpected demo result')
    require(trace['summary']['length']==7 and trace['summary']['wall_collision'],'Missing growth or wall collision')
    require({'splash','ready','playing','game_over'}<={f['state'] for f in frames},'Missing demonstration state')
    if capture:
        actual=[json.loads(line) for line in Path(capture).read_text().splitlines() if line.strip()]
        require(actual==frames,'Fresh C firmware capture differs from the published trace')
    print(f'PASS: {len(frames)} actual C firmware samples, four food pickups, length 3 to 7, wall collision and final score 04.')
    print('Source and GIF hashes verified'+('; fresh C capture matches exactly.' if capture else '.'))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture',type=Path,help='Fresh JSONL output from the host C recorder, seed 25')
    args=parser.parse_args()
    verify(args.capture)
