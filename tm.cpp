/*
 * Name        : tm.cpp
 * Author      : Aero Griffin
 */

#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <cstdlib>
#include <string>
#include "encodings.h"

#define ADDR_BITS 12
#define WORD_CNT (1 << ADDR_BITS)
#define BIN_SIZE (WORD_CNT * sizeof(tm_word))

struct letter
{
    bool is_blank = true;
    char letter = ' ';
};

typedef letter letter;

// makes it simpler to set up new turing machines and pass them around
struct turing_machine
{
    std::deque<letter> tape = {};
    tm_encoding pc;
    tm_encoding ram[WORD_CNT];
    tm_instruction ir;
    bool alphabet[256] = {false};
    bool equals_flag = false;
    int headPos = 0;
};

// need a function to draw
void draw_tape(turing_machine current_tm, bool halted, int moves)
{
    // remove only the leading blanks
    while (current_tm.tape[0].is_blank && current_tm.tape.size() > 0 && current_tm.headPos > 0)
    {
        current_tm.tape.erase(current_tm.tape.begin());
        current_tm.headPos--;
    }
    // draw stuff here
    for (long unsigned int i = 0; i < current_tm.tape.size(); i++)
    {
        std::cout << (current_tm.tape[i].is_blank ? ' ' : current_tm.tape[i].letter);
    }
    std::cout << std::endl;
    for (int i = 0; i < current_tm.headPos; i++)
    {
        std::cout << ' ';
    }
    std::cout << '^' << std::endl;
    std::cout << (halted ? "halted " : "failed ") << "in " << current_tm.ir.line_num << " instructions and " << moves << " moves."
              << "\n\n"
              << std::endl;
}

void animate(turing_machine current_tm)
{
    // draw stuff here
    for (long unsigned int i = 0; i < current_tm.tape.size(); i++)
    {
        std::cout << (current_tm.tape[i].is_blank ? ' ' : current_tm.tape[i].letter);
    }
    std::cout << std::endl;
    for (int i = 0; i < current_tm.headPos; i++)
    {
        std::cout << ' ';
    }
    std::cout << '^' << std::endl;
    std::cout << "instruction " << std::hex << current_tm.ir.encoding.word.u << std::endl;
}

void draw_total(int bin_size, int total_moves, int total_instructions)
{
    std::cout << "The size of the program was " << bin_size << " bits executed with a total of " << total_moves << " moves and " << total_instructions << " instructions." << std::endl;
}

// function to execute loaded program
bool executeProgram(turing_machine &current_tm, int &moves, int &total_moves, int &total_instructions, bool animating)
{

    current_tm.ir.line_num = 0;
    current_tm.pc.word.u = 0;
    bool halt = false;
    bool fail = false;

    while (!(halt || fail))
    {
        // Fetch
        current_tm.ir.encoding = current_tm.ram[current_tm.pc.word.u];
        current_tm.pc.word.u++;

        // Decode + Execute
        switch (current_tm.ir.encoding.generic.opcode)
        {
        case TM_OPCODE_ALPHA:
            current_tm.alphabet[current_tm.ir.encoding.alpha.letter] = true;
            break;
        case TM_OPCODE_BRA:
            current_tm.pc.word.u = current_tm.ir.encoding.bra.addr;
            break;
        case TM_OPCODE_BRAC:
            if (current_tm.equals_flag && current_tm.ir.encoding.brac.eq)
            {
                current_tm.pc.word.u = current_tm.ir.encoding.brac.addr;
            }
            else if (!current_tm.equals_flag && !current_tm.ir.encoding.brac.eq)
            {
                current_tm.pc.word.u = current_tm.ir.encoding.brac.addr;
            }
            break;
        case TM_OPCODE_CMP:
            if (!current_tm.tape[current_tm.headPos].is_blank && !current_tm.alphabet[(uint8_t)current_tm.tape[current_tm.headPos].letter])
            {
                fail = true;
            }
            else if (!(current_tm.ir.encoding.cmp.oring && current_tm.equals_flag))
            {
                if (current_tm.ir.encoding.cmp.blank)
                {
                    current_tm.equals_flag = current_tm.tape[current_tm.headPos].is_blank;
                }
                else
                {
                    current_tm.equals_flag = (!(current_tm.tape[current_tm.headPos].is_blank) && (current_tm.ir.encoding.cmp.letter == current_tm.tape[current_tm.headPos].letter));
                }
            }
            break;
        case TM_OPCODE_DRAW:
            if (current_tm.ir.encoding.draw.blank)
            {
                current_tm.tape[current_tm.headPos].is_blank = true;
            }
            else
            {
                current_tm.tape[current_tm.headPos].is_blank = false;
                current_tm.tape[current_tm.headPos].letter = current_tm.ir.encoding.draw.letter;
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
            moves++;
            if (current_tm.ir.encoding.move.left)
                if (current_tm.headPos == 0)
                    current_tm.tape.push_front(letter{true, 'a'});
                else
                    current_tm.headPos--;
            else
            {
                current_tm.headPos++;
                if ((long unsigned int)current_tm.headPos == current_tm.tape.size())
                    current_tm.tape.push_back(letter{true, 'a'});
            }
            break;
        default:
            break;
        }
        current_tm.ir.line_num++;
        if (animating)
        {
            animate(current_tm);
            // needs a delay function but had trouble finding one that I am sure is platform independent
        }
    }
    total_instructions += current_tm.ir.line_num;
    total_moves += moves;
    draw_tape(current_tm, halt, moves);
    moves = 0;
    return halt;
}

// function to load program
bool loadProgram(char *fileName, turing_machine &current_tm, int &bin_size)
{
    // open file
    std::ifstream codeFile;
    codeFile.open(fileName, std::ifstream::binary);

    // check to see if file opened successfully
    bool is_opened = !codeFile.fail();

    // check to see that it is the apropriate size
    codeFile.seekg(0, std::ifstream::end);
    bin_size = codeFile.tellg() * 8;
    is_opened = is_opened && ((long unsigned int)bin_size <= BIN_SIZE);
    codeFile.clear();
    codeFile.seekg(0, std::ifstream::beg);

    // load program into ram
    int i = 0;
    while (codeFile.read(reinterpret_cast<char *>(&current_tm.ram[i]), sizeof(tm_word)) && is_opened)
    {
        i++;
    }
    codeFile.close();
    return is_opened;
}

// function to get the index in args of a file with the given extension
int getFileOfType(int argc, char *argv[], std::string type)
{
    int i = 0;
    int result = -1;
    while (i < argc && result == -1)
    {
        std::string argument = argv[i];
        if (argument.length() >= type.length() && argument.substr(argument.length() - type.length()).compare(type) == 0)
        {
            result = i;
        }
        i++;
    }
    return result;
}

// checks to see if a flag provided is in argv at the moment just used to animate when provided the -a flag
bool getFlag(int argc, char *argv[], std::string flag)
{
    bool flagResult = false;
    for (int i = 0; i < argc; i++)
    {
        if (flag.compare(argv[i]) == 0)
        {
            flagResult = true;
        }
    }
    return flagResult;
}

// main sets everything up and loads a new tm and tape
int main(int argc, char *argv[])
{
    // variables used to keep track of how a program runs on the turing machine
    int moves = 0;
    int total_moves = 0;
    int total_instructions = 0;
    int bin_size = 0;

    // variables used to keep track of flags present at the moment only animation flag -a not working fully needs delay
    bool animating = false;

    // variables to store the found indexes in argv or the bin and tape files
    int binIndex = getFileOfType(argc, argv, ".bin");
    int tapeIndex = getFileOfType(argc, argv, ".tape");

    // check optional flags
    animating = getFlag(argc, argv, "-a");

    // if a sufficient amount of arguments and a bin and tape file where found
    // for now does not check for too many bin or tape files but ignores the order
    if (argc > 2 && binIndex > 0 && tapeIndex > 0)
    {
        // setup turing machines
        turing_machine current_tm;
        turing_machine base_tm;

        // alter the base_tm to have the program loaded into it
        bool program_loaded = loadProgram(argv[binIndex], base_tm, bin_size);

        // open tape file and check success of doing so
        std::ifstream tapeFile;
        tapeFile.open(argv[tapeIndex]);
        bool is_opened = !tapeFile.fail();

        // itterate through tape file and add lines onto a vector of tapes to run
        if (is_opened && program_loaded) // this can return on fail return 1
        {
            std::vector<std::string> tapes;
            std::string line;
            while (getline(tapeFile, line))
            {
                tapes.push_back(line);
            }
            tapeFile.close();

            // load and execute tapes
            for (long unsigned int i = 0; i < tapes.size(); i++)
            {
                // set current_tm to be a new turing machine with loaded program
                current_tm = base_tm;

                // compensate for empty lines instead of just having a null character ascii 0 have
                // just a blank letter previously compensated null dont know where null came from now
                // looks to see if line starts with \r carrage return a windows thing or just a \n new line
                if ((int)tapes[i].empty())
                {
                    current_tm.tape.push_back({true, '\0'});
                }
                else
                {
                    for (long unsigned int x = 0; x < tapes[i].size(); x++)
                    {
                        if (x == tapes[i].size() - 1 && (tapes[i][x] == '\n' || tapes[i][x] == '\r'))
                        {
                            current_tm.tape.push_back({true, tapes[i][x]});
                        }
                        else
                        {
                            current_tm.tape.push_back({false, tapes[i][x]});
                        }
                    }
                }

                executeProgram(current_tm, moves, total_moves, total_instructions, animating);
            }
            draw_total(bin_size, total_moves, total_instructions);
        }
        else
        {
            std::cout << "failed to load one of the files";
        }
    }
    return 0;
}