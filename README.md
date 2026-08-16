# RimLang (림언어)

[![C17 Standard](https://img.shields.io/badge/C-C17-blue.svg)](https://en.wikipedia.org/wiki/C17_(C_standard_revision))
[![NLP Engine](https://img.shields.io/badge/NLP-Kiwi%20Korean%20Morphology-green.svg)](https://github.com/bab2min/kiwipiepy)
[![LLM Support](https://img.shields.io/badge/LLM-Ollama%20(Llama%203.2%201B)-orange.svg)](https://ollama.com)

**RimLang**은 서브컬처 게임 *트릭컬 리바이브(Trickcal Revive)*의 사도 **림(Rim)**의 썰렁개그, 언어유희, 서브컬처 밈 구문을 바탕으로 동작하는 난해(Esoteric) 프로그래밍 언어입니다.

C17 튜링 완전 가상머신, 오픈소스 한국어 형태소 분석기 **Kiwi**, 그리고 경량 1B LLM(**Llama 3.2 1B**)이 결합된 **하이브리드(Hybrid) 아키텍처**로 동작합니다.

---

## 📸 실행 화면 (림의 유머극장)

![RimLang REPL Demo](rimlang.gif)

---

## 🛠️ 빌드 및 설치 방법 (Build & Installation)

### 1. 사전 요구사항 (Prerequisites)
- **C 컴파일러**: GCC (C17 지원) 또는 Clang
- **Python**: Python 3.8 이상
- **빌드 도구**: `make` (또는 `cmake`)
- **선택 사항 (LLM 연동 시)**: [Ollama](https://ollama.com) 및 `llama3.2:1b` 모델

### 2. 원클릭 가상환경 설치 마법사 (`install.sh`)
가상환경 생성부터 형태소 분석기(`kiwipiepy`), 이미지 렌더링 라이브러리(`pillow`) 설치 및 C 인터프리터 빌드까지 한 번에 완료됩니다.

```bash
# 실행 권한 부여 후 설치 스크립트 실행
chmod +x install.sh
./install.sh
```
> 설치 경로 프롬프트를 묻습니다. 기본값(`lang/`)을 사용하려면 Enter 키를 누르세요.

### 3. 수동 빌드 (Manual Build)
```bash
# 1. C 인터프리터 및 테스트 러너 빌드
make all

# 2. 단위 테스트 실행
make test

# 3. 빌드 산출물 정리
make clean
```

---

## 🚀 실행 방법 (Usage)

### 1. 대화형 인터프리터 (Interactive REPL)
```bash
./rimlang
```

### 2. 멀티프로세스 비동기 워커 모드 (-j 옵션)
```bash
./rimlang -j 4
```

### 3. 소스 파일 배치 실행
```bash
# 공식 예제 1: 원작 100% 반영 질문식 실행
./rimlang examples/trickcal_rim_sum_squares.rim

# 공식 예제 2: 창작 썰렁개그 및 방언 호환 예제 실행
./rimlang examples/creative_rim_jokes.rim
```

### 4. GIF 애니메이션 데모 생성
```bash
./util/make_gif.sh
```

---

## 🧠 핵심 문법 및 동작 원리

림언어는 소스 코드에 답변을 적지 않고, **질문 구문식(Question Expression)**만을 평가하여 동적으로 펀치라인을 산출합니다.

```
[ RimLang 런타임 입력 ]
          │
          ├────────────────────────────────────────────────────────┐
          ▼                                                        ▼
[ 1. 결정론적 언어 엔진 (C17 + Kiwi NLP) ]        [ 2. 경량 1B LLM 추론 (llama3.2:1b) ]
• OISC 튜링 완전 메모리 (SBN, FlipJump)           • 사전 정의되지 않은 임의의 열린 질의 처리
• 산술 및 변수 할당, 타입 단언 (assert)          • 자유 대화 및 창작 썰렁개그 확장 해석
• 불리언 상태 축약자 (.유 / .무)
• 삼항 조건 분기식 (낯.가.림)
• 형태소 음운 서브스트링 연산 (케이크 -> 이크)
• 한영 표기 상호 치환 및 사투리 음운 정규화
```

### 주요 문법 규약

| 문법 패턴 | 설명 | 실행 예시 | 산출 결과 |
|---|---|---|---|
| `X가 지르는 비명은...?` | 비명 소리 추출 함수 (첫 음절 탈락 서브스트링) | `케이크가 지르는 비명은...?` | `케이크가 비명을 지르면 이크.` |
| `X 중에 가장 예의가 없는 X는?` | 다대일 매핑 & 4개 점(....) 종결 | `빵 중에 가장 예의가 없는 빵은?` | `그건 바로...메론빵....` |
| `X의 반대말은?` | NOT 단항 부정 함수 | `통모짜핫도그의 반대말은?` | `요즘잘자쿨냥이. 풉. 푸흐흐흐....` |
| `이제 ... 있으니까 ...` | 실시간 처리(`이제`) & 불리언 축약자(`.무`) | `이제 그 냉각기가 있으니까...` | `냉.무인거에요...푸훗.` |
| `... 나누어 ... 낯.가.림 ...` | 삼항연산자 조건 완성 분기 | `해골 가면을 쓴 림과...` | `가면을 쓴 림은 바로 낯.가.림.입니다. 푸흡...` |
| `친구가 없을 때 ... not just a not` | 구체 타입 단언 (assert & 한영 치환) | `친구가 없을 때 낫이...` | `그러니까 이 낫은 낫 저스트 어 낫...후후후....` |

---

## 📁 프로젝트 구조

```
.
├── include/rim/         # C17 헤더 파일 (AST, Parser, Lexer, Runtime, Engine, NLP)
├── src/
│   ├── main.c          # REPL 및 CLI 진입점
│   ├── engine/         # 멀티프로세스 비동기 워커 풀
│   ├── parser/         # 렉서, 파서, Kiwi NLP IPC 바인딩
│   └── runtime/        # 튜링 완전 OISC VM 및 런타임
├── examples/           # 공식 및 창작 림언어 소스 (.rim)
│   ├── trickcal_rim_sum_squares.rim  # 예제 1 (원작 100% 반영)
│   └── creative_rim_jokes.rim        # 예제 2 (창작 및 방언 호환)
├── util/               # Python NLP/LLM 브리지 및 GIF 렌더러
│   ├── kiwi_bridge.py  # Kiwi 형태소 및 음운 정규화 브리지
│   ├── llm_pun_engine.py # Ollama 경량 모델 연동 모듈
│   └── make_gif.sh     # 터미널 세션 GIF 녹화 스크립트
├── tests/              # C 단위 테스트 스위트
├── install.sh          # 대화형 가상환경 및 종속성 설치기
├── Makefile            # C17 컴파일 빌드 스크립트
├── MANUAL.md           # 상세 언어 레퍼런스 매뉴얼
└── README.md           # 프로젝트 소개 및 빌드 가이드
```

---

## 📄 라이선스
MIT License
