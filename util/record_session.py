#!/usr/bin/env python3
"""
Simulates natural interactive human typing into RimLang REPL,
capturing both Example 1 (Original) and Example 2 (Creative) sessions into rimlang.gif.
"""

import os
import time
import subprocess
import pty
import select
from PIL import Image, ImageDraw, ImageFont

FONT_PATHS = [
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/truetype/nanum/NanumGothic.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
]

def get_font(size=18):
    for fp in FONT_PATHS:
        if os.path.exists(fp):
            try:
                return ImageFont.truetype(fp, size)
            except Exception:
                continue
    return ImageFont.load_default()

def render_terminal_frame(lines, width=820, height=520):
    img = Image.new("RGB", (width, height), color="#1e1e2e")
    draw = ImageDraw.Draw(img)
    font = get_font(18)

    # Window title bar
    draw.rectangle([(0, 0), (width, 36)], fill="#181825")
    draw.ellipse([(14, 12), (24, 22)], fill="#f38ba8")
    draw.ellipse([(32, 12), (42, 22)], fill="#f9e2af")
    draw.ellipse([(50, 12), (60, 22)], fill="#a6e3a1")

    title_text = "림의 유머극장 (Hybrid NLP & LLM)"
    draw.text((width // 2 - 120, 8), title_text, fill="#a6adc8", font=font)

    y = 50
    line_height = 24
    max_visible_lines = (height - y) // line_height

    visible_lines = lines[-max_visible_lines:] if len(lines) > max_visible_lines else lines

    for line in visible_lines:
        if line.startswith("림>"):
            draw.text((20, y), line, fill="#89b4fa", font=font)
        elif any(k in line for k in ["푸흡", "후후후", "푸훗", "푸하하", "이크", "메론빵", "바나나킥", "카놀라유", "킹받네", "쿨냥이", "낯.가.림", "냉.무", "바로"]):
            draw.text((20, y), line, fill="#f5c2e7", font=font)
        elif "거짓" in line or "참" in line:
            draw.text((20, y), line, fill="#a6e3a1", font=font)
        else:
            draw.text((20, y), line, fill="#cdd6f4", font=font)
        y += line_height

    return img

def main():
    commands = [
        # 예제 1 (원작 질문식)
        "케이크가 지르는 비명은...?",
        "빵 중에 가장 예의가 없는 빵은?",
        "이제 그 냉각기가 있으니까...난 이제 냉각기로도 무쓸모가 된 거잖아요...?",
        "해골 가면을 쓴 림과, 가면을 안 쓴 림으로 나누어 게임을 했지요...?",
        "친구가 없을 때 낫이 혼자 나의 개그를 들어줬었거든요...?",
        "통모짜핫도그의 반대말은?",
        # 예제 2 (창작 및 방언 호환)
        "바나나가 웃으면...?",
        "차가 지나가다 사람을 치면...?",
        "왕이 바다에 빠지면...?",
        "끝."
    ]

    master, slave = pty.openpty()
    proc = subprocess.Popen(
        ["./rimlang"],
        stdin=slave,
        stdout=slave,
        stderr=slave,
        close_fds=True
    )
    os.close(slave)

    frames = []
    durations = []
    screen_lines = []
    current_line = ""

    def capture_frame(duration_ms=40):
        nonlocal current_line
        lines = list(screen_lines)
        if current_line:
            lines.append(current_line + "█")
        elif lines:
            lines[-1] = lines[-1] + "█"
        f = render_terminal_frame(lines)
        frames.append(f)
        durations.append(duration_ms)

    def read_output(timeout=0.15):
        nonlocal current_line
        updated = False
        while True:
            r, _, _ = select.select([master], [], [], timeout)
            if not r:
                break
            try:
                data = os.read(master, 1024).decode("utf-8", errors="ignore")
            except Exception:
                break
            if not data:
                break
            for ch in data:
                if ch == "\n":
                    screen_lines.append(current_line)
                    current_line = ""
                    updated = True
                elif ch == "\r":
                    pass
                else:
                    current_line += ch
            timeout = 0.03
        if updated:
            capture_frame(140)

    # Initial prompt read
    read_output(0.3)
    capture_frame(300)

    for cmd in commands:
        for ch in cmd:
            os.write(master, ch.encode("utf-8"))
            read_output(0.01)
            capture_frame(35)
            time.sleep(0.02)

        time.sleep(0.12)
        os.write(master, b"\n")
        read_output(0.35)
        capture_frame(600)
        time.sleep(0.15)

    try:
        proc.wait(timeout=1.0)
    except Exception:
        proc.terminate()

    os.close(master)

    if frames:
        out_path = "rimlang.gif"
        frames[0].save(
            out_path,
            save_all=True,
            append_images=frames[1:],
            optimize=True,
            duration=durations,
            loop=0
        )
        print(f"[SUCCESS] Generated {out_path} ({len(frames)} frames)")

if __name__ == "__main__":
    main()
