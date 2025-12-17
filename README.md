
# IoT Sensor & Actuator System

Система моніторингу навколишнього середовища та керування актуаторами на базі Arduino Uno R4 WiFi та ПК-клієнта.

## Description

Цей проект реалізує комплексну систему IoT, яка складається з двох частин: прошивки для мікроконтролера та програмного забезпечення для ПК.
Система зчитує показники з сенсорів BME680 (температура, вологість, тиск, газ), відображає статус на LED-матриці та передає дані через Serial-порт на комп'ютер.
Клієнтська частина (Receiver) приймає дані, зберігає їх у локальну базу даних SQLite та дозволяє керувати підключеними актуаторами (сервоприводами, LED) через командний інтерфейс.

## Getting Started

### Dependencies
* **IDE**
    * CLion or VS Code 
* **Hardware:**
    * Arduino Uno R4 WiFi
    * Сенсори BME680 (через I2C/SPI)
    * Сервопривід (Servo)
    * USB-C кабель для з'єднання
* **PC Software (Linux Recommended):**
    * OS: Linux (код оптимізовано під шляхи `/dev/ttyACM0` та бібліотеки `pthread`, `rt`)
    * С++ Compiler: підтримка стандарту C++20 (GCC/Clang)
    * CMake: версія 4.0+ (або 3.20+ з правкою `CMakeLists.txt`)
    * Бібліотеки: `sqlite3`, `libserial` (або включена `vendor/serial`), `pthread`
* **Arduino Software:**
    * PlatformIO (CLion or VS Code Extension)
    * Framework: Arduino Renesas RA

### Installing

1.  **Клонування репозиторію:**
    ```bash
    git clone [https://github.com/yourusername/cursova.git](https://github.com/yourusername/cursova.git)
    cd cursova
    ```

2.  **Налаштування Arduino (Firmware):**
    * Відкрийте папку `Arduino` у PlatformIO.
    * **Виправлення шляху:** Відкрийте `Arduino/src/main.cpp`. Знайдіть рядок `#include "/home/rabilint/..."` і замініть його на:
      ```cpp
      #include <Adafruit_BME680.h>
      ```
      або замініть на вашу абсолютну адресу на файл:
      ```cpp
      #include ".../Adafruit_BME680-master/Adafruit_BME680.h"
      ```
    * Підключіть плату та натисніть **Upload**.

3.  **Компіляція ПК-частини (Receiver):**
    ```bash
    cd PC_part/Receiver
    mkdir build
    cd build
    cmake ..
    make
    ```

### Executing program

1.  Підключіть Arduino до USB-порту.
2.  Переконайтеся, що пристрій доступний за адресою `/dev/ttyACM0` (якщо ні — змініть порт у `PC_part/Receiver/main.cpp`).
3.  Запустіть програму:
    ```bash
    ./Receiver
    ```
4.  Використовуйте команди в консолі для взаємодії (наприклад, `help` для списку команд).

## Help

Типові проблеми:
* **Помилка CMake Version:** Якщо у вас старіший CMake, змініть перший рядок у `CMakeLists.txt` на `cmake_minimum_required(VERSION 3.20)`.
* **Доступ до порту:** Якщо виникає помилка "Access denied", виконайте:
  ```bash
  sudo chmod a+rw /dev/ttyACM0
  ```
## Authors

* Нікіта Майборода
* email: Nikita.Maiboroda@lnu.edu.ua

## Version History

* 0.1
  * Initial Release



## License

Цей проект є загальнодоступним (Public Domain / Open Source). Ви можете вільно використовувати, змінювати та розповсюджувати код.

## Acknowledgments

* [PlatformIO](https://platformio.org/)
* [Adafruit BME680 Library](https://github.com/adafruit/Adafruit_BME680)
* [SQLite](https://www.sqlite.org/)
