#pragma once

#include "../../Config.h"
#include "../../Include/Winheaders.h"
#include "../../Include/NativeStructures.h"
#include "../../Include/Types.h"

#include <memory>

namespace blackbone
{

#define DEFAULT_ACCESS_T  THREAD_SUSPEND_RESUME    | \
                          THREAD_GET_CONTEXT       | \
                          THREAD_SET_CONTEXT       | \
                          THREAD_QUERY_INFORMATION | \
                          THREAD_TERMINATE         | \
                          SYNCHRONIZE

#define MAXULONG64_2 ((uint64_t)~((uint64_t)0))
#define MAXULONG32_2 ((uint32_t)~((uint32_t)0))


// Breakpoint type
enum HWBPType
{
    hwbp_access  = 3,   // Read or write
    hwbp_write   = 1,   // Write only
    hwbp_execute = 0,   // Execute only
};

enum HWBPLength
{
    hwbp_1 = 0,         // 1 byte
    hwbp_2 = 1,         // 2 bytes
    hwbp_4 = 3,         // 4 bytes
    hwbp_8 = 2,         // 8 bytes
};


/// <summary>
/// Thread management
/// </summary>
class Thread
{
public:
    BLACKBONE_API Thread( DWORD id, class ProcessCore* hProcess, DWORD access = DEFAULT_ACCESS_T );
    BLACKBONE_API Thread( HANDLE handle, class ProcessCore* hProcess );
    BLACKBONE_API Thread( const Thread& other );
    BLACKBONE_API ~Thread();

    /// <summary>
    /// Get thread ID
    /// </summary>
    /// <returns>Thread ID</returns>
    BLACKBONE_API inline DWORD id() const { return _id; }

    /// <summary>
    /// Get thread handle
    /// </summary>
    /// <returns>Thread hande</returns>
    BLACKBONE_API inline HANDLE handle() const { return _handle; }

    /// <summary>
    /// Check if thread exists
    /// </summary>
    /// <returns>true if thread exists</returns>
    BLACKBONE_API inline bool valid() const { return (_handle != NULL && ExitCode() == STILL_ACTIVE); }

    /// <summary>
    /// Get WOW64 TEB
    /// </summary>
    /// <param name="pteb">Process TEB</param>
    /// <returns>TEB pointer</returns>
    BLACKBONE_API ptr_t teb( _TEB32* pteb = nullptr ) const;
   
    /// <summary>
    /// Get Native TEB
    /// </summary>
    /// <param name="pteb">Process TEB</param>
    /// <returns>TEB pointer</returns>
    BLACKBONE_API ptr_t teb( _TEB64* pteb = nullptr ) const;

    /// <summary>
    /// Get TEB
    /// </summary>
    /// <returns>TEB pointer</returns>
    BLACKBONE_API inline ptr_t teb() const { return teb( (TEB_T*)nullptr ); }

    /// <summary>
    /// Get thread creation time
    /// </summary>
    /// <returns>Thread creation time</returns>
    BLACKBONE_API uint64_t startTime();

    /// <summary>
    /// Get total execution time(user mode and kernel mode)
    /// </summary>
    /// <returns>Total execution time</returns>
    BLACKBONE_API uint64_t execTime();

    /// <summary>
    /// Suspend thread
    /// </summary>
    /// <returns>true on success</returns>
    BLACKBONE_API bool Suspend();

    /// <summary>
    /// Resumes thread.
    /// </summary>
    /// <returns>true on success</returns>
    BLACKBONE_API bool Resume();

    /// <summary>
    /// Check if thread is suspended
    /// </summary>
    /// <returns>true if suspended</returns>
    BLACKBONE_API bool Suspended();

    /// <summary>
    /// Get WOW64 thread context
    /// </summary>
    /// <param name="ctx">Returned context</param>
    /// <param name="flags">Context flags.</param>
    /// <param name="dontSuspend">true if thread shouldn't be suspended before retrieving context</param>
    /// <returns>true on success</returns>
    BLACKBONE_API bool GetContext( _CONTEXT32& ctx, DWORD flags = CONTEXT_ALL, bool dontSuspend = false );

    /// <summary>
    /// Get native thread context
    /// </summary>
    /// <param name="ctx">Returned context</param>
    /// <param name="flags">Context flags.</param>
    /// <param name="dontSuspend">true if thread shouldn't be suspended before retrieving context</param>
    /// <returns>true on success</returns>
    BLACKBONE_API bool GetContext( _CONTEXT64& ctx, DWORD flags = CONTEXT64_ALL, bool dontSuspend = false );

    /// <summary>
    /// Set WOW64 thread context
    /// </summary>
    /// <param name="ctx">Context to set</param>
    /// <param name="dontSuspend">true if thread shouldn't be suspended before retrieving context</param>
    /// <returns>true on success</returns>
    BLACKBONE_API bool SetContext( _CONTEXT32& ctx, bool dontSuspend = false );

    /// <summary>
    /// Set native thread context
    /// </summary>
    /// <param name="ctx">Context to set</param>
    /// <param name="dontSuspend">true if thread shouldn't be suspended before retrieving context</param>
    /// <returns>true on success</returns>
    BLACKBONE_API bool SetContext( _CONTEXT64& ctx, bool dontSuspend = false );

    /// <summary>
    /// Terminate thread
    /// </summary>
    /// <param name="code">Exit code</param>
    /// <returns>true on success</returns>
    BLACKBONE_API bool Terminate( DWORD code = 0 );

    /// <summary>
    /// Join thread
    /// </summary>
    /// <param name="timeout">Join timeout</param>
    /// <returns>true on success</returns>
    BLACKBONE_API bool Join( int timeout = INFINITE );

    /// <summary>
    /// Get thread exit code
    /// </summary>
    /// <returns>Thread exit code</returns>
    BLACKBONE_API DWORD ExitCode() const;

    /// <summary>
    /// Add hardware breakpoint to thread
    /// </summary>
    /// <param name="addr">Breakpoint address</param>
    /// <param name="type">Breakpoint type(read/write/execute)</param>
    /// <param name="length">Number of bytes to include into breakpoint</param>
    /// <returns>Index of used breakpoint; -1 if failed</returns>
    int BLACKBONE_API AddHWBP( ptr_t addr, HWBPType type, HWBPLength length );

    /// <summary>
    /// Remove existing hardware breakpoint
    /// </summary>
    /// <param name="idx">Breakpoint index</param>
    /// <returns>true on success</returns>
    BLACKBONE_API bool RemoveHWBP( int idx );

    /// <summary>
    /// Remove existing hardware breakpoint
    /// </summary>
    /// <param name="ptr">Breakpoint address</param>
    /// <returns>true on success</returns>
    BLACKBONE_API bool RemoveHWBP( ptr_t ptr );

    /// <summary>
    /// Close handle
    /// </summary>
    BLACKBONE_API void Close();

    BLACKBONE_API inline bool operator ==(const Thread& other) { return (_id == other._id); }

    BLACKBONE_API Thread& operator =(const Thread& other)
    {
        _id = other._id;
        _core = other._core;
        _handle = other._handle;

        // Transfer handle ownership
        other.ReleaseHandle();

        return *this;
    }

private:

    /// <summary>
    /// Release thread handle
    /// </summary>
    inline void ReleaseHandle() const { _handle = NULL; }

private:
    class ProcessCore* _core;           // Core routines

    DWORD _id = 0;                      // Thread ID
    mutable HANDLE _handle = NULL;      // Thread handle
};

}