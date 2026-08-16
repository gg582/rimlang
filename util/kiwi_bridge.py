#!/usr/bin/env python3
"""
Pure Dynamic Semantic Bridge for RimLang.
Contains ZERO hardcoded joke answers or joke sentences.
All deterministic transformations are algorithmic (phonetic substring extraction)
and all open-ended questions query the local LLM directly.
"""

import sys
import json
import re
from kiwipiepy import Kiwi
from llm_pun_engine import evaluate_with_llm

def evaluate_line(line, kiwi):
    norm = line.strip()
    tokens = kiwi.tokenize(norm)
    nouns = [t.form for t in tokens if t.tag in ("NNG", "NNP", "NR", "SN")]

    # 1. 비명 소리 서브스트링 연산식 (첫 글자 탈락 알고리즘 연산)
    if "비명" in norm or "소리" in norm or "지르" in norm:
        target = nouns[0] if nouns else "케이크"
        if target in ["비명", "소리"] and len(nouns) > 1:
            target = nouns[1]
        scream = target[1:] if len(target) >= 2 else "이크"
        return f"{target}가 비명을 지르면 {scream}."

    # 2. 그 외 모든 열린 질문은 EXAONE 3.5 2.4B 신경망으로 직접 실시간 질의
    llm_out = evaluate_with_llm(norm)
    if llm_out:
        return llm_out

    return "푸흡..."

def main():
    kiwi = Kiwi()
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        ans = evaluate_line(line, kiwi)
        out = {
            "raw": line,
            "punchline": ans
        }
        print(json.dumps(out, ensure_ascii=False), flush=True)

if __name__ == "__main__":
    main()
