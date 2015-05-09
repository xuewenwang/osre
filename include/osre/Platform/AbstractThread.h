#pragma once

#include <osre/Common/Types.h>

namespace OSRE {
namespace Platform {

class AbstractThreadEvent;

//-------------------------------------------------------------------------------------------------
///	@class		::ZFXCE2::Platform::IThread
///	@ingroup	Infrastructure
///
///	@brief	This abstract class declares the interface for a thread implementation.

///	Threads are OS-specific.
//-------------------------------------------------------------------------------------------------
class AbstractThread {
public:
    ///	The function pointer for a user-specific thread-function.
    typedef ui32 (*threadfunc) ( void * );

    ///	@enum	Priority
    ///	@brief	This enum describes the priority of the thread.
    enum Priority {
        Low,	///< Low prio thread.
        Normal,	///< Normal prio thread.
        High	///< High prio thread.
    };

    ///	@enum	ThreadState
    ///	@brief	Describes the current state of the thread.
    enum ThreadState {
        New,			///< In new state, just created
        Running,		///< thread is currently running
        Waiting,		///< Awaits a signal
        Suspended,		///< Is suspended
        Terminated		///< Thread is terminated, will be destroyed immediately
    };

public:
    ///	@brief	The destructor, virtual.
    virtual ~AbstractThread();

    ///	@brief	The thread will be started.
    ///	@param	pData	The data pointer.
    ///	@return	true, if the startup was successful, false if not.
    virtual bool start( void *pData ) = 0;

    ///	@brief	The thread will be stopped. The thread must be in running mode before.
    ///	@return	true, if the thread was stopped, false if not.
    virtual bool stop() = 0;

    ///	@brief	The running state of the thread will be returned.
    ///	@return	true, if the thread is in running-state. false if the thread is in stop-state.
    //virtual bool isRunning() = 0;

    ///	@brief	Returns the state of the thread.
    ///	@return	The current thread state.
    virtual ThreadState getCurrentState() const = 0;

    ///	@brief	The thread will be suspended.
    ///	@return	true, if the suspend was successful, false if not.
    virtual bool suspend() = 0;

    ///	@brief	Resumes the thread.
    ///	@return	true, if resuming was successful, false if not.
    virtual bool resume() = 0;

    ///	@brief	Sets the name of the thread.
    ///	@param	name	The new name for the thread.
    virtual void setName( const String &name ) = 0;

    ///	@brief	Returns the current name of the thread.
    ///	@return	The current name of the thread.
    virtual const String &getName() const = 0;

    ///	@brief	Set the new stack size.
    ///	@param	stacksize	The new stack size.
    virtual void setStackSize( ui32 stacksize ) = 0;

    ///	@brief	Returns the current stacksize.
    ///	@return	The current stacksize of the thread.
    virtual ui32 getStackSize() const = 0;

    ///	@brief	Waits until the thread is signaled by an event.
    ///	@param	ms				The timeout, set to 0 for infinite.
    virtual void waitForTimeout( ui32 ms ) = 0;

    ///	@brief	Waits for infinity for a signal.
    virtual void wait() = 0;

    ///	@brief	Returns a pointer showing to the thread event of this thread.
    ///	@return	A pointer showing to the thread event, equal NULL if nothing.
    virtual AbstractThreadEvent *getThreadEvent() const = 0;

    ///	@brief	Assigns a new thread priority to the thread.
    ///	@param	prio	The new thread prio.
    virtual void setPriority( Priority prio ) = 0;

    ///	@brief	Returns the current thread prio.
    ///	@return	The current thread prio.
    virtual Priority getPriority() const = 0;

    ///	@brief	The assigned name of the thread will be returned.
    ///	@return	The assigned name of the thread.
    virtual const String &getThreadName() const = 0;

protected:
    ///	@brief	Overwrite this for your own thread running method.
    virtual i32 run() = 0;

    ///	@brief	The new state will be set.
    ///	@param	newState	[in] The new state.
    virtual void setState( ThreadState newState ) = 0;
};

//-------------------------------------------------------------------------------------------------
inline
AbstractThread::~AbstractThread() {
    // empty
}

//-------------------------------------------------------------------------------------------------

} // Namespace Platform
} // Namespace ZFXCE2
