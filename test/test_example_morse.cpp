// JLed Unit tests  (run on host).
// Tests for the morse code example.
// Copyright 2018 Jan Delgado jdelgado@gmx.net
#include "catch.hpp"

#include <morse.h>  // NOLINT

TEST_CASE("calc len of bit array", "[morse_example_bitset]") {
    class TestBitset : public Bitset {
     public:
        static void test() {
            REQUIRE(0 == num_bytes(0));
            REQUIRE(1 == num_bytes(1));
            REQUIRE(1 == num_bytes(7));
            REQUIRE(1 == num_bytes(8));
            REQUIRE(2 == num_bytes(9));
            REQUIRE(2 == num_bytes(16));
            REQUIRE(3 == num_bytes(17));
        }
    };
    TestBitset::test();
}

TEST_CASE("set and test bits in the bitset", "[morse_example_bitset]") {
    Bitset bf(18);  // 3 bytes
    REQUIRE(18 == bf.size());
    REQUIRE(!bf.test(0));
    REQUIRE(!bf.test(10));
    REQUIRE(!bf.test(17));
    bf.set(0, true);
    bf.set(10, true);
    bf.set(17, true);
    REQUIRE(bf.test(0));
    REQUIRE(bf.test(10));
    REQUIRE(bf.test(17));
}

TEST_CASE("treepos returns correct position in tree", "[morse_example]") {
    class TestMorse : public Morse {
     public:
        void test() {
            REQUIRE(2 == treepos('E'));
            REQUIRE(3 == treepos('T'));
            REQUIRE(4 == treepos('I'));
            REQUIRE(7 == treepos('M'));
            REQUIRE(8 == treepos('S'));
            REQUIRE(16 == treepos('H'));
            REQUIRE(32 == treepos('5'));
        }
    };
    TestMorse().test();
}

TEST_CASE("binary code of character is determined correctly",
          "[morse_example]") {
    class TestMorse : public Morse {
     public:
        void test() {
            uint16_t code = pos_to_morse_code(treepos('F'));  // F = ..-. = 1000
            REQUIRE(4 == (code >> 8));  // hi: length in bits
            REQUIRE(0b0100 ==
                    (code & 0xff));  // lo: morse code as bits, reversed
        }
    };
    TestMorse().test();
}

TEST_CASE("string is encoded correctly into sequence", "[morse_example]") {
    // A = . -   => 1 + 3 + (1) dits
    // E = . => 1 dit
    // B = - . . . => 3 + 1 + 1 + 1 + (3*1) dits
    Morse m("AE B");
    // clang-format off
    constexpr int expected[]{
            1, 0, 1, 1, 1,               // A
            0, 0, 0,                     // pause between chars
            1,                           // E
            0, 0, 0, 0, 0, 0, 0,         // 7 dits between words
            1, 1, 1, 0, 1, 0, 1, 0, 1};  // B
    // clang-format on
    REQUIRE(sizeof(expected) / sizeof(expected[0]) == m.size());
    for (auto i = 0u; i < m.size(); i++) {
        REQUIRE(expected[i] == m.test(i));
    }
}
