#define UNICODE
#define _UNICODE

#include <helpers/foobar2000+atl.h>
#include <libPPUI/win32_op.h>
#include <helpers/BumpableElem.h>

#include <libprojectM/projectM.h>
#include <GL/glew.h>


DECLARE_COMPONENT_VERSION("projectM visualizer", "0.0.4",
"projectM - The most advanced open-source music visualizer\n"
"Copyright (C) 2003 - 2022 projectM Team\n"
"foobar2000 plugin by djdron (C) 2020 - 2022.\n\n"

"Distributed under the terms of GNU LGPL v2.1\n"
"Source code can be obtained from:\n"
"https://github.com/projectM-visualizer/projectm\n"
"https://github.com/djdron/foo_vis_projectM\n"
);

VALIDATE_COMPONENT_FILENAME("foo_vis_projectM.dll");

namespace
{

static const GUID guid_cfg_preset_lock		= { 0xe5be745e, 0xab65, 0x4b69, { 0xa1, 0xf3, 0x1e, 0xfb, 0x08, 0xff, 0x4e, 0xcf } };
static const GUID guid_cfg_preset_shuffle	= { 0x659c6787, 0x97bb, 0x485b, { 0xa0, 0xfc, 0x45, 0xfb, 0x12, 0xb7, 0x3a, 0xa0 } };
static const GUID guid_cfg_preset_name		= { 0x186c5741, 0x701e, 0x4f2c, { 0xb4, 0x41, 0xe5, 0x57, 0x5c, 0x18, 0xb0, 0xa8 } };
static const GUID guid_cfg_preset_duration	= { 0x48d9b7f5, 0x4446, 0x4ab7, { 0xb8, 0x71, 0xef, 0xc7, 0x59, 0x43, 0xb9, 0xcd } };

static cfg_bool cfg_preset_lock(guid_cfg_preset_lock, false);
static cfg_bool cfg_preset_shuffle(guid_cfg_preset_shuffle, true);
static cfg_string cfg_preset_name(guid_cfg_preset_name, "");
static cfg_int cfg_preset_duration(guid_cfg_preset_duration, 20);

class ui_element_instance_projectM : public ui_element_instance, public CWindowImpl<ui_element_instance_projectM>
{
public:
	DECLARE_WND_CLASS_EX(TEXT("{09E9C47E-87E7-45CD-9C16-1A0926E90FAD}"), CS_DBLCLKS|CS_OWNDC, (-1));

	ui_element_instance_projectM(ui_element_config::ptr p_config, ui_element_instance_callback_ptr p_callback) : m_config(p_config), m_callback(p_callback) {}

	void initialize_window(HWND parent);

	BEGIN_MSG_MAP_EX(ui_element_instance_projectM)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CONTEXTMENU(OnContextMenu)
	END_MSG_MAP()

	virtual void set_configuration(ui_element_config::ptr config) override { m_config = config; }
	virtual ui_element_config::ptr get_configuration() override { return m_config; }

	enum ui_menu_id
	{
		ID_FULLSCREEN = 1,
		ID_PRESET_LOCK, ID_PRESET_SHUFFLE, ID_PRESET_NEXT, ID_PRESET_PREVIOUS, ID_PRESET_RANDOM,
		ID_DURATION_5, ID_DURATION_10, ID_DURATION_20, ID_DURATION_30, ID_DURATION_45, ID_DURATION_60
	};
	virtual bool edit_mode_context_menu_test(const POINT& p_point, bool p_fromkeyboard) { return true; }
	virtual void edit_mode_context_menu_build(const POINT& p_point, bool p_fromkeyboard, HMENU p_menu, unsigned p_id_base) override { ContextMenuBuild(p_menu, p_id_base); }
	virtual void edit_mode_context_menu_command(const POINT& p_point, bool p_fromkeyboard, unsigned p_id, unsigned p_id_base) override { ContextMenuCommand(p_id - p_id_base); }
	virtual bool edit_mode_context_menu_get_description(unsigned p_id, unsigned p_id_base, pfc::string_base& p_out) override { return ContextMenuGetDesc(p_id - p_id_base, p_out); }

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
	void OnLButtonDblClk(UINT nFlags, CPoint point) { static_api_ptr_t<ui_element_common_methods_v2>()->toggle_fullscreen(g_get_guid(), core_api::get_main_window()); }
	void OnPaint(CDCHandle);
	void OnSize(UINT nType, CSize size);
	void OnContextMenu(CWindow wnd, CPoint point);

	void AddPCM();

	static VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);
	static void PresetSwitchedCallback(bool is_hard_cut, unsigned int index, void* user_data);

	void OnTimer();
	void OnPresetSwitched(bool is_hard_cut, unsigned int index);

	bool SetupGLContext()
	{
		if(m_projectM && m_GLrc)
		{
			wglMakeCurrent(GetDC(), m_GLrc);
			return true;
		}
		return false;
	}

	void ContextMenuBuild(HMENU p_menu, unsigned p_id_base);
	bool ContextMenuGetDesc(int p_id, pfc::string_base& p_out);
	void ContextMenuCommand(int cmd);

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
	settings.preset_duration = cfg_preset_duration; // seconds
	settings.hard_cut_enabled = true;
	settings.hard_cut_duration = 20;
	settings.hard_cut_sensitivity = 1.0;
	settings.beat_sensitivity = 1.0;
	settings.aspect_correction = true;
	settings.shuffle_enabled = cfg_preset_shuffle;
	std::string data_zip = base_path + "data.zip";
	settings.data_dir = const_cast<char*>(data_zip.c_str());
	// init with settings
	m_projectM = projectm_create_settings(&settings, PROJECTM_FLAG_NONE);
	projectm_set_preset_switched_event_callback(m_projectM, PresetSwitchedCallback, this);
	bool select_random_preset = true;
	if(!cfg_preset_name.isEmpty())
	{
		auto idx = projectm_get_preset_index(m_projectM, cfg_preset_name.c_str());
		if(idx > 0)
		{
			projectm_select_preset(m_projectM, idx, true);
			select_random_preset = false;
		}
	}
	if(select_random_preset)
		projectm_select_random_preset(m_projectM, true);
	if(cfg_preset_lock)
		projectm_lock_preset(m_projectM, true);

	VsyncGL(true);

//	console::formatter() << "projectM: OnCreate";

	return 0;
}
void ui_element_instance_projectM::OnDestroy()
{
	m_vis_stream.release();
	SetupGLContext();
	if(m_projectM)
	{
		projectm_destroy(m_projectM);
		m_projectM = NULL;
	}
	wglMakeCurrent(NULL, NULL);
	if(m_GLrc)
	{
		wglDeleteContext(m_GLrc);
		m_GLrc = NULL;
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

void ui_element_instance_projectM::OnPresetSwitched(bool is_hard_cut, unsigned int index)
{
	const char* name = projectm_get_preset_name(m_projectM, index);
	cfg_preset_name = name;
	projectm_free_string(name);
}

void ui_element_instance_projectM::PresetSwitchedCallback(bool is_hard_cut, unsigned int index, void* user_data)
{
	auto ui = (ui_element_instance_projectM*)user_data;
	ui->OnPresetSwitched(is_hard_cut, index);
}

void ui_element_instance_projectM::OnPaint(CDCHandle)
{
	if(SetupGLContext())
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		projectm_render_frame(m_projectM);

		glFlush();

		SwapBuffers(GetDC());
	}

	ValidateRect(NULL);

	AddPCM();

	ResetEvent(m_timerDoneEvent);
	CreateTimerQueueTimer(&m_timer, m_timerQueue, (WAITORTIMERCALLBACK)TimerRoutine, this, 10, 0, 0);
}

void ui_element_instance_projectM::OnSize(UINT nType, CSize size)
{
	if(size.cx && size.cy && SetupGLContext())
	{
		projectm_set_window_size(m_projectM, size.cx, size.cy);
//		console::formatter() << "projectM: OnSize " << size.cx << ", " << size.cy;
	}
}
void ui_element_instance_projectM::OnContextMenu(CWindow wnd, CPoint point)
{
	if(m_callback->is_edit_mode_enabled())
	{
		SetMsgHandled(FALSE);
		return;
	}
	if(point == CPoint(-1, -1))
	{
		CRect rc;
		WIN32_OP(wnd.GetWindowRect(&rc));
		point = rc.CenterPoint();
	}
	CMenu menu;
	WIN32_OP(menu.CreatePopupMenu());
	ContextMenuBuild(menu, 0);
	menu.SetMenuDefaultItem(ID_FULLSCREEN);

	CMenuSelectionReceiver_UiElement descriptions(this, 0);
	auto cmd = menu.TrackPopupMenuEx(TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, point.x, point.y, descriptions, NULL);
	ContextMenuCommand(cmd);
}

void ui_element_instance_projectM::ContextMenuBuild(HMENU p_menu, unsigned p_id_base)
{
	CMenuHandle menu(p_menu);
	menu.AppendMenu(MF_STRING, p_id_base + ID_FULLSCREEN, L"Toggle Full-Screen Mode");
	menu.AppendMenu(MF_SEPARATOR);

	menu.AppendMenu(MF_STRING|(cfg_preset_lock ? MF_CHECKED : 0), p_id_base + ID_PRESET_LOCK, L"Lock Current Preset");
	menu.AppendMenu(MF_STRING|(cfg_preset_shuffle ? MF_CHECKED : 0), p_id_base + ID_PRESET_SHUFFLE, L"Shuffle Presets");
	menu.AppendMenu(MF_STRING, p_id_base + ID_PRESET_NEXT, L"Next Preset");
	menu.AppendMenu(MF_STRING, p_id_base + ID_PRESET_PREVIOUS, L"Previous Preset");
	menu.AppendMenu(MF_STRING, p_id_base + ID_PRESET_RANDOM, L"Random Preset");

	CMenuHandle menu_duration;
	WIN32_OP(menu_duration.CreatePopupMenu());
	menu_duration.AppendMenu(MF_STRING, p_id_base + ID_DURATION_5,  L"5");
	menu_duration.AppendMenu(MF_STRING, p_id_base + ID_DURATION_10, L"10");
	menu_duration.AppendMenu(MF_STRING, p_id_base + ID_DURATION_20, L"20");
	menu_duration.AppendMenu(MF_STRING, p_id_base + ID_DURATION_30, L"30");
	menu_duration.AppendMenu(MF_STRING, p_id_base + ID_DURATION_45, L"45");
	menu_duration.AppendMenu(MF_STRING, p_id_base + ID_DURATION_60, L"60");
	auto DurationToId = [](int duration)
	{
		switch(duration)
		{
		case 5:  return ID_DURATION_5;
		case 10: return ID_DURATION_10;
		case 20: return ID_DURATION_20;
		case 30: return ID_DURATION_30;
		case 45: return ID_DURATION_45;
		case 60: return ID_DURATION_60;
		}
		return ID_DURATION_20;
	};
	menu_duration.CheckMenuRadioItem(p_id_base + ID_DURATION_5, p_id_base + ID_DURATION_60, p_id_base + DurationToId(cfg_preset_duration), MF_BYCOMMAND);
	menu.AppendMenu(MF_STRING, menu_duration, L"Preset Duration");
}

bool ui_element_instance_projectM::ContextMenuGetDesc(int p_id, pfc::string_base& p_out)
{
	switch(p_id)
	{
	case ID_FULLSCREEN:		p_out = "Toggles full-screen mode.";	break;
	case ID_PRESET_LOCK:	p_out = "Lock the current preset.";		break;
	case ID_PRESET_SHUFFLE:	p_out = "Shuffle presets.";				break;
	case ID_PRESET_NEXT:	p_out = "Switch to next preset (without shuffle)."; break;
	case ID_PRESET_PREVIOUS:p_out = "Switch to previous preset (without shuffle)."; break;
	case ID_PRESET_RANDOM:	p_out = "Switch to random preset.";		break;
	case ID_DURATION_5:		p_out = "Duration 5 seconds.";			break;
	case ID_DURATION_10:	p_out = "Duration 10 seconds.";			break;
	case ID_DURATION_20:	p_out = "Duration 20 seconds.";			break;
	case ID_DURATION_30:	p_out = "Duration 30 seconds.";			break;
	case ID_DURATION_45:	p_out = "Duration 45 seconds.";			break;
	case ID_DURATION_60:	p_out = "Duration 60 seconds.";			break;
	default: return false;
	}
	return true;
}

void ui_element_instance_projectM::ContextMenuCommand(int cmd)
{
	switch(cmd)
	{
	case ID_FULLSCREEN:
		static_api_ptr_t<ui_element_common_methods_v2>()->toggle_fullscreen(g_get_guid(), core_api::get_main_window());
		break;
	case ID_PRESET_LOCK:
		cfg_preset_lock = !cfg_preset_lock;
		projectm_lock_preset(m_projectM, cfg_preset_lock);
		break;
	case ID_PRESET_SHUFFLE:
		cfg_preset_shuffle = !cfg_preset_shuffle;
		projectm_set_shuffle_enabled(m_projectM, cfg_preset_shuffle);
		break;
	case ID_PRESET_NEXT:
		if(SetupGLContext())
			projectm_select_next_preset(m_projectM, true);
		break;
	case ID_PRESET_PREVIOUS:
		if(SetupGLContext())
			projectm_select_previous_preset(m_projectM, true);
		break;
	case ID_PRESET_RANDOM:
		if(SetupGLContext())
			projectm_select_random_preset(m_projectM, true);
		break;
	case ID_DURATION_5:
		cfg_preset_duration = 5;
		projectm_set_preset_duration(m_projectM, cfg_preset_duration);
		break;
	case ID_DURATION_10:
		cfg_preset_duration = 10;
		projectm_set_preset_duration(m_projectM, cfg_preset_duration);
		break;
	case ID_DURATION_20:
		cfg_preset_duration = 20;
		projectm_set_preset_duration(m_projectM, cfg_preset_duration);
		break;
	case ID_DURATION_30:
		cfg_preset_duration = 30;
		projectm_set_preset_duration(m_projectM, cfg_preset_duration);
		break;
	case ID_DURATION_45:
		cfg_preset_duration = 45;
		projectm_set_preset_duration(m_projectM, cfg_preset_duration);
		break;
	case ID_DURATION_60:
		cfg_preset_duration = 60;
		projectm_set_preset_duration(m_projectM, cfg_preset_duration);
		break;
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

}
//namespace
