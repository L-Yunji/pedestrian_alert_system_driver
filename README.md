# 보행자 경고 시스템용 커널 디바이스 드라이버

**보행자 감지 이벤트 발생 시, LCD 및 스피커를 동작시키기 위한 커널 모듈 세 개**로 구성되어 있습니다.

---

## 구성 모듈

| 파일명               | 디바이스 파일              | 설명                     |
| ----------------- | -------------------- | ---------------------- |
| `lcd_notify.c`    | `/dev/lcd_notify`    | LCD 깜빡임 이벤트 트리거        |
| `wav_notify.c`    | `/dev/wav_notify`    | 사운드 재생 이벤트 트리거         |
| `alert_trigger.c` | `/dev/alert_trigger` | 사용자 공간 ACK 처리용 이벤트 플래그 |

---

## 디바이스 동작 방식

### `/dev/lcd_notify`

* 사용자 공간에서 `"1"`을 write하면 내부 flag가 설정됨
* `poll()`을 통해 사용자 공간 데몬이 해당 이벤트를 감지함
* 이벤트 감지 시 flag는 자동 초기화됨

### `/dev/wav_notify`

* `"1"`을 write 시 내부 flag + 문자열 버퍼 설정
* `poll()` + `read()`로 사용자 공간에서 감지 가능
* 이후 `aplay` 등을 통해 음성 재생에 활용

### `/dev/alert_trigger`

* 사용자 공간에서 `"1"`을 write → alert 발생
* `poll()` + `read()`로 이벤트 완료 여부를 알림

---

## 빌드 방법

각 폴더에 들어가서 make 실행

```bash
make
```

---

## 설치 및 제거

### 설치

```bash
sudo insmod alert_trigger.ko
sudo insmod lcd_notify.ko
sudo insmod wav_notify.ko
```

### 디바이스 확인

```bash
ls /dev/lcd_notify /dev/wav_notify /dev/alert_trigger
```

> 등록되었는지 확인

### 제거

```bash
sudo rmmod wav_notify
sudo rmmod lcd_notify
sudo rmmod alert_trigger
```

---

## 예시 테스트

```bash
# LCD 알림 이벤트 트리거
echo 1 > /dev/lcd_notify

# WAV 알림 이벤트 트리거
echo 1 > /dev/wav_notify

# 알림 ACK 대기 후 완료 처리
echo 1 > /dev/alert_trigger
```

---

## 참고

이 드라이버들은 사용자 공간의 감지 시스템(`control_dev`) 및 알림 데몬(`alert_daemon`)과 `poll()` 기반 이벤트 통신을 하기 위한 목적으로 설계되었습니다.

각 드라이버는 모두 **커널 character device**로 구현되어 있으며, 공통적으로:

* `alloc_chrdev_region()` + `cdev_add()`로 등록됨
* `poll()`을 통한 비동기 이벤트 알림 제공
* `write()`를 통한 트리거 방식 사용

---

## 라이선스

```
GPL v2
```

---

## 작성자

* Yunji ([yunji965@gmail.com](mailto:yunji965@gmail.com))
