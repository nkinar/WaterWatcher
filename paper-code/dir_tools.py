from pathlib import Path

def make_dir(dir):
    Path(dir).mkdir(parents=True, exist_ok=True)