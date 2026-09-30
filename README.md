# It_runs_doom_notepad-
a plugin for notepad++ (both x32 &amp;x64)
& a separate ZIP folder with source code
# Doom Launcher for Notepad++
Feel free to see my video instruction on my YouTube channel:
https://www.youtube.com/watch?v=c5ZejRRrpLs

For example you can download chocolate Doom by link:
https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom

This plugin adds a **Plugins → Doom Launcher → Launch Doom...** menu item to Notepad++.
It prompts you to select a game WAD and launches it using a genuine Doom-compatible engine in a separate
graphical window. It does not use character-based or ASCII graphics.

## Requirements

- Notepad++ for Windows (typically 64-bit).
- A Doom-compatible engine: GZDoom, Chocolate Doom, Crispy Doom, DSDA-Doom, or Woof.
- A legally obtained base WAD: `DOOM.WAD` or the shareware file `DOOM1.WAD`.

The plugin itself does not distribute Doom, the engine, or copyrighted game data.


1. Close Notepad++.
2. Create a folder named `DoomLauncher` inside the `plugins` directory of your Notepad++ installation.
3. Copy `DoomLauncher.dll` into it, resulting in the path
`Notepad++\plugins\DoomLauncher\DoomLauncher.dll`. 4. Launch Notepad++ and select **Plugins → Doom Launcher → Launch Doom...**.
5. Upon the first launch, select the engine EXE, then select the game IWAD.

The engine path is saved in the Notepad++ plugin configuration directory.
You can change it using the **Select Engine...** command.

If using GitHub Actions: open the completed **Build** run, download the `DoomLauncher-x64`
or `DoomLauncher-x86` artifact, and extract the DLL before installation.

## Features

- WADs are validated by signature before launch.
- `IWAD`s are launched using the `-iwad` parameter.
- `PWAD`s are rejected with a clear error message, as a mod cannot function without a base IWAD.
- Engine and WAD paths are passed safely, even when containing spaces or quotes.
- If the engine EXE is located next to the DLL, supported names are detected automatically.

## License

The plugin source code is distributed under the MIT License. Doom is a trademark of id Software.


            *     *     *  

# Doom Launcher для Notepad++

Плагин добавляет в Notepad++ меню **Плагины → Doom Launcher → Запустить Doom...**.
Он просит выбрать игровой WAD и запускает его настоящим Doom-совместимым движком в отдельном
графическом окне. Символьная/ASCII-графика не используется.
Для примера можно скачатьChocolate Doom по ссылке:
https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom

## Что потребуется

- Notepad++ для Windows (обычно 64-разрядный).
- Один Doom-совместимый движок: GZDoom, Chocolate Doom, Crispy Doom, DSDA-Doom или Woof.
- Законно полученный базовый WAD: `DOOM.WAD` или shareware-файл `DOOM1.WAD`.

Сам плагин не распространяет Doom, движок или защищённые авторским правом игровые данные.


## Установка

1. Закройте Notepad++.
2. Создайте папку `DoomLauncher` внутри каталога `plugins` вашей установки Notepad++.
3. Скопируйте туда `DoomLauncher.dll`, чтобы получился путь
   `Notepad++\plugins\DoomLauncher\DoomLauncher.dll`.
4. Запустите Notepad++ и выберите **Плагины → Doom Launcher → Запустить Doom...**.
5. При первом запуске выберите EXE движка, затем выберите игровой IWAD.

Путь к движку сохраняется в пользовательском каталоге конфигурации плагинов Notepad++.
Изменить его можно командой **Выбрать движок...**.

Если используете GitHub Actions: откройте завершённый запуск **Build**, скачайте артефакт
`DoomLauncher-x64` или `DoomLauncher-x86` и извлеките DLL перед установкой.

## Особенности

- WAD проверяется по сигнатуре до запуска.
- `IWAD` запускается с параметром `-iwad`.
- `PWAD` отклоняется с понятным сообщением, поскольку мод не может работать без базового IWAD.
- Путь к движку и WAD безопасно передаётся даже при наличии пробелов и кавычек.
- Если EXE движка лежит рядом с DLL, поддерживаемые имена обнаруживаются автоматически.

## Лицензия

Исходный код плагина распространяется по лицензии MIT. Doom является товарным знаком id Software.

