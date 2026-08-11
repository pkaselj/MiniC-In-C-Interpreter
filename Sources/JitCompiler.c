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

// Lower DoubleWord of a QuadWord
#define LODWORD(_Q) (_Q & 0xFFFFFFFF)
// Upper DoubleWord of a QuadWord
#define HIDWORD(_Q) ((_Q >> 32) & 0xFFFFFFFF)


// Mask and Left-Shift
#define _MLS(_V, _M, _S) (( _V & _M ) << _S)

// Right-Shift and Mask
#define _RSM(_V, _M, _S) (( _V >> _S ) & _M)

// Take i-th least significant byte from multibyte word
#define _BYTE_OF(_V, _B) _RSM(_V, 0xFF, (8 * _B))

// MODRM MOD values
typedef enum _enu_modrm_mod
{
	MOD_RR = 0b11,	/* Register to Register */
	MOD_O0 = 0b00,	/* Addressing mode - immediate */
	MOD_O8 = 0b01,   /* Addressing mode - 8-bit offset */
	MOD_O32 = 0b10,  /* Addressing mode - 32-bit offset */
} enu_modrm_mod;

#define _MODRM(_MOD, _REG, _RM) (_MLS(_MOD, 0x03, 6) | _MLS(_REG, 0x07, 3) | _MLS(_RM, 0x07, 0))
// RM value to signal SIB usage
#define _RM_SIB 4 

#define _SIB(_S, _I, _B) (_MLS(_S, 0x03, 6) | _MLS(_I, 0x07, 3) | _MLS(_B, 0x07, 0))
typedef enum _enu_sib_scale
{
	SIB_S_1 = 0b00,
	SIB_S_2 = 0b01,
	SIB_S_4 = 0b10,
	SIB_S_8 = 0b11
} enu_sib_scale;


#define _REX(_W, _R, _X, _B) (0x40 | (_W << 3) | (_R << 2) | (_X << 1) | (_B << 0))

typedef enum _enu_register
{
	/* Special values */
	REG_SIB_NO_REG = 4, // SIB value for specifying that register is not used

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

typedef struct _local
{
	String loc_id;
	size_t rbp_offset; // Absolute value, offset from rbp is always negative
} local;

typedef struct _symbol
{
	String sym_id;
	const char* sym_entry_point;
	List* locals; // List<local>, list of local variables
} symbol;

typedef struct _executable
{
	unsigned char* page_base;
	unsigned char* entry_point;
	unsigned char* current_pos;
	size_t max_size;
	size_t current_size;
	List* symbol_table; // List<symbol>
	NONOWNING symbol* current_symbol;
} executable;

executable* _jit_exe_initialize()
{
	executable* exe = (executable*)malloc(sizeof(executable));
	assert(exe);

	exe->max_size = 4096;
	exe->page_base = VirtualAlloc(NULL, exe->max_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	exe->current_pos = exe->page_base;
	exe->entry_point = NULL;
	exe->current_size = 0;
	exe->symbol_table = list_create();
	exe->current_symbol = NULL;

	return exe;
}

void _jit_exe_free(executable* exe)
{
	if (exe)
	{
		list_free(exe->symbol_table);
		memset(exe, 0, sizeof(executable));
		free(exe);
	}
}

void __jit_symbol_dtor(void* sym)
{
	if (!sym)
		return;

	symbol* sym2 = (symbol*)sym;
	sv_free(sym2->sym_id);
	sym2->sym_entry_point = NULL;
	list_free(sym2->locals);
	free(sym2);
}

NONOWNING symbol* _jit_exe_register_symbol(executable* exe, const StringView sym_id)
{
	assert(exe);
	assert(exe->symbol_table);

	symbol* sym = (symbol*)malloc(sizeof(symbol));
	assert(sym);
	sym->sym_entry_point = exe->current_pos;
	sym->sym_id = sv_create_copy(sym_id);
	sym->locals = list_create();

	ListNode* n = list_create_node(sym, __jit_symbol_dtor);
	list_push(exe->symbol_table, n);

	return sym; // NONOWNING ref
}

void __jit_local_dtor(void* loc)
{
	if (!loc)
		return;

	local* loc2 = (local*)loc;

	sv_free(loc2->loc_id);
	loc2->rbp_offset = 0;

	free(loc);
}

NONOWNING local* _jit_exe_allocate_local_variable(executable* exe) // TODO: Implement named variables
{
	assert(exe);
	assert(exe->current_symbol);
	assert(exe->current_symbol->locals);

	local* loc = (local*)malloc(sizeof(local));
	assert(loc);
	loc->loc_id = sv_create_empty(); // unnamed var

	size_t var_count = list_size(exe->current_symbol->locals);
	loc->rbp_offset = (var_count + 1) * 8; // Starts on 8 (first var is [rbp - 8], assumes all vars are 64-bit

	ListNode* n = list_create_node(loc, __jit_local_dtor);
	list_push(exe->current_symbol->locals, n);

	return loc; // NONWONING ref
}

const char* _jit_exe_get_function_entry_point(executable* exe, const StringView sym_id)
{
	assert(exe);
	assert(!sv_is_empty(sym_id));

	const char* sym_entry = NULL;

	ListConstIterator* it = list_create_iterator(exe->symbol_table);
	for (; list_iterator_valid(it); list_iterator_advance(it))
	{
		symbol* sym = (symbol*)list_iterator_get(it);
		if (sv_equal(sym_id, sym->sym_id))
		{
			sym_entry = sym->sym_entry_point;
			break;
		}
	}
	list_free_iterator(it);

	return sym_entry;
}

size_t _jit_exe_get_size_of_local_variables(executable* exe)
{
	assert(exe);
	assert(exe->current_symbol);
	assert(exe->current_symbol->locals);

	// Return 8 Bytes for each local variable
	return 8 * list_size(exe->current_symbol->locals);
}

void _jit_exe_dump_symbol_table(executable* exe)
{
	assert(exe);
	assert(exe->symbol_table);

	printf("====== [SYMBOL TABLE] ======\n");
	ListConstIterator* it = list_create_iterator(exe->symbol_table);
	for (int i = 0; list_iterator_valid(it); list_iterator_advance(it), i++)
	{
		symbol* sym = (symbol*)list_iterator_get(it);
		printf("Symbol [%d] - Id: [%.*s] - Entry Point: [0x%llX] / offset [%llX]\n",
			i,
			(int)sym->sym_id.size,
			sym->sym_id.data,
			sym->sym_entry_point,
			(sym->sym_entry_point - exe->page_base));
		
		ListConstIterator* it_loc = list_create_iterator(sym->locals);
		for (size_t j = 0; list_iterator_valid(it_loc); list_iterator_advance(it_loc), j++)
		{
			local* loc = (local*)list_iterator_get(it_loc);
			printf("\tLocal: [%.*s] - RBP offset: [-%u / -0x%X]\n", (int)loc->loc_id.size, loc->loc_id.data, loc->rbp_offset, loc->rbp_offset);
		}
		list_free_iterator(it_loc);
	}
	list_free_iterator(it);
	printf("====== [            ] ======\n");

}

void _jit_exe_flush_instruction_cache(executable* exe)
{
	assert(exe);
	assert(exe->page_base);

	BOOL bSuccess = FlushInstructionCache(GetCurrentProcess(), exe->page_base, exe->current_size);
	assert(bSuccess);
}

NONOWNING symbol* _jit_exe_register_entry_point(executable* exe)
{
	assert(exe);
	assert(exe->page_base);
	assert(exe->current_pos);

	StringView id = sv_create("_entry");
	symbol* sym = _jit_exe_register_symbol(exe, id);

	exe->entry_point = exe->current_pos;

	return sym; // NONOWNING ref
}

void _jit_exe_w8(executable* exe, uint8_t byte)
{
	assert(exe);
	assert(exe->current_size + 1 <= exe->max_size);

	*(exe->current_pos++) = byte;
	exe->current_size++;
}


void _jit_exe_w32(executable* exe, uint32_t value)
{
	_jit_exe_w8(exe, _BYTE_OF(value, 0));
	_jit_exe_w8(exe, _BYTE_OF(value, 1));
	_jit_exe_w8(exe, _BYTE_OF(value, 2));
	_jit_exe_w8(exe, _BYTE_OF(value, 3));
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

void _amd64_emit_mov_r64_imm64(executable* exe, enu_register r64_0, uint64_t imm64_1);
void _amd64_emit_mov_m32_imm32_disp8(executable* exe, uint32_t imm32_0, enu_register rbase, enu_sib_scale scale, uint8_t index, uint8_t disp8);

// ------

// Note: x86-64 Sign extends imm32_0 to 64 bits on stack
void _amd64_emit_push_imm32(executable* exe, uint32_t imm32_0)
{
	_jit_exe_w8(exe, 0x68);
	_jit_exe_w32(exe, imm32_0);
}

void _amd64_emit_push_r64(executable* exe, enu_register r64_0)
{
	_jit_exe_w8(exe, 0x50 + r64_0);
}

void _amd64_emit_push_imm64(executable* exe, uint64_t imm64_0)
{
	_amd64_emit_push_imm32(exe, LODWORD(imm64_0));
	_amd64_emit_mov_m32_imm32_disp8(exe, HIDWORD(imm64_0), REG_RSP, SIB_S_1, REG_SIB_NO_REG, 4); // [RSP + 4] <- HIWORD(imm64)
}

void _amd64_emit_mov_m32_imm32_disp8(executable* exe, uint32_t imm32_0, enu_register rbase, enu_sib_scale scale, uint8_t rindex, uint8_t disp8)
{
	_jit_exe_w8(exe, 0xC7);
	_jit_exe_w8(exe, _MODRM(MOD_O8, 0, _RM_SIB)); // Use SIB with 8b offset
	_jit_exe_w8(exe, _SIB(scale, rindex, rbase)); // [R(rbase) + scale * R(rindex) + ... disp8 (below)]
	_jit_exe_w8(exe, disp8); 
	_jit_exe_w32(exe, imm32_0);
}

// r64_0 <- r64_1
void _amd64_emit_mov_r64_r64(executable* exe, enu_register r64_0, enu_register r64_1)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0)); // REX.W
	_jit_exe_w8(exe, 0x89);
	_jit_exe_w8(exe, _MODRM(MOD_RR, r64_1 /*op2*/, r64_0/*op1*/));
}

// [R(rbase) + scale * R(rindex)] <- r64_1
void _amd64_emit_mov_m64_r64_disp8(executable* exe, enu_register r64_1, enu_register rbase, enu_sib_scale scale, enu_register rindex, uint8_t disp8)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0)); // REX.W
	_jit_exe_w8(exe, 0x89);
	_jit_exe_w8(exe, _MODRM(MOD_O8, r64_1 /*op2*/, rbase));
	_jit_exe_w8(exe, disp8);
}

void _amd64_util_emit_store_local(executable* exe, enu_register dest, uint32_t local_offset)
{
	uint8_t offset = _BYTE_OF(local_offset, 0);
	offset = (uint8_t)(-offset); // Two's complement

	_amd64_emit_mov_m64_r64_disp8(exe, dest, REG_RBP, SIB_S_1, REG_SIB_NO_REG, offset);
}

// r64_0 <- [R(rbase) + scale * R(rindex) + disp8]
void _amd64_emit_mov_r64_m64_disp8(executable* exe, enu_register r64_0, enu_register rbase, enu_sib_scale scale, enu_register rindex, uint8_t disp8)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0)); // REX.W
	_jit_exe_w8(exe, 0x8B);
	_jit_exe_w8(exe, _MODRM(MOD_O8, r64_0 /*op2*/, rbase));
	_jit_exe_w8(exe, disp8);
}

void _amd64_util_emit_load_local(executable* exe, enu_register dest, uint32_t local_offset)
{
	uint8_t offset = _BYTE_OF(local_offset, 0);
	offset = (uint8_t)(-offset); // Two's complement

	_amd64_emit_mov_r64_m64_disp8(exe, dest, REG_RBP, SIB_S_1, REG_SIB_NO_REG, offset);
}

void _amd64_emit_pop_r64(executable* exe, enu_register r64_0)
{
	_jit_exe_w8(exe, 0x58 + r64_0);
}

void _amd64_emit_ret(executable* exe)
{
	_jit_exe_w8(exe, 0xC3); // RET (near)
}

// r64_0 <- imm64_0
void _amd64_emit_mov_r64_imm64(executable* exe, enu_register r64_0, uint64_t imm64_0)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0));
	_jit_exe_w8(exe, 0xB8 + r64_0);
	_jit_exe_w64(exe, imm64_0);
}

// r64_0 <- r64_0 + r64_1
void _amd64_emit_add_r64_r64(executable* exe, enu_register r64_0, enu_register r64_1)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0)); // REX.W
	_jit_exe_w8(exe, 0x01);
	_jit_exe_w8(exe, _MODRM(MOD_RR, r64_1, r64_0));
}

// reg64_0 <- (reg64_0 + imm32_1)
void _amd64_emit_add_r64_imm32(executable* exe, enu_register reg64_0, uint32_t imm32_1)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0));
	_jit_exe_w8(exe, 0x81);
	_jit_exe_w8(exe, _MODRM(MOD_RR, 0, reg64_0));
	_jit_exe_w32(exe, imm32_1);
}

// r64_0 <- r64_0 - r64_1
void _amd64_emit_sub_r64_r64(executable* exe, enu_register r64_0, enu_register r64_1)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0));
	_jit_exe_w8(exe, 0x29);
	_jit_exe_w8(exe, _MODRM(MOD_RR, r64_1, r64_0));
}

// r64_0 <- r64_0 * r64_1
void _amd64_emit_imul_r64_r64(executable* exe, enu_register r64_0, enu_register r64_1)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0));
	_jit_exe_w8(exe, 0x0F);
	_jit_exe_w8(exe, 0xAF);
	_jit_exe_w8(exe, _MODRM(MOD_RR, r64_0, r64_1));
}

// reg64_0 <- (reg64_0 - imm32_1)
void _amd64_emit_sub_r64_imm32(executable* exe, enu_register r64_0, uint32_t imm32_1)
{
	_jit_exe_w8(exe, _REX(1, 0, 0, 0));
	_jit_exe_w8(exe, 0x81);
	_jit_exe_w8(exe, _MODRM(MOD_RR, 5, r64_0));
	_jit_exe_w32(exe, imm32_1);
}

// Encodes SUB RSP, 0xDEADBEEF and returns the pointer to 0xDEADBEEF to be overwritten
// later by calculated offset
uint32_t* _amd64_emit_sub_rsp_imm32_placeholder(executable* exe)
{
	unsigned char* placeholder = NULL;

	_jit_exe_w8(exe, _REX(1, 0, 0, 0));
	_jit_exe_w8(exe, 0x81);
	_jit_exe_w8(exe, _MODRM(MOD_RR, 5, REG_RSP));

	placeholder = exe->current_pos;

	_jit_exe_w32(exe, 0xDEADBEEF);

	return (uint32_t*)placeholder;
}

void _amd64_emit_int3(executable* exe)
{
	_jit_exe_w8(exe, 0xCC);
}

// CALL r64_0
void _amd64_emit_call_r64_near(executable* exe, enu_register r64_0)
{
	_jit_exe_w8(exe, 0xFF);
	_jit_exe_w8(exe, _MODRM(MOD_RR, 2, r64_0));
}

// CALL rel32
void _amd64_emit_call_rel32(executable* exe, uint32_t rel32_0)
{
	_jit_exe_w8(exe, 0xE8);
	_jit_exe_w32(exe, rel32_0);
}

void _amd64_util_emit_call_rel32_from_abs(executable* exe, unsigned char* abs_sym_entry)
{
	// Calculate base for call offset, it is start of next instruction
	// so we add 5B for this CALL instruction
	unsigned char* base = exe->current_pos + 5;

	int64_t offset = (int64_t)abs_sym_entry - (int64_t)base;
	
	uint32_t encoded_offset = (uint32_t)(int32_t)offset;
	_amd64_emit_call_rel32(exe, encoded_offset);
}

// --------------------------------------------

void _jit_compile_statement(AstNode* tree, executable* exe);

// -----------

void _jit_compile_function_prologue(executable* exe)
{
	_amd64_emit_push_r64(exe, REG_RBP);
	_amd64_emit_mov_r64_r64(exe, REG_RBP, REG_RSP);
}

void _jit_compile_function_epilogue(executable* exe)
{
	_amd64_emit_mov_r64_r64(exe, REG_RSP, REG_RBP);
	_amd64_emit_pop_r64(exe, REG_RBP);
	_amd64_emit_ret(exe);

	// Visual padding to separate function at first glance
	// TODO: Function alignment?
	for (size_t i = 0; i < 10; i++)
	{
		_amd64_emit_int3(exe);
	}
}

void _jit_compile_numeric_expression(AstNode* tree, executable* exe)
{
	assert(exe);
	assert(tree);
	assert(tree->type == AST_NUM_EXPR);

	uint64_t imm64_0 = (uint64_t)tree->u.number.value; // TODO: Cast to uint64 for now
	_amd64_emit_mov_r64_imm64(exe, REG_RAX, imm64_0); // Store value in RAX (accumulator register)
}

void _jit_compile_binary_expression(AstNode* tree, executable* exe)
{
	assert(exe);
	assert(tree);
	assert(tree->type == AST_BINARY_EXPR);

	AstNode* left = tree->u.binary_expr.left;
	AstNode* right = tree->u.binary_expr.right;

	local* _l_right = _jit_exe_allocate_local_variable(exe);

	_jit_compile_statement(right, exe); // TODO: Not sure if i would use compile statement or expression?
	// Result of the operation is kept in accumulator register (RAX)
	// Store it to a local variable
	_amd64_util_emit_store_local(exe, REG_RAX, _l_right->rbp_offset);

	_jit_compile_statement(left, exe); // TODO: Not sure if i would use compile statement or expression?
	// Result of the operation is kept in accumulator register (RAX)

	// Load RHS to RCX
	_amd64_util_emit_load_local(exe, REG_RCX, _l_right->rbp_offset);


	LexTokenType op = tree->u.binary_expr.op;
	switch (op)
	{
	case TT_OP_ADD:
		_amd64_emit_add_r64_r64(exe, REG_RAX, REG_RCX);
		break;
	case TT_OP_SUB:
		_amd64_emit_sub_r64_r64(exe, REG_RAX, REG_RCX);
		break;
	case TT_OP_MUL:
		_amd64_emit_imul_r64_r64(exe, REG_RAX, REG_RCX);
		break;
	// TODO: More operators, DIV?
	default:
		LogError("JIT :: _jit_compile_binary_expression() - cannot handle operator [%d / %s]\n", op, GetLexTokenTypeString(op));
		break;
	}

	// Result of the operation is kept in accumulator register (RAX)
}

void _jit_compile_function_call_expression(AstNode* tree, executable* exe)
{
	assert(exe);
	assert(tree);
	assert(tree->type == AST_FN_CALL_EXPR);

	// TODO: For now only functions without arguments

	const char* sym_entry = _jit_exe_get_function_entry_point(exe, tree->u.fn_call.symbol->u.string.value);
	if (!sym_entry)
	{
		StringView id = tree->u.fn_call.symbol->u.string.value;
		LogError("[JIT] :: _jit_compile_function_call_expression() - could not find symbol [%.s]\n",
			(int)id.size,
			id.data);
		// BREAK?
	}

	// We assume preallocated size for local vars up to now is aligned to 16B

	// TODO: Save volatile registers
	// Allocate 32B shadow space
	_amd64_emit_sub_r64_imm32(exe, REG_RSP, 32);

	_amd64_util_emit_call_rel32_from_abs(exe, sym_entry);

	// Restore shadow space
	_amd64_emit_add_r64_imm32(exe, REG_RSP, 32);
	// TODO: Restore volatile registers
	
}

// NOTE: By convention RAX register is used to store last statement's value
// TODO: Maybe split statements from expressions, implement 'return'
void _jit_compile_statement(AstNode* tree, executable* exe)
{
	assert(exe);
	assert(tree);
	//assert(tree->type == ???);

	switch (tree->type)
	{
	case AST_BINARY_EXPR:
		_jit_compile_binary_expression(tree, exe);
		break;
	case AST_NUM_EXPR:
		_jit_compile_numeric_expression(tree, exe);
		break;
	case AST_FN_CALL_EXPR:
		_jit_compile_function_call_expression(tree, exe);
		break;
	default:
		LogError("JIT :: _jit_compile_statement() - cannot handle node type [%d / %s]\n", tree->type, GetAstNodeTypeString(tree->type));
		break;
	}
}

void _jit_compile_function_definition(AstNode* tree, executable* exe)
{
	assert(exe);
	assert(tree);
	assert(tree->type == AST_FN_DEF_STMT);

	// Save current symbol
	symbol* old_sym = exe->current_symbol;

	exe->current_symbol = _jit_exe_register_symbol(exe, tree->u.fn_def.symbol->u.string.value);

	_jit_compile_function_prologue(exe);

	// Preallocate stack values for local varaibles (placeholder for size, populate later)
	uint32_t* local_prealloc_size_ptr = _amd64_emit_sub_rsp_imm32_placeholder(exe);
	
	ListConstIterator* it = list_create_iterator(tree->u.fn_def.block->u.block.statements);
	for (;list_iterator_valid(it); list_iterator_advance(it))
	{
		AstNode* stmt = (AstNode*)list_iterator_get(it);
		_jit_compile_statement(stmt, exe);
	}
	list_free_iterator(it);

	// Whatever is last written in RAX will be returned
	// Should be last expression's (statement's) value

	_jit_compile_function_epilogue(exe);

	// After compiling the function and registering all local variables,
	// calculate total size and replace placeholder in preallocation step
	// at the beginning of the function.
	size_t preallocated_locals_size = _jit_exe_get_size_of_local_variables(exe);

	// Before any function calls, we need 16-Byte aligned stack
	// In addition to preallocating size for local variables
	// add padding to align the stack to 16B.

	// Now we need to weatch out for stack modification.
	// Beware that any stack modifications must be undone or 
	// aligned to 16B before any function call

	size_t preallocated_aligned = (preallocated_locals_size + 8 /*RBP on stack*/);
	preallocated_aligned += ( 16 - (preallocated_aligned % 16)) % 16; // TODO: power of 2 trick

	*local_prealloc_size_ptr = (uint32_t)preallocated_aligned;

	// Restore old symbol
	exe->current_symbol = old_sym;
}

void _jit_compile_program(AstNode* tree, executable* exe)
{
	assert(tree->type == AST_S);

	// Generate functions before generating entry point for compiled program

	ListConstIterator* it = list_create_iterator(tree->u.program.function_definitions);
	for (;list_iterator_valid(it); list_iterator_advance(it))
	{
		AstNode* stmt = (AstNode*)list_iterator_get(it);
		_jit_compile_function_definition(stmt, exe);
	}
	list_free_iterator(it);

	// -----------------
	// Generate entry point for the compiled program
	symbol* ep = _jit_exe_register_entry_point(exe); // Entry point starts here
	exe->current_symbol = ep;

	_jit_compile_function_prologue(exe);

	// Preallocate stack values for local varaibles (placeholder for size, populate later)
	uint32_t* local_prealloc_size_ptr = _amd64_emit_sub_rsp_imm32_placeholder(exe);

	it = list_create_iterator(tree->u.program.statements);
	for (;list_iterator_valid(it); list_iterator_advance(it))
	{
		AstNode* stmt = (AstNode*)list_iterator_get(it);
		_jit_compile_statement(stmt, exe);
	}
	list_free_iterator(it);


	// Whatever is last written in RAX will be returned
	// Should be last expression's (statement's) value
	_jit_compile_function_epilogue(exe);

	// After compiling the function and registering all local variables,
	// calculate total size and replace placeholder in preallocation step
	// at the beginning of the function.
	size_t preallocated_locals_size = _jit_exe_get_size_of_local_variables(exe);

	// Before any function calls, we need 16-Byte aligned stack
	// In addition to preallocating size for local variables
	// add padding to align the stack to 16B.

	// Now we need to weatch out for stack modification.
	// Beware that any stack modifications must be undone or 
	// aligned to 16B before any function call

	size_t preallocated_aligned = (preallocated_locals_size + 8 /*RBP on stack*/);
	preallocated_aligned += (16 - (preallocated_aligned % 16)) % 16; // TODO: power of 2 trick

	*local_prealloc_size_ptr = (uint32_t)preallocated_aligned;
}


// --------------------------------------------

fn_compiled_entry JIT_compile(AstNode* tree)
{
	assert(tree);
	// TODO: Allocate page from OS
	// for now just ordinary buffer
	executable* exe = _jit_exe_initialize();


	_jit_compile_program(tree, exe);
	_jit_exe_flush_instruction_cache(exe);

	_jit_exe_dump_symbol_table(exe);

	// TODO: Make page executable
	_jit_exe_dump_file(exe, "S:\\output.bin");
	
	fn_compiled_entry program_entry = (fn_compiled_entry)exe->entry_point;
	assert(program_entry);
	_jit_exe_free(exe);

	return program_entry; //TODO: Return ptr to compiled function
}

void JIT_release(fn_compiled_entry fn_address)
{
	// TODO: Release OS page
}