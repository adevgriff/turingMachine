#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include "encodings.h"

#define RAM_SIZE 4096

struct letter
{
    bool blank = true;
    char character;
} start;

struct tm
{
    letter tape[20000] = {start};
    tm_word ram[RAM_SIZE];
    tm_instruction ir;
    tm_word pc;
    bool eq = false;
    bool alpha[256] = {false};
    int headPos = 10000;
    int moves = 0;
};

void draw(tm current_tm, bool halted)
{
    for (int i = -15; i < 31; i++)
    {
        std::cout << '|';
        if (current_tm.tape[current_tm.headPos + i].blank)
        {
            std::cout << ' ';
        }
        else
        {
            std::cout << current_tm.tape[current_tm.headPos + i].character;
        }
    }
    std::cout << '|' << std::endl;
    for (int i = -15; i < 31; i++)
    {
        if (i != 0)
            std::cout << "  ";
        else
            std::cout << " ^";
    }
    std::cout << std::endl
              << "PC: " << current_tm.pc.u << "\tMoves: " << current_tm.moves << std::endl;
    std::cout << "IR: " << current_tm.ir.encoding.word.u << std::endl;
    std::cout << "EQ: " << current_tm.eq << std::endl;
    std::cout << "instructions processed: " << current_tm.ir.line_num << std::endl;
    if (halted)
        std::cout << "halt" << std::endl;
    else
        std::cout << "fail" << std::endl;
}

void loadProgram(char *fileName, tm &current_tm)
{
    std::ifstream codeFile;
    codeFile.open(fileName, std::ifstream::binary);
    int i = 0;
    while (!codeFile.eof())
    {
        codeFile.read(reinterpret_cast<char *>(&current_tm.ram[i]), sizeof(tm_word));
        i++;
    }
    codeFile.close();
}

bool executeProgram(tm &current_tm)
{

    current_tm.ir.line_num = 0;
    current_tm.pc.u = 0;
    current_tm.ir.encoding.word = current_tm.ram[current_tm.pc.u];

    bool halt = false;
    bool fail = false;

    while (!(halt || fail))
    {
        // Fetch
        current_tm.ir.encoding.word = current_tm.ram[current_tm.pc.u];
        current_tm.pc.u++;
        // Decode + Execute
        switch (current_tm.ir.encoding.generic.opcode)
        {
        case TM_OPCODE_ALPHA:
            current_tm.alpha[current_tm.ir.encoding.alpha.letter] = true;
            break;
        case TM_OPCODE_BRA:
            current_tm.pc.u = current_tm.ir.encoding.bra.addr;
            break;
        case TM_OPCODE_BRAC:
            if (current_tm.eq && current_tm.ir.encoding.brac.eq)
            {
                current_tm.pc.u = current_tm.ir.encoding.brac.addr;
            }
            else if (!current_tm.eq && !current_tm.ir.encoding.brac.eq)
            {
                current_tm.pc.u = current_tm.ir.encoding.brac.addr;
            }
            break;
        case TM_OPCODE_CMP:
            if (current_tm.ir.encoding.cmp.oring)
            {
                if (current_tm.ir.encoding.cmp.blank)
                {
                    current_tm.eq = current_tm.eq || current_tm.tape[current_tm.headPos].blank;
                }
                else
                {
                    current_tm.eq = current_tm.eq || (!(current_tm.tape[current_tm.headPos].blank) && (current_tm.ir.encoding.cmp.letter == current_tm.tape[current_tm.headPos].character));
                }
            }
            else
            {
                if (current_tm.ir.encoding.cmp.blank)
                {
                    current_tm.eq = current_tm.tape[current_tm.headPos].blank;
                }
                else
                {
                    current_tm.eq = (!(current_tm.tape[current_tm.headPos].blank) && (current_tm.ir.encoding.cmp.letter == current_tm.tape[current_tm.headPos].character));
                }
            }
            break;
        case TM_OPCODE_DRAW:
            if (current_tm.ir.encoding.draw.blank)
            {
                current_tm.tape[current_tm.headPos].blank = true;
            }
            else
            {
                current_tm.tape[current_tm.headPos].blank = false;
                current_tm.tape[current_tm.headPos].character = current_tm.ir.encoding.draw.letter;
            }
            break;
        case TM_OPCODE_END:
            if (current_tm.ir.encoding.end.halt)
            {
                halt = true;
            }
            else
            {
                fail = true;
            }
            break;
        case TM_OPCODE_MOVE:
            current_tm.moves++;
            if (current_tm.ir.encoding.move.left)
                current_tm.headPos--;
            else
                current_tm.headPos++;
            break;
        default:
            break;
        }
        current_tm.ir.line_num++;
    }
    return halt;
}

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        tm default_tm;
        loadProgram(argv[1], default_tm);

        tm current_tm = default_tm;

        std::ifstream tapeFile;
        tapeFile.open(argv[2]);
        std::string line;

        bool success;

        int i = 0;
        while (!tapeFile.eof())
        {
            getline(tapeFile, line);

            for (int z = 0; z < line.length(); z++)
            {
                int location = 10000 + z;
                current_tm.tape[location].blank = false;
                current_tm.tape[location].character = line[z];
            }
            i++;

            success = executeProgram(current_tm);
            draw(current_tm, success);
            current_tm = default_tm;
        }

        tapeFile.close();
    }
    else
    {
        std::cout << "please provide the program with the following.\n1.\t*.bin\n2.\t*.tape" << std::endl;
    }

    return 0;
}