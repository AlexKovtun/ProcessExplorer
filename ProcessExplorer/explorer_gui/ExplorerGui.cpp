#include "ExplorerGui.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>




LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
HWND window,
UINT message,
WPARAM wideParameter,
LPARAM longParameter);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (gui::device != NULL && wParam != SIZE_MINIMIZED)
		{
			gui::present_params.BackBufferWidth = LOWORD(lParam);
			gui::present_params.BackBufferHeight = HIWORD(lParam);
			gui::ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			//const int dpi = HIWORD(wParam);
			//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
			const RECT* suggested_rect = (RECT*)lParam;
			::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}


void gui::CreateHWindow(const char* windowName, const char* className) noexcept
{
	gui::windowClass = {
		sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ProcessExplorer"), NULL };


	::RegisterClassEx(&windowClass);
	window = ::CreateWindow(windowClass.lpszClassName, _T("ProcessExplorer"), WS_OVERLAPPEDWINDOW,
		100, 100, 760, 520, NULL, NULL, windowClass.hInstance, NULL);
		
	// Initialize Direct3D
	if (!CreateDevice(gui::window))
	{
		CleanupDevice();
		::UnregisterClass(gui::windowClass.lpszClassName, gui::windowClass.hInstance);
		return ;
	}

	// Show the window
	::ShowWindow(gui::window, SW_SHOWDEFAULT);
	::UpdateWindow(gui::window);
}


void gui::DestroyHWindow() noexcept
{
	DestroyWindow(gui::window);
	UnregisterClass(gui::windowClass.lpszClassName, gui::windowClass.hInstance);
}


// Helper functions

bool gui::CreateDevice(HWND hWnd) noexcept
{
	if ((gui::d3d = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the device
	ZeroMemory(&gui::present_params, sizeof(gui::present_params));
	gui::present_params.Windowed = TRUE;
	gui::present_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	gui::present_params.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	gui::present_params.EnableAutoDepthStencil = TRUE;
	gui::present_params.AutoDepthStencilFormat = D3DFMT_D16;
	gui::present_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//present_params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (gui::d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &present_params, &gui::device) < 0)
		return false;

	return true;
}

void gui::CleanupDevice() noexcept
{
	if (gui::device) { gui::device->Release(); gui::device = NULL; }
	if (gui::d3d) { gui::d3d->Release(); gui::d3d = NULL; }
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = gui::device->Reset(&gui::present_params);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

// handle ImGui creating and destruction
void gui::CreateImGui() noexcept
{

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	io.ConfigDockingAlwaysTabBar = true;
	
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// handling 
void gui::BeginRender()noexcept
{
	// Poll and handle messages (inputs, window resize, etc.)
	// See the WndProc() function below for our to dispatch events to the Win32 backend.
	MSG msg;
	while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			done = true;
	}
	if (done)
		return;

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

void gui::Render()noexcept
{
	static float f = 0.0f;
	static int counter = 0;
	ImGuiWindowFlags window_flags = 2;
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	bool x = true; // for what is this boolean?
	ImGui::Begin("main", &x, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);

	//trying to add a table
	ImVec2 size = { 150, 250 };
	ImGui::BeginTable("ProcessesTable", 4, 0, size, 16);
	ImGui::Text("Alex");
	ImGui::TableNextRow(1, 25);
	ImGui::EndTable();
	/*
	IMGUI_API bool          BeginTable(const char* str_id, int column, ImGuiTableFlags flags = 0, const ImVec2 & outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f);
	IMGUI_API void          EndTable();                                         // only call EndTable() if BeginTable() returns true!
	IMGUI_API void          TableNextRow(ImGuiTableRowFlags row_flags = 0, float min_row_height = 0.0f); // append into the first cell of a new row.
	IMGUI_API bool          TableNextColumn();                                  // append into the next column (or first column of next row if currently in last column). Return true when column is visible.
	IMGUI_API bool          TableSetColumnIndex(int column_n);                  // append into the specified column. Return true when column is visible.
	*/
	ImGui::ShowDemoWindow();
	ImGui::End();
}

void gui::EndRender()noexcept
{
	// Rendering
	ImGui::EndFrame();
	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		
	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0,0,0,255), 1.0f, 0);
	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Update and Render additional Platform Windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
	

	HRESULT result = device->Present(NULL, NULL, NULL, NULL);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}


