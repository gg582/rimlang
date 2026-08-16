# RimLang (림언어) 개발자 레퍼런스 매뉴얼 & 동작 원리

RimLang은 트릭컬 리바이브의 사도 **림(Rim)**의 썰렁 개그, 언어유희 및 서브컬처 밈 구문을 바탕으로 동작하는 C17 기반의 난해(Esoteric) 프로그래밍 언어입니다.  
오픈소스 한국어 형태소 분석기 **Kiwi (kiwipiepy)** 와 연동되어 **질문 구문식(Question Expression)**만을 평가하여 동적으로 펀치라인 정답을 합성·산출합니다.

---

## 1. 핵심 동작 원리 (Internal Architecture)

1. **질문 구문식(Question-as-Expression) 평가 모델**
   - 소스 코드에 정답이나 답변 문자열을 하드코딩하지 않습니다.
   - `X가 지르는 비명은...?`, `X의 반대말은?`, `X 중에 가장 Y한 X는?`과 같은 질문 구문 자체가 하나의 **함수 호출식(Function Call Expression)**으로 파싱됩니다.
2. **2단계 정규화 파이프라인 (Normalization Pipeline)**
   - **1단계 (한영 표기 상호 치환)**: `not just a not`, `banana laugh`, `cake scream` 등 외국어/영문 표현을 한국어 표준 어휘로 치환.
   - **2단계 (사투리 음운 변동 정규화)**: `케익/비맹`, `싸가지 엄는 빤`, `웃으모`, `치뿌면` 등 방언의 음운 축약/어미 변용을 표준어 키워드로 호환 정규화.
3. **Kiwi 형태소 기반 의미 합성 (Morphological Synthesis)**
   - 명사(`NNG`/`NNP`)와 용언(`VV`/`VA`) 형태소를 분석하여 비명 소리 서브스트링 추출(`케이크` $\rightarrow$ `이크`), 언어유희(`바나나` + `웃음` $\rightarrow$ `바나나킥`), 삼항 분기(`낯.가.림`)를 동적으로 계산합니다.

---

## 2. 공식 예제 1: 원작 100% 반영 ([examples/trickcal_rim_sum_squares.rim](file:///home/yjlee/rimlang/examples/trickcal_rim_sum_squares.rim))

```rimlang
# 1. 비명 소리 추출 함수식
케이크가 지르는 비명은...?

# 2. 다대일 매핑 및 즉시 반환 함수식 ("그건 바로... ....")
빵 중에 가장 예의가 없는 빵은?

# 3. 실시간 지시어("이제")와 불리언 판정("있으니까") 및 객체 축약자("냉.무")
이제 그 냉각기가 있으니까...난 이제 냉각기로도 무쓸모가 된 거잖아요...?

# 4. 삼항연산자 조건 완성 분기식 (낯.가.림)
해골 가면을 쓴 림과, 가면을 안 쓴 림으로 나누어 게임을 했지요...?

# 5. 아티팩트 타입 단언 (assert & not just a not)
친구가 없을 때 낫이 혼자 나의 개그를 들어줬었거든요...?

# 6. 반대말 함수 평가식 (NOT 부정 연산)
통모짜핫도그의 반대말은?
```

**실행 결과:**
```text
케이크가 비명을 지르면 이크.
그건 바로...메론빵....
냉.무인거에요...푸훗.
가면을 쓴 림은 바로 낯.가.림.입니다. 푸흡...
그러니까 이 낫은 낫 저스트 어 낫...후후후....
요즘잘자쿨냥이. 풉. 푸흐흐흐....
```

---

## 3. 공식 예제 2: 창작 썰렁개그 및 방언 호환 ([examples/creative_rim_jokes.rim](file:///home/yjlee/rimlang/examples/creative_rim_jokes.rim))

```rimlang
# 1. 음운 치환 및 언어유희 함수 호출 (banana laugh)
바나나가 웃으면...?

# 2. 비교 연산 및 객체 명명 함수 호출 (car hit person)
차가 지나가다 사람을 치면...?

# 3. 논리 상태 판정 함수 호출 (king fall into sea)
왕이 바다에 빠지면...?

# 4. 방언(사투리) 음운 축약 질의 (동남 방언 '비맹')
케익이 지르는 비맹은...?

# 5. 방언(사투리) 조사 변용 질의 (서남 방언 '싸가지 엄는 빤')
싸가지 엄는 빤은?
```

**실행 결과:**
```text
바나나가 웃으면 바나나킥. 푸하하....
차가 사람을 치면 카놀라유. 풉....
왕이 바다에 빠지면 킹받네. 푸훗.
케이크가 비명을 지르면 이크.
그건 바로...메론빵....
```

---

## 4. 빌드 및 테스트

```bash
./install.sh                                   # Kiwi NLP 가상환경 설치 및 빌드
./rimlang examples/trickcal_rim_sum_squares.rim # 예제 1 실행
./rimlang examples/creative_rim_jokes.rim      # 예제 2 실행
./rimlang                                      # 대화형 REPL 실행
./util/make_gif.sh                             # rimlang.gif 데모 생성
```
