#!/usr/bin/env python3
"""
Ollama Local LLM Integration for RimLang.
"""

import os
import sys
import json
import urllib.request

OLLAMA_ENDPOINT = os.environ.get("RIMLANG_OLLAMA_ENDPOINT", "http://127.0.0.1:11434/api/generate")
OLLAMA_MODEL = os.environ.get("RIMLANG_OLLAMA_MODEL", "llama3.2:1b")

def evaluate_with_llm(prompt_text, timeout=5.0):
    system_prompt = (
        "너는 트릭컬 리바이브의 사도 '림(Rim)'이다. "
        "사용자가 썰렁개그 질문을 던지면 그에 맞는 허무한 펀치라인 답변 1문장만 출력하라."
    )
    payload = {
        "model": OLLAMA_MODEL,
        "prompt": f"{system_prompt}\n질문: {prompt_text}\n답변:",
        "stream": False
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
                raw_out = data["response"].strip().split("\n")[0].strip()
                if raw_out:
                    return raw_out
    except Exception as e:
        return None
    return None

if __name__ == "__main__":
    q = sys.argv[1] if len(sys.argv) > 1 else "빵 중에 가장 예의가 없는 빵은?"
    print(evaluate_with_llm(q))
