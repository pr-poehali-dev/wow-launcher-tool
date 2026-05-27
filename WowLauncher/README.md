# WoW Multi-Instance Launcher

Нативная Win32 программа на C++ для запуска нескольких копий World of Warcraft с автологином.

---

## Требования

- Windows 10 (32-bit или 64-bit)
- **w64devkit** — скачать с https://github.com/skeeto/w64devkit/releases
- Выбрать версию: `w64devkit-i686-*.zip` (32-bit, `-m32`)

---

## Сборка (w64devkit)

1. Запустите `w64devkit.exe` (откроется терминал)
2. Перейдите в папку с проектом:
   ```
   cd /путь/к/WowLauncher
   ```
3. Соберите:
   ```
   g++ -m32 -O2 -mwindows -o WowLauncher.exe main.cpp -lcomctl32 -lcomdlg32 -lshlwapi
   ```
4. Готово — в папке появится `WowLauncher.exe`

---

## Файл log.conf

Создаётся автоматически рядом с `WowLauncher.exe` при первом запуске.

Формат:
```
login=YOUR_LOGIN
password=YOUR_PASSWORD
realm=Warmane
region=EU
```

Заполните поля и сохраните. При следующем нажатии «ЗАПУСТИТЬ» данные будут использованы.

---

## Как работает автологин

Программа передаёт логин и пароль как аргументы командной строки:
```
Wow.exe -login LOGIN -password PASSWORD
```

> Это стандартный метод для большинства WoW-клиентов (Warmane, Light's Hope и др.).
> Для официального Battle.net клиента потребуется другой механизм.

---

## Лог запусков

Все события сохраняются в файл `launcher.log` рядом с exe.

---

## Структура файлов

```
WowLauncher/
├── main.cpp          ← исходный код
├── WowLauncher.exe   ← после сборки
├── log.conf          ← учётные данные (создаётся автоматически)
└── launcher.log      ← лог запусков
```
