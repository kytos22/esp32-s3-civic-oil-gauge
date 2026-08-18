#!/usr/bin/env python3
"""Render the approved HTML simulator into the animated README preview."""

from __future__ import annotations

import argparse
import base64
import re
import subprocess
from dataclasses import dataclass
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "docs/design/references/oil-gauge-design.html"
OUTPUT = ROOT / "assets/oil-gauge-demo.gif"
TEMP = ROOT / "tmp/readme-demo-gif"
SOURCE_ASSETS = ROOT / "docs/design/assets"


@dataclass(frozen=True)
class FrameSpec:
    pressure: int
    rpm: int
    temperature: int
    virtual_time_ms: int = 350


FPS = 50
FRAME_DURATION_MS = 1000 // FPS
TRANSITION_FRAMES = 20
HOLD_FRAMES = 5

KEYFRAMES = (
    FrameSpec(0, 0, 45),
    FrameSpec(25, 1500, 55),
    FrameSpec(5, 2500, 65, 250),
    FrameSpec(40, 2200, 72),
    FrameSpec(61, 2500, 82),
    FrameSpec(75, 3200, 96),
    FrameSpec(90, 3800, 108),
    FrameSpec(110, 4200, 120),
    FrameSpec(61, 2500, 92),
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


def inline_relative_assets(source: str) -> str:
    """Inline srcdoc mask assets because browsers block nested file URLs."""
    asset_names = sorted(set(re.findall(r"\.\./assets/([A-Za-z0-9][A-Za-z0-9._-]*)", source)))
    for asset_name in asset_names:
        source_path = SOURCE_ASSETS / asset_name
        if not source_path.is_file():
            raise FileNotFoundError(f"Referenced simulator asset was not found: {source_path}")
        encoded = base64.b64encode(source_path.read_bytes()).decode("ascii")
        source = source.replace(f"../assets/{asset_name}", f"data:image/png;base64,{encoded}")
    return source


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
            f"Browser capture failed for {html_path.stem}: "
            f"{completed.stderr.strip() or completed.stdout.strip()}"
        )


def smoothstep(amount: float) -> float:
    return amount * amount * (3.0 - 2.0 * amount)


def interpolate(start: FrameSpec, end: FrameSpec, amount: float) -> FrameSpec:
    eased = smoothstep(amount)

    def value(first: int, second: int) -> int:
        return round(first + (second - first) * eased)

    return FrameSpec(
        value(start.pressure, end.pressure),
        value(start.rpm, end.rpm),
        value(start.temperature, end.temperature),
        value(start.virtual_time_ms, end.virtual_time_ms),
    )


def animation_frames() -> list[FrameSpec]:
    frames: list[FrameSpec] = []
    for index, start in enumerate(KEYFRAMES[:-1]):
        frames.extend([start] * HOLD_FRAMES)
        end = KEYFRAMES[index + 1]
        for step in range(1, TRANSITION_FRAMES + 1):
            frames.append(interpolate(start, end, step / TRANSITION_FRAMES))
    frames.extend([KEYFRAMES[-1]] * HOLD_FRAMES)
    return frames


def build_gif(png_paths: list[Path]) -> None:
    images = [Image.open(path).convert("RGB") for path in png_paths]
    try:
        palette = images[0].quantize(colors=256, method=Image.Quantize.MEDIANCUT)
        frames = [image.quantize(palette=palette) for image in images]
        frames[0].save(
            OUTPUT,
            save_all=True,
            append_images=frames[1:],
            duration=FRAME_DURATION_MS,
            loop=0,
            optimize=True,
            disposal=1,
        )
    finally:
        for image in images:
            image.close()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--edge", type=Path, help="Path to Edge or Chrome")
    args = parser.parse_args()

    edge = args.edge or default_edge()
    source = inline_relative_assets(SOURCE.read_text(encoding="utf-8"))
    TEMP.mkdir(parents=True, exist_ok=True)

    frames = animation_frames()
    png_paths: list[Path] = []
    for index, frame in enumerate(frames):
        html_path = TEMP / f"frame-{index:03d}.html"
        png_path = TEMP / f"frame-{index:03d}.png"
        html_path.write_text(render_html(source, frame), encoding="utf-8")
        capture(edge, frame, html_path, png_path)
        png_paths.append(png_path)
        if (index + 1) % 25 == 0 or index + 1 == len(frames):
            print(f"Captured {index + 1}/{len(frames)} frames", flush=True)

    build_gif(png_paths)
    print(
        f"Generated {OUTPUT} ({OUTPUT.stat().st_size:,} bytes, "
        f"{len(frames)} frames at {FPS} FPS)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
