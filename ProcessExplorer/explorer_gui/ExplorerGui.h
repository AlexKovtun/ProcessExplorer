#pragma once
#include <d3d9.h>

namespace gui
{
	constexpr int WIDTH = 700;
	constexpr int HEIGHT = 560;

	inline bool done = false;

	//winapi window vars
	inline HWND window = nullptr;
	inline WNDCLASSEX windowClass = {};

	//points for window movement
	inline POINTS position = {};

	//drirectX  state vars
	inline PDIRECT3D9 d3d = nullptr;
	inline LPDIRECT3DDEVICE9 device = nullptr;
	inline D3DPRESENT_PARAMETERS present_params = {};

	//handle creating and distrcution of our window
	void CreateHWindow(
		const char* windowName,const char* className)noexcept;
	void DestroyHWindow() noexcept;

	//handle device creation and manipulation
	bool CreateDevice(HWND hWnd) noexcept;
	void CleanupDevice() noexcept;
	void ResetDevice() noexcept;

	// handle ImGui creating and destruction
	void CreateImGui() noexcept;
	void DestroyImGui() noexcept;

	// handling rendering
	void BeginRender()noexcept;
	void EndRender()noexcept;
	void Render()noexcept;

}

