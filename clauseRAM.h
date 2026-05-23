//
// Created by anonymized on 12/7/21.
//

#ifndef ZKUNSAT_NEW_CLAUSERAM_H
#define ZKUNSAT_NEW_CLAUSERAM_H

//
// Created by anonymized on 10/4/21.
//

#include "emp-zk/emp-zk.h"
#include "clause.h"
#include "commons.h"
#define INDEX_SZ 20
typedef vector<uint64_t> clause_raw;

// A read-only memory of clauses backed by emp-zk's native ZKROM (Yang–Heath
// "Two Shuffles Make a RAM", read-only path: a single permutation / version
// chain — no freshness set). This replaces the old hand-rolled, sort-based
// set-equality RAM.
//
// ZKROM stores one uint64_t value per cell, so a clause (DEGREE literals of
// VAL_SZ bits each) can't be one cell. Instead we keep DEGREE parallel ROMs:
// rom[j] holds literal j of every clause. A get(index) reads one literal from
// each ROM (binding the whole clause to its stored contents by position) and
// rebuilds the clause polynomial from the prover's revealed literals — exactly
// as the original RAM rebuilt a clause from its cleartext store on each read.
class clauseRAM {
public:
    int party;
    int index_sz;
    int64_t max_reads;          // bound on get() calls (sizes ROM timestamps)
    int ncls = 0;
    vector<ZKROM*> roms;        // one per literal position (size DEGREE)

    clauseRAM(int _party, int index_sz, int64_t max_reads)
        : party(_party), index_sz(index_sz), max_reads(max_reads) {}

    ~clauseRAM() {
        for (auto *rom : roms) delete rom;
    }

    void init(vector<clause> &data) {
        ncls = (int)data.size();
        for (int j = 0; j < DEGREE; ++j) {
            ZKROM *rom = new ZKROM(party, index_sz, VAL_SZ, max_reads);
            vector<Integer> content;
            content.reserve(ncls);
            for (auto &c : data) {
                uint64_t lit = (party == ALICE && j < (int)c.literals.size())
                                   ? c.literals[j]
                                   : 0;
                content.push_back(Integer(VAL_SZ, lit, ALICE));
            }
            rom->init(content);
            roms.push_back(rom);
        }
    }

    // Read the clause at `index`: one literal per ROM, then rebuild the clause
    // (which commits its polynomial). The verifier still issues every read and
    // reveal in lockstep, learning nothing.
    clause get(const Integer &index) {
        vector<uint64_t> lits(DEGREE, 0);
        for (int j = 0; j < DEGREE; ++j) {
            Integer v = roms[j]->read(index);
            uint64_t got = v.reveal<uint64_t>(ALICE);
            if (party == ALICE) lits[j] = got;
        }
        return clause(lits);
    }

    // Finalize the RAM: run each ROM's permutation check. A consistency
    // violation aborts inside ZKROM::check().
    void check() {
        for (auto *rom : roms) rom->check();
    }
};

inline pair<double, double> check_chain(vector<Integer>& indice, vector<uint64_t> pivots, int ptr, clauseRAM* formula, bool last_clause){
    double cost_resolve = 0;
    double cost_access = 0;
    auto timer_0 = chrono::high_resolution_clock::now();
    vector<clause> intermediate;
    vector<clause> resource;
    for (Integer index : indice){
        if (index.geq(Integer(INDEX_SZ, ptr, PUBLIC)).reveal())  error("cheat!");
        resource.push_back(formula->get(index));
    }

    auto timer_1 = chrono::high_resolution_clock::now();

    cost_access = chrono::duration<double>(timer_1 - timer_0).count();


    auto timer_2 = chrono::high_resolution_clock::now();

    intermediate.push_back(resource[0]);


    for (int i = 1; i < pivots.size(); i++){

        clause a = intermediate[i-1];

        clause b = resource[i];

        clause tmp = get_res_f2k(a, b, pivots[i]);

        intermediate.push_back(tmp);

    }
    for (int i = 1; i < pivots.size(); i++){

        clause c0 = intermediate[i-1];

        clause c1 = resource[i];

        check_xres(c0, c1, intermediate[i], pivots[i]);

    }
    clause end_clause = formula -> get(Integer(INDEX_SZ, ptr, PUBLIC));
    end_clause.poly.Equal(intermediate[intermediate.size()-1].poly);
    if (last_clause) {
        vector <uint64_t> empty_literals;
        clause empty_clause(empty_literals);
        end_clause.poly.Equal(empty_clause.poly);
    }
    auto timer_3 = chrono::high_resolution_clock::now();
    cost_resolve = chrono::duration<double>(timer_3 - timer_2).count();
    return pair<double, double>{cost_access, cost_resolve};

}

#endif //ZKUNSAT_NEW_CLAUSERAM_H
