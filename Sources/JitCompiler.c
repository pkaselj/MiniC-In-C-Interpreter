#include <JitCompiler.h>

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#if defined(_WIN32)

#if !defined(_M_AMD64) || !defined(_M_X64)
#error "JIT Compiler for Win32 supoprts only AMD64 processors (Intel x86-64)"
#endif // !defined(_M_AMD64)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#else

#error "JIT Compiler only supported on the following platforms: Win32x64"

#endif // defined(WIN32)

// --------------------------------------------

// Mask and Left-Shift
#define _MLS(_V, _M, _S) (( _V & _M ) << _S)

// Right-Shift and Mask
#define _RSM(_V, _M, _S) (( _V >> _S ) & _M)

// Take i-th least significant byte from multibyte word
#define _BYTE_OF(_V, _B) _RSM(_V, 0xFF, (8 * _B))

#define _MODRM(_MOD, _REG, _RM) (_MLS(_MOD, 0x03, 6) | _MLS(_REG, 0x07, 3) | _MLS(_RM, 0x07, 0))

#define _REX(_W, _R, _X, _B) (0x40 | (_W << 3) | (_R << 2) | (_X << 1) | (_B << 0))

typedef enum _enu_register
{
	/* 8-bit GP */
	REG_AL		= 0,
	REG_CL		= 1,
	REG_DL		= 2,
	REG_BL		= 3,
	REG_AH		= 4,	REG_SPL1 = 4,
	REG_CH		= 5,	REG_BPL1 = 5,
	REG_DH		= 6,	REG_SIL1 = 6,
	REG_BH		= 7,	REG_DIL1 = 7,
	REG_R8L		= 8,
	REG_R9L		= 9,
	REG_R10L	= 10,
	REG_R11L	= 11,
	REG_R12L	= 12,
	REG_R13L	= 13,
	REG_R14L	= 14,
	REG_R15L	= 15,

	/* 16-bit GP */
	REG_AX = 0,
	REG_CX = 1,
	REG_DX = 2,
	REG_BX = 3,
	REG_SP = 4,
	REG_BP = 5,
	REG_SI = 6,
	REG_DI = 7,
	REG_R8W = 8,
	REG_R9W = 9,
	REG_R10W = 10,
	REG_R11W = 11,
	REG_R12W = 12,
	REG_R13W = 13,
	REG_R14W = 14,
	REG_R15W = 15,

	/* 32-bit GP */
	REG_EAX = 0,
	REG_ECX = 1,
	REG_EDX = 2,
	REG_EBX = 3,
	REG_ESP = 4,
	REG_EBP = 5,
	REG_ESI = 6,
	REG_EDI = 7,
	REG_R8D = 8,
	REG_R9D = 9,
	REG_R10D = 10,
	REG_R11D = 11,
	REG_R12D = 12,
	REG_R13D = 13,
	REG_R14D = 14,
	REG_R15D = 15,

	/* 64-bit GP */
	REG_RAX = 0,
	REG_RCX = 1,
	REG_RDX = 2,
	REG_RBX = 3,
	REG_RSP = 4,
	REG_RBP = 5,
	REG_RSI = 6,
	REG_RDI = 7,
	REG_R8 = 8,
	REG_R9 = 9,
	REG_R10 = 10,
	REG_R11 = 11,
	REG_R12 = 12,
	REG_R13 = 13,
	REG_R14 = 14,
	REG_R15 = 15,

	/* 80-bit x87 */
	REG_ST0 = 0,
	REG_ST1 = 1,
	REG_ST2 = 2,
	REG_ST3 = 3,
	REG_ST4 = 4,
	REG_ST5 = 5,
	REG_ST6 = 6,
	REG_ST7 = 7,

	/* 64-bit MMX */
	REG_MMX0 = 0,
	REG_MMX1 = 1,
	REG_MMX2 = 2,
	REG_MMX3 = 3,
	REG_MMX4 = 4,
	REG_MMX5 = 5,
	REG_MMX6 = 6,
	REG_MMX7 = 7,

	// Alterinative numerations
	REG_MMX0_ALT = 8,
	REG_MMX1_ALT = 9,
	REG_MMX2_ALT = 10,
	REG_MMX3_ALT = 11,
	REG_MMX4_ALT = 12,
	REG_MMX5_ALT = 13,
	REG_MMX6_ALT = 14,
	REG_MMX7_ALT = 15,

	/* 128-bit XMM */
	REG_XMM0 = 0,
	REG_XMM1 = 1,
	REG_XMM2 = 2,
	REG_XMM3 = 3,
	REG_XMM4 = 4,
	REG_XMM5 = 5,
	REG_XMM6 = 6,
	REG_XMM7 = 7,
	REG_XMM8 = 8,
	REG_XMM9 = 9,
	REG_XMM10 = 10,
	REG_XMM11 = 11,
	REG_XMM12 = 12,
	REG_XMM13 = 13,
	REG_XMM14 = 14,
	REG_XMM15 = 15,

	/* 256-bit YMM */
	REG_YMM0 = 0,
	REG_YMM1 = 1,
	REG_YMM2 = 2,
	REG_YMM3 = 3,
	REG_YMM4 = 4,
	REG_YMM5 = 5,
	REG_YMM6 = 6,
	REG_YMM7 = 7,
	REG_YMM8 = 8,
	REG_YMM9 = 9,
	REG_YMM10 = 10,
	REG_YMM11 = 11,
	REG_YMM12 = 12,
	REG_YMM13 = 13,
	REG_YMM14 = 14,
	REG_YMM15 = 15,

	/* 16-bit segment */
	REG_ES = 0,
	REG_CS = 1,
	REG_SS = 2,
	REG_DS = 3,
	REG_FS = 4,
	REG_GS = 5,

	// Alternative numerations
	REG_ES_ALT = 8,
	REG_CS_ALT = 9,
	REG_SS_ALT = 10,
	REG_DS_ALT = 11,
	REG_FS_ALT = 12,
	REG_GS_ALT = 13,

	/* 32-bit control */
	REG_CR0 = 0,
	REG_CR1 = 1,
	REG_CR2 = 2,
	REG_CR3 = 3,
	REG_CR4 = 4,
	REG_CR5 = 5,
	REG_CR6 = 6,
	REG_CR7 = 7,
	REG_CR8 = 8,
	REG_CR9 = 9,
	REG_CR10 = 10,
	REG_CR11 = 11,
	REG_CR12 = 12,
	REG_CR13 = 13,
	REG_CR14 = 14,
	REG_CR15 = 15,

	/* 32-bit debug */
	REG_DR0 = 0,
	REG_DR1 = 1,
	REG_DR2 = 2,
	REG_DR3 = 3,
	REG_DR4 = 4,
	REG_DR5 = 5,
	REG_DR6 = 6,
	REG_DR7 = 7,
	REG_DR8 = 8,
	REG_DR9 = 9,
	REG_DR10 = 10,
	REG_DR11 = 11,
	REG_DR12 = 12,
	REG_DR13 = 13,
	REG_DR14 = 14,
	REG_DR15 = 15

} enu_register;

// --------------------------------------------

typedef struct _executable
{
	unsigned char* page_base;
	unsigned char* current_pos;
	size_t max_size;
	size_t current_size;
} executable;

executable* _jit_exe_initialize()
{
	executable* exe = (executable*)malloc(sizeof(executable));
	assert(exe);

	exe->max_size = 4096;
	exe->page_base = VirtualAlloc(NULL, exe->max_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	exe->current_pos = exe->page_base;
	exe->current_size = 0;

	return exe;
}

void _jit_exe_free(executable* exe)
{
	if (exe)
	{
		memset(exe, 0, sizeof(executable));
		free(exe);
	}
}

void _jit_exe_w8(executable* exe, uint8_t byte)
{
	assert(exe);
	assert(exe->current_size + 1 <= exe->max_size);

	*(exe->current_pos++) = byte;
	exe->current_size++;
}

void _jit_exe_w64(executable* exe, uint64_t value)
{
	_jit_exe_w8(exe, _BYTE_OF(value, 0));
	_jit_exe_w8(exe, _BYTE_OF(value, 1));
	_jit_exe_w8(exe, _BYTE_OF(value, 2));
	_jit_exe_w8(exe, _BYTE_OF(value, 3));
	_jit_exe_w8(exe, _BYTE_OF(value, 4));
	_jit_exe_w8(exe, _BYTE_OF(value, 5));
	_jit_exe_w8(exe, _BYTE_OF(value, 6));
	_jit_exe_w8(exe, _BYTE_OF(value, 7));
}

void _jit_exe_dump_file(executable* exe, const char* output_path)
{
	assert(exe);
	assert(output_path);

	FILE* fp = NULL;
	if (!(fp = fopen(output_path, "wb")))
	{
		return;
	}

	fwrite(exe->page_base, 1, exe->current_size, fp);

	fclose(fp);
}

// --------------------------------------------

void _amd64_emit_push_rbp(executable* exe)
{
	_jit_exe_w8(exe, 0x50 + REG_RBP);
}

void _amd64_emit_mov_rbp_rsp(executable* exe)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0)); // REX.W
	_jit_exe_w8(exe, 0x89);
	_jit_exe_w8(exe, _MODRM(0b11 /*RR*/, REG_RSP/*op2*/, REG_RBP/*op1*/));
}

void _amd64_emit_pop_r64(executable* exe, enu_register r64_0)
{
	_jit_exe_w8(exe, 0x58 + r64_0);
}

void _amd64_emit_ret(executable* exe)
{
	_jit_exe_w8(exe, 0xC3); // RET (near)
}

void _amd64_emit_mov_rax_imm64(executable* exe, uint64_t imm64_0)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0));
	_jit_exe_w8(exe, 0xB8 + REG_RAX);
	_jit_exe_w64(exe, imm64_0);
}

// --------------------------------------------

void _jit_compile_program_prolog(executable* exe)
{
	_amd64_emit_push_rbp(exe);
	_amd64_emit_mov_rbp_rsp(exe);
}

void _jit_compile_program_epilog(executable* exe)
{
	_amd64_emit_pop_r64(exe, REG_RBP);
	_amd64_emit_ret(exe);
}

void _jit_compile_program(AstNode* tree, executable* exe)
{
	assert(tree->type == AST_S);

	_jit_compile_program_prolog(exe);


	// TODO: Expression
	// For now return 0xAB
	_amd64_emit_mov_rax_imm64(exe, 0xAABBCCDDEEFF0011);

	_jit_compile_program_epilog(exe);
}


// --------------------------------------------

fn_compiled_entry JIT_compile(AstNode* tree)
{
	assert(tree);
	// TODO: Allocate page from OS
	// for now just ordinary buffer
	executable* exe = _jit_exe_initialize();


	_jit_compile_program(tree, exe);

	// TODO: Make page executable
	_jit_exe_dump_file(exe, "S:\\output.bin");
	
	fn_compiled_entry program_entry = (fn_compiled_entry)exe->page_base;
	_jit_exe_free(exe);

	return program_entry; //TODO: Return ptr to compiled function
}

void JIT_release(fn_compiled_entry fn_address)
{
	// TODO: Release OS page
}