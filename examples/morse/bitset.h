// Copyright (c) 2019 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled

#ifndef EXAMPLES_MORSE_BITSET_H_
#define EXAMPLES_MORSE_BITSET_H_

// a simple bit set with capacity of N bits, just enough for the morse demo
class Bitset {
 private:
    size_t n_;
    uint8_t* bits_;

 public:
    Bitset() = delete;
    Bitset(const Bitset& b) { *this = b; }
    explicit Bitset(size_t N)
        : n_(N), bits_{N > 0 ? new uint8_t[((N - 1) >> 3) + 1] : nullptr} {
        if (n_) memset(bits_, 0, ((n_ - 1) >> 3) + 1);
    }
    Bitset& operator=(const Bitset& b) {
        n_ = b.n_;
        auto size = n_ > 0 ? ((n_ - 1) >> 3) + 1 : 0;
        if (size > 0) {
            bits_ = new uint8_t[size];
            memcpy(bits_, b.bits_, size);
        } else {
            bits_ = nullptr;
        }
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
