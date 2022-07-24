#define UNICODE
#define _UNICODE

#include <helpers/foobar2000+atl.h>
#include <libPPUI/win32_op.h>
#include <helpers/BumpableElem.h>

#undef _UNICODE
#undef UNICODE

#include <libprojectM/projectM.h>
#include <GL/glew.h>

#include <memory>

DECLARE_COMPONENT_VERSION("projectM visualizer", "0.0.2",
"projectM - The most advanced open-source music visualizer\n"
"Copyright (C) 2003 - 2022 projectM Team\n"
"foobar2000 plugin by djdron (C) 2020 - 2022.\n\n"

"Distributed under the terms of GNU LGPL v2.1\n"
"Source code can be obtained from:\n"
"https://github.com/projectM-visualizer/projectm\n"
"https://github.com/djdron/foo_vis_projectM\n"
);

VALIDATE_COMPONENT_FILENAME("foo_vis_projectM.dll");


class ui_element_instance_projectM : public ui_element_instance, public CWindowImpl<ui_element_instance_projectM>
{
public:
	DECLARE_WND_CLASS_EX(TEXT("{09E9C47E-87E7-45CD-9C16-1A0926E90FAD}"), CS_DBLCLKS|CS_OWNDC, (-1));

	ui_element_instance_projectM(ui_element_config::ptr p_config, ui_element_instance_callback_ptr p_callback) : m_config(p_config), m_callback(p_callback) {}

	void initialize_window(HWND parent);

	BEGIN_MSG_MAP_EX(ui_element_instance_projectM)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_SIZE(OnSize)
	END_MSG_MAP()

	HWND get_wnd() { return *this; }
	void set_configuration(ui_element_config::ptr config) { m_config = config; }
	ui_element_config::ptr get_configuration() { return m_config; }
	static GUID g_get_guid() {
		static const GUID guid_myelem = { 0x489c7f0e, 0x2073, 0x442b, {0xaf, 0x4a, 0x00, 0x51, 0x99, 0x12, 0xaf, 0x70 } };
		return guid_myelem;
	}
	static GUID g_get_subclass() { return ui_element_subclass_playback_visualisation; }
	static void g_get_name(pfc::string_base & out) { out = "projectM"; }
	static ui_element_config::ptr g_get_default_configuration() { return ui_element_config::g_create_empty(g_get_guid()); }
	static const char * g_get_description() { return "projectM visualization."; }
	
private:
	LRESULT OnCreate(LPCREATESTRUCT cs);
	void OnDestroy();
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnLButtonDblClk(UINT nFlags, CPoint point) { static_api_ptr_t<ui_element_common_methods_v2>()->toggle_fullscreen(g_get_guid(), core_api::get_main_window()); }
	void OnPaint(CDCHandle);
	void OnSize(UINT nType, CSize size);

	void AddPCM();

	static VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);
	void OnTimer();

private:
	visualisation_stream_v2::ptr m_vis_stream;
	projectm* m_projectM = NULL;
	HGLRC m_GLrc = NULL;
	double m_last_time = 0.0;

	HANDLE m_timerQueue = CreateTimerQueue();
	HANDLE m_timer = NULL;
	HANDLE m_timerDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

protected:
	ui_element_config::ptr m_config;
	const ui_element_instance_callback_ptr m_callback;
};

void ui_element_instance_projectM::initialize_window(HWND parent)
{
	WIN32_OP(Create(parent) != NULL);
}

typedef BOOL(WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int interval);
static void VsyncGL(bool on)
{
	static bool inited = false;
	static PFNWGLSWAPINTERVALEXTPROC si = NULL;
	if(!inited)
	{
		si = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		inited = true;
	}
	if(si) si(on);
}

LRESULT ui_element_instance_projectM::OnCreate(LPCREATESTRUCT cs)
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Flags
		PFD_TYPE_RGBA, // The kind of framebuffer. RGBA or palette.
		32, // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		24, // Number of bits for the depthbuffer
		8,	// Number of bits for the stencilbuffer
		0,	// Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	HDC dc = GetDC();
	int pf = ChoosePixelFormat(dc, &pfd);
	SetPixelFormat(dc, pf, &pfd);

	m_GLrc = wglCreateContext(dc);
	wglMakeCurrent(dc, m_GLrc);

	static_api_ptr_t<visualisation_manager> vis_manager;
	vis_manager->create_stream(m_vis_stream, 0);
	m_vis_stream->request_backlog(0.8);

	std::string base_path = core_api::get_my_full_path();
	std::string::size_type t = base_path.rfind('\\');
	if(t != std::string::npos) base_path.erase(t + 1);

	RECT r;
	GetClientRect(&r);
	int width = r.right - r.left;
	int height = r.bottom - r.top;
	if(width < 128) width = 128;
	if(height < 128) height = 128;

	float heightWidthRatio = (float)height / (float)width;

	projectm_settings settings = {};
	settings.window_width = width;
	settings.window_height = height;
	settings.mesh_x = 128;
	settings.mesh_y = int(settings.mesh_x * heightWidthRatio);
	settings.fps = 60;
	settings.soft_cut_duration = 3; // seconds
	settings.preset_duration = 30; // seconds
	settings.hard_cut_enabled = true;
	settings.hard_cut_duration = 20;
	settings.hard_cut_sensitivity = 1.0;
	settings.beat_sensitivity = 1.0;
	settings.aspect_correction = true;
	settings.shuffle_enabled = true;
	std::string preset_url = base_path + "presets";
	settings.preset_url = const_cast<char*>(preset_url.c_str());
	// init with settings
	m_projectM = projectm_create_settings(&settings, PROJECTM_FLAG_NONE);
	projectm_select_random_preset(m_projectM, true);

	VsyncGL(true);

//	console::formatter() << "projectM: OnCreate";

	return 0;
}
void ui_element_instance_projectM::OnDestroy()
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(m_GLrc);
	m_GLrc = NULL;
	m_vis_stream.release();
	if(m_projectM)
	{
		projectm_destroy(m_projectM);
		m_projectM = NULL;
	}

	if(m_timer)
		WaitForSingleObject(m_timerDoneEvent, INFINITE);
	CloseHandle(m_timerDoneEvent);
	DeleteTimerQueue(m_timerQueue);

//	console::formatter() << "projectM: OnDestroy";
}

void ui_element_instance_projectM::OnTimer()
{
	Invalidate();
	SetEvent(m_timerDoneEvent);
}

VOID CALLBACK ui_element_instance_projectM::TimerRoutine(
	PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	auto ui = (ui_element_instance_projectM *)lpParam;
	ui->OnTimer();
}

void ui_element_instance_projectM::OnRButtonDown(UINT nFlags, CPoint point)
{
	if(m_projectM && m_GLrc)
	{
		wglMakeCurrent(GetDC(), m_GLrc);
		projectm_select_random_preset(m_projectM, true);
	}
}

void ui_element_instance_projectM::OnPaint(CDCHandle)
{
	if(!m_projectM || !m_GLrc) return;
	wglMakeCurrent(GetDC(), m_GLrc);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projectm_render_frame(m_projectM);

	glFlush();

	SwapBuffers(GetDC());

	ValidateRect(NULL);

	AddPCM();

	ResetEvent(m_timerDoneEvent);
	CreateTimerQueueTimer(&m_timer, m_timerQueue, (WAITORTIMERCALLBACK)TimerRoutine, this, 10, 0, 0);
}

void ui_element_instance_projectM::OnSize(UINT nType, CSize size)
{
	if(m_projectM && m_GLrc && size.cx && size.cy)
	{
		wglMakeCurrent(GetDC(), m_GLrc);
		projectm_set_window_size(m_projectM, size.cx, size.cy);
//		console::formatter() << "projectM: OnSize " << size.cx << ", " << size.cy;
	}
}

void ui_element_instance_projectM::AddPCM()
{
	if(!m_vis_stream.is_valid() || !m_projectM) return;

	double time;
	if(!m_vis_stream->get_absolute_time(time)) return;

	double dt = time - m_last_time;
	m_last_time = time;

	double min_time = 1.0/1000.0;
	double max_time = 1.0/10.0;

	bool use_fake = false;

	if(dt < min_time)
	{
		dt = min_time;
		use_fake = true;
	}
	if(dt > max_time) dt = max_time;

	audio_chunk_impl chunk;
	if(use_fake || !m_vis_stream->get_chunk_absolute(chunk, time - dt, dt))
		m_vis_stream->make_fake_chunk_absolute(chunk, time - dt, dt);
	t_size count = chunk.get_sample_count();
	auto channels = chunk.get_channel_count();
	if(channels == 2)
		projectm_pcm_add_float(m_projectM, chunk.get_data(), count, PROJECTM_STEREO);
	else
		projectm_pcm_add_float(m_projectM, chunk.get_data(), count, PROJECTM_MONO);
}

class ui_element_projectM : public ui_element_impl_visualisation<ui_element_instance_projectM> {};
static service_factory_single_t<ui_element_projectM> g_ui_element_projectM_factory;
