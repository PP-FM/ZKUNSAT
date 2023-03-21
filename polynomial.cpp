//
// Created by anonymized on 12/7/21.
//

#include "polynomial.h"

polynomial::polynomial(vector<block> _coefficient) {
    vector<block> mcoefficient;
    vector<block> coefficient;
    for (int i = 0; i < _coefficient.size(); i ++){
        block d, m;
        fill_data_and_mac(d, m);
        if (ostriple->party == ALICE) {
            block diff_coefficient;
            diff_coefficient= d^_coefficient[i];
            ostriple->io ->send_data(&diff_coefficient, sizeof(block));
            coefficient.push_back(_coefficient[i]);
            mcoefficient.push_back(m);
        }
        if (ostriple->party == BOB){
            block diff_coefficient;
            ostriple->io ->recv_data(&diff_coefficient, sizeof(block));
            gfmul(ostriple->delta, diff_coefficient, &diff_coefficient);
            coefficient.push_back(d);
            mcoefficient.push_back(m ^diff_coefficient);
        }
    }
    this->coefficient = coefficient;
    this->mcoefficient = mcoefficient;
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
        for(int i = 0; i < 128; i++){
            if (IsOne(NTL::coeff(raw, i)))
                tmp = set_bit(tmp, i);
        }
        _coefficient.push_back(tmp);
    }
    vector<block> mcoefficient;
    vector<block> coefficient;
    for (int i = 0; i < DEGREE; i ++){
        block d, m;
        fill_data_and_mac(d, m);

        if (ostriple->party == ALICE) {
            block diff_coefficient;
            diff_coefficient= d^_coefficient[i];
            ostriple->io ->send_data(&diff_coefficient, sizeof(block));
            coefficient.push_back(_coefficient[i]);
            mcoefficient.push_back(m);
        }
        if (ostriple->party == BOB){
            block diff_coefficient;
            ostriple->io ->recv_data(&diff_coefficient, sizeof(block));
            gfmul(ostriple->delta, diff_coefficient, &diff_coefficient);
            coefficient.push_back(d);
            mcoefficient.push_back(m ^diff_coefficient);
        }
    }
    this->coefficient = coefficient;
    this->mcoefficient = mcoefficient;
}

void polynomial::Evaluate(block &res, block &mres, block &input) const {
    // cout << (input) << endl; 
    int degree = this->coefficient.size(); 
    assert(degree > 2);
    multiply_const(res, mres, this->coefficient[degree-1], this->mcoefficient[degree-1], input, ostriple->party);
    // cout << "degree " << degree -1  << ": " << (res) << endl; 
    compute_xor(res, mres, res, mres, this->coefficient[degree-2], this->mcoefficient[degree-2]);
    // cout << "degree " << degree - 2 << ": " << (res) << endl; 
    for (int i = degree-3; i > -1; i--){
        multiply_const(res, mres, res, mres, input, ostriple->party);
        // cout << "degree  mul" << i << ": " << (res) << endl; 
        compute_xor(res, mres, res, mres, this->coefficient[i] ,this->mcoefficient[i]);
        // cout << "degree  xor" << i << ": " << (res) << endl; 
    }
}

void polynomial::Equal(const polynomial& lhs) const{
    io->flush();
    block r = io->get_hash_block();
    // cout << r << endl;
    block res[2], mac[2];
    this->Evaluate(res[0], mac[0], r);
    // cout << "?????" << endl;
    lhs.Evaluate(res[1], mac[1], r);
    check_zero_MAC(mac[0]^mac[1]);
    // cout << (mac[0]^mac[1] ) << endl; 
}

void polynomial::InnerProductEqual(vector<polynomial> &p1, vector<polynomial> &p2) {
    io->flush();
    block r =io->get_hash_block();
    int d = p1.size();
    assert(p1.size() == p2.size());
    block res = zero_block;
    block mres = zero_block;
    for (int  i = 0; i < d; i ++){
        block tmp, mt, xx, xm, yy, ym;
        p1[i].Evaluate(xx, xm, r);
        p2[i].Evaluate(yy, ym, r);
        ostriple->compute_mul(tmp, mt, xx, xm, yy, ym);
        res = res ^ tmp;
        mres = mres ^ mt;
    }
    block resp, mresp;
    this->Evaluate(resp, mresp, r);
    check_zero_MAC(mresp^mres);
   
}

void polynomial::ProductEqual(polynomial& p1, polynomial &p2) {
    io->flush();
    block r =io->get_hash_block();
    block res = zero_block;
    block mres = zero_block;
    block xx, xm, yy, ym;
    p1.Evaluate(xx, xm, r);
    p2.Evaluate(yy, ym, r);
    ostriple->compute_mul(res, mres, xx, xm, yy, ym);
    block resp, mresp;
    this->Evaluate(resp, mresp, r);

    check_zero_MAC(mresp^mres );
    cout << " product block: " << (mresp^mres)  << endl; 
}

void polynomial::ConverseCheck(polynomial & lhs) {
    // for (int i = 0; i < this-> coefficient.size(); i ++) cout << i << ":" << (this-> coefficient[i]) << ", mac: " << (this-> mcoefficient[i]) << endl; 
    // cout << "========\n";
    //for (int i = 0; i < lhs.coefficient.size(); i ++) cout << (lhs.coefficient[i]) << ", mac: " << (lhs.mcoefficient[i]) << endl; 


    io->flush();
    block r =io->get_hash_block();
     cout << "====last coefficient=======" << endl;
    block converse_r = ((block) get_128uint_from_uint64(constant))^r;
    cout << (lhs.coefficient[0] ^ this -> coefficient[0]) << endl; 
    cout << (converse_r) << endl; 
    cout << (r) << endl; 
    block xx, xm, yy, ym;
    cout << "====rhs =======" << endl;
    this->Evaluate(xx, xm, r);
    cout << "====lhs =======" << endl;
    lhs.Evaluate(yy, ym, converse_r);
    check_zero_MAC(xm^ym);
    cout << "converse block: " << (xx ^ yy)  << endl; 
}