#!/usr/bin/env python3
"""
EXAONE 3.5 (2.4B) Bilingual Korean Pun Reasoning Engine for RimLang.
Zero-hardcoding, purely prompt-instructed LLM oracle bridge.
"""

import os
import sys
import json
import urllib.request
import re

OLLAMA_ENDPOINT = os.environ.get("RIMLANG_OLLAMA_ENDPOINT", "http://127.0.0.1:11434/api/generate")
OLLAMA_MODEL = os.environ.get("RIMLANG_OLLAMA_MODEL", "exaone3.5:2.4b")

SYSTEM_PROMPT = """당신은 한국어 썰렁개그와 언어유희를 생성하는 AI입니다.
주어진 질문에 대해 한국어 단어의 동음이의어나 연관 언어유희를 활용하여 완성된 문장 한 줄만 응답하세요. 설명이나 부연은 절대 하지 마세요.

질문: {question}
대답:"""

def evaluate_with_llm(prompt_text, timeout=12.0):
    prompt = SYSTEM_PROMPT.format(question=prompt_text)
    payload = {
        "model": OLLAMA_MODEL,
        "prompt": prompt,
        "stream": False,
        "options": {
            "temperature": 0.3,
            "stop": ["\n", "질문:"]
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
            ans = data.get("response", "").strip()
            ans = re.sub(r'^(대답:|\s*[\"\'])', '', ans).strip()
            ans = re.sub(r'[\"\']$', '', ans).strip()
            return ans
    except Exception:
        return ""

if __name__ == "__main__":
    if len(sys.argv) > 1:
        q = sys.argv[1]
        print(evaluate_with_llm(q))
