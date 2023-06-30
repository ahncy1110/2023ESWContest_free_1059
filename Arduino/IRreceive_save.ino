#include <SPI.h>
#include <SD.h>
#include <IRremote.h>
const int CS = 10;
const long RECV_PIN = 2; // ir 수신 핀

// Device 입력하기 //

String saveDevice() {
  String device;

  while (Serial.available() == 0) {
    // 입력이 없을 때까지 대기
  }

  String serialInput = Serial.readStringUntil('\n');
  serialInput.trim();  // 앞뒤 공백 제거

  device = serialInput;
  Serial.println("Device: " + device);

  return device;  // device 값을 반환
}


// IR 받아오기 //
void check_receive(String& a, String& b, String& c, String& d, String& e) {
  static String previousValue;
  static uint32_t sameValueCount = 0;

  if (IrReceiver.decode()) {
    Serial.println();
    Serial.println("Receive");
    Serial.print("LSB: ");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX); // 받아지는 LSB raw-data
    unsigned long lsbValue = IrReceiver.decodedIRData.decodedRawData;

    // 이진수로 표현
    String binaryString = String(lsbValue, BIN);

    // 이진수의 길이가 32비트보다 작을 경우, 앞쪽에 0을 추가하여 32비트로 맞춤
    while (binaryString.length() < 32) {
      binaryString = "0" + binaryString;
    }

    // 이진수를 MSB로 반전
    String msbBinaryString;
    for (int i = 31; i >= 0; i--) {
      msbBinaryString += binaryString[i];
    }

    // 16진수로 변환
    unsigned long msbValue = strtoul(msbBinaryString.c_str(), nullptr, 2);
    String msbHexString = String(msbValue, HEX);

    // MSB 결과 출력
    Serial.print("MSB: ");
    Serial.println(msbHexString);

    // 이전 값과 현재 값 비교
    if (previousValue != "0" && msbHexString.equals(previousValue)) {
      sameValueCount++;

      // 같은 값이 3번 들어왔을 때
      if (sameValueCount == 3) {
        // 값 저장
        if (a == "a") {
          a = msbHexString;
          Serial.print("Value stored in a: ");
          Serial.println(a);
        } else if (b == "b") {
          b = msbHexString;
          Serial.print("Value stored in b: ");
          Serial.println(b);
        } else if (c == "c") {
          c = msbHexString;
          Serial.print("Value stored in c: ");
          Serial.println(c);
        } else if (d == "d") {
          d = msbHexString;
          Serial.print("Value stored in d: ");
          Serial.println(d);
        } else if (e == "e") {
          e = msbHexString;
          Serial.print("Value stored in e: ");
          Serial.println(e);
        }
        sameValueCount = 0;
      }
    } else {
      sameValueCount = 1;
      if (msbHexString != "0") {
        previousValue = msbHexString;
      }
    }
    IrReceiver.resume();
  }
}


// sd카드에 값 쓰기//
String generateFileName() {
  static int number = 0;
  String filename;
  while (SD.exists("device" + String(number) + ".txt")) {
    number++;
  }
  filename = "device" + String(number) + ".txt";
  return filename;
}

void sd_write(const String& filename, const String& dev, const String& a, const String& b, const String& c, const String& d, const String& e) {
  File myFile;
  bool fileOpened = false;
  sd_check();

  while (!fileOpened) {
    myFile = SD.open(filename, FILE_WRITE);

    if (myFile) {
      fileOpened = true;
    } else {
      Serial.println("Error opening file: " + filename);
      delay(1000); // 1초 대기 후 다시 시도
    }
  }
  /* 파일에 데이터 쓰기 */ 
  if (myFile) {
    // 디바이스 이름 쓰기
    myFile.println(dev);

    // 데이터 값들 쓰기
    myFile.println(a);
    myFile.println(b);
    myFile.println(c);
    myFile.println(d);
    myFile.println(e);

    myFile.close();
    Serial.println("Data written to file: " + filename);
    Serial.println("Data written to SD card.");
  }
}


void sd_check()
{ 
  while (!SD.begin(CS)) {
    Serial.println(F("Initializing SD card..."));
    if (SD.begin(CS))
    {
      Serial.println(F("Card initializing complete."));
    }
    else {
      Serial.println(F("Card Initializing failed, or not present"));
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {
  String a = "a";
  String b = "b";
  String c = "c";
  String d = "d";
  String e = "e";
  String device;

  Serial.print("save device name: ");
  device = saveDevice();

  while(1){
    check_receive(a,b,c,d,e);

    if (a != "a" && b != "b" && c != "c" && d != "d" && e != "e") {
      break;
    }
  }

  String fileName = generateFileName();

  sd_write(fileName, device, a, b, c, d, e);

  a = "a"; b = "b"; c = "c"; d = "d"; e = "e";
}
