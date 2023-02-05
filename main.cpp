#include "main.h"
#include <string>
#include <windows.h>
#include <cmath>
const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
const int screenHeight = GetSystemMetrics(SM_CYSCREEN);


static void DrawFilledRect(HDC hdc2, int x, int y, int w, int h)
{
	MoveToEx(hdc2, x - 2, y, NULL); // yan çizgi
	LineTo(hdc2, x - 2 + w, y - h); // yan çizgi
	MoveToEx(hdc2, x - 3, y, NULL); // yan çizgi
	LineTo(hdc2, x - 3 + w, y - h); // yan çizgi
}


HFONT Font;
void DrawString(HDC hdc, int x, int y, const char* text) {
	SetTextAlign(hdc, TA_CENTER | TA_NOUPDATECP);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 128, 0));
	SelectObject(hdc, Font);
	TextOutA(hdc, x, y, text, strlen(text));
	DeleteObject(Font);
}

bool NormalBox = true;
bool Healthbar = true;
bool Snapline = true;
bool NameESP = true;	

Vector3 screenHeadPos;
Vector3 screenPos;
uintptr_t pEntity;
uintptr_t entHp;
uintptr_t entTeam;
float boxHeight;
float boxWeight;


int main()
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE); // HIDE CONSOLE WINDOW
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	HWND counter = FindWindow(NULL, L"Counter-Strike: Global Offensive - Direct3D 9");
	GetWindowRect(counter, &Rect);
	mem.GetInfoWindow(L"Counter-Strike: Global Offensive - Direct3D 9", procces_id, game);
	moduleBase_client = mem.GetModuleBaseAddress(procces_id, L"client.dll");
	moduleBase_engine = mem.GetModuleBaseAddress(procces_id, L"engine.dll");

	while (true) {
		if (GetAsyncKeyState(VK_END))
			exit(0);

		playerLocal = mem.Read<uintptr_t>(moduleBase_client + hazedumper::signatures::dwLocalPlayer);
		uintptr_t localTeam = mem.Read<uintptr_t>(playerLocal + 0xF4);
		mem.Read(moduleBase_client + hazedumper::signatures::dwViewMatrix, viewMatrix, 64);

		for (int i = 0; i <= 32; i++) {
			pEntity = mem.Read<uintptr_t>(moduleBase_client + hazedumper::signatures::dwEntityList + (i * 0x10)); if (!pEntity) { continue; }
			entHp = mem.Read<uintptr_t>(pEntity + hazedumper::netvars::m_iHealth); if (!entHp) { continue; }
			entTeam = mem.Read<uintptr_t>(pEntity + hazedumper::netvars::m_iTeamNum); if (localTeam == entTeam) { continue; }
			if (mem.Read<bool>(pEntity + hazedumper::signatures::m_bDormant)) { continue; }
			Vector3 entPos = mem.Read<Vector3>(pEntity + hazedumper::netvars::m_vecOrigin);
			Vector3 entHeadPos = getBonePos(pEntity, 8);
			if (!WorldToScreen(entPos, screenPos, viewMatrix)) { continue; }
			if (!WorldToScreen(entHeadPos, screenHeadPos, viewMatrix)) { continue; }
			boxHeight = abs(screenHeadPos.y - screenPos.y);
			boxWeight = boxHeight / 2;

			HPEN EspPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
			HPEN EspPen2 = CreatePen(PS_SOLID, 1, RGB(80, 220, 100));
			HPEN EspPen3 = CreatePen(PS_SOLID, 1, RGB(30, 30, 30));
			HDC hdc = GetDC(counter);  SelectObject(hdc, EspPen);

			if (NormalBox)
			{
				MoveToEx(hdc, screenHeadPos.x - boxWeight / 2, screenHeadPos.y, NULL); // üst
				LineTo(hdc, screenHeadPos.x + boxWeight / 2, screenHeadPos.y); // üst

				MoveToEx(hdc, screenHeadPos.x - boxWeight / 2, screenPos.y, NULL); // alt
				LineTo(hdc, screenHeadPos.x + boxWeight / 2, screenPos.y); // alt

				MoveToEx(hdc, screenHeadPos.x - boxWeight / 2, screenHeadPos.y, NULL); // yan çizgi
				LineTo(hdc, screenHeadPos.x - boxWeight / 2, screenPos.y); // yan çizgi

				MoveToEx(hdc, screenHeadPos.x + boxWeight / 2, screenHeadPos.y, NULL); // yan çizgi
				LineTo(hdc, screenHeadPos.x + boxWeight / 2, screenPos.y); // yan çizgi
			}
			if (Healthbar)
			{
				if (entHp > 101) entHp = 100;
				float HealthHeightCalc = ((float)entHp / 100) * (float)boxHeight;

				SelectObject(hdc, EspPen3);
				DrawFilledRect(hdc, screenHeadPos.x - (boxWeight / 2) - 1, screenPos.y, 0, boxHeight);
				SelectObject(hdc, EspPen2);
				DrawFilledRect(hdc, screenHeadPos.x - (boxWeight / 2) - 1, screenPos.y, 0, HealthHeightCalc);
			}
			if (NameESP)
			{
				uintptr_t dwClientState = mem.Read< uintptr_t >(moduleBase_engine + hazedumper::signatures::dwClientState);
				uintptr_t UserInfoTable = mem.Read< uintptr_t >(dwClientState + hazedumper::signatures::dwClientState_PlayerInfo);
				uintptr_t x = mem.Read< std::uintptr_t >(mem.Read< uintptr_t >(UserInfoTable + 0x40) + 0xC);
				player_info_t p = mem.Read< player_info_t >(mem.Read< uintptr_t >(x + 0x28 + 0x34 * i));
				DrawString(hdc, screenPos.x, screenPos.y, p.name);
			}
			if (Snapline)
			{
				MoveToEx(hdc, screenWidth / 2, screenHeight, NULL); // yan çizgi
				LineTo(hdc, screenPos.x, screenPos.y); // yan çizgi
			}
			
			ReleaseDC(NULL, hdc); DeleteObject(EspPen); DeleteObject(EspPen2); DeleteObject(EspPen3);
		}
	}

}

