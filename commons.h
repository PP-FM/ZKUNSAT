//
// Created by anonymized on 12/7/21.
//
#pragma once
#ifndef ZKUNSAT_NEW_COMMONS_H
#define ZKUNSAT_NEW_COMMONS_H
#include "emp-zk/emp-zk.h"
#include <NTL/GF2EX.h>
#include <NTL/GF2E.h>
#include <NTL/GF2EXFactoring.h>

using namespace emp;

// Degree bound for clause polynomials (max literals per clause), read from
// the proof file at startup.
extern int DEGREE;

// The single bit-level channel driving the proof. Detemplated in the new
// emp-zk: BoolIO wraps an inner IOChannel and the Fiat-Shamir transcript
// lives on that inner channel (read via io->io->get_digest()).
extern BoolIO *io;

// Sign bit mask for the literal encoding (see wrap()/encode()).
extern uint64_t constant;

// The unified bool backend carries both the gate-level wires and the f2k
// (authenticated F(2^128)) wire ops the polynomial machinery needs.
inline ZKBoolBase *bb() { return get_bool_backend(); }

#endif //ZKUNSAT_NEW_COMMONS_H
