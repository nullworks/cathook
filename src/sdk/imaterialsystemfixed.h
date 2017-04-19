//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef IMATERIALSYSTEMFIXED_H
#define IMATERIALSYSTEMFIXED_H

#ifdef _WIN32
#pragma once
#endif

#define OVERBRIGHT 2.0f
#define OO_OVERBRIGHT ( 1.0f / 2.0f )
#define GAMMA 2.2f
#define TEXGAMMA 2.2f

#include "tier1/interface.h"
#include "tier1/refcount.h"
#include "mathlib/vector.h"
#include "mathlib/vector4d.h"
#include "mathlib/vmatrix.h"
#include "appframework/IAppSystem.h"
#include "bitmap/imageformat.h"
#include "texture_group_names.h"
#include "vtf/vtf.h"
#include "materialsystem/deformations.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/IColorCorrection.h"


//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
class IMaterial;
class IMesh;
class IVertexBuffer;
class IIndexBuffer;
struct MaterialSystem_Config_t;
class VMatrix;
struct matrix3x4_t;
class ITexture;
class ITextureCompositor;
struct MaterialSystemHardwareIdentifier_t;
class KeyValues;
class IShader;
class IVertexTexture;
class IMorph;
class IMatRenderContext;
class ICallQueue;
struct MorphWeight_t;
class IFileList;

abstract_class IMaterialSystemFixed : public IAppSystem
{
public:

	// Placeholder for API revision
	virtual bool Connect( CreateInterfaceFn factory ) = 0;
	virtual void Disconnect() = 0;
	virtual void *QueryInterface( const char *pInterfaceName ) = 0;
	virtual InitReturnVal_t Init() = 0;
	virtual void Shutdown() = 0;

	//---------------------------------------------------------
	// Initialization and shutdown
	//---------------------------------------------------------

	// Call this to initialize the material system
	// returns a method to create interfaces in the shader dll
	virtual CreateInterfaceFn	Init( char const* pShaderAPIDLL,
		IMaterialProxyFactory *pMaterialProxyFactory,
		CreateInterfaceFn fileSystemFactory,
		CreateInterfaceFn cvarFactory=NULL ) = 0;

	// Call this to set an explicit shader version to use
	// Must be called before Init().
	virtual void				SetShaderAPI( char const *pShaderAPIDLL ) = 0;

	// Must be called before Init(), if you're going to call it at all...
	virtual void				SetAdapter( int nAdapter, int nFlags ) = 0;

	// Call this when the mod has been set up, which may occur after init
	// At this point, the game + gamebin paths have been set up
	virtual void				ModInit() = 0;
	virtual void				ModShutdown() = 0;

	//---------------------------------------------------------
	//
	//---------------------------------------------------------
	virtual void					SetThreadMode( MaterialThreadMode_t mode, int nServiceThread = -1 ) = 0;
	virtual MaterialThreadMode_t	GetThreadMode( ) = 0;
	virtual bool					IsRenderThreadSafe( ) = 0;
	virtual void					ExecuteQueued() = 0;

	//---------------------------------------------------------
	// Config management
	//---------------------------------------------------------

	virtual IMaterialSystemHardwareConfig *GetHardwareConfig( const char *pVersion, int *returnCode ) = 0;


	// Call this before rendering each frame with the current config
	// for the material system.
	// Will do whatever is necessary to get the material system into the correct state
	// upon configuration change. .doesn't much else otherwise.
	virtual bool				UpdateConfig( bool bForceUpdate ) = 0;

	// Force this to be the config; update all material system convars to match the state
	// return true if lightmaps need to be redownloaded
	virtual bool				OverrideConfig( const MaterialSystem_Config_t &config, bool bForceUpdate ) = 0;

	// Get the current config for this video card (as last set by UpdateConfig)
	virtual const MaterialSystem_Config_t &GetCurrentConfigForVideoCard() const = 0;

	// Gets *recommended* configuration information associated with the display card,
	// given a particular dx level to run under.
	// Use dxlevel 0 to use the recommended dx level.
	// The function returns false if an invalid dxlevel was specified

	// UNDONE: To find out all convars affected by configuration, we'll need to change
	// the dxsupport.pl program to output all column headers into a single keyvalue block
	// and then we would read that in, and send it back to the client
	virtual bool				GetRecommendedConfigurationInfo( int nDXLevel, KeyValues * pKeyValues ) = 0;


	// -----------------------------------------------------------
	// Device methods
	// -----------------------------------------------------------

	// Gets the number of adapters...
	virtual int					GetDisplayAdapterCount() const = 0;

	// Returns the current adapter in use
	virtual int					GetCurrentAdapter() const = 0;

	// Returns info about each adapter
	virtual void				GetDisplayAdapterInfo( int adapter, MaterialAdapterInfo_t& info ) const = 0;

	// Returns the number of modes
	virtual int					GetModeCount( int adapter ) const = 0;

	// Returns mode information..
	virtual void				GetModeInfo( int adapter, int mode, MaterialVideoMode_t& info ) const = 0;

	virtual void				AddModeChangeCallBack( ModeChangeCallbackFunc_t func ) = 0;

	// Returns the mode info for the current display device
	virtual void				GetDisplayMode( MaterialVideoMode_t& mode ) const = 0;

	// Sets the mode...
	virtual bool				SetMode( void* hwnd, const MaterialSystem_Config_t &config ) = 0;

	virtual bool				SupportsMSAAMode( int nMSAAMode ) = 0;

	// FIXME: REMOVE! Get video card identitier
	virtual const MaterialSystemHardwareIdentifier_t &GetVideoCardIdentifier( void ) const = 0;

	// Use this to spew information about the 3D layer
	virtual void				SpewDriverInfo() const = 0;

	virtual void				GetDXLevelDefaults(uint &max_dxlevel,uint &recommended_dxlevel) = 0;

	// Get the image format of the back buffer. . useful when creating render targets, etc.
	virtual void				GetBackBufferDimensions( int &width, int &height) const = 0;
	virtual ImageFormat			GetBackBufferFormat() const = 0;

	virtual bool				SupportsHDRMode( HDRType_t nHDRModede ) = 0;


	// -----------------------------------------------------------
	// Window methods
	// -----------------------------------------------------------

	// Creates/ destroys a child window
	virtual bool				AddView( void* hwnd ) = 0;
	virtual void				RemoveView( void* hwnd ) = 0;

	// Sets the view
	virtual void				SetView( void* hwnd ) = 0;


	// -----------------------------------------------------------
	// Control flow
	// -----------------------------------------------------------

	virtual void				BeginFrame( float frameTime ) = 0;
	virtual void				EndFrame( ) = 0;
	virtual void				Flush( bool flushHardware = false ) = 0;

	/// FIXME: This stuff needs to be cleaned up and abstracted.
	// Stuff that gets exported to the launcher through the engine
	virtual void				SwapBuffers( ) = 0;

	// Flushes managed textures from the texture cacher
	virtual void				EvictManagedResources() = 0;

	virtual void				ReleaseResources(void) = 0;
	virtual void				ReacquireResources(void ) = 0;


	// -----------------------------------------------------------
	// Device loss/restore
	// -----------------------------------------------------------

	// Installs a function to be called when we need to release vertex buffers + textures
	virtual void				AddReleaseFunc( MaterialBufferReleaseFunc_t func ) = 0;
	virtual void				RemoveReleaseFunc( MaterialBufferReleaseFunc_t func ) = 0;

	// Installs a function to be called when we need to restore vertex buffers
	virtual void				AddRestoreFunc( MaterialBufferRestoreFunc_t func ) = 0;
	virtual void				RemoveRestoreFunc( MaterialBufferRestoreFunc_t func ) = 0;

	// Release temporary HW memory...
	virtual void				ResetTempHWMemory( bool bExitingLevel = false ) = 0;

	// For dealing with device lost in cases where SwapBuffers isn't called all the time (Hammer)
	virtual void				HandleDeviceLost() = 0;


	// -----------------------------------------------------------
	// Shaders
	// -----------------------------------------------------------

	// Used to iterate over all shaders for editing purposes
	// GetShaders returns the number of shaders it actually found
	virtual int					ShaderCount() const = 0;
	virtual int					GetShaders( int nFirstShader, int nMaxCount, IShader **ppShaderList ) const = 0;

	// FIXME: Is there a better way of doing this?
	// Returns shader flag names for editors to be able to edit them
	virtual int					ShaderFlagCount() const = 0;
	virtual const char *		ShaderFlagName( int nIndex ) const = 0;

	// Gets the actual shader fallback for a particular shader
	virtual void				GetShaderFallback( const char *pShaderName, char *pFallbackShader, int nFallbackLength ) = 0;


	// -----------------------------------------------------------
	// Material proxies
	// -----------------------------------------------------------

	virtual IMaterialProxyFactory *GetMaterialProxyFactory() = 0;

	// Sets the material proxy factory. Calling this causes all materials to be uncached.
	virtual void				SetMaterialProxyFactory( IMaterialProxyFactory* pFactory ) = 0;


	// -----------------------------------------------------------
	// Editor mode
	// -----------------------------------------------------------

	// Used to enable editor materials. Must be called before Init.
	virtual void				EnableEditorMaterials() = 0;


	// -----------------------------------------------------------
	// Stub mode mode
	// -----------------------------------------------------------

	// Force it to ignore Draw calls.
	virtual void				SetInStubMode( bool bInStubMode ) = 0;


	//---------------------------------------------------------
	// Debug support
	//---------------------------------------------------------

	virtual void				DebugPrintUsedMaterials( const char *pSearchSubString, bool bVerbose ) = 0;
	virtual void				DebugPrintUsedTextures( void ) = 0;

	virtual void				ToggleSuppressMaterial( char const* pMaterialName ) = 0;
	virtual void				ToggleDebugMaterial( char const* pMaterialName ) = 0;


	//---------------------------------------------------------
	// Misc features
	//---------------------------------------------------------
	//returns whether fast clipping is being used or not - needed to be exposed for better per-object clip behavior
	virtual bool				UsingFastClipping( void ) = 0;

	virtual int					StencilBufferBits( void ) = 0; //number of bits per pixel in the stencil buffer


	// CATHOOK - missing VFuncs
	virtual void SuspendTextureStreaming(void);
	virtual void ResumeTextureStreaming(void);

	//---------------------------------------------------------
	// Material and texture management
	//---------------------------------------------------------

	// uncache all materials. .  good for forcing reload of materials.
	virtual void				UncacheAllMaterials( ) = 0;

	// Remove any materials from memory that aren't in use as determined
	// by the IMaterial's reference count.
	virtual void				UncacheUnusedMaterials( bool bRecomputeStateSnapshots = false ) = 0;

	// Load any materials into memory that are to be used as determined
	// by the IMaterial's reference count.
	virtual void				CacheUsedMaterials( ) = 0;

	// Force all textures to be reloaded from disk.
	virtual void				ReloadTextures( ) = 0;

	// Reloads materials
	virtual void				ReloadMaterials( const char *pSubString = NULL ) = 0;

	// Create a procedural material. The keyvalues looks like a VMT file
	virtual IMaterial *			CreateMaterial( const char *pMaterialName, KeyValues *pVMTKeyValues ) = 0;

	// Find a material by name.
	// The name of a material is a full path to
	// the vmt file starting from "hl2/materials" (or equivalent) without
	// a file extension.
	// eg. "dev/dev_bumptest" refers to somethign similar to:
	// "d:/hl2/hl2/materials/dev/dev_bumptest.vmt"
	//
	// Most of the texture groups for pTextureGroupName are listed in texture_group_names.h.
	//
	// Note: if the material can't be found, this returns a checkerboard material. You can
	// find out if you have that material by calling IMaterial::IsErrorMaterial().
	// (Or use the global IsErrorMaterial function, which checks if it's null too).
	virtual IMaterial *			FindMaterial( char const* pMaterialName, const char *pTextureGroupName, bool complain = true, const char *pComplainPrefix = NULL ) = 0;

	// Query whether a material is loaded (eg, whether FindMaterial will be nonblocking)
	virtual bool				IsMaterialLoaded( char const* pMaterialName ) = 0;

	//---------------------------------
	// This is the interface for knowing what materials are available
	// is to use the following functions to get a list of materials.  The
	// material names will have the full path to the material, and that is the
	// only way that the directory structure of the materials will be seen through this
	// interface.
	// NOTE:  This is mostly for worldcraft to get a list of materials to put
	// in the "texture" browser.in Worldcraft
	virtual MaterialHandle_t	FirstMaterial() const = 0;

	// returns InvalidMaterial if there isn't another material.
	// WARNING: you must call GetNextMaterial until it returns NULL,
	// otherwise there will be a memory leak.
	virtual MaterialHandle_t	NextMaterial( MaterialHandle_t h ) const = 0;

	// This is the invalid material
	virtual MaterialHandle_t	InvalidMaterial() const = 0;

	// Returns a particular material
	virtual IMaterial*			GetMaterial( MaterialHandle_t h ) const = 0;

	// Get the total number of materials in the system.  These aren't just the used
	// materials, but the complete collection.
	virtual int					GetNumMaterials( ) const = 0;

	//---------------------------------

	virtual void				SetAsyncTextureLoadCache( void* hFileCache ) = 0;

	virtual ITexture *			FindTexture( char const* pTextureName, const char *pTextureGroupName, bool complain = true, int nAdditionalCreationFlags = 0  ) = 0;

	// Checks to see if a particular texture is loaded
	virtual bool				IsTextureLoaded( char const* pTextureName ) const = 0;

	// Creates a procedural texture
	virtual ITexture *			CreateProceduralTexture( const char	*pTextureName,
		const char *pTextureGroupName,
		int w,
		int h,
		ImageFormat fmt,
		int nFlags ) = 0;

	//
	// Render targets
	//
	virtual void				BeginRenderTargetAllocation() = 0;
	virtual void				EndRenderTargetAllocation() = 0; // Simulate an Alt-Tab in here, which causes a release/restore of all resources

	// Creates a render target
	// If depth == true, a depth buffer is also allocated. If not, then
	// the screen's depth buffer is used.
	// Creates a texture for use as a render target
	virtual ITexture *			CreateRenderTargetTexture( int w,
		int h,
		RenderTargetSizeMode_t sizeMode,	// Controls how size is generated (and regenerated on video mode change).
		ImageFormat	format,
		MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED ) = 0;

	virtual ITexture *			CreateNamedRenderTargetTextureEx(  const char *pRTName,				// Pass in NULL here for an unnamed render target.
		int w,
		int h,
		RenderTargetSizeMode_t sizeMode,	// Controls how size is generated (and regenerated on video mode change).
		ImageFormat format,
		MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED,
		unsigned int textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		unsigned int renderTargetFlags = 0 ) = 0;

	virtual ITexture *			CreateNamedRenderTargetTexture( const char *pRTName,
		int w,
		int h,
		RenderTargetSizeMode_t sizeMode,	// Controls how size is generated (and regenerated on video mode change).
		ImageFormat format,
		MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED,
		bool bClampTexCoords = true,
		bool bAutoMipMap = false ) = 0;

	// Must be called between the above Begin-End calls!
	virtual ITexture *			CreateNamedRenderTargetTextureEx2( const char *pRTName,				// Pass in NULL here for an unnamed render target.
		int w,
		int h,
		RenderTargetSizeMode_t sizeMode,	// Controls how size is generated (and regenerated on video mode change).
		ImageFormat format,
		MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED,
		unsigned int textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		unsigned int renderTargetFlags = 0 ) = 0;

	// -----------------------------------------------------------
	// Lightmaps
	// -----------------------------------------------------------

	// To allocate lightmaps, sort the whole world by material twice.
	// The first time through, call AllocateLightmap for every surface.
	// that has a lightmap.
	// The second time through, call AllocateWhiteLightmap for every
	// surface that expects to use shaders that expect lightmaps.
	virtual void				BeginLightmapAllocation( ) = 0;
	virtual void				EndLightmapAllocation( ) = 0;

	// returns the sorting id for this surface
	virtual int 				AllocateLightmap( int width, int height,
		int offsetIntoLightmapPage[2],
		IMaterial *pMaterial ) = 0;
	// returns the sorting id for this surface
	virtual int					AllocateWhiteLightmap( IMaterial *pMaterial ) = 0;

	// lightmaps are in linear color space
	// lightmapPageID is returned by GetLightmapPageIDForSortID
	// lightmapSize and offsetIntoLightmapPage are returned by AllocateLightmap.
	// You should never call UpdateLightmap for a lightmap allocated through
	// AllocateWhiteLightmap.
	virtual void				UpdateLightmap( int lightmapPageID, int lightmapSize[2],
		int offsetIntoLightmapPage[2],
		float *pFloatImage, float *pFloatImageBump1,
		float *pFloatImageBump2, float *pFloatImageBump3 ) = 0;

	// fixme: could just be an array of ints for lightmapPageIDs since the material
	// for a surface is already known.
	virtual int					GetNumSortIDs( ) = 0;
	virtual void				GetSortInfo( MaterialSystem_SortInfo_t *sortInfoArray ) = 0;

	// Read the page size of an existing lightmap by sort id (returned from AllocateLightmap())
	virtual void				GetLightmapPageSize( int lightmap, int *width, int *height ) const = 0;

	virtual void				ResetMaterialLightmapPageInfo() = 0;



	virtual void				ClearBuffers( bool bClearColor, bool bClearDepth, bool bClearStencil = false ) = 0;

	// -----------------------------------------------------------
	// X360 specifics
	// -----------------------------------------------------------

#if defined( _X360 )
	virtual void				ListUsedMaterials( void ) = 0;
	virtual HXUIFONT			OpenTrueTypeFont( const char *pFontname, int tall, int style ) = 0;
	virtual void				CloseTrueTypeFont( HXUIFONT hFont ) = 0;
	virtual bool				GetTrueTypeFontMetrics( HXUIFONT hFont, XUIFontMetrics *pFontMetrics, XUICharMetrics charMetrics[256] ) = 0;
	// Render a sequence of characters and extract the data into a buffer
	// For each character, provide the width+height of the font texture subrect,
	// an offset to apply when rendering the glyph, and an offset into a buffer to receive the RGBA data
	virtual bool				GetTrueTypeGlyphs( HXUIFONT hFont, int numChars, wchar_t *pWch, int *pOffsetX, int *pOffsetY, int *pWidth, int *pHeight, unsigned char *pRGBA, int *pRGBAOffset ) = 0;
	virtual void				PersistDisplay() = 0;
	virtual void				*GetD3DDevice() = 0;
	virtual bool				OwnGPUResources( bool bEnable ) = 0;
#endif

	// -----------------------------------------------------------
	// Access the render contexts
	// -----------------------------------------------------------
	virtual IMatRenderContext *	GetRenderContext() = 0;

	virtual bool				SupportsShadowDepthTextures( void ) = 0;
	virtual void				BeginUpdateLightmaps( void ) = 0;
	virtual void				EndUpdateLightmaps( void ) = 0;

	// -----------------------------------------------------------
	// Methods to force the material system into non-threaded, non-queued mode
	// -----------------------------------------------------------
	virtual MaterialLock_t		Lock() = 0;
	virtual void				Unlock( MaterialLock_t ) = 0;

	// Vendor-dependent shadow depth texture format
	virtual ImageFormat			GetShadowDepthTextureFormat() = 0;

	virtual bool				SupportsFetch4( void ) = 0;

	// Create a custom render context. Cannot be used to create MATERIAL_HARDWARE_CONTEXT
	virtual IMatRenderContext *CreateRenderContext( MaterialContextType_t type ) = 0;

	// Set a specified render context to be the global context for the thread. Returns the prior context.
	virtual IMatRenderContext *SetRenderContext( IMatRenderContext * ) = 0;

	virtual bool				SupportsCSAAMode( int nNumSamples, int nQualityLevel ) = 0;

	virtual void				RemoveModeChangeCallBack( ModeChangeCallbackFunc_t func ) = 0;

	// Finds or create a procedural material.
	virtual IMaterial *			FindProceduralMaterial( const char *pMaterialName, const char *pTextureGroupName, KeyValues *pVMTKeyValues ) = 0;

	virtual ImageFormat			GetNullTextureFormat() = 0;

	virtual void				AddTextureAlias( const char *pAlias, const char *pRealName ) = 0;
	virtual void				RemoveTextureAlias( const char *pAlias ) = 0;

	// returns a lightmap page ID for this allocation, -1 if none available
	// frameID is a number that should be changed every frame to prevent locking any textures that are
	// being used to draw in the previous frame
	virtual int					AllocateDynamicLightmap( int lightmapSize[2], int *pOutOffsetIntoPage, int frameID ) = 0;

	virtual void				SetExcludedTextures( const char *pScriptName ) = 0;
	virtual void				UpdateExcludedTextures( void ) = 0;

	virtual bool				IsInFrame( ) const = 0;

	virtual void				CompactMemory() = 0;

	// For sv_pure mode. The filesystem figures out which files the client needs to reload to be "pure" ala the server's preferences.
	virtual void ReloadFilesInList( IFileList *pFilesToReload ) = 0;
	virtual	bool				AllowThreading( bool bAllow, int nServiceThread ) = 0;

	// Extended version of FindMaterial().
	// Contains context in so it can make decisions (i.e. if it's a model, ignore certain cheat parameters)
	virtual IMaterial *			FindMaterialEx( char const* pMaterialName, const char *pTextureGroupName, int nContext, bool complain = true, const char *pComplainPrefix = NULL ) = 0;

#ifdef DX_TO_GL_ABSTRACTION
	virtual void				DoStartupShaderPreloading( void ) = 0;
#endif

	// Sets the override sizes for all render target size tests. These replace the frame buffer size.
	// Set them when you are rendering primarily to something larger than the frame buffer (as in VR mode).
	virtual void				SetRenderTargetFrameBufferSizeOverrides( int nWidth, int nHeight ) = 0;

	// Returns the (possibly overridden) framebuffer size for render target sizing.
	virtual void				GetRenderTargetFrameBufferDimensions( int & nWidth, int & nHeight ) = 0;

	// returns the display device name that matches the adapter index we were started with
	virtual char *GetDisplayDeviceName() const = 0;

	// creates a texture suitable for use with materials from a raw stream of bits.
	// The bits will be retained by the material system and can be freed upon return.
	virtual ITexture*			CreateTextureFromBits(int w, int h, int mips, ImageFormat fmt, int srcBufferSize, byte* srcBits) = 0;

	// Lie to the material system to pretend to be in render target allocation mode at the beginning of time.
	// This was a thing that mattered a lot to old hardware, but doesn't matter at all to new hardware,
	// where new is defined to be "anything from the last decade." However, we want to preserve legacy behavior
	// for the old games because it's easier than testing them.
	virtual void				OverrideRenderTargetAllocation( bool rtAlloc ) = 0;

	// creates a texture compositor that will attempt to composite a new textuer from the steps of the specified KeyValues.
	virtual ITextureCompositor*	NewTextureCompositor( int w, int h, const char* pCompositeName, int nTeamNum, uint64 randomSeed, KeyValues* stageDesc, uint32 texCompositeCreateFlags = 0 ) = 0;

	// Loads the texture with the specified name, calls pRecipient->OnAsyncFindComplete with the result from the main thread.
	// once the texture load is complete. If the texture cannot be found, the returned texture will return true for IsError().
	virtual void AsyncFindTexture( const char* pFilename, const char *pTextureGroupName, IAsyncTextureOperationReceiver* pRecipient, void* pExtraArgs, bool bComplain = true, int nAdditionalCreationFlags = 0 ) = 0;

	// creates a texture suitable for use with materials from a raw stream of bits.
	// The bits will be retained by the material system and can be freed upon return.
	virtual ITexture*			CreateNamedTextureFromBitsEx( const char* pName, const char *pTextureGroupName, int w, int h, int mips, ImageFormat fmt, int srcBufferSize, byte* srcBits, int nFlags ) = 0;
};

#endif // IMATERIALSYSTEMFIXED_H
