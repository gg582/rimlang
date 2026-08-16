#!/usr/bin/env bash
# ==============================================================================
# RimLang: Environment and Kiwi NLP Installer
# Creates a Python virtualenv and installs kiwipiepy & pillow dependencies.
# ==============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEFAULT_LANG_DIR="${SCRIPT_DIR}/lang"

echo "=================================================="
echo "   RimLang 환경 및 Kiwi 형태소 분석기 설치 마법사"
echo "=================================================="
echo ""

# Ask user for installation directory with default prompt
read -r -p "설치할 디렉터리 경로를 입력하세요 [기본값: ${DEFAULT_LANG_DIR}]: " USER_INPUT_DIR

if [ -z "${USER_INPUT_DIR}" ]; then
    TARGET_DIR="${DEFAULT_LANG_DIR}"
else
    # Resolve relative path or absolute path
    if [[ "${USER_INPUT_DIR}" = /* ]]; then
        TARGET_DIR="${USER_INPUT_DIR}"
    else
        TARGET_DIR="${SCRIPT_DIR}/${USER_INPUT_DIR}"
    fi
fi

echo ""
echo "-> 설치 대상 디렉터리: ${TARGET_DIR}"
mkdir -p "${TARGET_DIR}"

VENV_DIR="${TARGET_DIR}/venv"

echo "[1/4] Python3 가상환경(venv) 생성 중: ${VENV_DIR}..."
if [ ! -d "${VENV_DIR}" ]; then
    python3 -m venv "${VENV_DIR}"
fi

echo "[2/4] 가상환경 활성화 및 최신 pip 업그레이드..."
# shellcheck source=/dev/null
source "${VENV_DIR}/bin/activate"
pip install --upgrade pip -q

echo "[3/4] 오픈소스 한국어 형태소 분석기 (Kiwi) 및 의존성 설치 중..."
pip install kiwipiepy pillow -q

echo "[4/4] RimLang C 엔진 빌드 중..."
cd "${SCRIPT_DIR}"
make clean -s
make -s all

echo ""
echo "=================================================="
echo "   RimLang 및 Kiwi NLP 설치가 성공적으로 완료되었습니다!"
echo "=================================================="
echo "가상환경 활성화 명령: source ${VENV_DIR}/bin/activate"
echo "인터프리터 실행: ./rimlang (또는 ./rimlang -j 4)"
echo "GIF 녹화 실행: ./util/make_gif.sh"
echo "=================================================="
