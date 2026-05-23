//
// Created by anonymized on 12/7/21.
//

#ifndef ZKUNSAT_NEW_UTILS_H
#define ZKUNSAT_NEW_UTILS_H

//
// Created by anonymized on 8/9/21.
//
#pragma  once

#include "commons.h"

using namespace  std;
using  namespace  NTL;
using namespace emp;


#define VAL_SZ 61

#define RAM_VAL_SZ 4

/* Encode literal: the highest bit indicates the sign of that literal;
 * the rest bits indicate the value of the index;
 * for example: x2 is encoded as [1,0, ..., 0, 1, 0] , and not x2 is encoded as [0,0, ..., 0, 1, 0];
 * [0,0, ..., 0, 0, 0] and [1,0, ..., 0, 0, 0] are dummy bits;
 * used by prover locally;
 */
const block one_block = makeBlock(0, 1);

inline uint64_t wrap(int64_t input){
    uint64_t  res = 0UL;
    if(input >  0) {
        res = (uint64_t) input | 1UL<<(VAL_SZ-2);
        res = res | 1UL<<(VAL_SZ-1);
    }
    if(input <  0) {
        res = (uint64_t)(-input);
        res = res | 1UL<<(VAL_SZ-1);
    }
    return  res;
}


inline __uint128_t get_128uint_from_uint64(uint64_t value) {
    return (((__uint128_t)value)) ;
}


inline block encode(int64_t input){
    uint64_t  res = 0UL;
    if(input >  0) {
        res = (uint64_t) input | 1UL<<(VAL_SZ-2);
        res = res | 1UL<<(VAL_SZ-1);
    }
    if(input <  0) {
        res = (uint64_t)(-input);
        res = res | 1UL<<(VAL_SZ-1);
    }
    return  (block) get_128uint_from_uint64(res) ;
}

// Debug helper: open one authenticated wire and check its MAC. X/MAC are the
// caller's local (value, mac) shares; the verifier folds Δ to verify.
inline void check_MAC_valid(block X, block MAC) {
    int party = bb()->party;
    if(party == ALICE) {
        io->send_data(&X, sizeof(block));
        io->send_data(&MAC, sizeof(block));
        int64_t res = -1;
        io-> recv_data(&res, 8);
        if (res != 1) error("check_MAC failed!\n");
    } else {
        block M = zero_block, x = zero_block;
        io->recv_data(&x, sizeof(block));
        io->recv_data(&M, sizeof(block));
        gfmul(x, bb()->delta, &x);
        x = x ^ MAC;
        if (memcmp(&x, &M, sizeof(block))!=0) {
            int64_t  res = 0;
            io->send_data(&res, 8);
            error("check_MAC failed!\n");
        }else {
            int64_t  res = 1;
            io->send_data(&res, 8);
        }

    }

}

/*
 * check if a block is a mac of zero.
 * It is an accumulator that takes all mac blocks to be checked.
 * At the end of the proof, prover by setting end = 1 will hash all blocks, and send it to verifier.
 *
 * Each authenticated wire satisfies mac_A == key_B ^ val·Δ, so for a wire
 * committed to zero the prover's mac equals the verifier's key. Both sides
 * feed their own .mac share here; if every wire is really zero the two
 * accumulated digests match.
*/

inline void check_zero_MAC(block MAC, int end = 0) {
    static Hash hash;
    hash.put(&MAC, sizeof(block));
    if (end == 1){
        int party = bb()->party;
        char dig[Hash::DIGEST_SIZE];
        hash.digest(dig);

        if(party == ALICE) {
            io->send_data(dig, sizeof(dig));
        } else {
            char receive[Hash::DIGEST_SIZE];
            io->recv_data(receive, sizeof(receive));
            if (memcmp(receive, dig, sizeof(dig))!=0) {
                error("check_zero failed!\n");
            }else {
                cout << "check_zero succeed!!!" << endl;
            }
        }
        return;
    }

}




/*
 * padding a vector of int64 to the size of degree
 * used when prover input a clause
 */
inline void padding(vector<uint64_t>& input){
    for (int i = input.size() ; i < DEGREE; i ++){
        input.push_back(0UL);
    }
}

/*
 * conversion from block to Galois field element of size 128
 * */

inline void block2GF(GF2E& res, const block a ){
    GF2X raw;
    auto low = _mm_extract_epi64(a, 0);
    auto high = _mm_extract_epi64(a, 1);

    raw.SetMaxLength(128);
    for(int i = 0; i < 64; i++){
        SetCoeff(raw, long(i), (1 &(low >> (i))));
        SetCoeff(raw, long(i+64), (1 &(high >> (i))));
    }
    res._GF2E__rep = raw;
}


/*
 * get inverse of an block in the field. In particular, inv* input = 1 in F_{2^k}
 * */

inline void inverse(block& inv, block& input){
    GF2E _input, _inv;
    block2GF(_input, input );
    NTL::inv(_inv, _input);
    GF2X raw = _inv._GF2E__rep;
    inv = zero_block;
    for (int i = 0; i < 128; i++) {
        if (IsOne(NTL::coeff(raw, i)))
            inv = set_bit(inv, i);
    }
}

inline GF2EX get_GF2EX_with_roots(vector<uint64_t>& roots){
    GF2EX res, tmp;
    SetCoeff(res, 0); // res = 1
    for (auto r : roots){
        tmp = GF2EX();
        GF2E coefficient, constant;
        if (r == 0){
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
    return res;
}

typedef  vector<int64_t> CLS;
typedef vector<int64_t> SPT;
typedef vector<int64_t> PVT;

inline void readproof(string filename, int& d, vector<CLS>& clauses, vector<SPT>& supports, vector<PVT>& pivots, int& ncls, int& nres){
    std::ifstream file(filename);
    std::string str;
    ncls = 0;
    nres = 0;
    d = 0;

    while (std::getline(file, str)) {
        istringstream ss(str);
        string word;
        while (ss >> word) {
            if (word == "clause:") {
                int nltr  = 0;
                CLS clause;
                ss >> word;
                while (word != "support:") {
                    int i = stoi(word);
                    clause.push_back(i);
                    nltr = nltr + 1;
                    ss >> word;
                }
                if (d < nltr) d = nltr;
                clauses.push_back(clause);
                ncls ++ ;
            }
            if (word == "support:") {
                SPT support;
                ss >> word;
                while (word != "pivot:") {
                    int i = stoi(word);
                    support.push_back(i);
                    ss >> word;
                }
                supports.push_back(support);
            }
            if (word == "pivot:") {
                SPT pchain;
                ss >> word;
                while (word != "end:") {
                    int i = stoi(word);
                    pchain.push_back(wrap(i));
                    ss >> word;
//                    nres = nres + 1;
                }
                if (pchain.size() > 0){
                    nres = nres + 1;
                }
                pivots.push_back(pchain);

            }
            if (word == "DEGREE:"){
                ss >> word;
                d = stoi(word);
            }
        }
    }
}

#endif //ZKUNSAT_NEW_UTILS_H
