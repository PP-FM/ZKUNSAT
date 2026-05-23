//
// Created by anonymized on 12/7/21.
//

#include "polynomial.h"

// Commit a list of cleartext coefficients as authenticated f2k wires. On the
// verifier the coefficient values are ignored (f2k_input zeroes the val lane);
// both parties must commit the same number in the same order so the underlying
// VOLE draws and masking-difference exchange stay in lockstep.
polynomial::polynomial(vector<block> _coefficient) {
    for (auto &c : _coefficient)
        coeff.push_back(bb()->f2k_input(c));
}

polynomial::polynomial(vector <uint64_t> roots) {
    GF2EX res, tmp;
    SetCoeff(res, 0); // res = 1
    for (auto r : roots){
        tmp = GF2EX();
        GF2E coefficient, constant;
        if (r  == 0){
            block2GF(coefficient, zero_block);
            block2GF(constant, one_block);
            SetCoeff(tmp, 0, constant);
            SetCoeff(tmp, 1, coefficient);
        }else{
            block2GF(constant, (block)get_128uint_from_uint64(r));
            block2GF(coefficient, one_block);
            SetCoeff(tmp, 0, constant);
            SetCoeff(tmp, 1, coefficient);
        }
        res = tmp * res;
    }

    std::vector<block> _coefficient;
    for (long i = 0; i < DEGREE; i ++){
        GF2E c = NTL::coeff(res, i);
        GF2X raw = c._GF2E__rep;
        block tmp = zero_block;
        for(int j = 0; j < 128; j++){
            if (IsOne(NTL::coeff(raw, j)))
                tmp = set_bit(tmp, j);
        }
        _coefficient.push_back(tmp);
    }
    for (auto &c : _coefficient)
        coeff.push_back(bb()->f2k_input(c));
}

// Horner: res = ((c[d-1]·x + c[d-2])·x + … )·x + c[0]. Multiply-by-public-x and
// add-coefficient are both local f2k ops, so an evaluation is communication-free.
F2kAuthValue polynomial::Evaluate(block input) const {
    int degree = (int)this->coeff.size();
    assert(degree > 2);
    F2kAuthValue res = bb()->f2k_mul_const(coeff[degree-1], input);
    res = bb()->f2k_add(res, coeff[degree-2]);
    for (int i = degree-3; i > -1; i--){
        res = bb()->f2k_mul_const(res, input);
        res = bb()->f2k_add(res, coeff[i]);
    }
    return res;
}

void polynomial::Equal(const polynomial& lhs) const{
    io->flush();
    block r = io->io->get_digest();
    F2kAuthValue a = this->Evaluate(r);
    F2kAuthValue b = lhs.Evaluate(r);
    check_zero_MAC(a.mac ^ b.mac);
}

void polynomial::InnerProductEqual(vector<polynomial> &p1, vector<polynomial> &p2) {
    io->flush();
    block r = io->io->get_digest();
    int d = p1.size();
    assert(p1.size() == p2.size());
    F2kAuthValue acc{zero_block, zero_block};
    for (int  i = 0; i < d; i ++){
        F2kAuthValue x = p1[i].Evaluate(r);
        F2kAuthValue y = p2[i].Evaluate(r);
        F2kAuthValue prod;
        bb()->f2k_mul(prod, x, y);
        acc = bb()->f2k_add(acc, prod);
    }
    F2kAuthValue self = this->Evaluate(r);
    check_zero_MAC(self.mac ^ acc.mac);
}

void polynomial::ProductEqual(polynomial& p1, polynomial &p2) {
    io->flush();
    block r = io->io->get_digest();
    F2kAuthValue x = p1.Evaluate(r);
    F2kAuthValue y = p2.Evaluate(r);
    F2kAuthValue prod;
    bb()->f2k_mul(prod, x, y);
    F2kAuthValue self = this->Evaluate(r);
    check_zero_MAC(self.mac ^ prod.mac);
}

void polynomial::ConverseCheck(polynomial & lhs) {
    io->flush();
    block r = io->io->get_digest();
    block converse_r = ((block) get_128uint_from_uint64(constant)) ^ r;
    F2kAuthValue x = this->Evaluate(r);
    F2kAuthValue y = lhs.Evaluate(converse_r);
    check_zero_MAC(x.mac ^ y.mac);
}
