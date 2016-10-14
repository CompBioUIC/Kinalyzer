/*
  Copyright(C) 2006, William Chan
  All rights reserved.
 
      Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    
      1) Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      2) Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      3) Redistributions of source code must be provided at free of charge.
      4) Redistributions in binary forms must be provided at free of charge.
      5) Redistributions of source code within another distribution must be
        provided at free of charge including the distribution which is
        redistributing the source code. Also, the distribution which is
        redistributing the source code must have its source code
        redistributed as well.
      6) Redistribution of binary forms within another distribution must be
        provided at free of charge.
 
      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
 
void X_aligned_memcpy_sse2(void* dest, const void* src, const unsigned long size_t)
{
 
  __asm
  {
    mov esi, src;    //src pointer
    mov edi, dest;   //dest pointer
 
    mov ebx, size_t; //ebx is our counter 
    shr ebx, 7;      //divide by 128 (8 * 128bit registers)
 
 
    loop_copy:
      prefetchnta 128[ESI]; //SSE2 prefetch
      prefetchnta 160[ESI];
      prefetchnta 192[ESI];
      prefetchnta 224[ESI];
 
      movdqa xmm0, 0[ESI]; //move data from src to registers
      movdqa xmm1, 16[ESI];
      movdqa xmm2, 32[ESI];
      movdqa xmm3, 48[ESI];
      movdqa xmm4, 64[ESI];
      movdqa xmm5, 80[ESI];
      movdqa xmm6, 96[ESI];
      movdqa xmm7, 112[ESI];
 
      movntdq 0[EDI], xmm0; //move data from registers to dest
      movntdq 16[EDI], xmm1;
      movntdq 32[EDI], xmm2;
      movntdq 48[EDI], xmm3;
      movntdq 64[EDI], xmm4;
      movntdq 80[EDI], xmm5;
      movntdq 96[EDI], xmm6;
      movntdq 112[EDI], xmm7;
 
      add esi, 128;
      add edi, 128;
      dec ebx;
 
      jnz loop_copy; //loop please
    loop_copy_end:
  }
}

#define PkFastMemCpy X_aligned_memcpy_sse2
