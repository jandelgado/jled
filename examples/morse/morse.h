#include <Arduino.h>
#include <inttypes.h>
#include <stddef.h>
#include "bitset.h" // NOLINT

#ifndef EXAMPLES_MORSE_MORSE_H_
#define EXAMPLES_MORSE_MORSE_H_

class Morse {
    // pre-ordered tree of morse codes. Bit 1 = 'dah',  0 = 'dit'.
    // see https://www.pocketmagic.net/morse-encoder/ for info on encoding
    static constexpr auto LATIN =
        "*ETIANMSURWDKGOHVF*L*PJBXCYZQ**54*3***2*******16*******7***8*90";

    static constexpr auto DURATION_DIT = 1;
    static constexpr auto DURATION_DAH = 3 * DURATION_DIT;
    static constexpr auto DURATION_PAUSE_CHAR = DURATION_DAH;
    static constexpr auto DURATION_PAUSE_WORD = 7 * DURATION_DIT;

    // stores morse bit sequence
    Bitset* bits_ = nullptr;

 protected:
    char upper(char c) const { return c >= 'a' && c <= 'z' ? c - 32 : c; }
    bool isspace(char c) const { return c == ' '; }

    size_t treepos(char c) const {
        auto i = 0u;
        while (LATIN[i++] != c) {}
        return i;
    }

    // returns uint16_t with size of morse sequence (dit's and dah's) in MSB
    // and the morse sequence in the LSB
    uint16_t pos_to_morse_code(int code) const {
        uint8_t res = 0;
        uint8_t size = 0;
        while (code > 1) {
            size++;
            res <<= 1;
            res |= (code & 1);
            code >>= 1;
        }
        return res | (size << 8);
    }

    // calculate time in 'dits' for the given code. One 'dah' is three 'dits',
    // the pause between two symbols is one 'dit' long. E.g.
    //   duration( '.' ) = 1
    //   duration( '-' ) = 3
    //   duration( '-.-' ) = 3+1+1+1+3 = 9
    uint16_t duration_code(uint16_t code) const {
        uint8_t len = code >> 8;
        code &= 0xff;
        auto duration = 0;
        duration += len - 1;  // pause of len 'dit' between symbols
        while (len--) {
            duration += (code & 1) ? DURATION_DAH : DURATION_DIT;
            code >>= 1;
        }
        return duration;
    }


    // calculate duration in 'dits' of a string converted in morse code.
    // Duration between symbols in characteris is a 'dit', between characters
    // of a word is a 'dah' (== 3 'dits'), between words it is a 7 'dits'.
    uint16_t duration_str(const char* p) const {
        auto duration = 0;
        while (*p) {
            const auto c = upper(*p++);
            if (isspace(c)) {
                duration += DURATION_PAUSE_WORD;
                continue;
            }
            duration += duration_code(pos_to_morse_code(treepos(upper(c))));
            if (*p && !isspace(*p)) duration += DURATION_PAUSE_CHAR;
        }
        return duration;
    }

 public:
    // returns ith bit of morse sequence 
    bool test(uint16_t i) const {return bits_->test(i);}

    // length of complete morse sequence in in bits
    size_t size() const {return bits_->size();}

    explicit Morse(const char* s) {
        bits_ = new Bitset(duration_str(s));
        auto p = s;
        auto bitcount = 0;
        while (*p) {
            const auto c = upper(*p++);
            if (isspace(c)) {  // space not part of alphabet, treat separately
                bitcount += DURATION_PAUSE_WORD;
                continue;
            }
            const auto morse_code = pos_to_morse_code(treepos(upper(c)));
            auto code = morse_code & 0xff;  // dits (0) and dahs (1)
            auto size = morse_code >> 8;    // number of dits and dahs in code

            while (size--) {
                for (auto i = 0; i < ((code & 1) ? DURATION_DAH : DURATION_DIT);
                     i++) {
                    bits_->set(bitcount++, true);
                }

                if (size)
                    bitcount += DURATION_DIT;  // pause between symbols := 1 dit
                code >>= 1;
            }

            if (*p && !isspace(*p)) bitcount += DURATION_PAUSE_CHAR;
        }
    }
    ~Morse() { delete bits_; }

    // make sure that the following, currently not needed, methods are not used
    Morse(const Morse&) = delete;
    Morse(Morse&&) = delete;
    Morse& operator=(const Morse&) = delete;
    Morse& operator=(Morse&&) = delete;
};

#endif  // EXAMPLES_MORSE_MORSE_H_
