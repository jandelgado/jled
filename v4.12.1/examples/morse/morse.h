// Copyright (c) 2019 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
#include <Arduino.h>
#include <inttypes.h>
#include <stddef.h>
#include "bitset.h"  // NOLINT

#ifndef EXAMPLES_MORSE_MORSE_H_
#define EXAMPLES_MORSE_MORSE_H_

// The Morse class converts a text sequence into a bit sequence representing
// a morse code sequence.
class Morse {
    // pre-ordered tree of morse codes. Bit 1 = 'dah',  0 = 'dit'.
    // Position in string corresponds to position in binary tree starting w/ 1
    // see https://www.pocketmagic.net/morse-encoder/ for info on encoding
    static constexpr auto LATIN =
        "*ETIANMSURWDKGOHVF*L*PJBXCYZQ**54*3***2*******16*******7***8*90";

    static constexpr auto DURATION_DIT = 1;
    static constexpr auto DURATION_DAH = 3 * DURATION_DIT;
    static constexpr auto DURATION_PAUSE_CHAR = DURATION_DAH;
    static constexpr auto DURATION_PAUSE_WORD = 7 * DURATION_DIT;

 protected:
    char upper(char c) const { return c >= 'a' && c <= 'z' ? c - 32 : c; }
    bool isspace(char c) const { return c == ' '; }

    // returns position of char in morse tree. Count starts with 1, i.e.
    // E=2, T=3, etc.
    size_t treepos(char c) const {
        auto i = 1u;
        while (LATIN[i++] != c) {
        }
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

    template <typename F>
    uint16_t iterate_sequence(const char* p, F f) const {
        // call f(count,val) num times, incrementing count each time
        // and returning num afterwards.
        auto set = [](int num, int count, bool val, F f) -> int {
            for (auto i = 0; i < num; i++) f(count + i, val);
            return num;
        };

        auto bitcount = 0;
        while (*p) {
            const auto c = upper(*p++);
            if (isspace(c)) {  // space not part of alphabet, treat separately
                bitcount += set(DURATION_PAUSE_WORD, bitcount, false, f);
                continue;
            }

            const auto morse_code = pos_to_morse_code(treepos(upper(c)));
            auto code = morse_code & 0xff;  // dits (0) and dahs (1)
            auto size = morse_code >> 8;    // number of dits and dahs in code
            while (size--) {
                bitcount += set((code & 1) ? DURATION_DAH : DURATION_DIT,
                                bitcount, true, f);

                // pause between symbols := 1 dit
                if (size) {
                    bitcount += set(DURATION_DIT, bitcount, false, f);
                }
                code >>= 1;
            }

            if (*p && !isspace(*p)) {
                bitcount += set(DURATION_PAUSE_CHAR, bitcount, false, f);
            }
        }
        return bitcount;
    }

 public:
    // returns ith bit of morse sequence
    bool test(uint16_t i) const { return bits_->test(i); }

    // length of complete morse sequence in in bits
    size_t size() const { return bits_->size(); }

    Morse() : bits_(new Bitset(0)) {}

    explicit Morse(const char* s) {
        const auto length = iterate_sequence(s, [](int, bool) -> void {});
        auto bits = new Bitset(length);
        iterate_sequence(s, [bits](int i, bool v) -> void { bits->set(i, v); });
        bits_ = bits;
    }

    ~Morse() { delete bits_; }

    // make sure that the following, currently not needed, methods are not used
    Morse(const Morse&m) {*this = m;}
    Morse& operator=(const Morse&m) {
        delete bits_;
        bits_ = new Bitset(*m.bits_);
        return *this;
    }

 private:
    // stores morse bit sequence
    const Bitset* bits_ = nullptr;
};

#endif  // EXAMPLES_MORSE_MORSE_H_
