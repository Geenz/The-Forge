// ---------------------------------------------------------------------------------------------------------------------------------
//                                     _     
//                                    | |    
//  _ __ ___  _ __ ___   __ _ _ __    | |___  
// | '_ ` _ \| '_ ` _ \ / _` | '__|   | '_  |
// | | | | | | | | | | | (_| | |    _ | | | |
// |_| |_| |_|_| |_| |_|\__, |_|   (_)|_| |_|
//                       __/ |               
//                      |___/                
//
// Memory manager & tracking software
//
// Best viewed with 8-character tabs and (at least) 132 columns
//
// ---------------------------------------------------------------------------------------------------------------------------------
//
// Restrictions & freedoms pertaining to usage and redistribution of this software:
//
//  * This software is 100% free
//  * If you use this software (in part or in whole) you must credit the author.
//  * This software may not be re-distributed (in part or in whole) in a modified
//    form without clear documentation on how to obtain a copy of the original work.
//  * You may not use this software to directly or indirectly cause harm to others.
//  * This software is provided as-is and without warrantee. Use at your own risk.
//
// For more information, visit HTTP://www.FluidStudios.com
//
// ---------------------------------------------------------------------------------------------------------------------------------
// Originally created on 12/22/2000 by Paul Nettle
//
// Copyright 2000, Fluid Studios, Inc., all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------

#ifndef	_H_MMGR
#define	_H_MMGR

// ---------------------------------------------------------------------------------------------------------------------------------
// For systems that don't have the __FUNCTION__ variable, we can just define it here
// ---------------------------------------------------------------------------------------------------------------------------------

#if !defined _WIN32 && !defined DURANGO
#define	__FUNCTION__ __func__
#endif

// ---------------------------------------------------------------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------------------------------------------------------------

typedef	struct tag_au
{
	size_t		actualSize;
	size_t		reportedSize;
	size_t		alignment;
	void		*actualAddress;
	void		*reportedAddress;
	char		sourceFile[140];
	char		sourceFunc[140];
	unsigned int	sourceLine;
	unsigned int	allocationType;
	bool		breakOnDealloc;
	bool		breakOnRealloc;
	unsigned int	allocationNumber;
	struct tag_au	*next;
	struct tag_au	*prev;
} sAllocUnit;

typedef	struct
{
	unsigned int	totalReportedMemory;
	unsigned int	totalActualMemory;
	unsigned int	peakReportedMemory;
	unsigned int	peakActualMemory;
	unsigned int	accumulatedReportedMemory;
	unsigned int	accumulatedActualMemory;
	unsigned int	accumulatedAllocUnitCount;
	unsigned int	totalAllocUnitCount;
	unsigned int	peakAllocUnitCount;
} sMStats;

// ---------------------------------------------------------------------------------------------------------------------------------
// Defaults for the constants & statics in the MemoryManager class
// ---------------------------------------------------------------------------------------------------------------------------------

enum
{
	m_alloc_unknown,
	m_alloc_new,
	m_alloc_new_array,
	m_alloc_malloc,
	m_alloc_calloc,
	m_alloc_memalign,
	m_alloc_realloc,
	m_alloc_delete,
	m_alloc_delete_array,
	m_alloc_free,
};

// ---------------------------------------------------------------------------------------------------------------------------------
// Used by the macros
// ---------------------------------------------------------------------------------------------------------------------------------

void		m_setOwner(const char *file, const unsigned int line, const char *func);

// ---------------------------------------------------------------------------------------------------------------------------------
// Allocation breakpoints
// ---------------------------------------------------------------------------------------------------------------------------------

bool		&m_breakOnRealloc(void *reportedAddress);
bool		&m_breakOnDealloc(void *reportedAddress);

// ---------------------------------------------------------------------------------------------------------------------------------
// The meat of the memory tracking software
// ---------------------------------------------------------------------------------------------------------------------------------

void		*m_allocator(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc,
	const unsigned int allocationType, const size_t alignment, const size_t reportedSize);
void		*m_reallocator(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc,
	const unsigned int reallocationType, const size_t reportedSize, void *reportedAddress);
void		m_deallocator(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc,
	const unsigned int deallocationType, const void *reportedAddress);

// ---------------------------------------------------------------------------------------------------------------------------------
// Utilitarian functions
// ---------------------------------------------------------------------------------------------------------------------------------

bool		m_validateAddress(const void *reportedAddress);
bool		m_validateAllocUnit(const sAllocUnit *allocUnit);
bool		m_validateAllAllocUnits();

// ---------------------------------------------------------------------------------------------------------------------------------
// Unused RAM calculations
// ---------------------------------------------------------------------------------------------------------------------------------

unsigned int	m_calcUnused(const sAllocUnit *allocUnit);
unsigned int	m_calcAllUnused();

// ---------------------------------------------------------------------------------------------------------------------------------
// Logging and reporting
// ---------------------------------------------------------------------------------------------------------------------------------

void 		mmgr_setExecutableName(const char* name, size_t length);
void		mmgr_setLogFileDirectory(const char* directory);
void		m_dumpAllocUnit(const sAllocUnit *allocUnit, const char *prefix = "");
void		m_dumpMemoryReport(const char *filename = "memreport.log", const bool overwrite = true);
sMStats		m_getMemoryStatistics();

#endif // _H_MMGR

// ---------------------------------------------------------------------------------------------------------------------------------
// mmgr.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
