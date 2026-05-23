#include <iostream>
#include "clause.h"
#include "clauseRAM.h"
#include "commons.h"

int port, party;
int DEGREE = 4;
uint64_t constant;
BoolIO *io;

int main(int argc, char **argv) {
    cout << "---- begin ----" << endl;
    constant = 1UL << (VAL_SZ - 2);
    parse_party_and_port(argv, &party, &port);

    // Single-threaded in the new emp-zk: one NetIO wrapped in one BoolIO,
    // which carries both the gate-level bit stream and Ferret's OT data and
    // owns the Fiat-Shamir transcript on its inner channel.
    NetIO *netio = new NetIO(party == ALICE ? nullptr : argv[3], port);
    io = new BoolIO(netio, party == ALICE);
    char *prooffile = argv[4];

    setup_zk_bool(io, party);
    cout << "----set up----" << endl;

    GF2X P;
    SetCoeff(P, 128, 1);
    SetCoeff(P, 7, 1);
    SetCoeff(P, 2, 1);
    SetCoeff(P, 1, 1);
    SetCoeff(P, 0, 1);
    GF2E::init(P);


    int ncls = 0, nres = 0;

    vector <CLS> clauses;
    vector <SPT> supports;
    vector <SPT> pivots;


    if (party == ALICE) {
        readproof(string(prooffile), DEGREE, clauses, supports, pivots, ncls, nres);
        cout << string(prooffile) << endl;
        cout << "----input proof----" << endl;
        io->send_data(&nres, 4);
        io->send_data(&ncls, 4);
        io->send_data(&DEGREE, 4);

    }

    if (party == BOB) {
        io->recv_data(&nres, 4);
        io->recv_data(&ncls, 4);
        io->recv_data(&DEGREE, 4);

        clauses = vector<CLS>(ncls);
        supports = vector<SPT>(ncls);
        pivots = vector < vector < int64_t >> (ncls);
    }

    cout << "nres " << nres << endl;
    cout << "ncls " << ncls << endl;
    cout << "DEGREE " << DEGREE << endl;

    double cost_input = 0;
    double cost_resolve = 0;
    double cost_access = 0 ;

/*
 * encode the formula and resolution proof
 * */
    auto timer_0 = chrono::high_resolution_clock::now();
    vector<clause> raw_formula;


    float delta = 0 ;


    for (int i = 0; i < ncls; i++) {
        delta = delta + 1;
        if ((delta / ncls) > 0.1){
            float  progress = (float(i) / ncls);
            delta = 0;
            int barWidth = 70;
            std::cout << "[";
            int pos = barWidth * progress;
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << int(progress * 100.0) << " %\r";
            cout << endl;
        }


        vector <uint64_t> literals;
        for (int64_t lit: clauses[i]) {
            literals.push_back((wrap(lit)));
        }
        padding(literals);
        clause c(literals);
        raw_formula.push_back(c);
    }

    // Bound on the number of formula->get() lookups, used to size the ROM
    // timestamps. Each resolution reads its support chain plus the resolved
    // clause. ALICE knows the chain lengths and shares the total.
    int64_t max_reads = 0;
    if (party == ALICE) {
        for (int64_t i = ncls - nres; i < ncls; i++)
            max_reads += (int64_t)supports[i].size() + 1;
        io->send_data(&max_reads, 8);
    } else {
        io->recv_data(&max_reads, 8);
    }

    clauseRAM* formula = new clauseRAM(party, INDEX_SZ, max_reads);
    formula->init(raw_formula);
    cout <<"finish  input!\n";
    auto timer_1 = chrono::high_resolution_clock::now();
    cost_input = chrono::duration<double>(timer_1 - timer_0).count();

    delta = 0;


    for (int64_t i = ncls - nres; i < ncls; i++) {
	    delta = delta + 1;
        if ((delta / nres) > 0.1){
            float  progress = (float(i) / ncls);
            delta = 0;
            int barWidth = 70;
            std::cout << "[";
            int pos = barWidth * progress;
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << int(progress * 100.0) << " %\r";
            cout << endl;
        }

        int64_t chain_length = 0L;
        vector<uint64_t> pvt;
        vector<Integer> chain;

        if (party == ALICE) {
            chain_length = supports[i].size();
            io->send_data(&chain_length, 8);
        }else{
            io->recv_data(&chain_length, 8);
        }

        SPT s = supports[i];
        PVT p = pivots[i];


        if (party == BOB) {
            for (int j = s.size(); j < chain_length; j++) {
                s.push_back(0L);
            }
            for (int j = p.size(); j < chain_length - 1; j++) {
                p.push_back(0L);
            }
        }


        assert(s.size() == p.size() +1);
        for (uint64_t index: s) {
            chain.push_back(Integer(INDEX_SZ, index, ALICE));
        }

        pvt.push_back(0UL);
        for(uint64_t pp: p){
            pvt.push_back(pp);
        }

        bool last_clause = (i == (ncls - 1));
        auto cost = check_chain(chain, pvt, i, formula, last_clause);
        cost_resolve = cost_resolve + cost.second;
        cost_access = cost_access + cost.first;
    }

    check_zero_MAC(zero_block, 1);
    auto timer_4 = chrono::high_resolution_clock::now();

    formula->check();

    auto timer_5 = chrono::high_resolution_clock::now();
    cost_access = cost_access +  chrono::duration<double>(timer_5 - timer_4).count();
    cout << "a "<< cost_access << " " << "r " << cost_resolve << " "<< "i "<< cost_input << " t "<< cost_access + cost_resolve + cost_input << endl;

    // Tear down the RAM (runs its permutation checks' cleanup) before the
    // backend, then finalize the backend. A cheating prover triggers error()
    // inside the ROM / MAC checks or the backend teardown.
    delete formula;
    finalize_zk_bool();

    // Total bytes across the proof live on the inner channel: byte sends are
    // forwarded through it and gate bits are flushed straight into it.
    uint64_t counter = netio->send_counter + netio->recv_counter;
    cout << "communication:" << counter << endl;
    cout << "----end----" << endl;

    delete io;
    delete netio;
    return 0;

}
