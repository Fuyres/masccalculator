#include <windows.h>
#include <wchar.h>

#include "stdafx.h"
#include "MaskCalculator.h"

#include <stdint.h>

#define TOTAL_BITS 32

#define ID_CHECK_BASE 1000
#define ID_EDIT_BIN   2000
#define ID_EDIT_HEX   2001

const wchar_t CLASS_NAME[] = L"SimpleWinAPIWindow";

#define LEFT_COUNT 17
#define RIGHT_COUNT 15

struct Item
{
    wchar_t text[64];
    bool checked;
    HWND hLabel;
    HWND hCheck;
};

const wchar_t* bitNames[TOTAL_BITS] =
{
    L"1  Зона негабаритности (левый нижний боковой)  БИ1.9 – БФ1.9 ",
    L"2  Зона негабаритности (левый 1-й)   БИ1.10 – БФ1.10  отсутств. в упр. системе",
    L"3  Зона негабаритности (левый 2-й)   БИ1.11 – БФ1.11",
    L"4  Зона негабаритности (левый 3-й)   БИ1.1 – БФ1.1  отсутств. в упр. системе",
    L"5  Зона негабаритности (вертикальный)   БИ1.2 – БФ1.2",
    L"6  Зона негабаритности (правый 4-й)   БИ1.3 – БФ1.3  отсутств. в упр. системе",
    L"7  Зона негабаритности (правый 5-й)   БИ1.4 – БФ1.4",
    L"8  Зона негабаритности (правый 6-й)   БИ1.7 – БФ1.7  отсутств. в упр. системе",
    L"9  Зона негабаритности (правый нижний боковой)   БИ1.8 – БФ1.8",
    L"10 Зона негабаритности (левый ПС)   БИ1.13 – БФ1.13",
    L"11 Зона негабаритности (правый ПС)   БИ1.14 – БФ1.14",
    L"12 Зона негабаритности (левый основной)   БИ1.15 – БФ1.15  отсутств. в упр. системе",
    L"13 Зона негабаритности (правый основной)   БИ1.16 – БФ1.16  отсутств. в упр. системе",
    L"14",
    L"15",
    L"16",
    L"17",
    L"18 Наличие состава 17  16  15  14",
    L"19 Наличие состава",
    L"20 Реле времени",
    L"21 Реле КН (ВИ-2010)",
    L"22",
    L"23",
    L"24",
    L"25",
    L"26",
    L"27",
    L"28 Датчик вскрытия шкафа",
    L"29 Счет колес  Педаль 2 сенсор 2  БИ1.6 – БФ1.6",
    L"30 Датчик скорости  Педаль 2 сенсор 1  БИ1.17 – БФ1.17",
    L"31 Счет вагонов  Педаль 1 сенсор 2  БИ1.5 – БФ1.5",
    L"32 Начало состава  Педаль 1 сенсор 1  БИ1.12 – БФ1.12"
};

HWND hChecks[TOTAL_BITS];

HWND hEditBin;
HWND hEditHex;

uint32_t gMask = 0;

bool gInternalUpdate = false;

void SetLabelText(HWND hLabel, const wchar_t* text)
{
    SetWindowText(hLabel, text);
}

void MaskToBin(uint32_t value, wchar_t* out)
{
    for (int i = 31; i >= 0; --i)
    {
        out[31 - i] = (value & (1u << i)) ? L'1' : L'0';
    }

    out[32] = 0;
}


uint32_t BinToMask(const wchar_t* str)
{
    uint32_t v = 0;

    for (int i = 0; i < 32 && str[i]; i++)
    {
        if (str[i] == L'1')
        {
            v |= (1u << (31 - i));
        }
    }

    return v;
}

void UpdateChecksFromMask()
{
    for (int i = 0; i < 32; i++)
    {
        BOOL checked = (gMask & (1u << i)) ? BST_CHECKED : BST_UNCHECKED;

        SendMessage(
            hChecks[i],
            BM_SETCHECK,
            checked,
            0
        );
    }
}

void UpdateEdits()
{
    gInternalUpdate = true;

    wchar_t bin[33];
    wchar_t hex[16];

    MaskToBin(gMask, bin);

    wsprintf(hex, L"%08X", gMask);

    SetWindowText(hEditBin, bin);
    SetWindowText(hEditHex, hex);

    gInternalUpdate = false;
}

struct EditBlock
{
    wchar_t label[64];
    HWND hLabel;
    HWND hEdit;
};

Item leftItems[LEFT_COUNT];
Item rightItems[RIGHT_COUNT];

EditBlock bottomEdits[2];

HWND hTopLabel;
HFONT hSmallFont;

// ---- UI creation helper ----
HWND CreateLabel(HWND parent, int x, int y, int w, int h, const wchar_t* text)
{
    return CreateWindowEx(
        0, L"STATIC", text,
        WS_CHILD | WS_VISIBLE,
        x, y, w, h,
        parent, NULL, NULL, NULL
    );
}


HWND CreateCheckbox(HWND parent, int id, int x, int y)
{
    return CreateWindowEx(
        0, L"BUTTON", L"",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, y, 20, 20,
        parent, (HMENU)id, NULL, NULL
    );
}

HWND CreateEdit(HWND parent, int id, int x, int y, int w, int h)
{
    return CreateWindowEx(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        x, y, w, h,
        parent, (HMENU)id, NULL, NULL
    );
}

// ---- Window Proc ----
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
		hSmallFont = CreateFont(
		14, 0, 0, 0,
		FW_NORMAL,
		FALSE, FALSE,FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"Tahoma"
		);

		int winW = 920;
        int winH = 640;

        SetWindowPos(hwnd, NULL, 100, 100, winW, winH, SWP_NOZORDER);

        // --- TOP LABEL ---
        hTopLabel =  CreateWindowEx(0,L"STATIC",L"Калькулятор Маски", WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 10, winW, 25, hwnd, NULL, NULL, NULL);

        // --- LEFT / RIGHT LISTS ---
        int startY = 60;
        int rowH = 28;

		int leftX = 20;
		int rightX = 520;

		int y = 60;

		for (int i = 0; i < 16; i++)
		{
			HWND tLabet = CreateLabel(hwnd, leftX, y, 350, 20, bitNames[31 - i]);
			SendMessage(tLabet, WM_SETFONT, (WPARAM)hSmallFont, TRUE);

			hChecks[31 - i] =
				CreateCheckbox(
					hwnd,
					ID_CHECK_BASE + (31 - i),
					leftX + 350, 
					y
				);

			y += 28;
		}

		y = 60;

		for (int i = 0; i < 16; i++)
		{
			HWND tLabet = CreateLabel(hwnd, rightX-100, y, 450, 20, bitNames[15 - i]);
			SendMessage(tLabet, WM_SETFONT, (WPARAM)hSmallFont, TRUE);

			hChecks[15 - i] =
				CreateCheckbox(
					hwnd,
					ID_CHECK_BASE + (15 - i),
					rightX-100 + 450,
					y
				);

			y += 28;
		}


        // --- BOTTOM EDIT BLOCKS ---
        //int bottomY = 60 + LEFT_COUNT * rowH + 30;

		swprintf(bottomEdits[0].label, 64, L"BIN");
		bottomEdits[0].hLabel = CreateLabel(hwnd, 200, 520, 80, 20, bottomEdits[0].label);

		swprintf(bottomEdits[1].label, 64, L"HEX");
		bottomEdits[1].hLabel = CreateLabel(hwnd, 200, 560, 80, 20, bottomEdits[1].label);

		hEditBin = CreateEdit(hwnd, ID_EDIT_BIN, 250, 520, 400, 25);
		hEditHex = CreateEdit(hwnd, ID_EDIT_HEX, 250, 560, 400, 25);

        return 0;
    }

	case WM_COMMAND:
	{
		int id = LOWORD(wParam);
		int code = HIWORD(wParam);

		// ---- CHECKBOX ----
		if (id >= ID_CHECK_BASE &&
			id < ID_CHECK_BASE + 32 &&
			code == BN_CLICKED)
		{
			int bit = id - ID_CHECK_BASE;

			BOOL checked =
				SendMessage(
					hChecks[bit],
					BM_GETCHECK,
					0,
					0
				) == BST_CHECKED;

			if (checked)
				gMask |= (1u << bit);
			else
				gMask &= ~(1u << bit);

			UpdateEdits();
		}

		// ---- BIN EDIT ----
		if (id == ID_EDIT_BIN &&
			code == EN_CHANGE &&
			!gInternalUpdate)
		{
			wchar_t buf[64];

			GetWindowText(hEditBin, buf, 64);

			gMask = BinToMask(buf);

			UpdateChecksFromMask();
			UpdateEdits();
		}

		// ---- HEX EDIT ----
		if (id == ID_EDIT_HEX &&
			code == EN_CHANGE &&
			!gInternalUpdate)
		{
			wchar_t buf[64];

			GetWindowText(hEditHex, buf, 64);

			swscanf(buf, L"%X", &gMask);

			UpdateChecksFromMask();
			UpdateEdits();
		}

		return 0;
	}

    case WM_DESTROY:
        PostQuitMessage(0);
		DeleteObject(hSmallFont);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ---- WinMain ----
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASS wc = {0};

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Калькулятор Маски",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 800,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}