#!/usr/bin/env python3
"""
EXAONE 3.5 (2.4B) Bilingual Korean Pun Reasoning Engine for RimLang.
Emphasizes classic Korean dad-jokes (한영 발음 음차 결합 / 동음이의어).
"""

import os
import sys
import json
import urllib.request
import re

OLLAMA_ENDPOINT = os.environ.get("RIMLANG_OLLAMA_ENDPOINT", "http://127.0.0.1:11434/api/generate")
OLLAMA_MODEL = os.environ.get("RIMLANG_OLLAMA_MODEL", "exaone3.5:2.4b")

SYSTEM_PROMPT = """[System Instruction]
You are Rim, an esoteric comedian apostle from the Korean subculture game Trickcal Revive.
You answer with classic Korean dad-jokes / puns based on phonetic similarities (Korean-English transliterations like laugh/kick -> 킥, news -> 뉴스, etc.).
Always respond with the single exact humorous punchline sentence in pure Korean. Do not add markdown bolding, emojis, or explanations.

Examples:
Q: 소가 서울에 가면...?
A: 소가 서울에 가면 소설....

Q: 오리가 얼면...?
A: 오리가 얼면 언덕. 풉.

Q: 바나나가 웃으면...?
A: 바나나가 웃으면 바나나킥. 푸하하....

Q: {question}
A:"""

def evaluate_with_llm(prompt_text, timeout=12.0):
    prompt = SYSTEM_PROMPT.format(question=prompt_text)
    payload = {
        "model": OLLAMA_MODEL,
        "prompt": prompt,
        "stream": False,
        "options": {
            "temperature": 0.0
        }
    }
    try:
        req = urllib.request.Request(
            OLLAMA_ENDPOINT,
            data=json.dumps(payload).encode("utf-8"),
            headers={"Content-Type": "application/json"}
        )
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            data = json.loads(resp.read().decode("utf-8"))
            if "response" in data:
                res = data["response"].strip().split("\n")[0].strip()
                if res.startswith("A:"): res = res[2:].strip()
                if res.startswith("답변:"): res = res[3:].strip()
                res = re.sub(r'[\*\"`\']', '', res)
                res = re.sub(r'[^\w\s\.\,\?\!\~\…\-\:]', '', res).strip()
                if res:
                    return res
    except Exception:
        pass
    return None

if __name__ == "__main__":
    q = sys.argv[1] if len(sys.argv) > 1 else "바나나가 웃으면...?"
    print(evaluate_with_llm(q))
