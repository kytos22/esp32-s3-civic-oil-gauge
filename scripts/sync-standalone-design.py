#!/usr/bin/env python3
"""Embed the canonical editable oil-gauge fragment in the Pages wrapper."""

from __future__ import annotations

import html
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
FRAGMENT = ROOT / "docs/design/references/oil-gauge-design.fragment.html"
STANDALONE = ROOT / "docs/design/references/oil-gauge-design.html"
START = "&lt;div id=&quot;oilGaugeRefinedIcons&quot;&gt;"
END = "\n\n&lt;script src=&quot;https://unpkg.com/@floating-ui/core"


def main() -> None:
    wrapper = STANDALONE.read_text(encoding="utf-8")
    start = wrapper.index(START)
    end = wrapper.index(END, start)
    escaped = html.escape(FRAGMENT.read_text(encoding="utf-8"), quote=True)
    STANDALONE.write_text(
        wrapper[:start] + escaped + wrapper[end:], encoding="utf-8"
    )


if __name__ == "__main__":
    main()
