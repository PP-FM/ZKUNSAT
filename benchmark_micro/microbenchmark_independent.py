import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_pdf import PdfPages
import matplotlib as mpl
from matplotlib.patches import Rectangle
import math


class TestCase:
    def __init__(self, nres, ncls, degree, res_time, access_time, input_time, total_time):
        self.nres = nres
        self.ncls = ncls
        self.degree = degree
        self.res_time = res_time
        self.access_time = access_time
        self.total_time = total_time
        self.input_time = input_time
    def __repr__(self):
        res = ""
        res += "ncls:{}, ".format(self.ncls)
        res += "nres:{}, ".format(self.nres)
        res += "degree:{}, ".format(self.degree)
        return res

def data_process(filename):
    results = []
    with open(filename) as f:
        while (True):
            # Read a line.
            line = f.readline()

            if len(line) == 0:
                print("::DONE::")
                break
            # When a newline is returned, the line is empty.
            if line == "----set up----\n":
                res = f.readline()
                nres = int(res.split()[1])
                cls = f.readline()
                ncls = int(cls.split()[1])
                d = f.readline()
                degree = int(d.split()[1])
                time = f.readline()
                while (time.split()[0]!= "a"):
                    time = f.readline()
                # print(nres, ncls, degree)
                access_time = float(time.split()[1])
                res_time = float(time.split()[3])
                input_time = float(time.split()[5])
                total_time = float(time.split()[7])

                tc = TestCase(nres, ncls, degree, res_time, access_time, input_time, total_time)
                results.append(tc)
    return results


# dashes = ["-", "--", "-.", ":"]
marker = ['-.', '-', '--', '-p', '-v', '-+', '-X', '-*' ]
colors = ["#332288", "#88CCEE", "#44AA99", "#117733", "#999933", "#DDCC77"]
def init_plotting(fig_width = 8, fig_height = 0, font=30):
    golden_mean = (math.sqrt(5)-1.0)/2.0    # Aesthetic ratio
    if fig_height == 0:
        fig_height = fig_width*golden_mean # height in inches
    plt.rcParams['text.usetex'] = True
    plt.rcParams['figure.figsize'] =[fig_width,fig_height]
    plt.rcParams['font.size'] = font
    #plt.rcParams['font.family'] = 'Times New Roman'
    plt.rcParams['font.family'] = 'serif'
    plt.rcParams['font.serif'] = 'Times'
    plt.rcParams['axes.labelsize'] = plt.rcParams['font.size']
    plt.rcParams['axes.titlesize'] = 1.5*plt.rcParams['font.size']
    plt.rcParams['legend.fontsize'] = plt.rcParams['font.size']
    plt.rcParams['xtick.labelsize'] = plt.rcParams['font.size']
    plt.rcParams['ytick.labelsize'] = plt.rcParams['font.size']

init_plotting()
f, axs = plt.subplots(2,2, sharey= True, figsize = (24, 15))

results = data_process("micro.result")
d_axis = [50 * i for i in range(1, 10)]
nres_r = [6000, 7000, 8000, 9000]
cases = [(tc.ncls, tc.nres) for tc in results if tc.degree == 50 and tc.nres in nres_r and tc.ncls-tc.nres-1 == 3000 ]
print(len(cases))
ncls_r = [3000]
print(cases)
cases = sorted(cases,  key=lambda tc: tc[1])

for (ncls, nres) in cases:
    index_r = nres_r.index(nres)
    index_c = ncls_r.index(ncls - nres-1)
    tc_d = [tc for tc in results if tc.ncls == ncls and tc.nres == nres]
    tc_s = sorted(tc_d, key=lambda tc: tc.degree)
    time = [tc.res_time for tc in tc_s]
    axs[0][0].plot(d_axis, time, marker[index_r],  label = "R ="+ str(nres) )


for (ncls, nres) in cases:
    index = nres_r.index(nres)

    tc_d = [tc for tc in results if tc.ncls == ncls and tc.nres == nres]
    tc_s = sorted(tc_d, key=lambda tc: tc.degree)
    time = [tc.access_time for tc in tc_s]
    axs[0][1].plot(d_axis, time, marker[index],  label = "N - R = " + str(ncls-1-nres)+", R ="+ str(nres) +" )")

for (ncls, nres) in cases:
    index = nres_r.index(nres)

    tc_d = [tc for tc in results if tc.ncls == ncls and tc.nres == nres]
    tc_s = sorted(tc_d, key=lambda tc: tc.degree)
    time = [tc.total_time for tc in tc_s]
    axs[1][1].plot(d_axis, time,  marker[index],  label = "N - R = " + str(ncls-1-nres)+", R ="+ str(nres) +" )")


for (ncls, nres) in cases:
    index = nres_r.index(nres)
    tc_d = [tc for tc in results if tc.ncls == ncls and tc.nres == nres]
    tc_s = sorted(tc_d, key=lambda tc: tc.degree)
    time = [tc.input_time for tc in tc_s]
    axs[1][0].plot(d_axis, time, marker[index], label = "N - R = " + str(ncls-1-nres)+", R ="+ str(nres) +" )")



axs[0][0].set(xlabel='Sparsity (D)')
axs[0][0].set(ylabel= 'Time for Resolution (s)')
axs[0][1].set(xlabel='Sparsity (D)')
axs[0][1].set(ylabel= 'Time for Accessing (s)')
axs[1][0].set(xlabel='Sparsity (D)')
axs[1][0].set(ylabel= 'Input Time(s)')
axs[1][1].set(xlabel='Sparsity (D)')
axs[1][1].set(ylabel= 'Total Time(s)')

handles, labels = axs[0][0].get_legend_handles_labels()
f.legend(handles, labels, loc='lower center', bbox_to_anchor=(0.5, -0.05),  ncol = 4, fancybox=False, shadow= False)
plt.savefig("./microbenchmark_degree.pdf", bbox_inches='tight', dpi=200)
plt.show()
