#pragma once

#ifdef USE_DX11
	//////////////////////////////////////////
	#include "ResourceManager.h"
	//////////////////////////////////////////
	ENGINE_API BOOL isGraphicDebugging;
	//////////////////////////////////////////
	template<typename T>
	struct ShaderTypeTraits;
	template<>
	struct ShaderTypeTraits<SHS>
	{
		typedef CResourceManager::map_HS	MapType;
		typedef ID3D11HullShader DXIface;

		static inline const char* GetShaderExt() {return "hs_";}
		static inline const char* GetCompilationTarget() {return "hs_5_0";}
		static inline DXIface* CreateHWShader(DWORD const* buffer, size_t size)
		{
			DXIface* hs = 0;
			R_CHK(HW.pDevice->CreateHullShader(buffer, size, NULL, &hs));
			return hs;
		}

		static inline u32 GetShaderDest() {return RC_dest_hull;}
	};

	template<>
	struct ShaderTypeTraits<SDS>
	{
		typedef CResourceManager::map_DS	MapType;
		typedef ID3D11DomainShader			DXIface;

		static inline const char* GetShaderExt() {return "ds_";}
		static inline const char* GetCompilationTarget() {return "ds_5_0";}
		static inline DXIface* CreateHWShader(DWORD const* buffer, size_t size)
		{
			DXIface* hs = 0;
			R_CHK(HW.pDevice->CreateDomainShader(buffer, size, NULL, &hs));
			return hs;
		}

		static inline u32 GetShaderDest() {return RC_dest_domain;}
	};

	template<>
	struct ShaderTypeTraits<SCS>
	{
		typedef CResourceManager::map_CS	MapType;
		typedef ID3D11ComputeShader			DXIface;

		static inline const char* GetShaderExt() {return "cs_";}
		static inline const char* GetCompilationTarget() {return "cs_5_0";}
		static inline DXIface* CreateHWShader(DWORD const* buffer, size_t size)
		{
			DXIface* cs = 0;
			R_CHK(HW.pDevice->CreateComputeShader(buffer, size, NULL, &cs));
			return cs;
		}

		static inline u32 GetShaderDest() {return RC_dest_compute;}
	};

	template<>
	inline CResourceManager::map_DS& CResourceManager::GetShaderMap(){return m_ds;}

	template<>
	inline CResourceManager::map_HS& CResourceManager::GetShaderMap(){return m_hs;}

	template<>
	inline CResourceManager::map_CS& CResourceManager::GetShaderMap(){return m_cs;}

    template<typename T>
	inline T* CResourceManager::CreateShader(const char* name)
	{
		xrCriticalSectionGuard guard(creationGuard);
		ShaderTypeTraits<T>::MapType& sh_map = GetShaderMap<ShaderTypeTraits<T>::MapType>();
		LPSTR	N = LPSTR(name);
		ShaderTypeTraits<T>::MapType::iterator	I = sh_map.find(N);

		if (I!=sh_map.end())
			return		I->second;
		else
		{
			T*		sh = xr_new<T>();

			sh->dwFlags |= xr_resource_flagged::RF_REGISTERED;
			sh_map.insert(std::make_pair(sh->set_name(name),sh));
			if (0==stricmp(name,"null"))
			{
				sh->sh				= NULL;
				return sh;
			}

			string_path					shName;
			const char*	pchr = strchr(name, '(');
			ptrdiff_t	strSize = pchr?pchr-name:xr_strlen(name);
			strncpy(shName, name, strSize);
			shName[strSize] = 0;

            string64 PrependPath;
            ZeroMemory(PrependPath, sizeof(PrependPath));
            memcpy(PrependPath, shName, sizeof(PrependPath));
            char* ClearShaderName = NULL;
            char* PathStart = strtok_s(PrependPath, "\\", &ClearShaderName);
            memcpy(shName, PathStart, sizeof(string_path) - sizeof(PrependPath));

			// Open file
			string_path					cname;
			xr_strconcat				( cname, ::Render->getShaderPath(), PrependPath, "\\", ShaderTypeTraits<T>::GetShaderExt(),/*name*/ClearShaderName, ".hlsl");
			FS.update_path				(cname,	"$game_shaders$", cname);

			// duplicate and zero-terminate
			IReader* file				= FS.r_open(cname);
			R_ASSERT2					( file, cname );

			// Select target
			LPCSTR						c_target	= ShaderTypeTraits<T>::GetCompilationTarget();
			LPCSTR						c_entry		= "main";

			// Compile
           	DWORD shaderCompileFlags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
		if (isGraphicDebugging)
			shaderCompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PREFER_FLOW_CONTROL;
			
			HRESULT	const _hr			= ::Render->shader_compile(name,(DWORD const*)file->pointer(),file->length(), c_entry, c_target, shaderCompileFlags, (void*&)sh );

			FS.r_close					( file );

			VERIFY(SUCCEEDED(_hr));

			CHECK_OR_EXIT				(
				!FAILED(_hr),
				make_string("Your video card doesn't meet game requirements.\n\nTry to lower game settings.")
			);

			return			sh;
		}
	}

	template<typename T>
	inline void CResourceManager::DestroyShader(const T* sh)
	{
		xrCriticalSectionGuard guard(creationGuard);
		ShaderTypeTraits<T>::MapType& sh_map = GetShaderMap<ShaderTypeTraits<T>::MapType>();

		if (0==(sh->dwFlags&xr_resource_flagged::RF_REGISTERED))
			return;

		LPSTR N = LPSTR(*sh->cName);
		typename ShaderTypeTraits<T>::MapType::iterator I = sh_map.find(N);
		
		if (I!=sh_map.end())
		{
			sh_map.erase(I);
			return;
		}
		Msg	("! ERROR: Failed to find compiled geometry shader '%s'", *sh->cName);
	}

#endif
