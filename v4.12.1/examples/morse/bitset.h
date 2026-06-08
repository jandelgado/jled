// Copyright (c) 2019 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled

#ifndef EXAMPLES_MORSE_BITSET_H_
#define EXAMPLES_MORSE_BITSET_H_

// a simple bit set with capacity of N bits, just enough for the morse demo
class Bitset {
 private:
    size_t n_;
    uint8_t* bits_;

 protected:
    // returns num bytes needed to store n bits.
    static constexpr size_t num_bytes(size_t n) {
        return n > 0 ? ((n - 1) >> 3) + 1 : 0;
    }

 public:
    Bitset() : Bitset(0) {}

    Bitset(const Bitset& b) : Bitset() { *this = b; }

    explicit Bitset(size_t n) : n_(n), bits_{new uint8_t[num_bytes(n)]} {
        memset(bits_, 0, num_bytes(n_));
    }

    Bitset& operator=(const Bitset& b) {
        if (&b == this) return *this;
        const auto size_new = num_bytes(b.n_);
        if (num_bytes(n_) != size_new) {
            delete[] bits_;
            bits_ = new uint8_t[size_new];
            n_ = b.n_;
        }
        memcpy(bits_, b.bits_, size_new);
        return *this;
    }

    virtual ~Bitset() {
        delete[] bits_;
        bits_ = nullptr;
    }
    void set(size_t i, bool val) {
        if (val)
            bits_[i >> 3] |= (1 << (i & 7));
        else
            bits_[i >> 3] &= ~(1 << (i & 7));
    }
    bool test(size_t i) const { return (bits_[i >> 3] & (1 << (i & 7))) != 0; }
    size_t size() const { return n_; }
};
#endif  // EXAMPLES_MORSE_BITSET_H_
