/*
 * Name        : encodings.h
 * Author      : William "Amos" Confer
 *
 * License     : Copyright (C) 2022 All rights reserved
 */

#ifndef __ENCODINGS_H__ // you already know what this does
#define __ENCODINGS_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus // basically if c++ compiler know that this below is c code allows this to be used for c or c++
extern "C"
{
#endif

     typedef uint16_t tm_uword; // define types tm = turing machine
     typedef int16_t tm_sword;

     typedef union tm_word tm_word;
     typedef union tm_encoding tm_encoding;
     typedef struct tm_instruction tm_instruction;

     union tm_word
     {
          struct
          {
               uint16_t byte0 : 8;
               uint16_t byte1 : 8;
          } bytes;
          tm_uword u;
          tm_sword s;
     };

#define TM_OPCODE_ALPHA 0 // defs for opcodes a total of 7 opcodes 3 bits
#define TM_OPCODE_CMP 1
#define TM_OPCODE_BRAC 2
#define TM_OPCODE_BRA 3
#define TM_OPCODE_DRAW 4
#define TM_OPCODE_MOVE 5
#define TM_OPCODE_END 6

     union tm_encoding
     {
          struct
          {
               uint16_t rsvd : 13;
               uint16_t opcode : 3; // : used in a struct to specify how much memory is actually used but compiler treats this as 16 bit in this case
          } generic;
          struct
          {
               uint16_t letter : 8;
               uint16_t rsvd : 5;
               uint16_t opcode : 3;
          } alpha;
          struct
          {
               uint16_t letter : 8;
               uint16_t rsvd : 3;
               uint16_t blank : 1;
               uint16_t oring : 1;
               uint16_t opcode : 3;
          } cmp;
          struct
          {
               uint16_t addr : 12;
               uint16_t eq : 1;
               uint16_t opcode : 3;
          } brac;
          struct
          {
               uint16_t addr : 12;
               uint16_t rsvd : 1;
               uint16_t opcode : 3;
          } bra;
          struct
          {
               uint16_t letter : 8;
               uint16_t rsvd : 4;
               uint16_t blank : 1;
               uint16_t opcode : 3;
          } draw;
          struct
          {
               uint16_t rsvd : 12;
               uint16_t left : 1;
               uint16_t opcode : 3;
          } move;
          struct
          {
               uint16_t rsvd : 12;
               uint16_t halt : 1;
               uint16_t opcode : 3;
          } end;
          tm_word word;
     };

     struct tm_instruction
     {
          int line_num;
          tm_encoding encoding;
     };

#ifdef __cplusplus
}
#endif

#endif /* __ENCODINGS_H__ */
