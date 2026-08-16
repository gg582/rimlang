import subprocess
import pty
import os
import time
import select

master, slave = pty.openpty()
proc = subprocess.Popen(["./rimlang"], stdin=slave, stdout=slave, stderr=slave, close_fds=True)
os.close(slave)

def send_and_read(cmd):
    os.write(master, (cmd + "\n").encode("utf-8"))
    time.sleep(0.15)
    res = ""
    while True:
        r, _, _ = select.select([master], [], [], 0.1)
        if not r:
            break
        data = os.read(master, 1024).decode("utf-8", errors="ignore")
        if not data:
            break
        res += data
    print(f"INPUT: {cmd}")
    print(f"OUTPUT: {repr(res)}")

print("=== 1. 예의 없는 대상 동적 합성 (빵 -> 메론빵, 떡 -> 메롱떡, 과자 -> 메롱과자) ===")
send_and_read("빵 중에 가장 예의가 없는 빵은?")
send_and_read("떡 중에 가장 예의가 없는 떡은?")
send_and_read("과자 중에 가장 싸가지가 없는 과자는?")

print("\n=== 2. 비명 소리 서브스트링 동적 합성 (케이크 -> 이크, 바나나 -> 나나, 수박 -> 박) ===")
send_and_read("케이크가 지르는 비명은...?")
send_and_read("바나나가 지르는 비명은...?")
send_and_read("수박이 지르는 비명은...?")

print("\n=== 3. 웃음/킥 동적 합성 (바나나 -> 바나나킥, 감자 -> 감자킥) ===")
send_and_read("바나나가 웃으면...?")
send_and_read("감자가 웃으면...?")

proc.terminate()
os.close(master)
