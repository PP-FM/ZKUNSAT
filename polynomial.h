//
// Created by anonymized on 12/7/21.
//
#pragma once
#ifndef ZKUNSAT_NEW_POLYNOMIAL_H
#define ZKUNSAT_NEW_POLYNOMIAL_H
#include "utils.h"
#include "commons.h"

// A clause polynomial committed over authenticated F(2^128) values. Each
// coefficient is an f2k wire (val, mac): on the prover val is the cleartext
// coefficient, on the verifier val is zero. The wires are committed through
// the unified bool backend's f2k_input. Evaluations at a public challenge r
// are local (multiply by a public constant + add), so only the product/inner-
// product equality checks touch the network (one f2k_mul each).
class polynomial {
public:
    vector<F2kAuthValue> coeff;
    polynomial(){
    }
    polynomial(vector<block> coefficient);
    polynomial(vector<uint64_t> roots);
    // Horner evaluation at a public point; result is an f2k wire.
    F2kAuthValue Evaluate(block input) const;
    void Equal(const polynomial& lfh) const;
    void InnerProductEqual(vector<polynomial>& p1, vector<polynomial>& p2);
    void ProductEqual(polynomial& p1, polynomial& p2);
    void ConverseCheck(polynomial& lhs);
};

// Convert an NTL GF2EX polynomial to its DEGREE cleartext F(2^128) coefficient
// blocks (prover-side; the verifier passes a zero polynomial of the same size).
inline void GF2EX2polynomial(GF2EX& a, vector<block>& coeff){
    long d = deg(a);
    assert(!(d > DEGREE));
    coeff.clear();
    for (long i = 0; i < DEGREE; i ++){
        GF2E c = NTL::coeff(a, i);
        GF2X raw = c._GF2E__rep;
        block tmp = zero_block;
        for(int j = 0; j < 128; j++){
            if (IsOne(NTL::coeff(raw, j)))
                tmp = set_bit(tmp, j);
        }
        coeff.push_back(tmp);
    }
}

inline void GF2EX2polynomial(GF2EX& a, polynomial& b){
    vector<block> coeff;
    GF2EX2polynomial(a, coeff);
    b = polynomial(coeff);
}



#endif //ZKUNSAT_NEW_POLYNOMIAL_H
