from array import array
from dataclasses import dataclass
import sys
import faulthandler; faulthandler.enable()
import threading

import gc

def resolution(clsa, clsb):
    clsb_c = set();
    for lit in clsb:
        clsb_c.add(-lit)

    p = clsa.intersection(clsb_c)
    if not (len(p) == 1):
        print(p)
        print(clsa, clsb)
    assert (len(p) == 1);
    res = clsa.union(clsb);
  #  print(res)
    pivot = 0
    for pe in p:
        pivot  = pe
    res.remove(pivot)
    res.remove(-pivot)
    assert (not pivot == 0)

    return res, pivot



@dataclass
class ResolutionTuple:
    index:int;
    clause : set;
    support : array;
    pivot: array = 0;

    def print(self):
        print("index: ", self.index, " clause: ", " ".join([str(i) for i in self.clause]), " support: ", " ".join([str(i) for i in self.support]), " pivot: ", " ".join([str(i) for i in self.pivot]), "end: ", 0)

def parse(str):
    #ResTuple = resolution()
    line = str.split(" ")
    index = int(line[0])
    clause = set();
    support = []
    i = 1
    while (line[i].strip('\n') != '0'):
        clause.add(int(line[i]))
        i = i + 1
    i = i + 1
    while (line[i].strip('\n') != '0'):
        support.append(int(line[i]))
        i = i + 1
    #print(ResTuple)
    if len(clause) == 0:
        clause.add(0)
    res = ResolutionTuple( index, clause, support)
    return res



remap = {}

def regularize ( filename, start = 0):
    resolution_list = []
    proof = open(filename, 'r')
    Lines = proof.readlines()
    counter = start

    for line in Lines:
        if line == "0":
            continue
        res = parse(line)
        # if len(res) == continue
        remap[res.index] = counter
        res.index = counter
        support = []
        for index in res.support:
            support.append(remap[index])
        res.support = support
        counter = counter + 1
        resolution_list.append(res)

    return resolution_list




def contain(list, literal):
    for e in list:
        if (literal in e):
            return True

#find all clauses that covered by cls

def FindNextNode(clist, cls):
    result = []
    for e in clist:
        if not AllButOne(cls, e) == 0:
            result.append(e)
    return result



def AllButOne(clsa, clsb):
    count = 0
    pivot = 0

    for l in clsb:
        if not l in clsa:
            count  = count + 1
            pivot = l
    if count == 1:
        assert not pivot == 0
        return  pivot
    else:
        return 0




def sort (cls, candidate, clist):
    assert cls != None
    assert clist != None
    if len(clist) == 2:
        result, p = resolution(clist[0], clist[1])
        for l in result:
            if not l in cls:
                # print("Deep wrong guess")
                return []
        return clist
    for cls_e in candidate:
        result = []
        goal  = cls.copy()
        pivot = AllButOne(cls, cls_e)
        if pivot == 0:
            # print("BACKTRACK1!!!!!!!!!!!!!!!!")
            continue
        if pivot != 0:
            goal.add(-pivot)
            clist.remove(cls_e)
            next = FindNextNode(clist, goal)
            if len(next) == 0:
                # print("BACKTRACK2!!!!!!!!!!!!!!!!")
                clist.append(cls_e)
                continue
            a = sort(goal, next, clist)
            if len(a) < len(clist):
                # print("wrong guess")
                clist.append(cls_e)
                continue
            result.append(cls_e)
            result.extend(a)
            return result



def get_map(lista, listb):
    map = []
    assert len(listb) == len(lista)
    for e in lista:
        map.append(listb.index(e))
    return map

def reorder (map, list):
    res = []
    for i in range(len(list)):
        res.append(list[map[i]])
    return res




def ReorderResolutionChain(goal, chain, resolution_list):
    clausechain = []
    res = []
    # resolution_list_c = resolution_list.copy()
    for index in chain:
        clausechain.append(resolution_list[index].clause)
    sortedChain = sort( goal, clausechain, clausechain.copy())
    map = get_map(sortedChain, clausechain)
    orderedchain = reorder(map, chain)
    for i in range(len(orderedchain)):
        res.append(orderedchain[len(orderedchain) - i-1])
    return  res



def single_unfold(rlist, input) :
    if len(input.support) < 2:
        return [input]
    if len(input.support) == 2:
        tmp_cls, pivot = resolution(rlist[input.support[0]].clause, rlist[input.support[1]].clause)
        tmp_res = input;
        # print(pivot)
        tmp_res.pivot = pivot
        return [tmp_res]
    res = []
    support = ReorderResolutionChain(input.clause, input.support, rlist)
    tmp_cls, pivot = resolution(rlist[support[0]].clause, rlist[support[1]].clause)
    tmp_res = ResolutionTuple(len(rlist), tmp_cls, [support[0], support[1]], pivot)
    rewrite_input = input
    rewrite_support = [len(rlist)]
    rlist.append(tmp_res)
    res.append(tmp_res)
    for i in range(2, len(support)):
        rewrite_support.append(support[i])
    rewrite_input.support = rewrite_support

    res.extend(single_unfold(rlist, rewrite_input))
    return  res

#
def unfold_chain (resolution_list, nthread = 1):
    resolution_list_c = resolution_list.copy()
    length = 0

    for t in resolution_list_c:
        pivot_t = []
        if (len(t.support) > 0):
            support = ReorderResolutionChain(t.clause.copy(), t.support.copy(), resolution_list_c)
            t.support = support
            tmp_cls = resolution_list[support[0]].clause.copy()
            for i in range(1, len(support)):
                tmp_cls, p = resolution(tmp_cls, resolution_list[support[i]].clause.copy())
                if len(tmp_cls) > length:
                    length  = len(tmp_cls)
                pivot_t.append(p)
        t.pivot = pivot_t

    return  resolution_list_c, length


def GetChainLength(resolution_list):
    max = 0
    for e in resolution_list:
        if len(e.support) > max:
            max = len(e.support)
    return max



proofname = sys.argv[1]
resolution_raw = regularize(proofname)
chain_length =  GetChainLength(resolution_raw)
sys.setrecursionlimit(chain_length * 5)


resolution_unfold, degree= unfold_chain(resolution_raw)
for e in resolution_unfold:
    e.print()
print("DEGREE:", degree)

