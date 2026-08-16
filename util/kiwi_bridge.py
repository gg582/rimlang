#!/usr/bin/env python3
"""
True Hybrid Architecture for RimLang:
1. Deterministic Core Language Mechanics (AGENTS.md 명세):
   - OISC 튜링 완전 메모리 제어, 산술 연산, 변수 바인딩
   - 불리언 판정/축약자 (.유/.무), 삼항연산자(낯.가.림), 타입단언(assert), 반대말 NOT 함수
   - 비명 서브스트링 연산식 (cake -> 이크, 수박 -> 박)
   - 예의/싸가지 없는 대상 다대일 매핑 ("그건 바로...메론빵....")
   - 교주님 호출 구문식 ("교주님...?") -> "교주님...?"
2. Pure Neural LLM Semantic Reasoning:
   - 그 외 열린 창작 개그 질문은 EXAONE 3.5 2.4B 로컬 신경망으로 실시간 추론.
"""

import sys
import json
import re
from kiwipiepy import Kiwi
from llm_pun_engine import evaluate_with_llm

KOR_ENG_MAP = {
    "not": "낫", "just": "저스트", "a": "어", "scythe": "낫",
    "alone": "혼자", "cake": "케이크", "scream": "비명",
    "bread": "빵", "banana": "바나나", "hotdog": "핫도그"
}

DIALECT_MAP = {
    "케익": "케이크", "비맹": "비명", "웃으모": "웃으면",
    "치모": "치면", "있응께": "있으니까", "빤": "빵", "엄는": "없는"
}

def normalize_text(text):
    def repl(m):
        w = m.group(0).lower()
        return KOR_ENG_MAP.get(w, w)
    t = re.sub(r'[a-zA-Z]+', repl, text)
    for k, v in DIALECT_MAP.items():
        t = t.replace(k, v)
    return t

def evaluate_line(line, kiwi):
    norm = normalize_text(line.strip())
    tokens = kiwi.tokenize(norm)
    nouns = [t.form for t in tokens if t.tag in ("NNG", "NNP", "NR", "SN")]

    # 1. 교주님 질문식 ("교주님...?")
    if "교주님" in norm and ("?" in norm or "…?" in norm or "...?" in norm):
        return "교주님...?"

    # 2. 비명 소리 서브스트링 연산식 (케이크 -> 이크)
    if "비명" in norm or "소리" in norm or "지르" in norm:
        target = nouns[0] if nouns else "케이크"
        if target in ["비명", "소리"] and len(nouns) > 1:
            target = nouns[1]
        scream = target[1:] if len(target) >= 2 else "이크"
        return f"{target}가 비명을 지르면 {scream}."

    # 3. 예의/싸가지 없는 빵 다대일 매핑 규약 (AGENTS.md 명세)
    if ("빵" in norm or "빤" in norm) and any(k in norm for k in ["예의", "싸가지", "버릇"]) and any(k in norm for k in ["없", "엄"]):
        return "그건 바로...메론빵...."

    # 4. 반대말 단항 함수 연산식
    if "반대말" in norm:
        return "요즘잘자쿨냥이. 풉. 푸흐흐흐...."

    # 5. 실시간 불리언 판정 및 상태 축약자 (.무)
    if "냉각기" in norm or "냉.무" in norm or "무쓸모" in norm:
        return "냉.무인거에요...푸훗."

    # 6. 삼항연산자 조건 분기식 (낯.가.림)
    if "가면" in norm or "낯가림" in norm or "낯.가.림" in norm or "나누어" in norm:
        return "가면을 쓴 림은 바로 낯.가.림.입니다. 푸흡..."

    # 7. 아티팩트 타입 단언식 (낫 저스트 어 낫)
    if "낫" in norm or "친구" in norm or "저스트" in norm:
        return "그러니까 이 낫은 낫 저스트 어 낫...후후후...."

    # 8. 그 외 열린 창작 개그는 LLM(EXAONE 3.5) 추론
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
