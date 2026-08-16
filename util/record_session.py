#!/usr/bin/env python3
"""
Simulates natural interactive human typing into RimLang REPL,
capturing modern RimLang syntax (User Defined Functions, QA Pairs, Generic Delimiters) into rimlang.gif.
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

def render_terminal_frame(lines, width=860, height=540):
    img = Image.new("RGB", (width, height), color="#1e1e2e")
    draw = ImageDraw.Draw(img)
    font = get_font(18)

    # Window title bar
    draw.rectangle([(0, 0), (width, 36)], fill="#181825")
    draw.ellipse([(14, 12), (24, 22)], fill="#f38ba8")
    draw.ellipse([(32, 12), (42, 22)], fill="#f9e2af")
    draw.ellipse([(50, 12), (60, 22)], fill="#a6e3a1")

    title_text = "사도 림의 유머극장 (C17 & Kiwi & EXAONE 3.5)"
    draw.text((width // 2 - 160, 8), title_text, fill="#a6adc8", font=font)

    y = 50
    line_height = 24
    max_visible_lines = (height - y) // line_height

    visible_lines = lines[-max_visible_lines:] if len(lines) > max_visible_lines else lines

    for line in visible_lines:
        if line.startswith("림>"):
            draw.text((20, y), line, fill="#89b4fa", font=font)
        elif any(k in line for k in ["푸흡", "후후후", "푸훗", "푸하하", "이크", "바나나킥", "쿨냥이", "낯.가.림", "냉.무", "바로"]):
            draw.text((20, y), line, fill="#f5c2e7", font=font)
        elif "거짓" in line or "참" in line or "[결산완료]" in line:
            draw.text((20, y), line, fill="#a6e3a1", font=font)
        else:
            draw.text((20, y), line, fill="#cdd6f4", font=font)
        y += line_height

    return img

def main():
    commands = [
        # 1. 사용자 함수 정의 블록 (림하하... ~ 하하...림...)
        "림하하...",
        "통모짜핫도그",
        "통모짜핫도그의 반대말은?",
        "요즘잘자쿨냥이. 풉. 푸흐흐흐....",
        "하하...림...",
        # 2. 함수 호출
        "통모짜핫도그",
        # 3. 비명 소리 추출 문답
        "케이크가 지르는 비명은...?",
        "케이크가 비명을 지르면 이크.",
        # 4. 접두 구분자 기반 축약자 (.....)
        "이제 .....냉각기 .....무쓸모인거에요...푸훗.",
        # 5. 삼항연산자 조건 완성 분기
        "해골 가면을 쓴 림과, 가면을 안 쓴 림으로 나누어 게임을 했지요.",
        "가면을 쓴 림은 바로 낯.가.림.입니다. 푸흡...",
        # 6. 세션 종료
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
            capture_frame(30)
            time.sleep(0.015)

        time.sleep(0.1)
        os.write(master, b"\n")
        read_output(0.3)
        capture_frame(500)
        time.sleep(0.12)

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
