﻿#pragma once
// Note:
// ZNear - always 0.0f
// ZFar  - always 1.0f
#include <DirectXMath.h>
#include "pure.h"
#include "../xrcore/ftimer.h"
#include "stats.h"
#include "DirectXMathExternal.h"

#define VIEWPORT_NEAR  0.05f

#define DEVICE_RESET_PRECACHE_FRAME_COUNT 10

#include "../Include/xrRender/FactoryPtr.h"
#include "../Include/xrRender/RenderDeviceRender.h"

#ifdef INGAME_EDITOR
#	include "../Include/editor/interfaces.hpp"
#endif // #ifdef INGAME_EDITOR

//for window prop control
ENGINE_API extern u32 ps_vid_windowtype;

enum WindowPropStyle
{
    WPS_Windowed = 1,
    WPS_WindowedBorderless = 2,
    WPS_Fullscreen = 3,
    WPS_FullscreenBorderless = 4
};

class engine_impl;

#pragma pack(push,4)
#pragma warning(push)
#pragma warning(disable: 4366)

class IRenderDevice
{
public:
	virtual		CStatsPhysics*	WINAPI		StatPhysics		()							= 0;								
	virtual				void	WINAPI		AddSeqFrame		( pureFrame* f, bool mt )	= 0;
	virtual				void	WINAPI		RemoveSeqFrame	( pureFrame* f )			= 0;
};

class ENGINE_API CRenderDeviceData
{

public:
	u32										dwWidth;
	u32										dwHeight;
	
	u32										dwPrecacheFrame;
	BOOL									b_is_Ready;
	BOOL									b_is_Active;
public:

		// Engine flow-control
	u32										dwFrame;

	float									fTimeDelta;
	float									fTimeGlobal;
	u32										dwTimeDelta;
	u32										dwTimeGlobal;
	u32										dwTimeContinual;

	Fvector									vCameraPosition;
	Fvector									vCameraDirection;
	Fvector									vCameraTop;
	Fvector									vCameraRight;

	DirectX::XMMATRIX						mView;
	DirectX::XMMATRIX						mProject;
	DirectX::XMMATRIX						mFullTransform;

	// Copies of corresponding members. Used for synchronization.
	Fvector									vCameraPosition_saved;

	DirectX::XMMATRIX						mView_saved;
	DirectX::XMMATRIX						mProject_saved;
	DirectX::XMMATRIX						mFullTransform_saved;

	float									fFOV;
	float									fASPECT;
protected:

	u32										Timer_MM_Delta;
	CTimer_paused							Timer;
	CTimer_paused							TimerGlobal;
	CTimer									frame_timer;
public:

// Registrators
	CRegistrator	<pureRender			>			seqRender;
	CRegistrator	<pureAppActivate	>			seqAppActivate;
	CRegistrator	<pureAppDeactivate	>			seqAppDeactivate;
	CRegistrator	<pureAppStart		>			seqAppStart;
	CRegistrator	<pureAppEnd			>			seqAppEnd;
	CRegistrator	<pureFrame			>			seqFrame;
	CRegistrator	<pureScreenResolutionChanged>	seqResolutionChanged;

	HWND									m_hWnd;
//	CStats*									Statistic;

};

class	ENGINE_API CRenderDeviceBase :
	public IRenderDevice,
	public CRenderDeviceData
{
public:
};

#pragma pack(pop)
// refs
class ENGINE_API CRenderDevice: public CRenderDeviceBase
{
public:
	class ENGINE_API CSecondVPParams //+SecondVP+
	{
	public:
		bool m_bCamReady; // Флаг готовности камеры (FOV, позиция, и т.п) к рендеру второго вьюпорта
	private:
		bool m_bIsActive;  // Флаг активации рендера во второй вьюпорт
		u8   m_FrameDelay; // На каком кадре с момента прошлого рендера во второй вьюпорт мы начнём новый (не может быть меньше 2 - каждый второй кадр, чем больше тем более низкий FPS во втором вьюпорте)
	public:
		IC bool IsSVPActive() { return m_bIsActive; }
		void    SetSVPActive(bool bState);
		bool    IsSVPFrame();

		IC u8 GetSVPFrameDelay() { return m_FrameDelay; }
		void  SetSVPFrameDelay(u8 iDelay)
		{
			m_FrameDelay = iDelay;
			clamp<u8>(m_FrameDelay, 2, u8(-1));
		}
	};

private:
    // Main objects used for creating and rendering the 3D scene
    u64										m_dwWindowStyle;

	CTimer									TimerMM;

	void									_Create		(LPCSTR shName);
	void									_Destroy	(BOOL	bKeepTextures);
	void									_SetupStates();
public:
	// Get single WinAPI message and process it
	void									ProcessSingleMessage();

	u32										dwPrecacheTotal;

	float									fWidth_2, fHeight_2;
	void									OnWM_Activate(WPARAM wParam, LPARAM lParam);
public:
	IRenderDeviceRender						*m_pRender;

	BOOL									m_bNearer;
	void									SetNearer(BOOL enabled)
	{
		if (enabled && !m_bNearer)
		{
			m_bNearer = TRUE;
			mProject -= DirectX::XMMATRIX(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, EPS_L);
		}
		else if (!enabled && m_bNearer)
		{
			m_bNearer = FALSE;
			mProject += DirectX::XMMATRIX(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, EPS_L);
		}
		m_pRender->SetCacheXform(CastToGSCMatrix(mView), CastToGSCMatrix(mProject));
	}
public:
	// Registrators
	CRegistrator	<pureFrame			>			seqFrameMT;
	CRegistrator	<pureDeviceReset	>			seqDeviceReset;
	xr_vector		<fastdelegate::FastDelegate0<> >	seqParallel;
	CSecondVPParams m_SecondViewport;	//--#SM+#-- +SecondVP+

	// Dependent classes
	CStats*									Statistic;

	// Cameras & projection
	Fmatrix									mInvFullTransform;
	
	CRenderDevice			()
		:
		m_pRender(0)
#ifdef INGAME_EDITOR
		,m_editor_module(0),
		m_editor_initialize(0),
		m_editor_finalize(0),
		m_editor(0),
		m_engine(0)
#endif // #ifdef INGAME_EDITOR
	{
	    m_hWnd              = NULL;
		b_is_Active			= FALSE;
		b_is_Ready			= FALSE;
		Timer.Start			();
		m_bNearer			= FALSE;
		//+SecondVP+
		m_SecondViewport.SetSVPActive(false);
		m_SecondViewport.SetSVPFrameDelay(2);
		m_SecondViewport.m_bCamReady = false;
	};

	void	Pause							(BOOL bOn, BOOL bTimer, BOOL bSound, LPCSTR reason);
	BOOL	Paused							();

	// Scene control
	void PreCache							(u32 amount, bool b_draw_loadscreen, bool b_wait_user_input);
	BOOL Begin								();
	void Clear								();
	void End								();
	void FrameMove							();
	
	void overdrawBegin						();
	void overdrawEnd						();

	// Mode control
	void DumpFlags							();
	IC	 CTimer_paused* GetTimerGlobal		()	{ return &TimerGlobal;								}
	u32	 TimerAsync							()	{ return TimerGlobal.GetElapsed_ms();				}
	u32	 TimerAsync_MMT						()	{ return TimerMM.GetElapsed_ms() +	Timer_MM_Delta; }

	// Creation & Destroying
	void ConnectToRender();
	void Create								(bool bIsEditor);
	void Run								(void);

	void BeginToWork();

	void Destroy(void);
	void Reset								(bool precache = true);

	void Initialize							(void);
	HWND CreateXRayWindow (HWND parent = NULL, int Width = 0, int Height = 0);
	void ShutDown							(void);

    void UpdateWindowPropStyle              (WindowPropStyle PropStyle = (WindowPropStyle)ps_vid_windowtype);

public:
	void time_factor						(const float &time_factor)
	{
		Timer.time_factor		(time_factor);
		TimerGlobal.time_factor	(time_factor);
	}
	
	IC	const float &time_factor			() const
	{
		VERIFY					(Timer.time_factor() == TimerGlobal.time_factor());
		return					(Timer.time_factor());
	}

    WindowPropStyle GetCurrentWindowPropStyle() const { return (WindowPropStyle)ps_vid_windowtype; };

	// Multi-threading
	xrCriticalSection	mt_csEnter;
	xrCriticalSection	mt_csLeave;
	volatile BOOL		mt_bMustExit;

	ICF		void			remove_from_seq_parallel	(const fastdelegate::FastDelegate0<> &delegate)
	{
		xr_vector<fastdelegate::FastDelegate0<> >::iterator I = std::find(
			seqParallel.begin(),
			seqParallel.end(),
			delegate
		);
		if (I != seqParallel.end())
			seqParallel.erase	(I);
	}

	IC u32 frame_elapsed()
	{
		return frame_timer.GetElapsed_ms();
	}
public:
			void xr_stdcall		on_idle				();
			void				ResizeProc			(DWORD height, DWORD width);
			bool xr_stdcall		on_message			(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &result);

private:
			void					message_loop		();
virtual		void			_BCL	AddSeqFrame			( pureFrame* f, bool mt );
virtual		void			_BCL	RemoveSeqFrame		( pureFrame* f );
virtual		CStatsPhysics*	_BCL	StatPhysics			()	{ return  Statistic ;}
#ifdef INGAME_EDITOR
public:
	IC		editor::ide			*editor				() const { return m_editor; }

private:
			void				initialize_editor	();
			void				message_loop_editor	();

private:
	typedef editor::initialize_function_ptr			initialize_function_ptr;
	typedef editor::finalize_function_ptr			finalize_function_ptr;

private:
	HMODULE						m_editor_module;
	initialize_function_ptr		m_editor_initialize;
	finalize_function_ptr		m_editor_finalize;
	editor::ide					*m_editor;
	engine_impl					*m_engine;
#endif // #ifdef INGAME_EDITOR
};

extern		ENGINE_API		CRenderDevice		Device;

#ifndef	_EDITOR
#define	RDEVICE	Device
#else
#define RDEVICE	EDevice
#endif


extern		ENGINE_API		bool				g_bBenchmark;

using LOADING_EVENT = fastdelegate::FastDelegate0<bool>;
extern	ENGINE_API xr_list<LOADING_EVENT>		g_loading_events;

class ENGINE_API CLoadScreenRenderer :public pureRender
{
public:
					CLoadScreenRenderer	();
	void			start				(bool b_user_input);
	void			stop				();
	virtual void	OnRender			();

	bool			b_registered;
	bool			b_need_user_input;
};
extern ENGINE_API CLoadScreenRenderer load_screen_renderer;
#pragma warning(pop)
