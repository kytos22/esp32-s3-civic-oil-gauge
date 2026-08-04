#!/usr/bin/env python3
"""Render the approved HTML simulator into the animated README preview."""

from __future__ import annotations

import argparse
import re
import subprocess
from dataclasses import dataclass
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "docs/design/references/oil-gauge-design.html"
OUTPUT = ROOT / "assets/oil-gauge-demo.gif"
TEMP = ROOT / "tmp/readme-demo-gif"


@dataclass(frozen=True)
class FrameSpec:
    name: str
    pressure: int
    rpm: int
    temperature: int
    duration_ms: int = 650
    virtual_time_ms: int = 350


FRAMES = (
    FrameSpec("stopped-cold", 0, 0, 45, 850),
    FrameSpec("running-cold", 25, 1500, 55),
    FrameSpec("low-pressure", 5, 2500, 65, 850, 250),
    FrameSpec("warming", 40, 2200, 72),
    FrameSpec("optimal", 61, 2500, 82, 850),
    FrameSpec("hot", 75, 3200, 96),
    FrameSpec("very-hot", 90, 3800, 108, 850),
    FrameSpec("high-pressure", 110, 4200, 120),
    FrameSpec("optimal-loop", 61, 2500, 92, 850),
)


def default_edge() -> Path:
    candidates = (
        Path(r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"),
        Path(r"C:\Program Files\Microsoft\Edge\Application\msedge.exe"),
        Path(r"C:\Program Files\Google\Chrome\Application\chrome.exe"),
    )
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise FileNotFoundError("Microsoft Edge or Google Chrome was not found")


def set_input(source: str, control_id: str, value: int) -> str:
    pattern = re.compile(
        rf"(id=&quot;{re.escape(control_id)}&quot;[^\n]*?value=&quot;)\d+(&quot;)"
    )
    updated, count = pattern.subn(rf"\g<1>{value}\g<2>", source, count=1)
    if count != 1:
        raise RuntimeError(f"Could not set {control_id}")
    return updated


def render_html(source: str, frame: FrameSpec) -> str:
    rendered = set_input(source, "ogrPressureInput", frame.pressure)
    rendered = set_input(rendered, "ogrRpmInput", frame.rpm)
    return set_input(rendered, "ogrTemperatureInput", frame.temperature)


def capture(edge: Path, frame: FrameSpec, html_path: Path, png_path: Path) -> None:
    command = [
        str(edge),
        "--headless=new",
        "--disable-background-networking",
        "--disable-default-apps",
        "--disable-extensions",
        "--disable-gpu",
        "--hide-scrollbars",
        "--run-all-compositor-stages-before-draw",
        "--force-device-scale-factor=1",
        "--window-size=736,700",
        f"--virtual-time-budget={frame.virtual_time_ms}",
        f"--screenshot={png_path}",
        html_path.as_uri(),
    ]
    completed = subprocess.run(command, capture_output=True, text=True, check=False)
    if completed.returncode != 0 or not png_path.exists():
        raise RuntimeError(
            f"Browser capture failed for {frame.name}: "
            f"{completed.stderr.strip() or completed.stdout.strip()}"
        )


def build_gif(png_paths: list[Path], durations: list[int]) -> None:
    images = [Image.open(path).convert("RGB") for path in png_paths]
    try:
        palette = images[0].quantize(colors=256, method=Image.Quantize.MEDIANCUT)
        frames = [image.quantize(palette=palette) for image in images]
        frames[0].save(
            OUTPUT,
            save_all=True,
            append_images=frames[1:],
            duration=durations,
            loop=0,
            optimize=True,
            disposal=2,
        )
    finally:
        for image in images:
            image.close()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--edge", type=Path, help="Path to Edge or Chrome")
    args = parser.parse_args()

    edge = args.edge or default_edge()
    source = SOURCE.read_text(encoding="utf-8")
    TEMP.mkdir(parents=True, exist_ok=True)

    png_paths: list[Path] = []
    durations: list[int] = []
    for index, frame in enumerate(FRAMES):
        html_path = TEMP / f"{index:02d}-{frame.name}.html"
        png_path = TEMP / f"{index:02d}-{frame.name}.png"
        html_path.write_text(render_html(source, frame), encoding="utf-8")
        capture(edge, frame, html_path, png_path)
        png_paths.append(png_path)
        durations.append(frame.duration_ms)

    build_gif(png_paths, durations)
    print(f"Generated {OUTPUT} ({OUTPUT.stat().st_size:,} bytes, {len(FRAMES)} frames)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
