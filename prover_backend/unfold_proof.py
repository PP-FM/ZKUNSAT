from array import array
from dataclasses import dataclass
import sys

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
    pivot: array;

    def print(self):
        print("index: ", self.index, " clause: ", " ".join([str(i) for i in self.clause]), " support: ", " ".join([str(i) for i in self.support]), " pivot: ", " ".join([str(i) for i in self.pivot]), "end: ", str(0))

def single_unfold(rlist, input) :
    if len(input.support) < 2 or len(input.support) ==2:
        return [input]
    res = []

    tmp_cls, pivot = resolution(rlist[input.support[0]].clause, rlist[input.support[1]].clause)
    r = ResolutionTuple(index= input.index, clause= tmp_cls, support=[input.support[0], input.support[1]], pivot = [pivot])

    res.append(r)
    for i in range(2, len(input.support)):
        tmp_cls, pivot = resolution(tmp_cls, rlist[input.support[i]].clause)
        r = ResolutionTuple(index=input.index, clause= tmp_cls, support=[-1, input.support[i]], pivot=[pivot])
        res.append(r)

    return res


def parse(str):
    #ResTuple = resolution()
    line_tmp = str.split(" ")
    line = []
    for e in line_tmp:
        if e != '':
            line.append(e)
    index = int(line[1])
    clause = set();
    pivots = []
    support = []
    i = 3
    while (line[i].strip(" ") != 'support:'):
        clause.add(int(line[i]))
        i = i + 1
    i = i + 1
    while (line[i].strip(" ") != 'pivot:'):
        support.append(int(line[i]))
        i = i + 1
    i = i + 1
    while (line[i].strip(" ") != 'end:'):
        pivots.append(int(line[i]))
        i = i + 1
    if len(clause) == 0:
        clause.add(0)
    res = ResolutionTuple(index, clause, support, pivots)
    return res

def process ( filename, start = 0):
    resolution_list = []
    proof = open(filename, 'r')
    Lines = proof.readlines()

    for line in Lines:
        str = line.split(" ")
        if str[0] == "DEGREE:":
            degree = int(str[1])
            break
        res = parse(line)
        resolution_list.append(res)
    return resolution_list, degree

def remap(resolution_unfold_raw):
    result = []
    counter = 0
    remap = {}
    for res_list in resolution_unfold_raw:
        for i in range(len(res_list)):
            if i == len(res_list)-1:
                remap[res_list[i].index] = counter
            res_list[i].index = counter
            supprt_new = []
            for s in res_list[i].support:
                assert  s in remap or s == -1
                if s == -1:
                    supprt_new.append(counter - 1)
                else:
                    supprt_new.append(remap[s])
            res_list[i].support = supprt_new
            result.append(res_list[i])
            counter = counter + 1
    return result




proofname = sys.argv[1]
resolution_raw, degree = process(proofname)

degree = 0
resolution_unfold_raw = []
for r in resolution_raw:
    a = single_unfold(resolution_raw, r)
    resolution_unfold_raw.append(a)
result = remap(resolution_unfold_raw)

for r in result:
    if len(r.clause) > degree:
        degree = len(r.clause)

original_stdout = sys.stdout
f = open(proofname+".unfold", "a")
sys.stdout = f
for r in result:
    r.print()
print("DEGREE: "+ str(degree+ 4))
f.close()
sys.stdout = original_stdout
index = 0

