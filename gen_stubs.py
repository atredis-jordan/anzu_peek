import pefile

# if this doesn't work to export asm stubs, may need .def file instead
template_dec = """
__declspec(dllexport) extern void* {name}(void*,...);
FARPROC real_{name} = NULL;
"""

template_dec_impl = """
FARPROC real_{name} = NULL;
"""

template_init = """
    real_{name} = GetProcAddress(original, "{name}");
    if (real_{name} == NULL) {{
        __fastfail(FAST_FAIL_INVALID_IAT);
    }}
"""

template_asm_def = """
EXTERN real_{name} : QWORD

{name} proc EXPORT
    ; save volatile registers
    push rax
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11

    ; of course we need to save the MM registers
    ; and account for shadowspace
    sub rsp, 0120h
    movups xmmword ptr [rsp + 000h], xmm0
    movups xmmword ptr [rsp + 010h], xmm1
    movups xmmword ptr [rsp + 020h], xmm2
    movups xmmword ptr [rsp + 030h], xmm3
    movups xmmword ptr [rsp + 040h], xmm4
    movups xmmword ptr [rsp + 050h], xmm5
    movups xmmword ptr [rsp + 060h], xmm6
    movups xmmword ptr [rsp + 070h], xmm7
    movups xmmword ptr [rsp + 080h], xmm8
    movups xmmword ptr [rsp + 090h], xmm9
    movups xmmword ptr [rsp + 0a0h], xmm10
    movups xmmword ptr [rsp + 0b0h], xmm11
    movups xmmword ptr [rsp + 0c0h], xmm12
    movups xmmword ptr [rsp + 0d0h], xmm13
    movups xmmword ptr [rsp + 0e0h], xmm14
    movups xmmword ptr [rsp + 0f0h], xmm15

    ; call log

    lea rcx, [{name}_name]
    call log_proc

    ; restore
    movups xmm0, xmmword ptr [rsp + 000h]
    movups xmm1, xmmword ptr [rsp + 010h]
    movups xmm2, xmmword ptr [rsp + 020h]
    movups xmm3, xmmword ptr [rsp + 030h]
    movups xmm4, xmmword ptr [rsp + 040h]
    movups xmm5, xmmword ptr [rsp + 050h]
    movups xmm6, xmmword ptr [rsp + 060h]
    movups xmm7, xmmword ptr [rsp + 070h]
    movups xmm8, xmmword ptr [rsp + 080h]
    movups xmm9, xmmword ptr [rsp + 090h]
    movups xmm10, xmmword ptr [rsp + 0a0h]
    movups xmm11, xmmword ptr [rsp + 0b0h]
    movups xmm12, xmmword ptr [rsp + 0c0h]
    movups xmm13, xmmword ptr [rsp + 0d0h]
    movups xmm14, xmmword ptr [rsp + 0e0h]
    movups xmm15, xmmword ptr [rsp + 0f0h]

    add rsp, 0120h

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rax

    ; go to original
    jmp [real_{name}]
{name} ENDP

{name}_name db "{name}", 0
"""

def stub_exports(path, outpath, implemented):
    pe = pefile.PE(path)

    templates = (
        ((template_dec, template_dec_impl), [""], "declarations.h"),
        ((template_init, template_init), [""], "fn_inits.h"),
        ((template_asm_def, None), [""], "asm_stubs.asmh"),
    )

    for s in pe.DIRECTORY_ENTRY_EXPORT.symbols:
        for tmps, out, _ in templates:

            tmp = tmps[0]
            if s.name in implemented:
                tmp = tmps[1]

            if tmp is None:
                continue

            out[0] += tmp.format(
                name=str(s.name, "ascii"),
            )
        
        #DEBUG just do one for debugging
        #break

    for _, out, f in templates:
        print("Writing", f)
        with open(outpath + '/' + f, "w") as fp:
            fp.write(out[0])
    

def main():
    stub_exports(
        "./anzu.dll",
        "./anzu_intercept/anzu_intercept",
        [
            b"Anzu_InternalDebugging", # TODO easy automatic way to detect these
            b"Anzu__Texture_SetVisibilityScore",
            b"Anzu_SetLogLevel",
            b"Anzu_ApplicationActive",
            b"Anzu_RegisterMessageCallback",
            b"Anzu_RegisterLogCallback",
        ],
    )

if __name__ == '__main__':
    main()