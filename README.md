# IoT Sensor & Actuator System

Система моніторингу навколишнього середовища та керування актуаторами на базі Arduino Uno R4 WiFi та ПК-клієнта.

## Description

Цей проект реалізує комплексну IoT систему, що складається з прошивки мікроконтролера та клієнтського застосунку для ПК.
**Arduino частина** зчитує показники з сенсорів BME680 (температура, вологість, тиск, газ), керує сервоприводами та зовнішніми світлодіодами (LED), а також програє анімацію на вбудованій LED-матриці.
**ПК частина (Receiver)** отримує дані через Serial-порт, обробляє їх та зберігає історію подій у базі даних SQLite.

## Getting Started

### Dependencies

* **Hardware:**
    * Arduino Uno R4 WiFi
    * Сенсори BME680 (I2C/SPI інтерфейс)
    * Сервопривід (Servo)
    * Світлодіоди (LED): Червоний та Жовтий (підключення до GPIO)
    * USB-кабель (Type-C)
* **PC Software (Linux Recommended):**
    * OS: Linux (проект використовує системні виклики `pthread`, `rt` та шлях `/dev/ttyACM0`)
    * C++ Compiler: з підтримкою стандарту **C++20**
    * CMake: версія **4.0** (або новіша)
    * Libraries: `sqlite3`, `libserial` (або `vendor/serial`), `pthread`
* **Arduino Software:**
    * PlatformIO (VS Code Extension)
    * Framework: Arduino (Renesas RA)

### Installing

1.  **Клонування репозиторію:**
    ```bash
    git clone [https://github.com/rabilint/cursova.git](https://github.com/rabilint/cursova.git)
    cd cursova
    ```

2.  **Виправлення шляхів (Arduino):**
    Проект містить абсолютні шляхи до бібліотек, специфічні для середовища розробника. Перед компіляцією **необхідно** замінити їх у всіх файлах папки `Arduino/src/` та `Arduino/include/`.
    
    * Використовуйте "Find and Replace" у вашому редакторі.
    * **Шукати:** `/home/rabilint/CLionProjects/cursova/Arduino/lib/...` (або подібні довгі шляхи).
    * **Замінити на:** Відповідні стандартні include або ваші абсолютні посилання на файли.
    
    *Приклад (main.cpp, BME680_Sensor.h та ін.):*
    ```cpp
    // Змінити це:
    #include "/home/rabilint/CLionProjects/cursova/Arduino/lib/Adafruit_BME680-master/Adafruit_BME680.h"
    
    // На це:
    #include <Adafruit_BME680.h>
    ```

3.  **Запуск Arduino:**
    * Відкрийте папку `Arduino` у VS Code (PlatformIO).
    * Підключіть плату.
    * Виконайте **Upload**.

4.  **Компіляція ПК-частини (Receiver):**
    ```bash
    cd PC_part/Receiver
    mkdir build
    cd build
    cmake ..
    make
    ```

### Executing program

1.  Підключіть Arduino до USB.
2.  Переконайтеся, що порт доступний як `/dev/ttyACM0`.
    * *Якщо порт інший, змініть `SerialCommunicator("/dev/ttyACM0", ...)` у `PC_part/Receiver/main.cpp`.*
3.  Запустіть приймач:
    ```bash
    ./Receiver
    ```
4.  Слідуйте підказкам у консолі для введення команд.

## Help

**Поширені проблеми:**

* **CMake Error:** Якщо ваша версія CMake старіша за 4.0, змініть перший рядок у `CMakeLists.txt` на `cmake_minimum_required(VERSION 3.20)` (може знадобитися перевірка сумісності).
* **Compilation Error (No such file):** Ви пропустили якийсь файл при заміні абсолютних шляхів. Перевірте помилки компілятора, щоб знайти точний файл і рядок.
* **Permission denied:**
    ```bash
    sudo chmod a+rw /dev/ttyACM0
    ```

## Authors

Contributors names and contact info:

* **rabilint** [GitHub Profile](https://github.com/rabilint)

## Version History

* 0.1
    * Initial Release

## License

This project is licensed under the Public Domain (Unlicense) - see the LICENSE file for details (if present), or consider it free to use.

## Acknowledgments

* [PlatformIO Documentation](https://docs.platformio.org/)
* [Adafruit BME680 Library](https://github.com/adafruit/Adafruit_BME680)
* [DomPizzie README Template](https://gist.github.com/DomPizzie/7a5ff55ffa9081f2de27c315f5018afc)
