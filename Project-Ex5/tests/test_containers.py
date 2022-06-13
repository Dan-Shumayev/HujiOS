import subprocess
from pathlib import Path
import sys

from typing import List

CONTAINER_EXE = Path(__file__).parent.parent / "build" / "container"  # {$PROJECT_ROOT_DIR}/build/container

IMAGE_DIR = Path(__file__).parent.parent / "image"  # {$PROJECT_ROOT_DIR}/image

if not CONTAINER_EXE.exists():
    print(f"Expected container executable at path {CONTAINER_EXE}", file=sys.stderr)
    sys.exit(1)

if not IMAGE_DIR.exists():
    print(f"Expected image directory at {IMAGE_DIR}", file=sys.stderr)
    sys.exit(1)


def run_container(
    new_hostname: str,
    image_dir: Path,
    num_proc: int,
    program_path: Path,
    program_args: List[str]
):
    args = [str(CONTAINER_EXE), new_hostname, str(
        image_dir), str(num_proc), str(program_path)] + program_args

    return subprocess.run(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                         universal_newlines=True)


def test_hostname():
    res = run_container(new_hostname="testHostName", image_dir=IMAGE_DIR,
                  num_proc=5, program_path=Path("/bin/hostname"), program_args=[])

    assert res.returncode == 0
    assert res.stdout.strip() == "testHostName"

def test_program_args():
    res = run_container(new_hostname="testHostName", image_dir=IMAGE_DIR,
                  num_proc=5, program_path=Path("/bin/bash"), program_args=[ "--help"])

    assert res.returncode == 0
    assert "GNU bash, version" in res.stdout

def test_max_processes():
    res = run_container(new_hostname="testHostName", image_dir=IMAGE_DIR,
                        num_proc=2, program_path=Path("/bin/bash"), program_args=["-c", "sleep 3 | sleep 3"])

    assert res.returncode == 0
    assert "No child processes" in res.stderr
