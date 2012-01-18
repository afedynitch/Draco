//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   c4/Timer.hh
 * \author Thomas M. Evans
 * \date   Mon Mar 25 17:35:07 2002
 * \brief  Define class Timer, a POSIX standard timer.
 * \note   Copyright (C) 2002-2010 Los Alamos National Security, LLC.
 *         All rights reserved.
 */
//---------------------------------------------------------------------------//
// $Id$
//---------------------------------------------------------------------------//

#ifndef __c4_Timer_hh__
#define __c4_Timer_hh__

#include "C4_Functions.hh"
#include "ds++/Assert.hh"
#include <iostream>

namespace rtt_c4
{

//===========================================================================//
/*!
 * \class Timer
 *
 * \brief POSIX standard timer.
 *
 * The Timer class is used to calculate wall clock, user cpu, and system cpu
 * timings.  It uses the POSIX standard times function, so it should work
 * well on all (POSIX) systems.
 *
 * On systems where the PAPI performance tool is available, the Timer class
 * also records some basic cache perfomance statistics. This is much less
 * portable, but is also not as important.
 *
 * Usage:
 * \code
 * #include <iostream>
 * #include "c4/Timer.hh"
 * using rtt_c4::Timer;
 *
 * Timer t;
 * t.start();
 * // do stuff
 * t.stop();
 * std::cout << t.wall_clock() << std::endl;
 * \endcode
 *
 * The POSIX implementation of timers are described in Sec. 8.15 of "Advanced
 * Programming in the UNIX Environment" by Stevens.
 *
 * The MSVC implementation of timers is described on MSDN
 * - http://msdn.microsoft.com/en-us/library/4e2ess30%28vs.71%29.aspx
 * - http://msdn.microsoft.com/en-us/library/1f4c8f33%28v=vs.71%29.aspx
 *
 * \code
 * #include <sys/timeb.h>
 * clock_t start = clock();
 * // do stuff
 * clock_t end = clock();
 * double duration = static_cast<double>(end-start)/CLOCKS_PER_SEC;
 * \endcode
 * \code
 * #define tms __timeb64
 * struct tms
 * {
 *    clock_t tms_utime;   // User CPU time.
 *    clock_t tms_stime;   // System CPU time. 
 *    clock_t tms_cutime;  // User CPU time of dead children. 
 *    clock_t tms_cstime;  // System CPU time of dead children. 
 * };
 * \endocde
 * 
 * Store the CPU time used by this process and all its dead children (and
 * their dead children) in \c BUFFER. Return the elapsed real time, or (\c
 * clock_t) -1 for errors.  All times are in \c CLK_TCK ths of a second.
 *
 * \code
 * extern clock_t times (struct tms *__buffer) __THROW;
 * \endcode
 *
 * An alternate, Unix-style time model:
 * \code
 * #include <time.h>
 * #include <sys/types.h>
 * #include <sys/timeb.h>
 * __time64_t ltime;
 * // Get UNIX-style time and display as number and string. 
 * _time64( &ltime );
 * printf( "Time in seconds since UTC 1/1/70:\t%ld\n", ltime );
 * printf( "UNIX time and date:\t\t\t%s", _ctime64( &ltime ) );
 *
 *  // Print additional time information. 
 *  struct __timeb64 tstruct;
 * _ftime64( &tstruct );
 * printf( "Plus milliseconds:\t\t\t%u\n", tstruct.millitm );
 * printf( "Zone difference in hours from UTC:\t%u\n", 
 *          tstruct.timezone/60 );
 * printf( "Time zone name:\t\t\t\t%s\n", _tzname[0] );
 * printf( "Daylight savings:\t\t\t%s\n", 
 *          tstruct.dstflag ? "YES" : "NO" );
 * \endcode
 *
 * \example c4/test/tstTime.cc
 */
// revision history:
// -----------------
// 0) original
// 1) 2003/01/21 Added sum_* member functions (Lowrie).
// 2) 2010/09/27 Added support for MSVC & CMake (KT).
// 3) 2011/09/01 Added support for PAPI cache performance monitoring (KGB).
// 
//===========================================================================//

class Timer 
{
  private:
    //! Beginning wall clock time.
    double begin;
    
    //! Ending wall clock time.
    double end;

    //! POSIX tms structure for beginning time.
    DRACO_TIME_TYPE tms_begin;
    //! POSIX tms structure for ending time.
    DRACO_TIME_TYPE tms_end;

    //! The number of clock ticks per second.
    //! \sa man times
    int const posix_clock_ticks_per_second;

    //! Flag determining if timer is currently on.
    bool timer_on;

    //! True if we can access MPI timers.
    bool const isMPIWtimeAvailable;
    
    //! sum of wall clock time over all intervals.
    double sum_wall;

    //! sum of system clock time over all intervals.
    double sum_system;

    //! sum of system clock time over all intervals.
    double sum_user;

    //! number of time intervals.
    int num_intervals;

    //! determine if MPI Wtime is available.
    bool setIsMPIWtimeAvailable() const;
    
#ifdef HAVE_PAPI
    static unsigned const papi_max_counters_ = 3U;

    long long papi_start_[papi_max_counters_];
    long long papi_stop_[papi_max_counters_];
    long long papi_counts_[papi_max_counters_];

    int status_;

    static unsigned papi_num_counters_;
    static long long papi_raw_counts_[papi_max_counters_];
    static int papi_events_[papi_max_counters_];
#endif

  public:
    
    Timer();
    virtual ~Timer() { /* empty */ };
    // Disable copy and assignment operators
    Timer( Timer const & rhs );
    Timer const & operator=( Timer const & rhs );
    inline void start();
    inline void stop();
    inline double wall_clock() const;
    inline double system_cpu() const;
    inline double user_cpu()   const;
    inline double posix_err()  const;

    //! Return the wall clock time in seconds, summed over all intervals.
    double sum_wall_clock() const { Require(! timer_on); return sum_wall; }

    //! Return the system cpu time in seconds, summed over all intervals.
    double sum_system_cpu() const { Require(! timer_on); return sum_system; }

    //! Return the user cpu time in seconds, summed over all intervals.
    double sum_user_cpu() const { Require(! timer_on); return sum_user; }

    //! Return the number of time intervals used in the sums.
    int intervals() const { Require(! timer_on); return num_intervals; }

#ifdef HAVE_PAPI
    long long sum_L2_cache_misses()     const { return papi_counts_[0]; }
    long long sum_L2_cache_hits()       const { return papi_counts_[1]; }
    long long sum_floating_operations() const { return papi_counts_[2]; }
#else
    long long sum_L2_cache_misses()     const { return 0; }
    long long sum_L2_cache_hits()       const { return 0; }
    long long sum_floating_operations() const { return 0; }
#endif

    inline void reset();

    void print( std::ostream &, int p = 2 ) const;

    void printline( std::ostream &,
		    unsigned p = 2U,
		    unsigned width = 15U) const;

    inline void merge(Timer const &);
};

//---------------------------------------------------------------------------//
// INLINE FUNCTIONS
//---------------------------------------------------------------------------//

//! Set the beginning of the time interval.
void Timer::start()
{
    Require(! timer_on);
    timer_on = true;
    ++num_intervals;

#ifdef HAVE_PAPI
    status_ = PAPI_accum_counters(papi_raw_counts_, papi_num_counters_);
    for (unsigned i=0; i<papi_num_counters_; ++i)
      {
	papi_start_[i] = papi_raw_counts_[i];
      }
#endif

    // set both begin and tms_begin.
    begin = wall_clock_time( tms_begin );  
}

//---------------------------------------------------------------------------//
//! Set the end of the time interval.
void Timer::stop()
{
    Require( timer_on );
    // set both end and tms_end.
    end      = wall_clock_time( tms_end );
    timer_on = false; 

    sum_wall   += wall_clock();
    sum_system += system_cpu();
    sum_user   += user_cpu();

#ifdef HAVE_PAPI
    status_ = PAPI_accum_counters(papi_raw_counts_, papi_num_counters_);
    for (unsigned i=0; i<papi_num_counters_; ++i)
      {
	papi_stop_[i] = papi_raw_counts_[i];
	papi_counts_[i] += papi_stop_[i]-papi_start_[i];
      }
#endif
}

//---------------------------------------------------------------------------//
//! Return the wall clock time in seconds, for the last interval.
double Timer::wall_clock() const
{
    Require(! timer_on);
    return( end - begin );
}

//---------------------------------------------------------------------------//
//! Return the system cpu time in seconds, for the last interval.
double Timer::system_cpu() const
{
    Require(! timer_on);
#if defined(WIN32)
    return 0.0; // difftime( tms_end, tms_begin );
#else
    return( tms_end.tms_stime - tms_begin.tms_stime )
	/ static_cast<double>(posix_clock_ticks_per_second);
#endif
}

//---------------------------------------------------------------------------//
//! Return the user cpu time in seconds, for the last interval.
double Timer::user_cpu() const
{
    Require(! timer_on);
#if defined(WIN32)
    return difftime( tms_end, tms_begin );
#else
    return( tms_end.tms_utime - tms_begin.tms_utime )
	/ static_cast<double>(posix_clock_ticks_per_second); 
#endif
}

//---------------------------------------------------------------------------//
//! The error in the posix timings
double Timer::posix_err() const
{
    return 1.0/static_cast<double>(DRACO_CLOCKS_PER_SEC);
}

//---------------------------------------------------------------------------//
//! Reset the interval sums.
void Timer::reset()
{
    Require(! timer_on);

    begin         = 0.0;
    end           = 0.0;
    timer_on      = false;
    sum_wall      = 0.0;
    sum_system    = 0.0;
    sum_user      = 0.0;
    num_intervals = 0;
    
#ifdef HAVE_PAPI
    for (unsigned i=0; i<papi_num_counters_; ++i)
	papi_counts_[i] = 0;
#endif

    return;
}

//---------------------------------------------------------------------------//
//! Merge counts from another Timer.
void Timer::merge(Timer const &t)
{
    Require(! timer_on);

    sum_wall      += t.sum_wall;
    sum_system    += t.sum_system;
    sum_user      += t.sum_user;
    num_intervals += t.num_intervals;
    
#ifdef HAVE_PAPI
    for (unsigned i=0; i<papi_num_counters_; ++i)
	papi_counts_[i] += t.papi_counts_[i];
#endif

    return;
}

//---------------------------------------------------------------------------//
// OVERLOADED OPERATORS
//---------------------------------------------------------------------------//

inline std::ostream& operator<<( std::ostream &out, const Timer &t )
{
    t.print( out, 2 );
    return out;
}

} // end namespace rtt_c4


#endif // __c4_Timer_hh__

//---------------------------------------------------------------------------//
//                              end of c4/Timer.hh
//---------------------------------------------------------------------------//
