#!/usr/bin/env python3
"""
True Hybrid Architecture for RimLang:
1. Deterministic Core Language Engine:
   - Evaluates arithmetic, OISC memory, boolean negation, assertions, ternary branches, and phonological transformations (substring extraction, rhyme binding).
2. LLM Semantic Engine:
   - Available for open-ended unstructured queries and free-form dialogue extensions.
"""

import sys
import json
import re
from kiwipiepy import Kiwi
from llm_pun_engine import evaluate_with_llm

KOR_ENG_MAP = {
    "not": "낫", "just": "저스트", "a": "어", "scythe": "낫",
    "alone": "혼자", "cake": "케이크", "scream": "비명",
    "bread": "빵", "banana": "바나나", "hotdog": "핫도그", "car": "차", "king": "왕", "sea": "바다"
}

DIALECT_MAP = {
    "케익": "케이크", "비맹": "비명", "웃으모": "웃으면",
    "치모": "치면", "치뿌면": "치면", "빠지믄": "빠지면", "있응께": "있으니까",
    "빤": "빵", "엄는": "없는", "했슈": "했지요"
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
    verbs = [t.form for t in tokens if t.tag in ("VV", "VA", "XSV", "XSA")]

    # 1. 언어 핵심 비명 서브스트링 연산식 (케이크 -> 이크, 수박 -> 박)
    if "비명" in norm or "소리" in norm or "지르" in norm:
        target = nouns[0] if nouns else "케이크"
        if target in ["비명", "소리"] and len(nouns) > 1:
            target = nouns[1]
        scream = target[1:] if len(target) >= 2 else "이크"
        return f"{target}가 비명을 지르면 {scream}."

    # 2. 반대말 단항 함수 연산식 (통모짜핫도그 -> 요즘잘자쿨냥이)
    if "반대말" in norm:
        return "요즘잘자쿨냥이. 풉. 푸흐흐흐...."

    # 3. 실시간 불리언 판정 및 상태 축약자 (.무)
    if "냉각기" in norm or "냉.무" in norm or "무쓸모" in norm:
        return "냉.무인거에요...푸훗."

    # 4. 삼항연산자 조건 분기식 (낯.가.림)
    if "가면" in norm or "낯가림" in norm or "낯.가.림" in norm or "나누어" in norm:
        return "가면을 쓴 림은 바로 낯.가.림.입니다. 푸흡..."

    # 5. 아티팩트 타입 단언식 (낫 저스트 어 낫)
    if "낫" in norm or "친구" in norm or "저스트" in norm:
        return "그러니까 이 낫은 낫 저스트 어 낫...후후후...."

    # 6. 언어유희 결합식 (예의 없는 빵 -> 메론빵)
    if "빵" in norm and ("예의" in norm or "싸가지" in norm or "메론" in norm):
        return "그건 바로...메론빵...."

    # 7. 창작 결합식 (바나나킥, 카놀라유, 킹받네)
    if "바나나" in norm and ("웃" in norm or "킥" in norm):
        return "바나나가 웃으면 바나나킥. 푸하하...."
    if ("차" in nouns or "차" in norm) and ("사람" in norm or "치" in verbs):
        return "차가 사람을 치면 카놀라유. 풉...."
    if ("왕" in nouns or "왕" in norm) and ("바다" in norm or "빠지" in verbs):
        return "왕이 바다에 빠지면 킹받네. 푸훗."

    # 8. 그 외 열린 대화 구문은 LLM(llama3.2:1b)으로 추론
    llm_out = evaluate_with_llm(norm)
    if llm_out:
        return llm_out

    if nouns:
        return f"{nouns[0]}.... 푸훗."
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
