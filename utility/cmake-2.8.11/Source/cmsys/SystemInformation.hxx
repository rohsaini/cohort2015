/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmsys_SystemInformation_h
#define cmsys_SystemInformation_h


/* Define these macros temporarily to keep the code readable.  */
#if !defined (KWSYS_NAMESPACE) && !cmsys_NAME_IS_KWSYS
# define kwsys_stl cmsys_stl
# define kwsys_ios cmsys_ios
#endif
#include <cmsys/stl/string>
#include <stddef.h> /* size_t */

namespace cmsys
{

// forward declare the implementation class
class SystemInformationImplementation;

class cmsys_EXPORT SystemInformation
{
#if 1
  typedef long long LongLong;
#elif 0
  typedef __int64 LongLong;
#else
# error "No Long Long"
#endif
  friend class SystemInformationImplementation;
  SystemInformationImplementation* Implementation;
public:

  SystemInformation ();
  ~SystemInformation ();

  const char * GetVendorString();
  const char * GetVendorID();
  kwsys_stl::string GetTypeID();
  kwsys_stl::string GetFamilyID();
  kwsys_stl::string GetModelID();
  kwsys_stl::string GetModelName();
  kwsys_stl::string GetSteppingCode();
  const char * GetExtendedProcessorName();
  const char * GetProcessorSerialNumber();
  int GetProcessorCacheSize();
  unsigned int GetLogicalProcessorsPerPhysical();
  float GetProcessorClockFrequency();
  int GetProcessorAPICID();
  int GetProcessorCacheXSize(long int);
  bool DoesCPUSupportFeature(long int);

  // returns an informative general description of the cpu
  // on this system.
  kwsys_stl::string GetCPUDescription();

  const char * GetHostname();
  kwsys_stl::string GetFullyQualifiedDomainName();

  const char * GetOSName();
  const char * GetOSRelease();
  const char * GetOSVersion();
  const char * GetOSPlatform();

  int GetOSIsWindows();
  int GetOSIsLinux();
  int GetOSIsApple();

  // returns an informative general description of the os
  // on this system.
  kwsys_stl::string GetOSDescription();

  bool Is64Bits();

  unsigned int GetNumberOfLogicalCPU(); // per physical cpu
  unsigned int GetNumberOfPhysicalCPU();

  bool DoesCPUSupportCPUID();

  // Retrieve id of the current running process
  LongLong GetProcessId();

  // Retrieve memory information in megabyte.
  size_t GetTotalVirtualMemory();
  size_t GetAvailableVirtualMemory();
  size_t GetTotalPhysicalMemory();
  size_t GetAvailablePhysicalMemory();

  // returns an informative general description if the installed and
  // available ram on this system. See the  GetHostMmeoryTotal, and
  // Get{Host,Proc}MemoryAvailable methods for more information.
  kwsys_stl::string GetMemoryDescription(
        const char *hostLimitEnvVarName=NULL,
        const char *procLimitEnvVarName=NULL);

  // Retrieve amount of physical memory installed on the system in KiB
  // units.
  LongLong GetHostMemoryTotal();

  // Get total system RAM in units of KiB available colectivley to all
  // processes in a process group. An example of a process group
  // are the processes comprising an mpi program which is running in
  // parallel. The amount of memory reported may differ from the host
  // total if a host wide resource limit is applied. Such reource limits
  // are reported to us via an applicaiton specified environment variable.
  LongLong GetHostMemoryAvailable(const char *hostLimitEnvVarName=NULL);

  // Get total system RAM in units of KiB available to this process.
  // This may differ from the host available if a per-process resource
  // limit is applied. per-process memory limits are applied on unix
  // system via rlimit api. Resource limits that are not imposed via
  // rlimit api may be reported to us via an application specified
  // environment variable.
  LongLong GetProcMemoryAvailable(
        const char *hostLimitEnvVarName=NULL,
        const char *procLimitEnvVarName=NULL);

  // Get the system RAM used by all processes on the host, in units of KiB.
  LongLong GetHostMemoryUsed();

  // Get system RAM used by this process id in units of KiB.
  LongLong GetProcMemoryUsed();

  // enable/disable stack trace signal handler. In order to
  // produce an informative stack trace the application should
  // be dynamically linked and compiled with debug symbols.
  static
  void SetStackTraceOnError(int enable);

  /** Run the different checks */
  void RunCPUCheck();
  void RunOSCheck();
  void RunMemoryCheck();
};

} // namespace cmsys

/* Undefine temporary macros.  */
#if !defined (KWSYS_NAMESPACE) && !cmsys_NAME_IS_KWSYS
# undef kwsys_stl
# undef kwsys_ios
#endif

#endif
