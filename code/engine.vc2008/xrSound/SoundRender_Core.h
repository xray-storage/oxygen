#pragma once
#include <openal\al.h>
#include <openal/efx-presets.h>
#include "SoundRender.h"
#include "SoundRender_Environment.h"
#include "SoundRender_Cache.h"

class XRSOUND_API CSoundRender_Core : public CSound_manager_interface
{
	volatile bool bLocked;

protected:
	virtual void _create_data(ref_sound_data& S, const char* fName, esound_type sound_type, int game_type);
	virtual void _destroy_data(ref_sound_data& S);

protected:
	bool bListenerMoved;

	CSoundRender_Environment e_current;
	CSoundRender_Environment e_target;

public:
	typedef	std::pair<ref_sound_data_ptr, float> event;
	xr_vector<event> s_events;

public:
	bool bPresent;
	bool bUserEnvironment;
	bool bEFX;					// boolean variable to indicate presence of EAX Extension
	bool bReady;

	CTimer Timer;
	float lastTimestamp;
	float lastDeltaTime;
	sound_event* Handler;

protected:
	// Collider
#ifndef _EDITOR
	CDB::COLLIDER geom_DB;
#endif

	CDB::MODEL* geom_SOM;
	CDB::MODEL* geom_MODEL;
	CDB::MODEL* geom_ENV;

	// Containers
	xr_vector<CSoundRender_Source*>	s_sources;
	xr_vector<CSoundRender_Emitter*> emitters;
	u32 lastUpdateFrame;			// emitter update marker
	xr_vector<CSoundRender_Target*>	targets;
	xr_list<CSoundRender_Target*>	targetShouldUpdateLater;
	SoundEnvironment_LIB* s_environment;
	CSoundRender_Environment s_user_environment;

	int m_iPauseCounter;

public:
	// Cache
	CSoundRender_Cache cache;
	u32 cache_bytes_per_line;

public:
	CSoundRender_Core();
	virtual ~CSoundRender_Core();

	// General
	virtual void _initialize(int stage) = 0;
	virtual void _clear() = 0;
	virtual void _restart();

	// Sound
	void verify_refsound(ref_sound& S);
	virtual void create(ref_sound& S, const char* fName, esound_type sound_type, int	game_type);
	virtual void attach_tail(ref_sound& S, const char* fName);

	virtual void clone(ref_sound& S, const ref_sound& from, esound_type sound_type, int	game_type);
	virtual void destroy(ref_sound& S);
	virtual void stop_emitters();
	virtual int pause_emitters(bool val);

	virtual void play(ref_sound& S, CObject* O, u32 flags = 0, float delay = 0.f);
	virtual void play_at_pos(ref_sound& S, CObject* O, const Fvector &pos, u32 flags = 0, float delay = 0.f);
	virtual void play_no_feedback(ref_sound& S, CObject* O, u32 flags = 0, float delay = 0.f, Fvector* pos = 0, float* vol = 0, float* freq = 0, Fvector2* range = 0);
	virtual void set_master_volume(float			f) = 0;
	virtual void set_geometry_env(IReader*		I);
	virtual void set_geometry_som(IReader*		I);
	virtual void set_geometry_occ(CDB::MODEL*	M);
	virtual void set_handler(sound_event*	E);

	virtual void update(const Fvector& P, const Fvector& D, const Fvector& N);
	virtual void update_events();
	virtual void statistic(CSound_stats*  dest, CSound_stats_ext*  ext);

	// listener
	virtual void update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt) = 0;
	bool i_efx_commit_setting();
	void i_efx_listener_set(CSound_environment* _E);

public:
	CSoundRender_Source * i_create_source(const char* name);
	CSoundRender_Emitter* i_play(ref_sound* S, bool _loop, float delay);
	void i_start(CSoundRender_Emitter* E);
	void i_stop(CSoundRender_Emitter* E);
	void i_rewind(CSoundRender_Emitter* E);
	bool i_allow_play(CSoundRender_Emitter* E);
	virtual bool i_locked() { return bLocked; }

	virtual void object_relcase(CObject* obj);

	virtual float get_occlusion_to(const Fvector& hear_pt, const Fvector& snd_pt, float dispersion = 0.2f);
	float get_occlusion(Fvector& P, float R, Fvector* occ);
	CSoundRender_Environment* get_environment(const Fvector& P);

	void env_load();
	void env_unload();
	void env_apply();

protected: // EFX
	EFXEAXREVERBPROPERTIES efx_reverb;
	ALuint effect;
	ALuint slot;
	bool EFXTestSupport();
	void InitAlEFXAPI();
};
extern XRSOUND_API CSoundRender_Core* SoundRender;