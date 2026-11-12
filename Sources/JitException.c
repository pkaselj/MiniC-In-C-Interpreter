#include <JitException.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Common.h>

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define INSTR_DUMP_INTERVAL 16

// For now using globar vars, refactor...
static unsigned char* gp_image_base = NULL;
static size_t g_image_size = 0;
PVOID gp_exception_handler = NULL;

static LONG CALLBACK _ExceptionHandlerCallback(PEXCEPTION_POINTERS info);

void InstallExceptionHandler(void* image_base, size_t image_size)
{
	gp_exception_handler = AddVectoredExceptionHandler(1, _ExceptionHandlerCallback);
	assert(gp_exception_handler);

	assert(image_base);
	gp_image_base = image_base;
	g_image_size = image_size;
}

void RemoveExceptionHandler(void)
{
	if (gp_exception_handler)
	{
		RemoveVectoredExceptionHandler(gp_exception_handler);
		gp_exception_handler = 0;
	}

	gp_image_base = NULL;
	g_image_size = 0;
}

static LONG CALLBACK _ExceptionHandlerCallback(PEXCEPTION_POINTERS info)
{
    DWORD code = info->ExceptionRecord->ExceptionCode;

    // Ignore non-fatal debug/C++ exceptions (e.g. OutputDebugString)
    if (code == DBG_PRINTEXCEPTION_C) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    uint8_t* rip = (uint8_t*)info->ContextRecord->Rip;
    uint8_t* base = (uint8_t*)gp_image_base;

    // Check if the crash occurred inside our JIT buffer
    if (rip >= base && rip < (base + g_image_size)) {
        size_t offset = (size_t)(rip - base);

        fprintf(stderr, "\n=== JIT CRASH DETECTED ===\n");
        fprintf(stderr, "Exception : 0x%08lX ", code);
        switch (code) {
        case EXCEPTION_ACCESS_VIOLATION:    fprintf(stderr, "(ACCESS_VIOLATION)\n"); break;
        case EXCEPTION_ILLEGAL_INSTRUCTION: fprintf(stderr, "(ILLEGAL_INSTRUCTION)\n"); break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:  fprintf(stderr, "(INT_DIVIDE_BY_ZERO)\n"); break;
        case EXCEPTION_STACK_OVERFLOW:      fprintf(stderr, "(STACK_OVERFLOW)\n"); break;
        default:                            fprintf(stderr, "???"); break;
        }

        fprintf(stderr, "JIT Base  : 0x%p\n", (void*)base);
        fprintf(stderr, "Crash RIP : 0x%p\n", (void*)rip);
        fprintf(stderr, "Offset    : 0x%zX\n", offset);

        // Dump 16B (2*INSTR_DUMP_INTERVAL) before and after the insruction
        uint8_t* low = (rip - INSTR_DUMP_INTERVAL < base) ? base : (rip - INSTR_DUMP_INTERVAL);
        uint8_t* high = (rip + INSTR_DUMP_INTERVAL > (base + g_image_size)) ? (base + g_image_size) : (rip + INSTR_DUMP_INTERVAL);

        fprintf(stderr, "Bytes     : ");
        for (uint8_t* p = low; p != high; p++) {
            if (p == rip)
                fprintf(stderr, "[");
            fprintf(stderr, "%02X", *p);
            if (p == rip)
                fprintf(stderr, "]");
            fprintf(stderr, " ");
        }
        fprintf(stderr, "\n");

        // Dump key registers
        PCONTEXT ctx = info->ContextRecord;
        fprintf(stderr, "RAX=%016llX RCX=%016llX RDX=%016llX RBX=%016llX\n",
            ctx->Rax, ctx->Rcx, ctx->Rdx, ctx->Rbx);
        fprintf(stderr, "RSP=%016llX RBP=%016llX RSI=%016llX RDI=%016llX\n",
            ctx->Rsp, ctx->Rbp, ctx->Rsi, ctx->Rdi);
        fprintf(stderr, "R8 =%016llX R9 =%016llX R10=%016llX R11=%016llX\n",
            ctx->R8, ctx->R9, ctx->R10, ctx->R11);
        fprintf(stderr, "==========================\n");
        fflush(stderr);
    }

    // Pass to debugger / normal crash handler so you can also inspect in Visual Studio
    return EXCEPTION_CONTINUE_SEARCH;
}