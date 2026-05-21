/**
 * @file Main.cpp
 * @brief Реализация приложения-калькулятора маски WinAPI 98-03, Windows 7
 * 
 * Окно с 32 чекбоксами для выбора битовых позиций,
 * отображение текущего значения в BIN и HEX форматах.
 */

#include <windows.h>
#include <wchar.h>

#include "stdafx.h"
#include "MaskCalculator.h"

// =============================================================================
// DATA DEFINITIONS
// =============================================================================

/**
 * @brief Имена битов (описание каждой позиции из 0-31)
 * 
 * Соответствуют физическим зонам негабарита и датчикам поезда.
 */
const wchar_t* bitNames[TOTAL_BITS] =
{
    L"1  Зона негаб. (левый нижний боковой) БИ1.9 – БФ1.9 ",
    L"2  Зона негаб. (левый 1-й)  БИ1.10 – БФ1.10 отсут.",
    L"3  Зона негаб. (левый 2-й)  БИ1.11 – БФ1.11",
    L"4  Зона негаб. (левый 3-й)  БИ1.1 – БФ1.1 отсут.",
    L"5  Зона негаб. (вертикальный)  БИ1.2 – БФ1.2",
    L"6  Зона негаб. (правый 4-й)  БИ1.3 – БФ1.3 отсут.",
    L"7  Зона негаб. (правый 5-й)  БИ1.4 – БФ1.4",
    L"8  Зона негаб. (правый 6-й)  БИ1.7 – БФ1.7 отсут.",
    L"9  Зона негаб. (правый нижний боковой)  БИ1.8 – БФ1.8",
    L"10 Зона негаб. (левый ПС)  БИ1.13 – БФ1.13",
    L"11 Зона негаб. (правый ПС)  БИ1.14 – БФ1.14",
    L"12 Зона негаб. (левый основной) БИ1.15 – БФ1.15 отсут.",
    L"13 Зона негаб. (правый основной) БИ1.16 – БФ1.16  отсут.",
    L"14",
    L"15",
    L"16",
    L"17",
    L"18 Наличие состава 17  16  15  14",
    L"19 Реле времени",
    L"20 Реле КН (ВИ-2010)",
    L"21",
    L"22",
    L"23",
    L"24",
    L"25",
    L"26",
    L"27",
    L"28 Датчик вскрытия шкафа",
    L"29 Счет колес. Педаль 2 сенсор 2  БИ1.6 – БФ1.6",
    L"30 Датчик скорости. Педаль 2 сенсор 1  БИ1.17 – БФ1.17",
    L"31 Счет вагонов. Педаль 1 сенсор 2  БИ1.5 – БФ1.5",
    L"32 Начало состава. Педаль 1 сенсор 1  БИ1.12 – БФ1.12"
};

// =============================================================================
// GLOBAL VARIABLES DECLARATION
// =============================================================================

// Массив ручек для чекбоксов (индекс = номер бита)
HWND hChecks[TOTAL_BITS];

// Ручки редакторов BIN и HEX
HWND hEditBin;
HWND hEditHex;

// Текущая маска (значение 32 бита)
uint32_t gMask = 0;

// Флаг внутреннего обновления (для предотвращения циклических обновлений)
bool gInternalUpdate = false;

// Ручка верхнего заголовка окна
HWND hTopLabel;

// Мелкий шрифт для описаний битов
HFONT hSmallFont;

// =============================================================================
// UI HELPER FUNCTIONS IMPLEMENTATION
// =============================================================================

/**
 * @brief Создает шрифт Tahoma заданного размера
 */
HFONT CreateStyleFont(int size)
{
    return CreateFont(
        size,         // Высота шрифта в точках
        0,            // Ширина символа
        0,            // Наклон (0 - нет наклонения)
        0,            // Расстояние между буквами
        FW_NORMAL,    // Тип лица (FW_BOLD, FW_MEDIUM, FW_REGULAR и т.д.)
        FALSE,        // Underline (подчеркивание)
        FALSE,        // Strikeout (зачеркивание)
        FALSE,        // charset (0 - A-Z, a-z)
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Tahoma"     // Имя шрифта
    );
}

/**
 * @brief Создает статический лейбл с указанными параметрами
 * 
 * Параметры стиля:
 * - textColor - цвет текста
 * - bCentered - центрирование текста (SS_CENTER)
 * - border - рамка вокруг текста
 * 
 * @param parent Родительское окно (HWND родителя)
 * @param x     X-координата позиционирования
 * @param y     Y-координата позиционирования
 * @param w     Ширина в пикселях
 * @param h     Высота в пикселях
 * @param text  Текст лейбла (Unicode строка)
 * @param style Структура стилей лейбла
 * @return Ручка на созданный статический контрол, NULL при ошибке
 */
HWND CreateLabel(HWND parent, int x, int y, int w, int h, 
                 const wchar_t* text, LabelStyle style)
{
    UINT styleFlags = WS_CHILD | WS_VISIBLE;
    
    // Если флаг центрирования установлен - добавляем стиль SS_CENTER
    if (style.bCentered) 
        styleFlags |= SS_CENTER;
    
    return CreateWindowEx(
        0,                    // Экстра стили
        L"STATIC",            // Класс окна "STATIC"
        text,                 // Текст лейбла
        WS_CHILD | WS_VISIBLE | (style.bCentered ? SS_CENTER : SS_LEFT),
        x,                    // X-координата
        y,                    // Y-координата
        w,                    // Ширина в пикселях
        h,                    // Высота в пикселях
        parent,               // Родительское окно
        NULL,                 // Handle (NULL для статических)
        NULL,                 // HINSTANCE
        NULL                  // Pointer to extra data
    );
}

/**
 * @brief Создает автоматический чекбокс (BS_AUTOCHECKBOX)
 * 
 * Чекбокс без текста, используется только для бита/переключателя.
 * 
 * @param parent Родительское окно (HWND родителя)
 * @param id     ID контрола Windows-идентификатор
 * @param x      X-координата позиционирования
 * @param y      Y-координата позиционирования
 * @return Ручка на созданный чекбокс, NULL при ошибке
 */
HWND CreateCheckbox(HWND parent, int id, int x, int y)
{
    return CreateWindowEx(
        0,                    // Экстра стили
        L"BUTTON",            // Класс окна "BUTTON"
        L"",                  // Текст пустой
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x,                    // X-координата
        y,                    // Y-координата
        20,                   // Ширина в пикселях (стандартный размер)
        20,                   // Высота в пикселях
        parent,               // Родительское окно
        (HMENU)id,            // ID контрола
        NULL,                 // HINSTANCE
        NULL                  // Pointer to extra data
    );
}

/**
 * @brief Создает текстовый редактор (edit control) с горизонтальным скроллом
 * 
 * Редактор поддерживает автоскролл при наборе текста.
 * 
 * @param parent Родительское окно (HWND родителя)
 * @param id     ID контрола Windows-идентификатор
 * @param x      X-координата позиционирования
 * @param y      Y-координата позиционирования
 * @param w      Ширина в пикселях
 * @param h      Высота в пикселях
 * @return Ручка на созданный редактор, NULL при ошибке
 */
HWND CreateEdit(HWND parent, int id, int x, int y, int w, int h)
{
    return CreateWindowEx(
        WS_EX_CLIENTEDGE,     // Экстра стили (окантовка)
        L"EDIT",              // Класс окна "EDIT"
        L"",                  // Текст пустой
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        x,                    // X-координата
        y,                    // Y-координата
        w,                    // Ширина в пикселях
        h,                    // Высота в пикселях
        parent,               // Родительское окно
        (HMENU)id,            // ID контрола
        NULL,                 // HINSTANCE
        NULL                  // Pointer to extra data
    );
}

/**
 * @brief Устанавливает текст статического лейбла
 * 
 * Используется для динамического изменения текстового содержимого.
 * 
 * @param hLabel Ручка на статический контрол (лейбл)
 * @param text   Unicode строка для установки в качестве текста
 */
void SetLabelText(HWND hLabel, const wchar_t* text)
{
    if (hLabel && text)
        SetWindowText(hLabel, text);
}

// =============================================================================
// MASK CONVERSION FUNCTIONS IMPLEMENTATION
// =============================================================================

/**
 * @brief Преобразует числовое значение в строку BIN (MSB слева)
 * 
 * MSB - Most Significant Bit (старший бит) = бит 31
 * LSB - Least Significant Bit (младший бит) = бит 0
 * 
 * Формат: "1010..." где первая позиция = старший бит (31), последняя = младший (0)
 * 
 * @param value Число для преобразования (32-битное unsigned)
 * @param out   Выходной буфер шириной 33 символа (+NULL-терминатор)
 */
void MaskToBin(uint32_t value, wchar_t* out)
{
    for (int i = 31; i >= 0; --i)
    {
        // Если i-й бит установлен, ставим '1', иначе '0'
        // Вычисляем позицию в строке: 31-i (чтобы MSB был слева)
        out[31 - i] = (value & (1u << i)) ? L'1' : L'0';
    }

    // NULL-терминатор
    out[32] = 0;
}


/**
 * @brief Преобразует строку BIN в числовое значение (MSB слева)
 * 
 * Парсит строку типа "1010..." и возвращает 32-битное значение.
 * Строка должна иметь MSB (старший бит) на левом месте.
 * 
 * @param str Строка BIN длиной до 32 символов ('0' или '1')
 * @return Разработанное числовое значение (uint32_t)
 */
uint32_t BinToMask(const wchar_t* str)
{
    uint32_t v = 0;

    for (int i = 0; i < 32 && str[i]; i++)
    {
        if (str[i] == L'1')
        {
            // Устанавливаем бит на позиции (31 - индекс в строке)
            v |= (1u << (31 - i));
        }
    }

    return v;
}

// =============================================================================
// UPDATE FUNCTIONS IMPLEMENTATION
// =============================================================================

/**
 * @brief Обновляет состояние чекбоксов на основе текущей маски
 * 
 * Проходит по всем 32 битам и устанавливает состояние каждого чекбокса:
 * - Если бит установлен (1) в маске - BST_CHECKED
 * - Иначе - BST_UNCHECKED
 * 
 * @note Обновление происходит асинхронно через SendMessage, без использования флагов.
 */
void UpdateChecksFromMask()
{
    for (int i = 0; i < 32; i++)
    {
        // Проверяем, установлен ли i-й бит в текущей маске
        BOOL checked = (gMask & (1u << i)) ? BST_CHECKED : BST_UNCHECKED;

        // Устанавливаем состояние чекбокса черезSendMessage
        SendMessage(
            hChecks[i],
            BM_SETCHECK,
            checked,
            0
        );
    }
}

/**
 * @brief Обновляет значения в редакторах BIN и HEX на основе текущей маски
 * 
 * Устанавливает флаг gInternalUpdate = true перед обновлением, чтобы предотвратить
 * циклические обновления при вызове из обработчика событий.
 * 
 * После обновления сбрасывает флаг внутреннего обновления.
 */
void UpdateEdits()
{
    // Флаг предотвращает бесконечный цикл обновлений
    gInternalUpdate = true;

    wchar_t bin[33];      // Буфер для BIN (32 бита + NULL)
    wchar_t hex[17];      // Буфер для HEX (8 символов + NULL)

    // Преобразуем текущую маску в строку BIN
    MaskToBin(gMask, bin);

    // Форматируем значение в HEX с ведущими нулями (%08X = 8 шестнадцатеричных цифр)
    wsprintf(hex, L"%08X", gMask);

    // Устанавливаем текст в редакторы
    SetWindowText(hEditBin, bin);
    SetWindowText(hEditHex, hex);

	// caret в конец BIN
    //int binLen = GetWindowTextLength(hEditBin);
    //SendMessage(hEditBin, EM_SETSEL, binLen, binLen);

    // caret в конец HEX
    int hexLen = GetWindowTextLength(hEditHex);
    SendMessage(hEditHex, EM_SETSEL, hexLen, hexLen);


    // Сбрасываем флаг внутреннего обновления
    gInternalUpdate = false;
}

// =============================================================================
// WINDOW PROCEDURE IMPLEMENTATION
// =============================================================================

/**
 * @brief Обработчик сообщений окна (Window Procedure)
 * 
 * Обрабатывает сообщения Windows для окна калькулятора маски:
 * - WM_CREATE     - создание окна и UI
 * - WM_COMMAND    - обработка событий контролов
 * - WM_DESTROY    - разрушение окна
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Размеры главного окна
        int winW = 820;
        int winH = 640;

        // Позиционируем окно на экране (x=100, y=100) с размерами 820x640
        SetWindowPos(hwnd, NULL, 100, 100, winW, winH, SWP_NOZORDER);

        // Создаем заголовок окна по центру (красным цветом)
        LabelStyle CenteredStyle = {RGB(255, 0, 0), TRUE, FALSE};
        hTopLabel = CreateLabel(hwnd, 0, 10, winW, 25, L"Калькулятор Маски", CenteredStyle);

        // === ЛЕВАЯ/ПРАВАЯ СПИСКИ (ЧЕКБОКСЫ) ===
        
        // Создаем шрифт для описаний битов (размер 14 пикселей)
        hSmallFont = CreateStyleFont(14);
        
        // Стили лейблов списков (черный текст, не центрированные, без рамок)
        LabelStyle LeftStyle = {RGB(0, 0, 0), FALSE, FALSE};

        int startY = 60;          // Начальная Y-координата для списка чекбоксов
        int rowH = 28;             // Высота строки (интервал между чекбоксами)

        int leftX = 20;            // X-позиция левого списка
        int rightX = 520;          // X-позиция правого списка

        int y = 60;                // Текущая Y-координата

        // --- ЛЕВЫЙ СПИСОК (биты 31-16, верхняя половина) ---
        for (int i = 0; i < 16; i++)
        {
            // Создаем лейбл с описанием бита (сверху вниз: 31 -> 16)
            HWND tLabet = CreateLabel(hwnd, leftX, y, 350, 20, bitNames[31 - i], LeftStyle);
            SendMessage(tLabet, WM_SETFONT, (WPARAM)hSmallFont, TRUE);

            // Создаем чекбокс для этого бита справа от лейбла
            hChecks[31 - i] =
                CreateCheckbox(
                    hwnd,
                    ID_CHECK_BASE + (31 - i),  // ID = 1000 + номер бита
                    leftX + 350,               // X-координата чекбокса справа от лейбла
                    y                           // Y-координата
                );

            y += rowH;                        // Увеличиваем Y для следующей строки
        }

        // Сбрасываем Y-координату для правой колонки
        y = 60;

        // --- ПРАВЫЙ СПИСОК (биты 15-0, нижняя половина) ---
        for (int i = 0; i < 16; i++)
        {
            // Создаем лейбл с описанием бита (сверху вниз: 15 -> 0)
            HWND tLabet = CreateLabel(hwnd, rightX - 100, y, 350, 20, bitNames[15 - i], LeftStyle);
            SendMessage(tLabet, WM_SETFONT, (WPARAM)hSmallFont, TRUE);

            // Создаем чекбокс для этого бита справа от лейбла
            hChecks[15 - i] =
                CreateCheckbox(
                    hwnd,
                    ID_CHECK_BASE + (15 - i),   // ID = 1000 + номер бита
                    rightX - 100 + 350,         // X-координата чекбокса справа от лейбла
                    y                            // Y-координата
                );

            y += rowH;                        // Увеличиваем Y для следующей строки
        }

        // --- БЛОК РЕДАКТИРОВАНИЯ ВНИЗУ (BIN/HEX) ===
        HWND tLabetBIN = CreateLabel(hwnd, 200, 520, 80, 20, L"BIN", LeftStyle);
        HWND tLabetHEX = CreateLabel(hwnd, 200, 560, 80, 20, L"HEX", LeftStyle);

        // Редактор BIN (начинаем ввод слева)
        hEditBin = CreateEdit(hwnd, ID_EDIT_BIN, 250, 520, 400, 25);
		//SendMessage(hEditBin, EM_LIMITTEXT, 32, 0);
        // Редактор HEX
        hEditHex = CreateEdit(hwnd, ID_EDIT_HEX, 250, 560, 400, 25);
		//SendMessage(hEditHex, EM_LIMITTEXT, 8, 0);

		UpdateEdits();
        return 0;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);   // Извлекаем ID контрола
        int code = HIWORD(wParam);  // Извлекаем код сообщения

        // === ОБРАБОТКА ЧЕКБОКСОВ ===
        if (id >= ID_CHECK_BASE &&
            id < ID_CHECK_BASE + 32 &&
            code == BN_CLICKED)
        {
            int bit = id - ID_CHECK_BASE;  // Номер бита по ID

            // Получаем текущее состояние чекбокса
            BOOL checked =
                SendMessage(
                    hChecks[bit],
                    BM_GETCHECK,
                    0,
                    0
                ) == BST_CHECKED;

            // Обновляем маску в зависимости от состояния:
            // Если checked - устанавливаем бит (|=), если нет - сбрасывам его (&~)
            if (checked)
                gMask |= (1u << bit);
            else
                gMask &= ~(1u << bit);

            // Обновляем отображение в редакторах BIN/HEX
            UpdateEdits();
        }

        // === ОБРАБОТКА РЕДАКТОРА BIN ===
        if (id == ID_EDIT_BIN &&
            code == EN_CHANGE &&           // Событие изменения текста
            !gInternalUpdate)              // Не вызываем циклически
        {
            wchar_t buf[64];

            // Получаем текст из редактора
            GetWindowText(hEditBin, buf, 64);

            // Преобразуем строку BIN в числовую маску
            gMask = BinToMask(buf);

            // Обновляем чекбоксы на основе новой маски
            UpdateChecksFromMask();
            
            // Сбрасываем флаги перед возвращением
            UpdateEdits();
        }

        // === ОБРАБОТКА РЕДАКТОРА HEX ===
        if (id == ID_EDIT_HEX &&
            code == EN_CHANGE &&           // Событие изменения текста
            !gInternalUpdate)              // Не вызываем циклически
        {
            wchar_t buf[64];

            // Получаем текст из редактора HEX
            GetWindowText(hEditHex, buf, 64);

            // Преобразуем HEX строку в числовую маску
            swscanf(buf, L"%X", &gMask);

            // Обновляем чекбоксы на основе новой маски
            UpdateChecksFromMask();
            
            // Сбрасываем флаги перед возвращением
            UpdateEdits();
        }

        return 0;
    }

    case WM_DESTROY:
        // Уведомляем систему о разрушении окна
        PostQuitMessage(0);
        
        // Освобождаем ресурсы шрифта
        DeleteObject(hSmallFont);
        
        return 0;
    }

    // Передаем остальные сообщения в стандартный обработчик Windows
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Структура класса окна
    WNDCLASS wc = {0};

    // Устанавливаем обработчик сообщений окна
    wc.lpfnWndProc = WndProc;
    
    // Ручка на модуль приложения
    wc.hInstance = hInstance;
    
    // Название класса окна (регистрация)
    wc.lpszClassName = CLASS_NAME;
    
    // Стандартный курсор стрелки
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    // Фон: стандартная серо-белая щетка для окон (COLOR_WINDOW + 1)
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    // Регистрируем класс окна Windows
    RegisterClass(&wc);

    // Создаем окно класса
    HWND hwnd = CreateWindowEx(
        0,                    // Экстра стили
        CLASS_NAME,           // Название класса
        L"Калькулятор Маски", // Заголовок окна
        WS_OVERLAPPEDWINDOW,  // Стиль окна (перетаскивание, границы, кнопки и т.д.)
        CW_USEDEFAULT,        // Дефолтная X-позиция
        CW_USEDEFAULT,        // Дефолтная Y-позиция
        1000,                 // Ширина окна
        800,                  // Высота окна
        NULL,                 // Родительское окно (тоplevel)
        NULL,                 // МЕНЮ
        hInstance,            // HINSTANCE
        NULL                  // Pointer to extra data
    );

    if (!hwnd)
        return -1;  // Ошибка создания окна

    // Показываем окно в заданном состоянии (обычно SW_SHOWNORMAL)
    ShowWindow(hwnd, nCmdShow);
    
    // Обновляем отрисовку окна
    UpdateWindow(hwnd);

    // Сообщения Windows - бесконечный цикл обработки событий
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);     // Перевод кодов клавиш в формат Win32
        DispatchMessage(&msg);      // Распределение сообщений окнам
    }

    return (int)msg.wParam;         // Возвращаем значение из PostQuitMessage
}
