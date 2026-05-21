/**
 * @file MaskCalculator.h
 * @brief Заголовок для приложения-калькулятора маски
 * 
 * Окно с 32 чекбоксами для выбора битовых позиций,
 * отображение в BIN и HEX форматах.
 */

#pragma once

#ifndef MASK_CALCULATOR_H
#define MASK_CALCULATOR_H

// =============================================================================
// INCLUDES
// =============================================================================

#include <windows.h>
#include <wchar.h>
#include <stdint.h>

// =============================================================================
// CONSTANTS
// =============================================================================

// Общее количество битов (32-битный unsigned integer)
#define TOTAL_BITS           32

// Базовый ID для контролов чекбоксов
#define ID_CHECK_BASE        1000

// ID редактирования значений BIN и HEX
#define ID_EDIT_BIN          2000
#define ID_EDIT_HEX          2001

// Название класса окна
const wchar_t CLASS_NAME[] = L"SimpleWinAPIWindow";

// =============================================================================
// TYPE DEFINITIONS / STRUCTURES
// =============================================================================

/**
 * @brief Структура элемента интерфейса (чекбокс + лейбл)
 */
struct Item
{
    wchar_t text[64];     // Текст названия бита
    bool              checked;   // Состояние проверенности
    HWND              hLabel;    // Ручка лейбла
    HWND              hCheck;    // Ручка чекбокса
};

/**
 * @brief Имена битов (описание каждой позиции)
 */
//const wchar_t* bitNames[TOTAL_BITS];

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

// Массив ручек для чекбоксов
extern HWND hChecks[TOTAL_BITS];

// Ручки редакторов BIN и HEX
extern HWND hEditBin;
extern HWND hEditHex;

// Текущая маска (значение 32 бит)
extern uint32_t gMask;

// Флаг внутреннего обновления (для предотвращения циклических обновлений)
extern bool gInternalUpdate;

// Стили лейблов
struct LabelStyle
{
    COLORREF textColor;      // Цвет текста
    BOOL     bCentered;      // Центрирование текста
    BOOL     border;         // Рамка
};

/**
 * @brief Структура блока редактирования (BIN/HEX)
 */
struct EditBlock
{
    wchar_t  label[64];      // Текст лейбла
    HWND     hLabel;         // Ручка лейбла
    HWND     hEdit;          // Ручка редактора
};

// Блок редактирования внизу окна
extern EditBlock bottomEdits[2];

// Ручка верхнего заголовка окна
extern HWND hTopLabel;

// Мелкий шрифт для описаний битов
extern HFONT hSmallFont;

// =============================================================================
// UI HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Создает шрифт Tahoma заданного размера
 * @param size Размер шрифта в пунктах
 * @return Ручка на созданный шрифт
 */
HFONT CreateStyleFont(int size);

/**
 * @brief Создает статический лейбл с указанными параметрами
 * @param parent Родительское окно
 * @param x X-координата
 * @param y Y-координата
 * @param w Ширина в пикселях
 * @param h Высота в пикселях
 * @param text Текст лейбла
 * @param style Стили лейбла
 * @return Ручка на созданный лейбл
 */
HWND CreateLabel(HWND parent, int x, int y, int w, int h, 
                 const wchar_t* text, LabelStyle style);

/**
 * @brief Создает автоматический чекбокс
 * @param parent Родительское окно
 * @param id ID контрола
 * @param x X-координата
 * @param y Y-координата
 * @return Ручка на созданный чекбокс
 */
HWND CreateCheckbox(HWND parent, int id, int x, int y);

/**
 * @brief Создает текстовый редактор (edit control)
 * @param parent Родительское окно
 * @param id ID контрола
 * @param x X-координата
 * @param y Y-координата
 * @param w Ширина в пикселях
 * @param h Высота в пикселях
 * @return Ручка на созданный редактор
 */
HWND CreateEdit(HWND parent, int id, int x, int y, int w, int h);

/**
 * @brief Устанавливает текст статического лейбла
 * @param hLabel Ручка на лейбл
 * @param text Текст для установки
 */
void SetLabelText(HWND hLabel, const wchar_t* text);

// =============================================================================
// MASK CONVERSION FUNCTIONS
// =============================================================================

/**
 * @brief Преобразует числовое значение в строку BIN (MSB слева)
 * @param value Число для преобразования
 * @param out Выходной буфер шириной 33 символа (+NULL-терминатор)
 */
void MaskToBin(uint32_t value, wchar_t* out);

/**
 * @brief Преобразует строку BIN в числовое значение (MSB слева)
 * @param str Строка BIN длиной до 32 символов
 * @return Разработанное числовое значение
 */
uint32_t BinToMask(const wchar_t* str);

// =============================================================================
// UPDATE FUNCTIONS
// =============================================================================

/**
 * @brief Обновляет состояние чекбоксов на основе текущей маски
 * 
 * Проходит по всем 32 битам и устанавливает состояние каждого чекбокса.
 */
void UpdateChecksFromMask();

/**
 * @brief Обновляет значения в редакторах BIN и HEX на основе текущей маски
 * 
 * Устанавливает флаг gInternalUpdate, чтобы предотвратить циклические обновления.
 */
void UpdateEdits();

#endif // MASK_CALCULATOR_H
